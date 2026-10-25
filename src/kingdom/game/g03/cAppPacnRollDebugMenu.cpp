#include "kingdom/game/g03/cAppPacnRollDebugMenu.hpp"

#include "GamePad.hpp"
#include "WiiPad.hpp"
#include "KingdomGlobal.hpp"
#include <cstring>

#include "gfx/DebugText.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"

namespace kingdom::game::g03
{

// DOL 0x8012b008: FUN_800c2404(menu, "Pacn Roll", 8, table 0x804414e8)
cAppPacnRollDebugMenu::cAppPacnRollDebugMenu()
{
	SetName("Pacn Roll Debug Menu");
	SetMenu("Pacn Roll", {
		{"MENU", 0x0d200000u},
		{"STAGE", 0x0d400000u},
		{"EDIT", 0x0d600000u},
		{"EDIT2", 0x0d700000u},
		{"EDIT_ENEMY", 0x0d900000u},
		{"GAMEOVER", 0x0d500000u},
		{"STAFFROLL", 0x0de00000u},
		{"EXIT", 0x0df00000u},
	});
	m_stageRow = 0;                                  // 0x8012b0f8..0x8012b140: first entry whose job == 0x0d400000
	for (size_t i = 0; i < m_entries.size(); i++)
		if (m_entries[i].job == 0x0d400000u) { m_stageRow = (int)i; break; }
}

bool cAppPacnRollDebugMenu::Execute(cJetTask* pParam)
{
	auto* sys = KingdomGlobal::PacnRollSys;
	const uint sel = m_selected;
	if (sys && (sel == 0x0d400000u || sel == 0x0d600000u || sel == 0x0d700000u || sel == 0x0d900000u))
	{
		const GamePad::State& pad = GamePad::Get(0);
		// DOL: repeat & 0x8 (game RIGHT) -> next, & 0x4 (game LEFT) -> previous (0x8012bbd4 / 0x8012bc04); swapped here, see the header
		if (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_RIGHT)) sys->nRound() = sys->nRound() < 0x1b ? sys->nRound() + 1 : 0;
		else if (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_LEFT)) sys->nRound() = sys->nRound() == 0 ? 0x1b : sys->nRound() - 1;
	}
	return kingdom::cAppBaseDebugMenu::Execute(pParam);
}

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
