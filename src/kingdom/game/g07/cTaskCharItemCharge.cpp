#include "kingdom/game/g07/cTaskCharItemCharge.hpp"

#include "kingdom/cKingdomModel.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharItemCharge::cTaskCharItemCharge()
{
	Init();
}

cTaskCharItem* cTaskCharItemCharge::Construct()
{
	return new cTaskCharItemCharge();
}

void cTaskCharItemCharge::Init()
{
}

void cTaskCharItemCharge::Term()
{
	cTaskCharItem::Term();
}

bool cTaskCharItemCharge::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_ITEM_FORCE");
	string array[3] = {"TSA_ITEM_FORCE", "CLA_ITEM_FORCE", "CHA_ITEM_FORCE"};
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
