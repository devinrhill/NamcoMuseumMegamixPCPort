// Mirrors the Wii main() at 0x80121b78:
//   FUN_80137abc();                                   // create cSysCoreStructure (KingdomGlobal::SysCore)
//   Jbc_SetConfig(SysCore, 0xfff, 0x14, 0x1b0, tbl);  // mask 4095, shift 20, 432 entries
//   Jbc_SetProcNo(SysCore, 0x08000000);               // job 128
//   for (;;) cSysCoreStructure::Process(SysCore);
//
// The job table here only holds the system jobs of the original (indices 128-130 and 132, from the table at
// 0x804400c8). Minigame jobs register themselves into it later.
#include "platform/boot.hpp"

#include <array>
#include <vector>
#include <cstdio>

#include "KingdomGlobal.hpp"
#include "WiiPad.hpp"
#include "JBC_Element.hpp"
#include "cSysCoreStructure.hpp"
#include "cJetAnita.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/boot/cAppBoot.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cAppSaveLoad.hpp"
#include "gfx/DebugText.hpp"
#include <cstdlib>
#include "kingdom/cAppDebugMenu.hpp"
#include "kingdom/game/rso/cAppRSOOriginal.hpp"
#include "kingdom/menu/cAppG10StaffRoll.hpp"
#include "kingdom/game/g02/cAppRallyXDebugMenu.hpp"
#include "kingdom/game/g04/cAppG04WaniWaniDebugMenu.hpp"
#include "kingdom/game/g03/cAppPacnRollDebugMenu.hpp"
#include "kingdom/game/g07/cAppMotosDebugMenu.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cAppG03PacnRollMenu.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGameOver.hpp"
#include "kingdom/game/g03/cAppMenu.hpp"
#include "kingdom/game/g03/cAppG03PacnRollNunchuk.hpp"
#include "kingdom/game/g03/cAppG03PacnRollStaffRoll.hpp"
#include "platform/devtools.hpp"
#include "platform/platform.hpp"
#include "kingdom/cKingdomSave.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g01/cAppG01GalagaGame.hpp"
#include "kingdom/game/g02/cAppG02RallyXGame.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cAppG02RallyXGameOver.hpp"
#include "kingdom/game/g02/cAppG02RallyXMenu.hpp"
#include "kingdom/game/g02/cAppG02RallyXMultiMenu.hpp"
#include "kingdom/game/g02/cAppG02RallyXNunchuk.hpp"
#include "kingdom/game/g02/cAppG02RallyXStaffRoll.hpp"
#include "kingdom/game/g02/cAppG02RallyXMatching.hpp"
#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/game/g00/cAppG00Dummy.hpp"
#include "kingdom/game/g04/cAppG04WaniWaniGame.hpp"
#include "kingdom/game/g04/cAppG04WaniWaniMatching.hpp"
#include "kingdom/game/g04/cG04WaniWaniSys.hpp"
#include "kingdom/game/g11/cAppG11GrobdaGame.hpp"
#include "kingdom/game/g11/cAppG11GrobdaMatching.hpp"
#include "kingdom/game/g11/cAppG11GrobdaMenu.hpp"
#include "kingdom/game/g11/cAppG11GrobdaNunchuk.hpp"
#include "kingdom/game/g11/cAppG11GrobdaGameOver.hpp"
#include "kingdom/game/g11/cAppG11GrobdaStaffRoll.hpp"
#include "kingdom/game/g11/cAppGrobdaDebugMenu.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"
#include "kingdom/game/g01/cAppG01GalagaGameOver.hpp"
#include "kingdom/game/g01/cAppG01GalagaMatching.hpp"
#include "kingdom/game/g01/cAppG01GalagaStaffRoll.hpp"
#include "kingdom/game/g01/cAppG01GalagaNunchuk.hpp"
#include "kingdom/game/g04/cAppG04WaniWaniNunchuk.hpp"
#include "kingdom/game/g01/cAppG01GalagaMenu.hpp"
#include "kingdom/game/g01/cAppGalagaDebugMenu.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cAppG07MotosMatching.hpp"
#include "kingdom/game/g07/cAppG07MotosMultiMenu.hpp"
#include "kingdom/game/g07/cAppG07MotosNunchuk.hpp"
#include "kingdom/game/g07/cAppG07MotosStaffRoll.hpp"
#include "kingdom/game/g07/cAppG07MotosGameOver.hpp"
#include "kingdom/game/g07/cAppG07MotosMenu.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "kingdom/menu/cAppAttract.hpp"
#include "kingdom/menu/cAppTitle.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace
{
namespace job = kingdom::job;
constexpr uint JOB_MASK = job::MASK;
constexpr uint JOB_SHIFT = job::SHIFT;
constexpr uint JOB_COUNT = job::COUNT;

// claude... im gonna fuck you in the bunghole so hard its gonna feel like youve got 45 gnomes stuck up there going "im a gnome!! gnomotic gnome!!! hoo" youre gonna feel it in there for 7 days and 7 nights. so fuckjin good its gonna feel like youve got at least 12 pinecones up there and youre gonne be screaming "oh gnome daddy keemstar give it to me" "give me bad reams and the D later"
// this is not gay

std::array<JBC_Element, JOB_COUNT> g_table{};
JBC_Element& Slot(uint jobNo) { return g_table[job::Index(jobNo)]; }
uint g_entryJob = boot::NO_JOB;   // test hook (--job)
uint g_nextJob = boot::NO_JOB;    // test hook (--next)
bool g_g03StandIn = false;
int g_g03Round = 0;    // --round: stage of Pac-N-Roll (0..25), see docs/OPTIONS.md

// 0x8000e3bc: cKingdomSys::init(); Jbc_SetProcNo(SysCore, 0x08200000)
void Job128_Init()
{
	KingdomGlobal::KingSys = new kingdom::cKingdomSys();
	KingdomGlobal::SysCore->RegistSysTaskChild(0, KingdomGlobal::KingSys);
	KingdomGlobal::KingSys->AttachArcData("resident.arc"); // shared UI data (fade layout, fonts, ...)
	// cKingdomSys ctor 0x8000a8e4..0x8000a90c: decompresses the SSZL blob embedded in main.dol at 0x80336560 into +0xa0 (resident for the
	// whole run); museum/resource.lzs is a byte-identical copy (RESIDENT_FONT_NOFRAME = the tutorial / message text font, ...).
	KingdomGlobal::KingSys->AttachArcData("resource.lzs");
	// cKingdomSave ctor 0x8000b490 -> 0x80022320 builds banner.bin inside the cKingdomSys ctor; here the archives must be attached first
	KingdomGlobal::KingSys->pSave()->InitBanner();
	KingdomGlobal::SysCore->SetJobNo(job::MASTER);
}

// 0x8000e3e8 (KINGDOM_TERM): FUN_8000db24 = ReleaseSysTaskChild(0) (the cKingdomSys) and pKingSys = 0, then Jbc_SetProcNo(0): the developers'
// root menu (job 0). Nothing in retail asks for 129.
void Job129_Init()
{
	KingdomGlobal::SysCore->ReleaseSysTaskChild(0);
	KingdomGlobal::KingSys = nullptr;
	KingdomGlobal::SysCore->SetJobNo(job::DEBUG_MENU_ROOT);
}

// 0x800c25c0 (job 0, PRJ_JOBNO_DEBUGMENU): the root debug menu (term 0x800c26bc = ReleaseMainTask)
void Job0_Init() { KingdomGlobal::SysCore->RegistMainTask(new kingdom::cAppRootDebugMenu()); }

// the arcade collection (jobs 141..143; the module host is kingdom::game::rso::cAppRSOOriginal)
uint g_rsoOldJob = 0;   // DOL r13 - 0x71c0

// 0x800c20c0 (RSO_INIT): FUN_80015040 (frees a system buffer), the job to come back to (Jbc_GetProcNoOld), FUN_8000cf54 (cKingdomSys frees its
// model / sound heaps to make room for the module: no native counterpart), then job 142
void Job141_Init()
{
	g_rsoOldJob = KingdomGlobal::SysCore->GetLastJobNo();
	kingdom::Kingdom_SetJobNo(job::RSO_EXEC, false, 0);
}

// 0x800c2108 (RSO_EXEC): new cAppRSOOriginal (term 0x800c2228 = ReleaseMainTask)
void Job142_Init() { KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::rso::cAppRSOOriginal()); }

