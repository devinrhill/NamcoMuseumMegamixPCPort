#include "kingdom/boot/cAppBoot.hpp"

#include <cstdio>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "WiiSystem.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom::boot
{

namespace
{
constexpr uint STATE_COUNT = 20;          // jump table 0x80394aa8
constexpr uint ANIM_END = 0x10000;        // cJetAnita state bit: animation finished
constexpr uint ANITA_SHOW = 1;            // cJetAnita state bit: drawn
constexpr uint ANITA_ANIMATE = 2;         // cJetAnita state bit: animation advances
constexpr uint SKIP_AFTER_MSEC = 1000;    // a button press skips a timed screen only after this long

// Language dependent animations of the first two screens, by SCGetLanguage() (0x8009e430 / 0x8009e560).
struct LangAnim { int first; int second; };
LangAnim LangAnimFor(int sc)
{
	switch (sc)
	{
	case 0: return {0x00, 0x01};  // JP
	case 2: return {0x04, 0x05};  // DE
	case 5: return {0x0a, 0x0b};  // IT
	case 6: return {0x0c, 0x0d};  // NL
	case 3: return {0x08, 0x09};  // FR
	case 4: return {0x06, 0x07};  // ES
	case 9: return {0x11, 0x12};  // KR
	default: return {0x02, 0x03}; // EN and the rest
	}
}
}

cAppBoot::cAppBoot()
{
	SetName("cAppBoot");
	Init();
}

// DOL 0x8009e024 [audited]: attach boot.lzs and load the layouts (7 x FUN_8000bca0 key lookup + FUN_80141708 = cJetAnita::SetData)
void cAppBoot::Init()
{
	cKingdomSys* sys = KingdomGlobal::KingSys;
	sys->SetObjA50Flags(true, false); // 0x8009e054: FUN_8009dc08(KingSys[0xa50], 1, 0); state 3's leave step sets (0, 1)
	sys->AttachArcData("boot.lzs", false); // r5 = 0: no cKingdomLoading indicator

	// keys 0x962634df, 0x962634df, 0xe8f2e177, 0xe8f2e177, 0x9076f6de, 0xc5778ce6, 0xc5778ce6 (crc32 of the names)
	static const char* const kLayout[7] = {"ANITA_STRAP", "ANITA_STRAP", "ANITA_LOGO", "ANITA_LOGO",
	                                       "ANITA_NUNCHUK", "ANITA_RIGHT", "ANITA_RIGHT"};
	for (int i = 0; i < 7; i++)
	{
		const std::vector<byte>& data = sys->SearchData(kLayout[i]);
		if (data.empty())
		{
			std::fprintf(stderr, "cAppBoot: %s not found in boot.lzs\n", kLayout[i]);
			continue;
		}
		m_Anita.pAnita(i)->SetData(data);
		m_Anita.pAnita(i)->SetState(ANITA_SHOW, false); // loaded hidden (rlwinm ...,0,0x1e at 0x8009e0bc, ...)
	}
}

// DOL 0x8009e2cc [audited+dolphin]. The usual mode/step flow: state S (+0x34), previous state O (+0x3c), step P (+0x40):
// step 0 enters, step 1 runs, step 2 leaves (and always ends back at step 0).
bool cAppBoot::Execute(cJetTask* pParam)
{
	const GamePad::State& pad = GamePad::Get(0);   // 0x8009e378: local copy of get_pad1(0); word0 = hold, word1 = trig
	uint& S = nModeState(0);
	uint& O = nOldState(0);
	uint& P = nModeStep(0);

	auto layer = [this](int i) { return m_Anita.pAnita(i); };
	auto ended = [&](int i) { return layer(i)->IsState(ANIM_END); };
	auto play = [&](int i, int anim) { layer(i)->SetAnimNo(anim, true, 0.0f); };
	auto show = [&](int i, bool on) { layer(i)->SetState(ANITA_SHOW, on); };
	auto go = [&](uint next) {
		S = next;
		for (uint i = 0; i < 8; i++) nModeProc(0, (int)i) = 0; // memset(this + 0x44, 0, 0x20)
	};
	auto restartTimer = [&]() { StopWatchReset(0); StopWatchStart(0); };

	// The timed screens (states 2, 5, 8, 11, 14, 17): B held pauses the timer; any other button press skips once the
	// screen has been up for a second; otherwise the screen leaves by itself after `autoMsec`.
	auto timed = [&](uint autoMsec, uint next) {
		if (pad.hold & GamePad::BTN_B)
		{
			StopWatchStop(0);
		}
		else if (!StopWatchState(0))
		{
			StopWatchStart(0);
		}
		if (StopWatchCheck(0) > SKIP_AFTER_MSEC && (pad.trig & ~GamePad::BTN_B))
		{
			go(next);
		}
		else if (StopWatchCheck(0) > autoMsec)
		{
			go(next);
		}
	};

	static uint s_lastLogged = ~0u;
	if (S != s_lastLogged)
	{
		s_lastLogged = S;
		// std::printf("cAppBoot: state %u\n", S);
	}

	if (S < STATE_COUNT)
	{
		O = S;
		switch (P)
		{
		case 0: // enter
			P = 1;
			switch (S)
			{
			case 1: play(0, LangAnimFor(WiiSystem::ScLanguage()).first); show(0, true); break;
			case 2: play(0, LangAnimFor(WiiSystem::ScLanguage()).second); restartTimer(); break;
			case 3: play(1, 0x10); show(1, true); break;
			case 4: show(0, false); show(1, false); show(2, true); play(2, 0); break;
			case 5: play(2, 1); restartTimer(); break;
			case 6: play(2, 2); break;
			case 7: show(2, false); show(3, true); play(3, 3); break;
			case 8: show(2, false); show(3, true); play(3, 4); restartTimer(); break;
			case 9: play(3, 5); break;
			case 10: show(3, false); show(4, true); play(4, 0); break;
			case 11: show(3, false); show(4, true); play(4, 1); restartTimer(); break;
			case 12: play(4, 2); break;
			case 13: show(4, false); show(5, true); play(5, 0); break;
			case 14: restartTimer(); break;
			case 15: play(5, 1); break;
			case 16: show(5, false); show(6, true); play(6, 2); break;
			case 17: restartTimer(); break;
			case 18: play(6, 3); break;
			default: break; // states 0 and 19 have nothing to set up
			}
			[[fallthrough]];
		case 1: // run
			switch (S)
			{
			case 0: go(1); break;
			case 1: if (ended(0)) go(2); break;
			case 2: timed(20000, 3); break;
			case 3: if (ended(1)) go(4); break;
			case 4: if (ended(2)) go(5); break;
			case 5: timed(2000, 6); break;
			case 6: if (ended(2)) go(7); break;
			case 7: if (ended(3)) go(8); break;
			case 8: timed(2000, 9); break;
			case 9: if (ended(3)) go(10); break;
			case 10: if (ended(4)) go(11); break;
			case 11: timed(3000, 12); break;
			case 12: if (ended(4)) go(13); break;
			case 13: if (ended(5)) go(14); break;
			case 14: timed(3000, 15); break;
			case 15: if (ended(5)) go(16); break;
			case 16: if (ended(6)) go(17); break;
			case 17: timed(3000, 18); break;
			case 18: if (ended(6)) go(19); break;
			case 19:
				// 0x8009f56c: every frame until the job actually changes
				Kingdom_SetJobNo(job::TITLE, true, true);
				break;
			}
			if (O != S) P = 2; // state changed: run the leave step now
			if (P != 2) break;
			[[fallthrough]];
		case 2: // leave
			P = 0;
			if (O == 2)
			{
				layer(0)->SetState(ANITA_ANIMATE, false);          // 0x8009e6f4: state &= ~2
			}
			else if (O == 3)
			{
				KingdomGlobal::KingSys->SetObjA50Flags(false, true); // 0x8009e7b4: FUN_8009dc08(KingSys[0xa50], 0, 1)
			}
			break;
		}
	}

	m_Anita.UpdateFrame(); // 0x8009f5a0: FUN_80014d58
	return true;
}

// DOL 0x8009f5c4 [audited]: 2D projection (the game's 640 x 480 screen, handled by the gfx layer) then the seven layouts
void cAppBoot::Draw(int nStatus, cJetTask* pParam)
{
	m_Anita.Draw();
}

}
