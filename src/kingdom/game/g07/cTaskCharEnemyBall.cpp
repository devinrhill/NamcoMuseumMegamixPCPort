#include "kingdom/game/g07/cTaskCharEnemyBall.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyBall::cTaskCharEnemyBall()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyBall::Construct()
{
	return new cTaskCharEnemyBall();
}

void cTaskCharEnemyBall::Init()
{
}

void cTaskCharEnemyBall::Term()
{
	cTaskCharEnemy::Term();
}

void cTaskCharEnemyBall::FallEffect(int nRefID)
{
	cTaskChar* cTaskChar2 = nullptr;
	if (!KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)nRefID);
		if (cTaskChar2 != nullptr && cTaskChar2->bTestFlag(2u))
		{
			Constants::SpawnEffect(41, vMTrans());
		}
	}
}

bool cTaskCharEnemyBall::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	CustomKingdomUtil::Assert(no == 22 || no == 23, "error!");
	m_BallType = no;
	switch (no)
	{
	case 22u:
		{
			cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_BALL_A");
			string array2[3] = {"TSA_CHAR_ENEMY_BALL_A", "CLA_CHAR_ENEMY_BALL_A", "CHA_CHAR_ENEMY_BALL_A"};
			int num2 = ::Constants::ARRAY_SIZE(array2);
			for (int j = 0; j < num2; j++)
			{
				cKingdomModel2->RegistAnm(0u, array2[j]);
			}
			break;
		}
	case 23u:
		{
			cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_BALL_B");
			string array[3] = {"TSA_CHAR_ENEMY_BALL_B", "CLA_CHAR_ENEMY_BALL_B", "CHA_CHAR_ENEMY_BALL_B"};
			int num = ::Constants::ARRAY_SIZE(array);
			for (int i = 0; i < num; i++)
			{
				cKingdomModel2->RegistAnm(0u, array[i]);
			}
			break;
		}
	}
	cKingdomModel2->SetLightSetIdx(2);
	cKingdomModel2->SetFogIdx(2);
	cKingdomModel2->IsDrawShadowDepth(true);
	return true;
}

uint cTaskCharEnemyBall::Input()
{
	switch (m_BallType)
	{
	case 22u:
		RollMatrix(pMatrix(), 0, "polymsh15");
		break;
	case 23u:
		RollMatrix(pMatrix(), 0, "polymsh16");
		break;
	default:
		CustomKingdomUtil::Assert("error");
		break;
	}
	return cTaskChar::Input();
}

}
