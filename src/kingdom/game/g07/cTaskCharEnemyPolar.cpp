#include "kingdom/game/g07/cTaskCharEnemyPolar.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyPolar::cTaskCharEnemyPolar()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyPolar::Construct()
{
	return new cTaskCharEnemyPolar();
}

void cTaskCharEnemyPolar::Init()
{
	nChangeMoveFrame() = 16;
}

void cTaskCharEnemyPolar::Term()
{
	cTaskCharEnemy::Term();
}

bool cTaskCharEnemyPolar::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_POLAR");
	string array[3] = {"TSA_CHAR_ENEMY_POLAR", "CLA_CHAR_ENEMY_POLAR", "CHA_CHAR_ENEMY_POLAR"};
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

uint cTaskCharEnemyPolar::Input()
{
	float num = 1.0f - HereticRate();
	ushort num2 = nInputDir();
	if (bTestFlag(64u))
	{
		return 0u;
	}
	if (bTestFlag(2097152u))
	{
		return 0u;
	}
	if (bTestFlag(512u))
	{
		return 0u;
	}
	cTaskCharPlayer* cTaskCharPlayer2 = SearchNearPlayer();
	Vector4 v;
	if (cTaskCharPlayer2 != nullptr)
	{
		::Constants::jetGeoVecCopy(v, cTaskCharPlayer2->vMTrans());
	}
	else
	{
		Constants::MOTOS_VEC4_SET(v, 8.0f, 0.0f, 8.0f);
	}
	float fDx = v[0] - vMTrans()[0];
	float fDz = v[2] - vMTrans()[2];
	if ((long)Constants::get_GAME_COUNTER() % (long)nChangeMoveFrame() == 0L)
	{
		num2 = nTargetDir2(fDx, fDz);
	}
	ushort num3 = nFallPassDirTarget(num2, v, vSpeed(), fAccelSpeed() * num);
	if (num3 != num2)
	{
		num2 = num3;
		::Constants::jetGeoVecUnit(vSpeed());
	}
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, Constants::motosDir2Vector(num2));
	Constants::VEC3_MUL(v2, v2, fAccelSpeed() * num);
	Constants::VEC3_ADD(vSpeed(), vSpeed(), v2);
	AdjustMaxSpeed();
	vRotate()[1] -= Constants::TSP_DEG2RAD(5.0f);
	return num2;
}

bool cTaskCharEnemyPolar::ActionBoth()
{
	if (IsFall())
	{
		ChangeFallMove();
	}
	if (!Constants::IS_MASTER())
	{
		vRotate()[1] -= Constants::TSP_DEG2RAD(5.0f);
	}
	return true;
}

void cTaskCharEnemyPolar::SetMMSParam(int para1, int para2, int para3)
{
	nFormPart() = para1;
	if (CustomKingdomUtil::IF(nFormPart()))
	{
		nFormGroup() = para2;
	}
}

}
