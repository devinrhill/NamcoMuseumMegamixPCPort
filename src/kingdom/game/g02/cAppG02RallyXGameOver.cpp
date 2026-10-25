#include "kingdom/game/g02/cAppG02RallyXGameOver.hpp"

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"

namespace kingdom::game::g02
{

// DOL 0x800c68c4 (job 198)
void cAppG02RallyXGameOver::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG02RallyXGameOver());
}

// DOL 0x800b329c
cAppG02RallyXGameOver::cAppG02RallyXGameOver()
{
	SetName("cAppG02RallyXGameOver");
	KingdomGlobal::RallyXSys->AttachStageArc();   // FUN_800c5a1c
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANITA_GAMEOVER"));
	pAnita(0u)->SetAnimNo(0, true);
	nAnita() = 0u;               // +0x294
	nNextJob() = 0x0c200000u;    // +0x298: the stage select
}

void cAppG02RallyXGameOver::Free()
{
	cAppGameBaseGameOver::Free();
	KingdomGlobal::RallyXSys->DetachStageArc();   // FUN_800c5a78 (the dtor 0x800b3348)
}

}
