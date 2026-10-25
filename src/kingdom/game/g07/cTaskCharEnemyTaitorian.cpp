#include "kingdom/game/g07/cTaskCharEnemyTaitorian.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyTaitorian::cTaskCharEnemyTaitorian()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyTaitorian::Construct()
{
	return new cTaskCharEnemyTaitorian();
}

void cTaskCharEnemyTaitorian::Init()
{
	nChangeMoveFrame() = 12;
}

void cTaskCharEnemyTaitorian::Term()
{
	cTaskCharEnemy::Term();
}

bool cTaskCharEnemyTaitorian::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_TAITORIAN");
	string array[3] = {"TSA_CHAR_ENEMY_TAITORIAN", "CLA_CHAR_ENEMY_TAITORIAN", "CHA_CHAR_ENEMY_TAITORIAN"};
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

uint cTaskCharEnemyTaitorian::Input()
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
	Vector4 v = Vector4::zero;
	cTaskCharPlayer* cTaskCharPlayer2 = SearchNearPlayer();
	if (cTaskCharPlayer2 != nullptr)
	{
		::Constants::jetGeoVecCopy(v, cTaskCharPlayer2->vMTrans());
	}
	else
	{
		::Constants::jetGeoVecUnit(v);
		v[0] = 8.0f;
		v[2] = 8.0f;
	}
	float fDx = v[0] - vMTrans()[0];
	float fDz = v[2] - vMTrans()[2];
	num2 = (((long)Constants::get_GAME_COUNTER() % (long)nChangeMoveFrame() != 0L) ? nInputDir() : nTargetDir(fDx, fDz));
	ushort num3 = nFallPassDirRegular(num2, vSpeed(), fAccelSpeed() * num);
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
	if (num2 != 0)
	{
		nModelDir() = num2;
	}
	Dir2Rotate(nModelDir());
	return num2;
}

void cTaskCharEnemyTaitorian::SetMMSParam(int para1, int para2, int para3)
{
	nFormPart() = para1;
	if (CustomKingdomUtil::IF(nFormPart()))
	{
		nFormGroup() = para2;
	}
}

}