// 0x800c2230 (RSO_TERM): with the module's result flags (+0xa95) FUN_8009dcb0(+0xa50, +0xa92, +0xa93, +0xa94) hands them to the reset / power
// callbacks (not ported: the native module never sets them); else FUN_8000d024 (the heaps back) and back to the job before 141
void Job143_Init()
{
	if (KingdomGlobal::KingSys->bUnkA95()) return;
	kingdom::Kingdom_SetJobNo(g_rsoOldJob, false, 0);
}

// 0x800c1f98 (RSO_DEBUG_MENU): new cAppRSODebugMenu (term 0x800c20b8)
void Job174_Init() { KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::rso::cAppRSODebugMenu()); }

// the debug top menu's save-data rows. 0x8000e79c (DEBUG_LOAD): back to the previous job (force, fade 1)
void Job144_Init() { kingdom::Kingdom_SetJobNo(KingdomGlobal::SysCore->GetLastJobNo(), true, 1); }
// 0x8000e7d0 (DEBUG_SAVE): the save manager enabled (0x80023d74(0)), back to the previous job (force, no fade)
void Job145_Init()
{
	KingdomGlobal::KingSys->pSave()->SetDisabled(false);
	kingdom::Kingdom_SetJobNo(KingdomGlobal::SysCore->GetLastJobNo(), true, 0);
}
// 0x8000e81c (DEBUG_DELETE): enable, delete the files (0x800233e8), zero the 7 game blocks (0x80023dd8 with a cleared 0x2000 buffer),
// disable (0x80023d74(1)), then the debug top menu
void Job146_Init()
{
	kingdom::cKingdomSave* save = KingdomGlobal::KingSys->pSave();
	save->SetDisabled(false);
	save->Delete();
	std::vector<byte> zero(kingdom::cKingdomSave::BLOCK_SIZE, 0);
	for (uint i = 0; i < 7; i++) KingdomGlobal::KingSys->pSave()->WriteBlock(i, zero.data(), kingdom::cKingdomSave::BLOCK_SIZE);
	save->SetDisabled(true);
	KingdomGlobal::SysCore->SetJobNo(job::DEBUG_MENU);
}
// 0x8000e814 (DEBUG_WII_MENU): OSReturnToMenu, back to the Wii menu: natively the program ends
void Job147_Init() { platform::RequestClose(); }
// empty DEBUG_SELECT slots: 180 (init 0x800b0ce8 blr, term 0x800b0cec = ReleaseMainTask), 196 (init 0x800c6438 blr, term 0x800c643c = ReleaseMainTask)
void JobEmpty_Init() {}
// 340 (GROBDA_DEBUG_SELECT, 0x8000f0b0): straight to the Grobda game (339); term 0x8000f0c0 = ReleaseMainTask
void Job340_Init() { kingdom::Kingdom_SetJobNo(job::GROBDA_GAME, false, 0); }

// 0x8000e414: Jbc_SetProcNo(SysCore, 0x08400000)
void Job130_Init()
{
	if (g_nextJob != boot::NO_JOB) KingdomGlobal::KingSys->nNextJob() = g_nextJob;
	KingdomGlobal::SysCore->SetJobNo(g_entryJob != boot::NO_JOB ? g_entryJob : job::BOOT);
}

