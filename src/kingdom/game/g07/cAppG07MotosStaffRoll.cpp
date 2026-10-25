#include "kingdom/game/g07/cAppG07MotosStaffRoll.hpp"

#include <EBGMKind.hpp>

namespace kingdom::game::g07
{

void cAppG07MotosStaffRoll::Init()
{
	pSetBgmName(EBGMKind::MOTOS_SND_BGM07_ENDING);
	nSetNextJobNo(290455552u);
	RegistData();
}

void cAppG07MotosStaffRoll::Term()
{
}

cAppG07MotosStaffRoll::cAppG07MotosStaffRoll()
{
	Init();
}

void cAppG07MotosStaffRoll::Free()
{
	Term();
	cAppGameBaseStaffRoll::Free();
}

}
