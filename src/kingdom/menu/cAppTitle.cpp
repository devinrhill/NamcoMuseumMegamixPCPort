#include "kingdom/MslRand.hpp"
#include "kingdom/menu/cAppTitle.hpp"

#include <cstdio>
#include <cstdlib>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "WiiSystem.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom::menu
{

namespace
{
// Cue names the title passes to the sound system (strings at 0x80380fa0, 0x80380fb0, 0x80380fd0, 0x80380fe4,
// reached through small-data pointers r2-0x4548, r2-0x4544, r13-0x5e1c, r13-0x5e18).
constexpr const char* SND_TITLE_BGM = "m00bgm_Title";
constexpr const char* SND_DECIDE = "m00se_02Kettei";
constexpr const char* SND_START_VOICE_ODD = "m03se_107_Goal_1";   // rand() odd,  r13-0x5e1c
constexpr const char* SND_START_VOICE_EVEN = "m03se_109_Goal_3";  // rand() even, r13-0x5e18

constexpr uint FADE_IN_A = 1;          // Fade(1, 500) when cKingdomSys + 0xa44 (language) == 1 (JA), else Fade(2, 500)
constexpr uint FADE_IN_B = 2;
constexpr uint FADE_OUT = 0x10001;     // eFADE_TYPE_OUT_WHITE
constexpr int FADE_MSEC = 500;
constexpr int START_WAIT_MSEC = 1000;  // state 2 holds this long before the exit fade
}

cAppTitle::cAppTitle()
{
	SetName("cAppTitle");
	Init();
	AutoRelease(true); // 0x80203038: vtable slot 12 with 0x80000001
}

// DOL 0x80202f48 [audited: 3D scene setup = TitleScene]
void cAppTitle::Init()
{
	KingdomGlobal::KingSys->AttachArcData("g10_menu/g10title.lzs", true);
	// FUN_80137648(pKingSys->0xa54, GROUP_pacn_SE): sound group load (native sounds need none).
	// 0x80202f90: srand(low word of OSGetTime()). --seed N fixes it so scripted runs repeat.
	const std::uint32_t seed = kingdom::RandSeedOverride(WiiSystem::OSGetTime());
	kingdom::MslSrand(seed); // DOL rand() 0x80211048
	m_bAttract = false;

	// Layers 2 and 3 both load MAINTITLE_ANITA (crc32 0x706a47ba, FUN_8000bca0 + FUN_80141708).
	const std::vector<byte>& anita = KingdomGlobal::KingSys->SearchData("MAINTITLE_ANITA");
	if (anita.empty())
	{
		std::fprintf(stderr, "cAppTitle: MAINTITLE_ANITA not found in g10title.lzs\n");
	}
	else
	{
		m_Anita.pAnita(2)->SetData(anita);
		m_Anita.pAnita(3)->SetData(anita);
	}

	SetTitleAnim(KingdomGlobal::KingSys->bUnkA91() ? 2 : 0);

	// Scene children, in construction order. The model tasks are placeholders (no g3d renderer yet).
	m_pFade = new cTaskFade();
	m_pFade->SetName("cTaskFade");
	ConnectChild(m_pFade);

	auto child = [this](const char* name) {
		cJetTask* t = new cJetTask();
		t->SetName(name);
		ConnectChild(t);
		return t;
	};
	m_Scene.Load();
	m_pBg = child("cTaskBg");
	m_pPacman = child("cTaskPacman");
	child("cTaskTape");
	child("cTaskEffect");
	child("cTaskCamera");
}

// DOL 0x80203fb8 [audited]: which clip each of the two layers plays; both get state bit 2 set
void cAppTitle::SetTitleAnim(int mode)
{
	static const int kLayer2[5] = {0, 3, 1, 4, 2};
	static const int kLayer3[5] = {5, 8, 6, 9, 7};
	if (mode < 0 || mode > 4) return;
	m_Anita.pAnita(2)->SetAnimNo(kLayer2[mode], true, 0.0f);
	m_Anita.pAnita(3)->SetAnimNo(kLayer3[mode], true, 0.0f);
	m_Anita.pAnita(2)->SetState(2); // ori r0,r0,2 on the state word at +4 (0x80204020)
	m_Anita.pAnita(3)->SetState(2);
}

// DOL 0x802039b4 [audited+dolphin]. Standard cTaskKingdom mode/step flow: state S, previous state O (+0x3c), step P (+0x40).
// Each state has steps 0 (enter), 1 (run) and 2 (leave); leaving always ends back at step 0.
bool cAppTitle::Execute(cJetTask* pParam)
{
	const GamePad::State& pad = GamePad::Get(0);
	static uint s_lastLogged = ~0u;
	if (nModeState(0) != s_lastLogged)
	{
		s_lastLogged = nModeState(0);
		// std::printf("cAppTitle: state %u\n", s_lastLogged);
	}
	uint& S = nModeState(0);
	uint& O = nOldState(0);
	uint& P = nModeStep(0);

	auto clearProc = [this]() {
		for (uint i = 0; i < 8; i++) nModeProc(0, (int)i) = 0;
	};
	auto stateChanged = [&]() {
		if (O != S) P = 2; // 0x80203b34: fall into the "leave" step, which resets to 0
	};

	switch (S)
	{
	case 0: // intro: start the music and fade in until the layout animation has played out
		O = S;
		if (P == 0)
		{
			P = 1;
			WiiSnd::Play(m_Snd, SND_TITLE_BGM);
			m_pFade->Fade(KingdomGlobal::KingSys->nUnkA44() == 1 ? FADE_IN_A : FADE_IN_B, FADE_MSEC);
		}
		if (P == 1)
		{
			if (m_pFade->IsEnd())
			{
				S = 1;
				clearProc();
			}
			stateChanged();
		}
		if (P == 2) P = 0;
		break;

	case 1: // waiting for the player
		O = S;
		if (P == 0)
		{
			P = 1;
			m_bAttract = false;
			StopWatchReset(0);
			StopWatchStart(0);
			SetTitleAnim(4);
		}
		if (P == 1)
		{
			if (pad.hold & GamePad::BTN_B)
			{
				StopWatchStop(0);
			}
			else if (pad.trig & ~GamePad::BTN_B)
			{
				StopWatchReset(0);
				StopWatchStart(0);
			}
			else if (!StopWatchState(0))
			{
				StopWatchStart(0);
			}
			StopWatchCheck(0);
			if (pad.trig & GamePad::TRIG_DECIDE)
			{
				WiiSnd::Handle se;                   // 0x80203c54: FUN_80025b6c(se, 0, "m00se_02Kettei", 0)
				WiiSnd::PlayRemote(se, 0, SND_DECIDE);
				WiiSnd::Play(se, (kingdom::MslRand() % 2 == 0) ? SND_START_VOICE_EVEN : SND_START_VOICE_ODD);
				S = 2;
				clearProc();
			}
			stateChanged();
		}
		if (P == 2) P = 0;
		break;

	case 2: // start pressed: let the effect play for a second
		O = S;
		if (P == 0)
		{
			P = 1;
			// FUN_8011fad0(m_pBg) / FUN_80121308(m_pPacman, 1): kick off the scene's start animation
			m_Scene.StartPressed();
			StopWatchReset(0);
			StopWatchStart(0);
		}
		if (P == 1)
		{
			if (StopWatchCheck(0) > (uint)START_WAIT_MSEC)
			{
				S = 3;
				clearProc();
			}
			stateChanged();
		}
		if (P == 2) P = 0;
		break;

	case 3: // fade out, then change job
		O = S;
		if (P == 0)
		{
			P = 1;
			m_pFade->Fade(FADE_OUT, FADE_MSEC);
			SetTitleAnim(m_bAttract ? 1 : 3);
			WiiSnd::FadeOut(m_Snd, FADE_MSEC);
		}
		if (P == 1)
		{
			if (m_pFade->IsEnd())
			{
				// attract demo (job 136) when idle, otherwise job 138 (0x08a00000)
				Kingdom_SetJobNo(m_bAttract ? job::ATTRACT : job::MENU, false, false);
			}
			stateChanged();
		}
		if (P == 2) P = 0;
		break;
	}

	// the scene tasks (cTaskBg / Pacman / Tape ...) stop executing while B is held (0x8011fa44, 0x80120944)
	m_Scene.Update((pad.hold & GamePad::BTN_B) != 0);

	// 0x80203e94: holding B freezes the layout animation
	if (!(pad.hold & GamePad::BTN_B))
	{
		m_Anita.UpdateFrame();
		// nw4r::g3d::ScnRoot::UpdateFrame(pTaskScnRoot->...): no g3d scene yet
	}
	return true;
}

// DOL 0x80203ee4 [audited]: 2D projection (width/height of the render mode) then the layout vector
void cAppTitle::Draw(int nStatus, cJetTask* pParam)
{
	m_Scene.Draw(); // 3D scene first, the layouts go on top
	m_Anita.Draw();
}

}