// 0x8000e1f8: new kingdom::cAppDebugTopMenu -> RegistMainTask (term 0x8000e3b4)
void Job131_Init() { WiiPad::SetUprightOverride(false); KingdomGlobal::SysCore->RegistMainTask(new kingdom::cAppDebugTopMenu()); }

// 0x8009f7a0: new kingdom::boot::cAppBoot -> RegistMainTask. Every app job's term is ReleaseMainTask.
void Job132_Init() { WiiPad::SetUprightOverride(false); KingdomGlobal::SysCore->RegistMainTask(new kingdom::boot::cAppBoot()); }

// 0x8000e420 (job 133, force = 1) / 0x8000e5dc (job 134, force = 0): new kingdom::cAppSaveLoad -> RegistMainTask
// Upright while the dialog is up (user, 2026-10-30): its Yes / No sit side by side and read the raw LEFT / RIGHT bits (0x80009598), so the Left / Right
// bindings pick them; the game bits are the DOL's. Port-only: the DOL has no orientation setting. Later jobs start sideways as before (the term).
void Job133_Init() { WiiPad::SetUprightOverride(true); KingdomGlobal::SysCore->RegistMainTask(new kingdom::cAppSaveLoad(true)); }
void Job134_Init() { WiiPad::SetUprightOverride(true); KingdomGlobal::SysCore->RegistMainTask(new kingdom::cAppSaveLoad(false)); }
void SaveLoad_Term() { WiiPad::SetUprightOverride(false); KingdomGlobal::SysCore->ReleaseMainTask(); }

// Pac-N-Roll. 0x8012b18c (job 208): new cG03PacnRollSys (stored in the global at r13-0x5678), then job 210 (0x0d200000);
// 0x8012b378 (job 210): new cAppG03PacnRollMenu -> RegistMainTask; 0x8012b4b4 (job 211): new cAppG03PacnRollGame -> RegistMainTask;
// 0x8012b964 (job 216): cAppG03PacnRollNunchuk, the "connect the Nunchuk" screen between the menu and the game, which then loads the stage
// archive (RegistArcDataAS) and goes to 0x0d300000: NOT ported, it passes straight on (the PC pad is user-remappable);
// 0x8012b21c (job 223): deletes the cG03PacnRollSys and goes to job 138 (0x08a00000).
void Job208_Init()
{
	WiiPad::SetUprightOverride(true);   // Pac-N-Roll is a Nunchuk game: the remote is held upright (DOL bits: game LEFT / RIGHT = the remote's LEFT / RIGHT)
	if (KingdomGlobal::PacnRollSys == nullptr) KingdomGlobal::PacnRollSys = new kingdom::game::g03::cG03PacnRollSys();
	KingdomGlobal::SysCore->SetJobNo(job::PACNROLL_MENU);
}
// 0x8012b008 (job 209): new cAppPacnRollDebugMenu -> RegistMainTask (term 0x8012b16c)
void Job209_Init()
{
	WiiPad::SetUprightOverride(true);
	if (KingdomGlobal::PacnRollSys == nullptr) KingdomGlobal::PacnRollSys = new kingdom::game::g03::cG03PacnRollSys();
	KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::g03::cAppPacnRollDebugMenu());
}
// 0x8012b698 (job 213): new cAppG03PacnRollGameOver -> RegistMainTask (term 0x8012b810)
void Job213_Init()
{
	WiiPad::SetUprightOverride(true);
	KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::g03::cAppG03PacnRollGameOver());
}
// Jobs 212 (DEBUG_SELECT), 214 (EDIT), 215 (EDIT2) and 217 (0x0d900000) share init 0x8012b174 = Kingdom_SetJobNo(0x0d300000): straight into the game
// with the round the cG03PacnRollSys holds (the debug menu's STAGE row sets it). Their term 0x8012b184 releases the main task (none is registered).
void Job212_Init() { KingdomGlobal::SysCore->SetJobNo(job::PACNROLL_GAME); }
void Job210_Init()
{
	WiiPad::SetUprightOverride(true);
	if (KingdomGlobal::PacnRollSys == nullptr) KingdomGlobal::PacnRollSys = new kingdom::game::g03::cG03PacnRollSys(); // --job 0x0d200000 skips job 208
	KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::g03::cAppG03PacnRollMenu());
}
void Job211_Init()
{
	WiiPad::SetUprightOverride(true);
	// the menu leaves the chosen stage in the cG03PacnRollSys; --job 0x0d300000 enters without one and uses --round
	// --job 0x0d300000 enters without the menu's cG03PacnRollSys: make one (the HUD tasks read the world / stage / save data from it)
	if (KingdomGlobal::PacnRollSys == nullptr)
	{
		KingdomGlobal::PacnRollSys = new kingdom::game::g03::cG03PacnRollSys();
		KingdomGlobal::PacnRollSys->nRound() = (unsigned)g_g03Round;
	}
	const int round = (int)KingdomGlobal::PacnRollSys->nRound();
	(void)round;   // the native stand-in game (g03native) was removed: the DOL port is the only Pac-N-Roll stage
	KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::g03::cAppG03PacnRollGame());
}
// 0x8012b964 (job 216): new cAppG03PacnRollNunchuk -> RegistMainTask (term 0x8012bac8). The "connect the Nunchuk" card; skipped when a Nunchuk is in.
void Job216_Init()
{
	WiiPad::SetUprightOverride(true);
	KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::g03::cAppG03PacnRollNunchuk());
}
// 0x8012b818 (job 222): new cAppG03PacnRollStaffRoll -> RegistMainTask (term 0x8012b95c). Entered after the last stage; goes on to 0x0d500000.
void Job222_Init()
{
	WiiPad::SetUprightOverride(true);
	if (KingdomGlobal::PacnRollSys == nullptr) KingdomGlobal::PacnRollSys = new kingdom::game::g03::cG03PacnRollSys(); // --job 0x0de00000 skips job 208 (213 needs its archives)
	KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::g03::cAppG03PacnRollStaffRoll());
}
void Job223_Init()
{
	WiiPad::SetUprightOverride(false);
	delete KingdomGlobal::PacnRollSys;
	KingdomGlobal::PacnRollSys = nullptr;
	KingdomGlobal::SysCore->SetJobNo(job::MENU);
}
// Job 138 (0x08a00000, the Museum Megamix main menu): DOL 0x8019ab28 builds a cG03PacnRollSys (+4 = 1: top menu mode, round = 27 =
// g03wXXs04, the castle garden), attaches "g10_menu/g10menu.lzs" and creates kingdom::game::g03::cAppMenu, a cAppG03PacnRollGame
// derived app (vtable 0x80389d30, 0xca0 bytes). Stage 1: the game core runs as is in top menu mode; the menu logic (cAppMenu::Execute
// 0x80010eb4) is not ported yet.
void Job138_Init()
{
	WiiPad::SetUprightOverride(true);
	delete KingdomGlobal::PacnRollSys;
	KingdomGlobal::PacnRollSys = new kingdom::game::g03::cG03PacnRollSys(false);   // game = false: top menu mode
	KingdomGlobal::PacnRollSys->nRound() = 27;
	KingdomGlobal::KingSys->AttachArcData("g10_menu/g10menu.lzs");
	kingdom::game::g03::cAppG03PacnRollGame::s_hubEntry = true;
	KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::g03::cAppMenu());
	kingdom::game::g03::cAppG03PacnRollGame::s_hubEntry = false;
}

