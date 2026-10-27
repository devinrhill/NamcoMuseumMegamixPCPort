#include "kingdom/game/g11/cAppG11GrobdaGameOver.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"

namespace kingdom::game::g11
{

// DOL 0x8000f380 (job 341)
void cAppG11GrobdaGameOver::JobInit()
{
	if (KingdomGlobal::GrobdaSys == nullptr) KingdomGlobal::GrobdaSys = new cG11GrobdaSys();   // NATIVE: the sys is made by job 336 in the DOL; --job skips it
	KingdomGlobal::SysCore->RegistMainTask(new cAppG11GrobdaGameOver());
}

// DOL 0x80122d10
cAppG11GrobdaGameOver::cAppG11GrobdaGameOver()
{
	SetName("cAppG11GrobdaGameOver");
	pAnita(0u)->RegistData("BIN_ANT_GAMEOVER");
	pAnita(0u)->SetAnimNo(0, true);
	nAnita() = 0u;               // +0x294
	nNextJob() = job::GROBDA_MENU;    // +0x298: the menu
}

}
