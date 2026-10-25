#include "kingdom/game/g07/cTaskCharEnemyFireBee.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyFireBee::cTaskCharEnemyFireBee()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyFireBee::Construct()
{
	return new cTaskCharEnemyFireBee();
}

void cTaskCharEnemyFireBee::Init()
{
	nChangeMoveFrame() = 30;
	ForceMode(3);
}

void cTaskCharEnemyFireBee::Term()
{
	cTaskCharEnemy::Term();
}

bool cTaskCharEnemyFireBee::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_FIRE_BEE");
	string array[3] = {"TSA_CHAR_ENEMY_FIRE_BEE", "CLA_CHAR_ENEMY_FIRE_BEE", "CHA_CHAR_ENEMY_FIRE_BEE"};
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

uint cTaskCharEnemyFireBee::Input()
{
	float num = 1.0f - HereticRate();
	ushort num2 = nInputDir();
	if (bTestFlag(64u))
	{
		return 0u;
	}
	if (bTestFlag(2097152u))
	{
		ForceMode(3);
		nLTimer() = 0;
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
	nLTimer()++;
	if (bTestFlag(1024u))
	{
		ForceMode(5);
	}
	else if (GetMode() == 5 && !bTestFlag(1024u))
	{
		ForceMode(3);
		nLTimer() = 0;
	}
	if (!bTestFlag(2097152u) && bTestFlagOLD(2097152u))
	{
		ForceMode(3);
		nLTimer() = 0;
	}
	if (GetMode() == 2 && nLTimer() > 60)
	{
		ForceMode(3);
		nLTimer() = 0;
	}
	else if (GetMode() == 4 && nLTimer() > 20)
	{
		ForceMode(2);
		nLTimer() = 0;
		::Constants::jetGeoVecUnit(vSpeed());
		num2 = 0;
		nModelDir() = nTargetDir(fDx, fDz);
	}
	else if (GetMode() == 3)
	{
		num2 = nTargetDir(fDx, fDz);
		ForceMode(1);
		nLTimer() = 0;
	}
	ushort num3 = nFallPassDirRegular(num2, vSpeed(), fAccelSpeed() * num);
	if (num3 != num2)
	{
		num2 = num3;
		::Constants::jetGeoVecUnit(vSpeed());
		nLTimer() = 0;
		ForceMode(4);
	}
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, Constants::motosDir2Vector(num2));
	Constants::VEC3_MUL(v2, v2, fAccelSpeed() * num);
	Constants::VEC3_ADD(vSpeed(), vSpeed(), v2);
	AdjustMaxSpeed();
	if (num2 != 0)
	{
		nModelDir() = num2;
	}
	Dir2Rotate(nModelDir());
	return num2;
}

bool cTaskCharEnemyFireBee::ActionBoth()
{
	return cTaskCharEnemy::ActionBoth();
}

void cTaskCharEnemyFireBee::SetMMSParam(int para1, int para2, int para3)
{
	nFormPart() = para1;
	if (CustomKingdomUtil::IF(nFormPart()))
	{
		nFormGroup() = para2;
	}
}

}