// Job 140 (0x08c00000, MENU_CLASSIC, the classic arcade hall): DOL 0x8019aee4 = job 138's init with round 26 (g03wXXs03) and the same
// cAppMenu, which takes its classic branches from the current job; term 0x8019aea8 (detach g10menu, ReleaseMainTask, the sys global cleared)
void Job140_Init()
{
	WiiPad::SetUprightOverride(true);
	delete KingdomGlobal::PacnRollSys;
	KingdomGlobal::PacnRollSys = new kingdom::game::g03::cG03PacnRollSys(false);   // game = false: top menu mode
	KingdomGlobal::PacnRollSys->nRound() = 26;
	KingdomGlobal::KingSys->AttachArcData("g10_menu/g10menu.lzs");
	kingdom::game::g03::cAppG03PacnRollGame::s_hubEntry = true;
	KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::g03::cAppMenu());
	kingdom::game::g03::cAppG03PacnRollGame::s_hubEntry = false;
}

// Pac-Motos (g07), jobs 272..287 (0x11000000..): 272 (0x800a2b40) creates the cG07MotosSys (FUN_800a1350) and goes to the menu job 274; 273 debug menu;
// 274 (0x800a2ba0) new cAppG07MotosMenu; 275 (0x800a2e18) cAppG07MotosGame; 277 (0x800a2f9c) cAppG07MotosGameOver; 287 (0x800a2b70, the end job the hub
// reads as "previous job") deletes the sys (FUN_800a13a4) and goes back to the hub (job 138).
// 0x800b0c8c (job 177): the Galaga sys is made by job 176 in the DOL; --job 0x0b100000 skips it, so it is made here when missing
void Job177_Init()
{
	if (KingdomGlobal::GalagaSys == nullptr) KingdomGlobal::GalagaSys = new kingdom::game::g01::cG01GalagaSys();
	kingdom::game::g01::cAppGalagaDebugMenu::JobInit();
}
// devtools: MEGAMIX_G01_PLAYERS=N (2..4) plays the stage select / stage with N players (pads 0..N-1; empty names become "PLAYERn")
static void G01DevPlayers()
{
	const char* np = std::getenv("MEGAMIX_G01_PLAYERS");
	const int n = np ? std::atoi(np) : 0;
	if (n <= 1 || n > 4) return;
	KingdomGlobal::GalagaSys->nPlayer(n);
	for (int i = 0; i < n; i++)
	{
		auto& pl = KingdomGlobal::GalagaSys->pMatchingData()->player[i];
		pl.nWork = i;
		if (pl.szName[0] == '\0') std::snprintf(pl.szName, sizeof(pl.szName), "PLAYER%d", i + 1);
	}
}
// 0x800b0d98 (job 178): new cAppG01GalagaMenu -> RegistMainTask. --job 0x0b200000 without job 176 makes the sys (job 176 also loads the save block)
void Job178_Init()
{
	if (KingdomGlobal::GalagaSys == nullptr)
	{
		KingdomGlobal::GalagaSys = new kingdom::game::g01::cG01GalagaSys();
		KingdomGlobal::GalagaSys->nStage(1);
	}
	G01DevPlayers();
	kingdom::game::g01::cAppG01GalagaMenu::JobInit();
}
// 0x800b0ed4 (job 179): the stage (cAppG01GalagaGame; stage 10 = cAppG01GalagaBoss, not ported). --job 0x0b300000 alone plays stage 1.
void Job179_Init()
{
	WiiPad::SetUprightOverride(true);   // UNVERIFIED: Galaga wants the Nunchuk (job 184 shows the Nunchuk card first)
	if (KingdomGlobal::GalagaSys == nullptr)
	{
		KingdomGlobal::GalagaSys = new kingdom::game::g01::cG01GalagaSys();
		KingdomGlobal::GalagaSys->nStage(devtools::G01Stage() > 0 ? devtools::G01Stage() : 1);   // devtools: --g01-stage
	}
	G01DevPlayers();
	kingdom::game::g01::cAppG01GalagaGame::JobInit();
}
// 0x800b13d0 (job 182): the player matching (from the stage select's MULTI PLAYER); --job 0x0b600000 alone makes the sys
void Job182_Init()
{
	if (KingdomGlobal::GalagaSys == nullptr) KingdomGlobal::GalagaSys = new kingdom::game::g01::cG01GalagaSys();
	kingdom::game::g01::cAppG01GalagaMatching::JobInit();
}
// 0x800b1250 (job 181): the game over; --job 0x0b500000 alone makes the sys (its resident archive has ANT_GAMEOVER)
void Job181_Init()
{
	if (KingdomGlobal::GalagaSys == nullptr) KingdomGlobal::GalagaSys = new kingdom::game::g01::cG01GalagaSys();
	kingdom::game::g01::cAppG01GalagaGameOver::JobInit();
}
// Rally-X (g02), jobs 192..207 (0x0c000000..): 192 (0x800c6444) makes the sys and goes to the stage select 194 (0x800c6504); 201
// (0x800c6ccc) the Nunchuk card, then 195 (0x800c677c) the stage; 198 (0x800c68c4) game over -> 194; 206 (0x800c6b80) staff roll -> 198;
// 207 (0x800c64a8) deletes the sys and goes back to the hub. NOT PORTED: 199 (0x800c6a44) matching, 200 (0x800c6640) multiplayer menu.
// --job 0x0c200000 / 0x0c300000 / 0x0c600000 / 0x0c900000 alone make the sys.
// The sys ctor 0x800c54e0 also loads the save block 1 and sets the members cG02RallyXSysExt mirrors natively (cG02RallyXRecord.hpp).
void G02NewSysParts()
{
	kingdom::game::g02::cG02RallyXSysExt::Reset();
	kingdom::game::g02::cG02RallyXRecord::Get().Load();
}
void G02Sys()
{
	if (KingdomGlobal::RallyXSys != nullptr) return;
	KingdomGlobal::RallyXSys = new kingdom::game::g02::cG02RallyXSys();
	G02NewSysParts();
}
void Job192_Init()
{
	kingdom::game::g02::cG02RallyXSys::JobInit();
	G02NewSysParts();
}
void Job194_Init() { G02Sys(); kingdom::game::g02::cAppG02RallyXMenu::JobInit(); }
void Job198_Init() { G02Sys(); kingdom::game::g02::cAppG02RallyXGameOver::JobInit(); }
// UNVERIFIED (as g04's job 231): the card is shown for the Nunchuk control type, so the remote is held upright
void Job201_Init() { WiiPad::SetUprightOverride(true); G02Sys(); kingdom::game::g02::cAppG02RallyXNunchuk::JobInit(); }
void Job206_Init() { G02Sys(); kingdom::game::g02::cAppG02RallyXStaffRoll::JobInit(); }
void Job199_Init() { G02Sys(); kingdom::game::g02::cAppG02RallyXMatching::JobInit(); }
void Job200_Init() { G02Sys(); kingdom::game::g02::cAppG02RallyXMultiMenu::JobInit(); }
void Job195_Init()
{
	WiiPad::SetUprightOverride(true);   // UNVERIFIED: Rally-X steers with the Nunchuk (job 201 is its Nunchuk card)
	if (KingdomGlobal::RallyXSys == nullptr) KingdomGlobal::RallyXSys = new kingdom::game::g02::cG02RallyXSys();
	if (devtools::G02Stage() > 0) KingdomGlobal::RallyXSys->nStageIdx(devtools::G02Stage());   // devtools: --g02-stage
	if (const char* e = std::getenv("MEGAMIX_G02_PLAYERS")) kingdom::game::g02::cG02RallyXSysExt::nPlayer = std::atoi(e);   // devtools: direct --job 0x0c300000 with N players (stage 25..28 = the battle stages)
	kingdom::game::g02::cAppG02RallyXGame::JobInit();
}
void Job207_Init() { kingdom::game::g02::cG02RallyXSys::JobEnd(); }
// job 193 (0x0c100000): the DOL's slot is an empty stub (init 0x800c6430); native debug menu in the style of the Motos one (term: release the main task)
void Job193_Init()
{
	WiiPad::SetUprightOverride(false);
	G02Sys();
	KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::g02::cAppRallyXDebugMenu());
}
// g00 dummy game, jobs 160..175 (0x0a000000..): 160 (0x800c6fc0) makes the sys and asks for 161 (no init in the DOL: nothing runs);
// 162 (0x800c706c) the menu; 163 (0x800c7194) the game; 175 (0x800c7010) deletes the sys and goes to the debug menu (0x08300000).
void Job160_Init() { kingdom::game::g00::cG00DummySys::JobInit(); }
void Job162_Init() { kingdom::game::g00::cAppG00DummyMenu::JobInit(); }
void Job163_Init() { kingdom::game::g00::cAppG00DummyGame::JobInit(); }
void Job175_Init() { kingdom::game::g00::cG00DummySys::JobEnd(); }
// Gator Panic (g04), jobs 224..239 (0x0e000000..): 224 (0x8019a07c) makes the sys and goes to the Nunchuk card 231 (0x8019a5ac), then the game
// 227 (0x8019a13c); 239 (0x8019a0e0) deletes the sys and goes back to the hub. --job 0x0e300000 alone makes the sys. A Nunchuk game: the
// remote is held upright.
void Job224_Init() { WiiPad::SetUprightOverride(true); kingdom::game::g04::cG04WaniWaniSys::JobInit(); }
void Job231_Init()
{
	WiiPad::SetUprightOverride(true);
	if (KingdomGlobal::WaniWaniSys == nullptr) KingdomGlobal::WaniWaniSys = new kingdom::game::g04::cG04WaniWaniSys();
	kingdom::game::g04::cAppG04WaniWaniNunchuk::JobInit();
}
void Job227_Init()
{
	WiiPad::SetUprightOverride(true);
	if (KingdomGlobal::WaniWaniSys == nullptr) KingdomGlobal::WaniWaniSys = new kingdom::game::g04::cG04WaniWaniSys();
	kingdom::game::g04::cAppG04WaniWaniGame::JobInit();
}
// 0x8019a2a0 (job 230): the same app (vtable 0x80394114) started as a 4 player game: sys +0x15c = 4, player records {index 6, pad i}, names "Guest1".."Guest4"
void Job230_Init()
{
	WiiPad::SetUprightOverride(true);
	if (KingdomGlobal::WaniWaniSys == nullptr) KingdomGlobal::WaniWaniSys = new kingdom::game::g04::cG04WaniWaniSys();
	kingdom::game::g04::cG04WaniWaniSys* sys = KingdomGlobal::WaniWaniSys;
	sys->SetPlayers(4);
	for (int i = 0; i < 4; i++)
	{
		kingdom::game::g04::sWaniPlayer& p = sys->pMatchingData()->player[i];
		p.nIndex = 6;
		p.nWork = i;
		std::snprintf(p.szName, sizeof p.szName, "Guest%d", i + 1);
	}
	kingdom::game::g04::cAppG04WaniWaniGame::JobInit();
}
void Job239_Init() { kingdom::game::g04::cG04WaniWaniSys::JobEnd(); }
// Grobda (g11), jobs 336..351 (0x15000000..): 336 (0x8000f0c8) makes the sys and goes to the menu 338 (the menu 338);
// 339 (0x8000f338) the stage; 351 (0x8000f10c) deletes the sys and goes back to the hub. --job 0x15300000 alone makes the sys.
void Job336_Init() { kingdom::game::g11::cG11GrobdaSys::JobInit(); }
void Job338_Init()
{
	if (KingdomGlobal::GrobdaSys == nullptr) KingdomGlobal::GrobdaSys = new kingdom::game::g11::cG11GrobdaSys();
	kingdom::game::g11::cAppG11GrobdaMenu::JobInit();   // DOL 0x8000f164
}
void Job339_Init()
{
	if (KingdomGlobal::GrobdaSys == nullptr) KingdomGlobal::GrobdaSys = new kingdom::game::g11::cG11GrobdaSys();
	if (const char* e = std::getenv("MEGAMIX_G11_PLAYERS")) KingdomGlobal::GrobdaSys->nPlayer(std::atoi(e));   // devtools: direct --job 0x15300000 with N players
	kingdom::game::g11::cAppG11GrobdaGame::JobInit();
}
void Job351_Init() { kingdom::game::g11::cG11GrobdaSys::JobEnd(); }
// 0x800b1658 (job 184): the "connect the Nunchuk" card, then job 179 (skipped when every player's remote has one)
void Job184_Init()
{
	if (KingdomGlobal::GalagaSys == nullptr)
	{
		KingdomGlobal::GalagaSys = new kingdom::game::g01::cG01GalagaSys();
		KingdomGlobal::GalagaSys->nStage(devtools::G01Stage() > 0 ? devtools::G01Stage() : 1);   // devtools: --g01-stage
	}
	kingdom::game::g01::cAppG01GalagaNunchuk::JobInit();
}

