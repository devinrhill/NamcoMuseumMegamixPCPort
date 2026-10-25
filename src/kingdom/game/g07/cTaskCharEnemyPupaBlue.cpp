#include "kingdom/game/g07/cTaskCharEnemyPupaBlue.hpp"

#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyPupaBlue::cTaskCharEnemyPupaBlue()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyPupaBlue::Construct()
{
	return new cTaskCharEnemyPupaBlue();
}

void cTaskCharEnemyPupaBlue::Init()
{
}

void cTaskCharEnemyPupaBlue::Term()
{
	cTaskCharEnemyPupaRed::Term();
}

bool cTaskCharEnemyPupaBlue::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_PUPA_BLUE");
	string array[3] = {"TSA_CHAR_ENEMY_PUPA_BLUE", "CLA_CHAR_ENEMY_PUPA_BLUE", "CHA_CHAR_ENEMY_PUPA_BLUE"};
	int num = ::Constants::ARRAY_SIZE(array);
	for (int i = 0; i < num; i++)
	{
		cKingdomModel2->RegistAnm(0u, array[i]);
	}
	cKingdomModel2->SetLightSetIdx(2);
	cKingdomModel2->SetFogIdx(2);
	cKingdomModel2->IsDrawShadowDepth(true);
	return true;
}

}
