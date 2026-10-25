#include "kingdom/game/g07/cTaskCharItemJump.hpp"

#include "kingdom/cKingdomModel.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharItemJump::cTaskCharItemJump()
{
	Init();
}

cTaskCharItem* cTaskCharItemJump::Construct()
{
	return new cTaskCharItemJump();
}

void cTaskCharItemJump::Init()
{
}

void cTaskCharItemJump::Term()
{
	cTaskCharItem::Term();
}

bool cTaskCharItemJump::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_ITEM_MOBILITY");
	string array[3] = {"TSA_ITEM_MOBILITY", "CLA_ITEM_MOBILITY", "CHA_ITEM_MOBILITY"};
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
