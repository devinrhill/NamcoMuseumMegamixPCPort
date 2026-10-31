#include "kingdom/game/g03/cAppPacnRollDebugMenu.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "GamePad.hpp"
#include "WiiPad.hpp"
#include "KingdomGlobal.hpp"
#include <cstring>

#include "gfx/DebugText.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"

namespace kingdom::game::g03
{

// DOL 0x8012b008 [audited: per AUDIT_G03_G07 row]: FUN_800c2404(menu, "Pacn Roll", 8, table 0x804414e8)
cAppPacnRollDebugMenu::cAppPacnRollDebugMenu()
{
	SetName("Pacn Roll Debug Menu");
	SetMenu("Pacn Roll", {
		{"MENU", job::PACNROLL_MENU},
		{"STAGE", job::PACNROLL_DEBUG_SELECT},
		{"EDIT", job::PACNROLL_EDIT},
		{"EDIT2", job::PACNROLL_EDIT2},
		{"EDIT_ENEMY", job::PACNROLL_EDIT_ENEMY},
		{"GAMEOVER", job::PACNROLL_GAMEOVER},
		{"STAFFROLL", job::PACNROLL_STAFFROLL},
		{"EXIT", job::PACNROLL_TERM},
	});
	m_stageRow = 0;                                  // 0x8012b0f8..0x8012b140: first entry whose job == 0x0d400000
	for (size_t i = 0; i < m_entries.size(); i++)
		if (m_entries[i].job == job::PACNROLL_DEBUG_SELECT) { m_stageRow = (int)i; break; }
}

// DOL 0x8012bb78 [audited: per AUDIT_G03_G07 row] (rounds wrap 0..0x1b with the pad's repeat bits 8 / 4 while the cursor is on STAGE / EDIT / EDIT2 / EDIT_ENEMY)
bool cAppPacnRollDebugMenu::Execute(cJetTask* pParam)
{
	auto* sys = KingdomGlobal::PacnRollSys;
	const uint sel = m_selected;
	if (sys && (sel == job::PACNROLL_DEBUG_SELECT || sel == job::PACNROLL_EDIT || sel == job::PACNROLL_EDIT2 || sel == job::PACNROLL_EDIT_ENEMY))
	{
		const GamePad::State& pad = GamePad::Get(0);
		// DOL: repeat & 0x8 (game RIGHT) -> next, & 0x4 (game LEFT) -> previous (0x8012bbd4 / 0x8012bc04); swapped here, see the header
		if (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_RIGHT)) sys->nRound() = sys->nRound() < 0x1b ? sys->nRound() + 1 : 0;
		else if (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_LEFT)) sys->nRound() = sys->nRound() == 0 ? 0x1b : sys->nRound() - 1;
	}
	return kingdom::cAppBaseDebugMenu::Execute(pParam);
}

// DOL 0x8012bad0 [audited: per AUDIT_G03_G07 row] ("w%02ds%02d" at x = 0x20 + strlen(label of the +0x4c row) * 0x14, y = 0x30 + row * 0x14, then cAppBaseDebugMenu::Draw 0x800c24f8)
// vtable slot 13 (0x8012bc54) = cSaveLoadMan::ForceAllClear + return 1: nothing in the base Execute calls it; the dev window's Menu tab exposes it
void cAppPacnRollDebugMenu::Draw(int nStatus, cJetTask* pParam)
{
	if (auto* sys = KingdomGlobal::PacnRollSys; sys && m_stageRow < (int)m_entries.size())
	{
		const int len = (int)std::strlen(m_entries[m_stageRow].label);
		dbgtext::Print(0x20 + len * 0x14, 0x30 + m_stageRow * 0x14, dbgtext::DEFAULT_COLOR, "w%02ds%02d", (int)sys->nWorld() + 1, (int)sys->nStage() + 1);
	}
	kingdom::cAppBaseDebugMenu::Draw(nStatus, pParam);
}
}
