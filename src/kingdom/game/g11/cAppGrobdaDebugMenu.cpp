#include "kingdom/game/g11/cAppGrobdaDebugMenu.hpp"

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "WiiPad.hpp"
#include "cSysCoreStructure.hpp"
#include "gfx/DebugText.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"

namespace kingdom::game::g11
{

namespace
{
constexpr int ROW_MODE = 1, ROW_STAGE = 2;
}

// DOL 0x8000ef44 (the entry table 0x803899a0)
cAppGrobdaDebugMenu::cAppGrobdaDebugMenu()
{
	SetName("Grobda Debug Menu");
	SetMenu("Grobda", {{"MENU", 0x15200000u}, {"MODE", 0x15400000u}, {"STAGE", 0x15300000u}, {"GAMEOVER", 0x15500000u}, {"STAFFROLL", 0x15e00000u}, {"EXIT", 0x15f00000u}});
}

// DOL 0x8000fcf8
bool cAppGrobdaDebugMenu::Execute(cJetTask* pParam)
{
	cG11GrobdaSys* sys = cG11GrobdaSys::Instance();
	const GamePad::State& pad = GamePad::Get(0);
	const bool prev = (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_LEFT)) != 0;
	const bool next = !prev && (pad.repeat & GamePad::PhysicalBit(WiiPad::DIR_RIGHT)) != 0;
	if (m_cursor == ROW_MODE)
	{
		int n = sys->nPlayer();
		if (next) { if (++n > 4) n = 1; }
		else if (prev) { if (--n < 1) n = 4; }
		if (n != sys->nPlayer()) sys->nPlayer(n);
	}
	else if (m_cursor == ROW_STAGE)
	{
		int s = sys->nStage();
		if (next) s = s == 0 ? 1 : 0;
		else if (prev) s = s == 0 ? 1 : s - 1;
		sys->nStage(s);
	}
	return cAppBaseDebugMenu::Execute(pParam);
}

// DOL 0x8000fc38
void cAppGrobdaDebugMenu::Draw(int nStatus, cJetTask* pParam)
{
	if (m_title != nullptr)
	{
		const int x = 0x20 + 4 * 0x14;   // strlen("MODE") * 0x14
		dbgtext::Print(x, 0x44, dbgtext::DEFAULT_COLOR, "           Player Num %d", cG11GrobdaSys::Instance()->nPlayer());
		dbgtext::Print(x, 0x58, dbgtext::DEFAULT_COLOR, "           Stage%02d", cG11GrobdaSys::Instance()->nStage());
	}
	cAppBaseDebugMenu::Draw(nStatus, pParam);
}

// DOL 0x8000ef44: new -> RegistMainTask (the sys is made by job 336 in the DOL; --job 0x15100000 makes it here)
void cAppGrobdaDebugMenu::JobInit()
{
	if (KingdomGlobal::GrobdaSys == nullptr) KingdomGlobal::GrobdaSys = new cG11GrobdaSys();
	KingdomGlobal::SysCore->RegistMainTask(new cAppGrobdaDebugMenu());
}

}
