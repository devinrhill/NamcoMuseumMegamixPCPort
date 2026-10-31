#include "kingdom/game/g07/cAppMotosDebugMenu.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "GamePad.hpp"
#include "WiiPad.hpp"
#include "KingdomGlobal.hpp"
#include <cstring>

#include "gfx/DebugText.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

namespace kingdom::game::g07
{

// DOL 0x800a29bc [audited: described by the 43rd-session pass]: FUN_800c2404(menu, "Motos", 7, table 0x80394d08)
cAppMotosDebugMenu::cAppMotosDebugMenu()
{
	SetName("Motos Debug Menu");
	SetMenu("Motos", {
		{"MENU", job::MOTOS_MENU},
		{"MULTI MENU", job::MOTOS_MULTI_MENU},
		{"MATCHING", job::MOTOS_MATCHING},
		{"STAGE", job::MOTOS_DEBUG_SELECT},
		{"GAMEOVER", job::MOTOS_GAMEOVER},
		{"STAFFROLL", job::MOTOS_STAFFROLL},
		{"EXIT", job::MOTOS_TERM},
	});
	m_stageRow = 0;
	for (size_t i = 0; i < m_entries.size(); i++)
		if (m_entries[i].job == job::MOTOS_DEBUG_SELECT) { m_stageRow = (int)i; break; }
}

// DOL 0x800a36c4 [audited: per AUDIT_G03_G07 row]
bool cAppMotosDebugMenu::Execute(cJetTask* pParam)
{
	auto* sys = KingdomGlobal::MotosSys;
	if (sys && m_selected == job::MOTOS_DEBUG_SELECT)
	{
		const GamePad::State& pad = GamePad::Get(0);
		uint& idx = sys->nStageIdx();
		if (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_RIGHT)) idx = idx + 1 > 0x2d ? 0 : idx + 1;
		else if (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_LEFT)) idx = idx == 0 ? 0x2d : idx - 1;
	}
	return kingdom::cAppBaseDebugMenu::Execute(pParam);
}

// DOL 0x800a35a4 [audited: per AUDIT_G03_G07 row]
void cAppMotosDebugMenu::Draw(int nStatus, cJetTask* pParam)
{
	if (auto* sys = KingdomGlobal::MotosSys; sys && m_stageRow < (int)m_entries.size())
	{
		const int x = 0x20 + (int)std::strlen(m_entries[m_stageRow].label) * 0x14, y = 0x30 + m_stageRow * 0x14;
		if (sys->nStageIdx() == 0) dbgtext::Print(x, y, dbgtext::DEFAULT_COLOR, "wxxsxx");
		else if (sys->nStageIdx() < 0x24) dbgtext::Print(x, y, dbgtext::DEFAULT_COLOR, "w%02ds%02d", (int)sys->nWorld(), (int)sys->nStage());
		else dbgtext::Print(x, y, dbgtext::DEFAULT_COLOR, "vs%02d", (int)sys->nStage());
	}
	kingdom::cAppBaseDebugMenu::Draw(nStatus, pParam);
}
}
