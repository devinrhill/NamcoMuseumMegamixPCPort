#include "kingdom/game/g07/cTaskCharEnemyPupaBlack.hpp"

#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyPupaBlack::cTaskCharEnemyPupaBlack()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyPupaBlack::Construct()
{
	return new cTaskCharEnemyPupaBlack();
}

void cTaskCharEnemyPupaBlack::Init()
{
}

void cTaskCharEnemyPupaBlack::Term()
{
	cTaskCharEnemyPupaRed::Term();
}

bool cTaskCharEnemyPupaBlack::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_PUPA_BLACK");
	string array[3] = {"TSA_CHAR_ENEMY_PUPA_BLACK", "CLA_CHAR_ENEMY_PUPA_BLACK", "CHA_CHAR_ENEMY_PUPA_BLACK"};
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
