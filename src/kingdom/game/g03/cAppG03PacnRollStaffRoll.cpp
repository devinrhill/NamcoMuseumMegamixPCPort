#include "kingdom/game/g03/cAppG03PacnRollStaffRoll.hpp"

#include <EBGMKind.hpp>

namespace kingdom::game::g03
{

void cAppG03PacnRollStaffRoll::Init()
{
	pSetBgmName(EBGMKind::PACN_ROLL_SND_BGM09_ENDING);
	nSetNextJobNo(223346688u);
	RegistData();
}

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
