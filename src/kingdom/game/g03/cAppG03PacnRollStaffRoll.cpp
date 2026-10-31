#include "kingdom/game/g03/cAppG03PacnRollStaffRoll.hpp"

#include <EBGMKind.hpp>

namespace kingdom::game::g03
{

// DOL 0x8012c054 [audited: per AUDIT_G03_G07 row] (job init 0x8012b818): AttachArcData("staffroll.lzs"), BGM "m03bgm_Ending" (+0x28c), next job 0x0d500000 (+0x288), RegistCamera 0x80144348 (the base's RegistData)
void cAppG03PacnRollStaffRoll::Init()
{
	pSetBgmName(EBGMKind::PACN_ROLL_SND_BGM09_ENDING);
	nSetNextJobNo(223346688u);
	RegistData();
}

// DOL 0x8012bcf0 [audited: per AUDIT_G03_G07 row] (destructor chain only)
void cAppG03PacnRollStaffRoll::Term()
{
}

cAppG03PacnRollStaffRoll::cAppG03PacnRollStaffRoll()
{
	Init();
}

void cAppG03PacnRollStaffRoll::Free()
{
	Term();
	cAppGameBaseStaffRoll::Free();
}

}