void Job272_Init()
{
	WiiPad::SetUprightOverride(true);
	if (KingdomGlobal::MotosSys == nullptr) KingdomGlobal::MotosSys = new kingdom::game::g07::cG07MotosSys();
	KingdomGlobal::SysCore->SetJobNo(job::MOTOS_MENU);
}
void Job274_Init() { WiiPad::SetUprightOverride(true); KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::g07::cAppG07MotosMenu()); }
void Job275_Init()
{
	WiiPad::SetUprightOverride(true);
	if (KingdomGlobal::MotosSys == nullptr) KingdomGlobal::MotosSys = new kingdom::game::g07::cG07MotosSys();   // native: a direct --job entry has no sys yet
	KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::g07::cAppG07MotosGame());
}
void Job277_Init() { WiiPad::SetUprightOverride(true); KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::g07::cAppG07MotosGameOver()); }
// 0x800a29bc (job 273): new cAppMotosDebugMenu -> RegistMainTask (term 0x800a2b20). The sys is made by job 272 in the DOL; --job 0x11100000 skips it.
void Job273_Init()
{
	WiiPad::SetUprightOverride(false);
	if (KingdomGlobal::MotosSys == nullptr) KingdomGlobal::MotosSys = new kingdom::game::g07::cG07MotosSys();
	KingdomGlobal::SysCore->RegistMainTask(new kingdom::game::g07::cAppMotosDebugMenu());
}
// 0x800a2b28 (job 276, DEBUG_SELECT): Kingdom_SetJobNo(0x11300000) with the stage index the debug menu set (term 0x800a2b38 releases the main task)
void Job276_Init() { KingdomGlobal::SysCore->SetJobNo(job::MOTOS_GAME); }
// 0x800a3268 (278) cAppG07MotosMatching, 0x800a2cdc (279) cAppG07MotosMultiMenu, 0x800a33a4 (280) cAppG07MotosNunchuk, 0x800a311c (286) cAppG07MotosStaffRoll:
// new -> RegistMainTask; terms release the main task. --job entry without job 272 makes the sys like the other g07 jobs.
static void G07App(kingdom::cAppKingdom* app) { WiiPad::SetUprightOverride(true); KingdomGlobal::SysCore->RegistMainTask(app); }
static void G07Sys() { if (KingdomGlobal::MotosSys == nullptr) KingdomGlobal::MotosSys = new kingdom::game::g07::cG07MotosSys(); }
void Job278_Init() { G07Sys(); G07App(new kingdom::game::g07::cAppG07MotosMatching()); }
void Job279_Init() { G07Sys(); G07App(new kingdom::game::g07::cAppG07MotosMultiMenu()); }
void Job280_Init() { G07Sys(); G07App(new kingdom::game::g07::cAppG07MotosNunchuk()); }
void Job286_Init() { G07Sys(); G07App(new kingdom::game::g07::cAppG07MotosStaffRoll()); }
void Job287_Init()
{
	WiiPad::SetUprightOverride(false);
	delete KingdomGlobal::MotosSys;
	KingdomGlobal::MotosSys = nullptr;
	KingdomGlobal::SysCore->SetJobNo(job::MENU);
}

