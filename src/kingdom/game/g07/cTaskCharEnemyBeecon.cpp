#include "kingdom/game/g07/cTaskCharEnemyBeecon.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyBeecon::cTaskCharEnemyBeecon()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyBeecon::Construct()
{
	return new cTaskCharEnemyBeecon();
}

void cTaskCharEnemyBeecon::Init()
{
}

void cTaskCharEnemyBeecon::Term()
{
	cTaskCharEnemy::Term();
}

void cTaskCharEnemyBeecon::FallEffect(int nRefID)
{
	cTaskChar* cTaskChar2 = nullptr;
	if (!KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)nRefID);
		if (cTaskChar2 != nullptr && cTaskChar2->bTestFlag(2u))
		{
			Constants::SpawnEffect(40, vMTrans());
		}
	}
}

bool cTaskCharEnemyBeecon::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_BEACON");
	string array[3] = {"TSA_CHAR_ENEMY_BEACON", "CLA_CHAR_ENEMY_BEACON", "CHA_CHAR_ENEMY_BEACON"};
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
