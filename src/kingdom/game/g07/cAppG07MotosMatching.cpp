#include "kingdom/game/g07/cAppG07MotosMatching.hpp"
#include "kingdom/KingdomJobTable.hpp"
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

// DOL job init 0x800a3268, ctor 0x8012bfa4 (g07menu.lzs attach, ANT_CURSOR / ANT_MENU_MATCHING / MENU_CAM_MATCHING_SCA / MENU_SET_MATCHING_MDL)
void cAppG07MotosMatching::Init()
{
	KingdomGlobal::MotosSys->AttachMenuData();
	RegistData(ResourceDatabase::GetPath("ANT_CURSOR"), ResourceDatabase::GetPath("ANT_MENU_MATCHING"), ResourceDatabase::GetPath("MENU_CAM_MATCHING_SCA"), ResourceDatabase::GetPath("MENU_SET_MATCHING_MDL"));
	nMenuJob() = kingdom::job::MOTOS_MENU;
	nMultiJob() = kingdom::job::MOTOS_MULTI_MENU;
}

// DOL 0x8012c00c [audited: per AUDIT_G03_G07 row] (dtor body: copy the 0x154-byte matching data into the sys +0x10, detach g07menu.lzs)
void cAppG07MotosMatching::Term()
{
	KingdomGlobal::MotosSys->pGetMatchingData();
	KingdomGlobal::MotosSys->pGetMatchingData() = pMatchingData()->Clone();
	KingdomGlobal::MotosSys->DetachMenuData();
}

}
