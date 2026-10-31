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

// DOL 0x800acf24 [audited: per AUDIT_G03_G07 row] (vtable +0xe8 of the cTaskChar part): model, its TSA / CLA / CHA when present, light set 2, fog 2
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
