#include "kingdom/game/g11/cAppG11GrobdaStaffRoll.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"

namespace kingdom::game::g11
{

// DOL 0x8000f618 (job 350)
void cAppG11GrobdaStaffRoll::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG11GrobdaStaffRoll());
}

// DOL 0x80207f04: the second AttachArcData("staffroll.lzs") and its detach (0x80207f4c) are left out natively (as g02 / g01); RegistCamera = the base RegistData
// (UNVERIFIED that more is needed). The BGM is the base default.
cAppG11GrobdaStaffRoll::cAppG11GrobdaStaffRoll()
{
	nSetNextJobNo(job::GROBDA_GAMEOVER);   // +0x288: the game over, which goes back to the menu
	RegistData();
}

}
