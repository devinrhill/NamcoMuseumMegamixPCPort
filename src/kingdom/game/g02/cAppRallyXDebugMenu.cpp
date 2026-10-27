#include "kingdom/game/g02/cAppRallyXDebugMenu.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include <cstring>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "WiiPad.hpp"
#include "gfx/DebugText.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"

namespace kingdom::game::g02
{

namespace
{
constexpr uint STAGE_JOB = job::RALLYX_GAME;
constexpr int STAGE_MAX = 30;
}

cAppRallyXDebugMenu::cAppRallyXDebugMenu()
{
	SetName("Rally-X Debug Menu");
	SetMenu("Rally-X", {
		{"MENU", job::RALLYX_MENU},
		{"MULTI MENU", job::RALLYX_MULTI_MENU},
		{"MATCHING", job::RALLYX_MATCHING},
		{"STAGE", STAGE_JOB},
		{"GAMEOVER", job::RALLYX_GAMEOVER},
		{"STAFFROLL", job::RALLYX_STAFFROLL},
		{"EXIT", job::RALLYX_TERM},
	});
	m_stageRow = 0;
	for (size_t i = 0; i < m_entries.size(); i++)
		if (m_entries[i].job == STAGE_JOB) { m_stageRow = (int)i; break; }
}

bool cAppRallyXDebugMenu::Execute(cJetTask* pParam)
{
	auto* sys = KingdomGlobal::RallyXSys;
	if (sys && m_selected == STAGE_JOB)
	{
		const GamePad::State& pad = GamePad::Get(0);
		const int idx = sys->nStageIdx();
		if (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_RIGHT)) sys->nStageIdx(idx + 1 > STAGE_MAX ? 0 : idx + 1);
		else if (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_LEFT)) sys->nStageIdx(idx == 0 ? STAGE_MAX : idx - 1);
	}
	return cAppBaseDebugMenu::Execute(pParam);
}

void cAppRallyXDebugMenu::Draw(int nStatus, cJetTask* pParam)
{
	if (auto* sys = KingdomGlobal::RallyXSys; sys && m_stageRow < (int)m_entries.size())
	{
		const int x = 0x20 + (int)std::strlen(m_entries[m_stageRow].label) * 0x14, y = 0x30 + m_stageRow * 0x14;
		const int idx = sys->nStageIdx();
		const int stage = sys->nStageInWorld();
		if (idx == 0) dbgtext::Print(x, y, dbgtext::DEFAULT_COLOR, "wxxsxx");
		else if (idx > 24) dbgtext::Print(x, y, dbgtext::DEFAULT_COLOR, "vs%02d", stage);
		else if (sys->IsBossStage()) dbgtext::Print(x, y, dbgtext::DEFAULT_COLOR, "w%02d RED", (idx - 1) / 6 + 1);
		else dbgtext::Print(x, y, dbgtext::DEFAULT_COLOR, "w%02ds%02d", (idx - 1) / 6 + 1, stage);
	}
	cAppBaseDebugMenu::Draw(nStatus, pParam);
}

}
