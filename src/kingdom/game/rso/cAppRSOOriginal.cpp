#include "kingdom/game/rso/cAppRSOOriginal.hpp"

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include <cstring>
#include <vector>

#include "gotch/Console.hpp"
#include "gotch/DataStorage.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/KingdomJobTable.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom
{
namespace game
{
namespace rso
{
namespace
{
// save block 6 (0x2000 bytes) holds both volumes' gotch saves: nmwii.rso (game index < 9) the first 0x800 bytes, nmwii2.rso the next
constexpr uint kRsoSaveBlock = 6;
constexpr uint kRsoSaveSize = 0x2000;
std::size_t SliceOf(int game) { return (uint)game < 9 ? 0 : gotch::kBridgeSaveSize; }
}

// DOL 0x800c1700 (ctor): the game index from cKingdomSys +0xa70 (< 9 = nmwii.rso, else nmwii2.rso) and the volume's save slice of block 6 into
// the bridge (+0x29c = sRSOBridge +0x1c). The module load / link, the bridge's heap blocks and settings are the Wii side of the module and are
// not ported (the gotch port reads the module itself)
cAppRSOOriginal::cAppRSOOriginal()
{
	m_nGame = KingdomGlobal::KingSys->nRsoGame();
	std::vector<std::uint8_t> block(kRsoSaveSize, 0);
	KingdomGlobal::KingSys->LoadGameData(kRsoSaveBlock, block.data(), kRsoSaveSize);   // FUN_8000d8cc
	std::memcpy(gotch::BridgeSave(), block.data() + SliceOf(m_nGame), gotch::kBridgeSaveSize);
}

// DOL 0x800c1f00 (dtor) -> 0x800c1ab0: frees the bridge's heap blocks (+0x28c / +0x294 / +0xadc: nothing native), puts the bridge's save back
// into the volume's slice of block 6 (FUN_8000d8cc, copy, FUN_8000d8d4), re-enables the pointer of every remote (KPADEnableDPD x 4,
// DAT_80478fd8 = 0: no native counterpart). The module has written its save into the bridge when it left (PACMAN::ExitExtend).
void cAppRSOOriginal::Free()
{
	if (m_bRunning)
	{
		gotch::console::Stop();
		m_bRunning = false;
	}
	std::vector<std::uint8_t> block(kRsoSaveSize, 0);
	KingdomGlobal::KingSys->LoadGameData(kRsoSaveBlock, block.data(), kRsoSaveSize);
	std::memcpy(block.data() + SliceOf(m_nGame), gotch::BridgeSave(), gotch::kBridgeSaveSize);
	KingdomGlobal::KingSys->SaveGameData(kRsoSaveBlock, block.data(), kRsoSaveSize);
	cAppKingdom::Free();
}

// DOL 0x800c1b90. The DOL skips the frame while HOME is held (pad bit 0x400) or the HOME menu is up (+0xa50 +0x1b4), then calls MainA once:
// the module's loop returns when the player leaves. Here MainA's loop is one gotch::console::Frame per Execute (HOME = the exit stand-in, so
// the HOME test of the DOL is not repeated).
bool cAppRSOOriginal::Execute(cJetTask*)
{
	if (m_bDone) return true;
	bool left;
	if (!m_bRunning)
	{
		KingdomGlobal::KingSys->SetObjA50Flags(true, false);   // FUN_8009dc08(+0xa50, 1, 0)
		m_bRunning = gotch::console::Start(m_nGame);           // MainA (RSO2 0x80d53814) -> GoTitle -> X::Setup
		left = !m_bRunning;                                     // a game that is not ported returns at once
	}
	else
		left = !gotch::console::Frame();
	if (!left) return true;
	gotch::console::Stop();
	m_bRunning = false;
	m_bDone = true;
	// the result flags (+0xaa0 / +0xa9c / +0xad0 / +0xacc) are all 0 natively: FUN_8009dcb0(+0xa50, 0, 0, 0) re-installs the reset / power
	// callbacks (no native counterpart), then job 143 with force
	Kingdom_SetJobNo(job::RSO_TERM, true, 0);
	return true;
}

void cAppRSOOriginal::Draw(int, cJetTask*)
{
	if (m_bRunning) gotch::console::Draw();
}

// DOL 0x800c1f98: FUN_800c2404(menu, "RSO Release" (0x803709cc), 22, table 0x804382c0)
cAppRSODebugMenu::cAppRSODebugMenu()
{
	SetName("cJetTask");
	SetMenu("RSO Release", {
		{"RSO 00", job::RSO_INIT}, {"RSO 01", job::RSO_INIT}, {"RSO 02", job::RSO_INIT}, {"RSO 03", job::RSO_INIT},
		{"RSO 04", job::RSO_INIT}, {"RSO 05", job::RSO_INIT}, {"RSO 06", job::RSO_INIT}, {"RSO 07", job::RSO_INIT},
		{"RSO 08", job::RSO_INIT}, {"RSO 09", job::RSO_INIT}, {"RSO 10", job::RSO_INIT}, {"RSO 11", job::RSO_INIT},
		{"RSO 12", job::RSO_INIT}, {"RSO 13", job::RSO_INIT}, {"RSO 14", job::RSO_INIT}, {"RSO 15", job::RSO_INIT},
		{"RSO 16", job::RSO_INIT}, {"RSO 17", job::RSO_INIT},
		{"Load   Save Data", job::DEBUG_LOAD},
		{"Save   Save Data", job::DEBUG_SAVE},
		{"Delete Save Data", job::DEBUG_DELETE},
		{"EXIT", job::DEBUG_MENU},
	});
}

// DOL 0x800c22c4: the first frame puts the cursor on the last game index (+0xa70) and stores 2 at +0xa74 (the DOL also moves the raw pad's
// second word into the first and clears it, FUN_801411d8: not repeated, the native pad block has no such words); while an RSO row is the
// current one, its row is the game index
bool cAppRSODebugMenu::Execute(cJetTask* pParam)
{
	if (!m_bInit)
	{
		m_bInit = true;
		m_cursor = KingdomGlobal::KingSys->nRsoGame();
		KingdomGlobal::KingSys->nRsoA74() = 2;
	}
	if (m_selected == job::RSO_INIT)
	{
		KingdomGlobal::KingSys->nRsoGame() = m_cursor;
		KingdomGlobal::KingSys->nRsoA74() = 2;
	}
	return cAppBaseDebugMenu::Execute(pParam);   // 0x800951f0
}
}
}
}
