#include "kingdom/game/g07/cAppG07MotosMatching.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g07
{

cAppG07MotosMatching::cAppG07MotosMatching()
{
	Init();
}

void cAppG07MotosMatching::Free()
{
	Term();
	kingdom::game::menu::cAppGameBaseMenuMatching::Free();
}

void cAppG07MotosMatching::Init()
{
	KingdomGlobal::MotosSys->AttachMenuData();
	RegistData(ResourceDatabase::GetPath("ANT_CURSOR"), ResourceDatabase::GetPath("ANT_MENU_MATCHING"), ResourceDatabase::GetPath("MENU_CAM_MATCHING_SCA"), ResourceDatabase::GetPath("MENU_SET_MATCHING_MDL"));
	nMenuJob() = 287309824u;
	nMultiJob() = 292552704u;
}

void cAppG07MotosMatching::Term()
{
	KingdomGlobal::MotosSys->pGetMatchingData();
	KingdomGlobal::MotosSys->pGetMatchingData() = pMatchingData()->Clone();
	KingdomGlobal::MotosSys->DetachMenuData();
}

}
