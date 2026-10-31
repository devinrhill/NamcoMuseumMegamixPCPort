#include "kingdom/menu/cAppAttract.hpp"

#include <cstdio>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "wii/Archive.hpp"

namespace kingdom::menu
{

namespace
{
constexpr const char* kMovie = "g10_menu/attract.thp"; // string at 0x8037c244
}

// DOL 0x8019b65c (ctor, vtable 0x80449450) + setup DOL 0x8019b468 [audited]: the DOL's Prepare picks OSGetTick() % audio tracks, attract.thp has 1 track, so always 0
cAppAttract::cAppAttract()
{
	SetName("cAppAttract");
	AutoRelease(true);
	// FUN_80019e1c(KingSys+0xa5c, 0): cKingdomLoading indicator off (native loads are instant, no indicator). FUN_800c34f0(1) = THPPlayerInit.
	const std::string path = wii::ArchiveStore::ResolvePath(kMovie);
	// FUN_800c3670 = THPPlayerOpen, 0x800c3cf4 = Prepare(0, 0, 0), 0x800c3fa4 = Play. The DOL panics on failure ("kConsError").
	if (path.empty() || !m_Player.Open(path) || !m_Player.Prepare())
	{
		std::fprintf(stderr, "cAppAttract: cannot play %s\n", kMovie);
		return;
	}
	m_Player.Play();
	m_bOpen = true;
}

// DOL 0x8019b65c (dtor) [audited]: FUN_80019e38(KingSys+0xa5c, language == JA) picks the cKingdomLoading look; the player is closed with the object
cAppAttract::~cAppAttract() = default;

// DOL 0x8019b548 [audited]
bool cAppAttract::Execute(cJetTask* pParam)
{
	const GamePad::State& pad = GamePad::Get(0);
	auto toTitle = [](bool fromButton) {
		KingdomGlobal::KingSys->bUnkA91() = fromButton;   // stb 0xa91(KingSys)
		Kingdom_SetJobNo(job::TITLE, false, false);
	};

	if (!m_bOpen) { toTitle(false); return true; }              // nothing to show (the DOL would have panicked)
	if (pad.hold == 0xFFFFFFFFu) Kingdom_SetJobNo(job::TITLE, false, false); // 0x8019b560: addis r0, hold, 1; cmplwi r0, 0xffff = hold == -1

	if (pad.release & GamePad::BTN_B) m_Player.Play();           // 0x8019b588: word 3 = released -> FUN_800c3fa4
	else if (pad.hold & GamePad::BTN_B) m_Player.Pause();        // 0x8019b5a4 -> FUN_800c40e0
	else if (pad.hold & ~GamePad::BTN_B) toTitle(true);          // 0x8019b5c0: any other button

	m_Player.Update();                                           // (the SDK decodes on its own threads)
	if (m_Player.done()) toTitle(false);                         // 0x8019b5e8: FUN_800c4bb8 = state == 3
	return true;
}

// DOL 0x8019b624 [audited]: the video fills the render-mode sized screen (FUN_800c44f8(0, 0, w, h))
void cAppAttract::Draw(int nStatus, cJetTask* pParam)
{
	m_Player.Draw();
}

}
