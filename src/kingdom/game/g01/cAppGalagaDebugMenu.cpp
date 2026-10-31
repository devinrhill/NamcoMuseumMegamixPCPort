#include "kingdom/game/g01/cAppGalagaDebugMenu.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "WiiPad.hpp"
#include "cSysCoreStructure.hpp"
#include "gfx/DebugText.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"

namespace kingdom::game::g01
{
namespace
{
constexpr uint JOB_MENU = job::GALAGA_MENU, JOB_STAGE = job::GALAGA_GAME;
// 0x8036f540: label of each stage (index = stage, 0 unused)
const char* const STAGE_NAME[11] = {nullptr, "World1 Stage1", "World1 Stage2", "World1 Stage3", "World2 Stage1", "World2 Stage2",
                                    "World2 Stage3", "World3 Stage1", "World3 Stage2", "World3 Stage3", "World3 Boss"};
}

// DOL 0x800af848 [audited]
cAppGalagaDebugMenu::cAppGalagaDebugMenu()
{
	SetName("Galaga Debug Menu");
	SetMenu("Galaga", {{"MENU", JOB_MENU}, {"STAGE", JOB_STAGE}, {"EXIT", job::GALAGA_TERM}});
	m_nPlayer = 1;
	const int stage = KingdomGlobal::GalagaSys->nStage();
	m_nStage = stage > 10 ? 10 : stage;
}

// DOL 0x800af96c [audited: left / right read the physical arrows, not game bits 4 / 8]: MENU row: left / right = player count 1..4 (wrapping); STAGE row: stage 1..10 (wrapping); both are written to the sys
// every frame, then cAppGameDebugMenu::Execute (= cAppBaseDebugMenu::Execute 0x800c2414)
bool cAppGalagaDebugMenu::Execute(cJetTask* pParam)
{
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	const uint job = m_entries[m_cursor].job;
	const GamePad::State& pad = GamePad::Get(0);
	// DOL: repeat & 0x4 (game LEFT) previous, & 0x8 (game RIGHT) next; the port reads the physical arrows (see cAppDebugMenu.hpp)
	const bool prev = (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_LEFT)) != 0;
	const bool next = !prev && (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_RIGHT)) != 0;
	if (job == JOB_MENU)
	{
		if (prev) m_nPlayer = m_nPlayer == 1 ? 4 : m_nPlayer - 1;
		else if (next && ++m_nPlayer > 4) m_nPlayer = 1;
	}
	sys->nPlayer(m_nPlayer);
	if (job == JOB_STAGE)
	{
		if (prev) m_nStage = m_nStage == 1 ? 10 : m_nStage - 1;
		else if (next && ++m_nStage > 10) m_nStage = 1;
	}
	sys->nStage(m_nStage);
	return cAppBaseDebugMenu::Execute(pParam);
}

// DOL 0x800afabc [audited]: behind the MENU row "PLAYER COUNT : %d", behind the STAGE row the stage's name, at x 0xb6, y 0x30 + row * 0x14
void cAppGalagaDebugMenu::Draw(int nStatus, cJetTask* pParam)
{
	if (m_title != nullptr)
		for (size_t i = 0; i < m_entries.size(); i++)
		{
			const int y = 0x30 + (int)i * 0x14;
			if (m_entries[i].job == JOB_MENU) dbgtext::Print(0xb6, y, dbgtext::DEFAULT_COLOR, "PLAYER COUNT : %d", m_nPlayer);
			if (m_entries[i].job == JOB_STAGE) dbgtext::Print(0xb6, y, dbgtext::DEFAULT_COLOR, "%s", STAGE_NAME[m_nStage]);
		}
	cAppBaseDebugMenu::Draw(nStatus, pParam);
}

// DOL 0x800afba8 [audited]
bool cAppGalagaDebugMenu::SaveData()
{
	KingdomGlobal::GalagaSys->AllOpen();
	KingdomGlobal::GalagaSys->SaveData();
	return true;
}

// DOL 0x800b0c8c [audited] (job 177)
void cAppGalagaDebugMenu::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppGalagaDebugMenu());
}
}
