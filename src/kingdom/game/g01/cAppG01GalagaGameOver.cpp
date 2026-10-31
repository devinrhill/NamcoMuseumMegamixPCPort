#include "kingdom/game/g01/cAppG01GalagaGameOver.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"

namespace kingdom::game::g01
{

// DOL 0x800b1250 [audited] (job 181)
void cAppG01GalagaGameOver::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG01GalagaGameOver());
}

// DOL 0x800fbfd4 [audited]
cAppG01GalagaGameOver::cAppG01GalagaGameOver()
{
	pAnita(0u)->RegistData("ANT_GAMEOVER");
	pAnita(0u)->SetAnimNo(0);
	nAnita() = 0u;               // +0x294
	nNextJob() = job::GALAGA_MENU;    // +0x298
}

}