// 0x8019a864: new kingdom::menu::cAppAttract -> RegistMainTask
void Job136_Init() { WiiPad::SetUprightOverride(false); KingdomGlobal::SysCore->RegistMainTask(new kingdom::menu::cAppAttract()); }

// 0x8019a98c: new kingdom::menu::cAppTitle -> RegistMainTask
void Job137_Init() { WiiPad::SetUprightOverride(false); KingdomGlobal::SysCore->RegistMainTask(new kingdom::menu::cAppTitle()); }

void ReleaseMainTask() { KingdomGlobal::SysCore->ReleaseMainTask(); }
}

namespace boot
{
void SetG03Round(int round) { g_g03Round = round; }
void SetG03StandIn(bool on) { g_g03StandIn = on; }

void SetEntryJob(unsigned job, unsigned nextJob)
{
	g_entryJob = job;
	g_nextJob = nextJob;
}

void Init()
{
	Slot(job::DEBUG_MENU_ROOT) = {Job0_Init, ReleaseMainTask};
	Slot(job::STAFFROLL) = {kingdom::menu::cAppG10StaffRoll::JobInit, ReleaseMainTask};   // 0x8019b250
	Slot(job::RSO_INIT).init = Job141_Init;
	Slot(job::RSO_EXEC) = {Job142_Init, ReleaseMainTask};
	Slot(job::RSO_TERM).init = Job143_Init;
	Slot(job::RSO_DEBUG_MENU) = {Job174_Init, ReleaseMainTask};
	Slot(job::KINGDOM_INIT).init = Job128_Init;
	Slot(job::MASTER).init = Job130_Init;
	Slot(job::DEBUG_MENU) = {Job131_Init, ReleaseMainTask};
	Slot(job::KINGDOM_TERM).init = Job129_Init;
	Slot(job::DEBUG_LOAD).init = Job144_Init;
	Slot(job::DEBUG_SAVE).init = Job145_Init;
	Slot(job::DEBUG_DELETE).init = Job146_Init;
	Slot(job::DEBUG_WII_MENU).init = Job147_Init;
	Slot(job::GALAGA_DEBUG_SELECT) = {JobEmpty_Init, ReleaseMainTask};
	Slot(job::RALLYX_DEBUG_SELECT) = {JobEmpty_Init, ReleaseMainTask};
	Slot(job::GROBDA_DEBUG_SELECT) = {Job340_Init, ReleaseMainTask};
	Slot(job::PACNROLL_INIT).init = Job208_Init;
	Slot(job::PACNROLL_DEBUG_MENU) = {Job209_Init, ReleaseMainTask};
	Slot(job::PACNROLL_DEBUG_SELECT).init = Job212_Init;
	Slot(job::PACNROLL_GAMEOVER) = {Job213_Init, ReleaseMainTask};
	Slot(job::PACNROLL_MENU) = {Job210_Init, ReleaseMainTask};
	Slot(job::PACNROLL_GAME) = {Job211_Init, ReleaseMainTask};
	Slot(job::PACNROLL_EDIT).init = Job212_Init;
	Slot(job::PACNROLL_EDIT2).init = Job212_Init;
	Slot(job::PACNROLL_NUNCHUK) = {Job216_Init, ReleaseMainTask};
	Slot(job::PACNROLL_EDIT_ENEMY).init = Job212_Init;
	Slot(job::PACNROLL_STAFFROLL) = {Job222_Init, ReleaseMainTask};
	Slot(job::PACNROLL_TERM).init = Job223_Init;
	Slot(job::MENU) = {Job138_Init, ReleaseMainTask};
	Slot(job::MENU_CLASSIC) = {Job140_Init, ReleaseMainTask};
	// Galaga (g01), jobs 176..: 176 (0x800b0cf4) creates the cG01GalagaSys, stage 1, -> 178; 177 (0x800b0c8c) debug menu
	Slot(job::GALAGA_INIT).init = kingdom::game::g01::cG01GalagaSys::JobInit;
	Slot(job::GALAGA_DEBUG_MENU) = {Job177_Init, ReleaseMainTask};
	Slot(job::GALAGA_MENU) = {Job178_Init, ReleaseMainTask};   // 0x800b0d98 stage select
	Slot(job::GALAGA_GAME) = {Job179_Init, ReleaseMainTask};   // 0x800b0ed4 stage
	Slot(job::GALAGA_NUNCHUK) = {Job184_Init, ReleaseMainTask};
	Slot(job::GALAGA_TERM).init = kingdom::game::g01::cG01GalagaSys::JobEnd;   // 0x800b0d40: delete the sys, back to the hub
	Slot(job::RALLYX_INIT).init = Job192_Init;
	Slot(job::RALLYX_DEBUG_MENU) = {Job193_Init, ReleaseMainTask};   // native (retail: empty stub 0x800c6430)
	Slot(job::RALLYX_MENU) = {Job194_Init, ReleaseMainTask};   // 0x800c6504 stage select (term 0x800c6638)
	Slot(job::RALLYX_GAMEOVER) = {Job198_Init, ReleaseMainTask};   // 0x800c68c4 game over (term 0x800c6a3c)
	Slot(job::RALLYX_NUNCHUK) = {Job201_Init, ReleaseMainTask};   // 0x800c6ccc Nunchuk card (term 0x800c6e30)
	Slot(job::RALLYX_STAFFROLL) = {Job206_Init, ReleaseMainTask};   // 0x800c6b80 staff roll (term 0x800c6cc4)
	Slot(job::RALLYX_MATCHING) = {Job199_Init, ReleaseMainTask};   // 0x800c6a44 matching (term 0x800c6b78)
	Slot(job::RALLYX_MULTI_MENU) = {Job200_Init, ReleaseMainTask};   // 0x800c6640 multiplayer menu (term = ReleaseMainTask)
	Slot(job::RALLYX_GAME) = {Job195_Init, ReleaseMainTask};
	Slot(job::RALLYX_TERM).init = Job207_Init;
	Slot(job::DUMMY_INIT).init = Job160_Init;
	Slot(job::DUMMY_MENU) = {Job162_Init, ReleaseMainTask};
	Slot(job::DUMMY_GAME) = {Job163_Init, ReleaseMainTask};
	Slot(job::DUMMY_TERM).init = Job175_Init;
	Slot(job::WANIWANI_INIT).init = Job224_Init;
	Slot(job::WANIWANI_DEBUG_MENU) = {kingdom::game::g04::cAppG04WaniWaniDebugMenu::JobInit, ReleaseMainTask};   // 0x80199f64 (term 0x8019a074)
	Slot(job::WANIWANI_GAME) = {Job227_Init, ReleaseMainTask};
	Slot(job::WANIWANI_GAME_MULTI) = {Job230_Init, ReleaseMainTask};   // 0x8019a2a0
	Slot(job::WANIWANI_MATCHING) = {kingdom::game::g04::cAppG04WaniWaniMatching::JobInit, ReleaseMainTask};   // 0x8019a470
	Slot(job::WANIWANI_NUNCHUK).init = Job231_Init;
	Slot(job::WANIWANI_TERM).init = Job239_Init;
	Slot(job::GROBDA_INIT).init = Job336_Init;
	Slot(job::GROBDA_MENU) = {Job338_Init, ReleaseMainTask};
	Slot(job::GROBDA_DEBUG_MENU) = {kingdom::game::g11::cAppGrobdaDebugMenu::JobInit, ReleaseMainTask};   // 0x8000ef44
	Slot(job::GROBDA_GAMEOVER) = {kingdom::game::g11::cAppG11GrobdaGameOver::JobInit, ReleaseMainTask};   // 0x8000f380
	Slot(job::GROBDA_STAFFROLL) = {kingdom::game::g11::cAppG11GrobdaStaffRoll::JobInit, ReleaseMainTask};   // 0x8000f618
	Slot(job::GROBDA_MATCHING) = {kingdom::game::g11::cAppG11GrobdaMatching::JobInit, ReleaseMainTask};   // 0x8000fa64
	Slot(job::GROBDA_NUNCHUK) = {kingdom::game::g11::cAppG11GrobdaNunchuk::JobInit, ReleaseMainTask};   // 0x8000f804 (always skipped)
	Slot(job::GROBDA_GAME) = {Job339_Init, ReleaseMainTask};
	Slot(job::GROBDA_TERM).init = Job351_Init;
	Slot(job::GALAGA_GAMEOVER) = {Job181_Init, ReleaseMainTask};   // 0x800b1250 game over
	Slot(job::GALAGA_MATCHING) = {Job182_Init, ReleaseMainTask};   // 0x800b13d0 matching
	Slot(job::GALAGA_STAFFROLL) = {kingdom::game::g01::cAppG01GalagaStaffRoll::JobInit, ReleaseMainTask};   // 0x800b150c staff roll
	Slot(job::MOTOS_INIT).init = Job272_Init;
	Slot(job::MOTOS_MENU) = {Job274_Init, ReleaseMainTask};
	Slot(job::MOTOS_GAME) = {Job275_Init, ReleaseMainTask};
	Slot(job::MOTOS_GAMEOVER) = {Job277_Init, ReleaseMainTask};
	Slot(job::MOTOS_DEBUG_MENU) = {Job273_Init, ReleaseMainTask};
	Slot(job::MOTOS_DEBUG_SELECT).init = Job276_Init;
	Slot(job::MOTOS_MATCHING) = {Job278_Init, ReleaseMainTask};
	Slot(job::MOTOS_MULTI_MENU) = {Job279_Init, ReleaseMainTask};
	Slot(job::MOTOS_NUNCHUK) = {Job280_Init, ReleaseMainTask};
	Slot(job::MOTOS_STAFFROLL) = {Job286_Init, ReleaseMainTask};
	Slot(job::MOTOS_TERM).init = Job287_Init;
	Slot(job::BOOT) = {Job132_Init, ReleaseMainTask};
	Slot(job::LOAD) = {Job133_Init, SaveLoad_Term};
	Slot(job::SAVE) = {Job134_Init, SaveLoad_Term};
	Slot(job::ATTRACT) = {Job136_Init, ReleaseMainTask};
	Slot(job::TITLE) = {Job137_Init, ReleaseMainTask};

	KingdomGlobal::SysCore = new cSysCoreStructure();
	KingdomGlobal::SysCore->SetJobEnviron(JOB_MASK, JOB_SHIFT, JOB_COUNT, g_table.data());
	KingdomGlobal::SysCore->SetJobNo(job::KINGDOM_INIT);
}

void Frame()
{
	cJetAnita::DrawInit(); // resets the per-frame layer counter (PacMotosMain::Update did this before Process())
	KingdomGlobal::SysCore->Process();
	dbgtext::Flush(); // cSysTaskGFinish::PostDraw (0x801385fc): the debug text goes over everything
	devtools::TickFrame();
}

bool JobAvailable(unsigned job) { const unsigned i = job >> JOB_SHIFT; return i < JOB_COUNT && g_table[i].init != nullptr; }

cJetNode* Root()
{
	return KingdomGlobal::SysCore->pTaskRoot();
}
}
