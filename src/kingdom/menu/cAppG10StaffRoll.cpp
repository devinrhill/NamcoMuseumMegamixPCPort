#include "kingdom/menu/cAppG10StaffRoll.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include <EBGMKind.hpp>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"

namespace kingdom::menu
{

// DOL 0x8019b250 [audited] (job 139)
void cAppG10StaffRoll::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG10StaffRoll());
}

// DOL 0x80202ed0 [audited]: the AttachArcData("staffroll.lzs") and its detach (0x80202f20) are left out natively (as g02 / g01 / g11); RegistCamera = the
// base RegistData (UNVERIFIED that more is needed)
cAppG10StaffRoll::cAppG10StaffRoll()
{
	pSetBgmName(EBGMKind::PACN_ROLL_SND_BGM09_ENDING);   // +0x28c = "m03bgm_Ending" (0x8047d6b0)
	nSetNextJobNo(job::TITLE);                          // +0x288 = 0x08900000
	RegistData();
}

}
