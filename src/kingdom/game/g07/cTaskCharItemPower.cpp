#include "kingdom/game/g07/cTaskCharItemPower.hpp"

#include "kingdom/cKingdomModel.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharItemPower::cTaskCharItemPower()
{
	Init();
}

cTaskCharItem* cTaskCharItemPower::Construct()
{
	return new cTaskCharItemPower();
}

void cTaskCharItemPower::Init()
{
}

void cTaskCharItemPower::Term()
{
	cTaskCharItem::Term();
}

// DOL 0x800ad430 [audited: per AUDIT_G03_G07 row] (vtable +0xe8 of the cTaskChar part): model, its TSA / CLA / CHA when present, light set 2, fog 2
bool cTaskCharItemPower::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_ITEM_POWER");
	string array[3] = {"TSA_ITEM_POWER", "CLA_ITEM_POWER", "CHA_ITEM_POWER"};
	int num = ::Constants::ARRAY_SIZE(array);
	for (int i = 0; i < num; i++)
	{
		cKingdomModel2->RegistAnm(0u, array[i]);
	}
	cKingdomModel2->SetLightSetIdx(2);
	cKingdomModel2->SetFogIdx(2);
	return true;
}

}
