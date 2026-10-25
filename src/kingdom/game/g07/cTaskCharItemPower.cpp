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
