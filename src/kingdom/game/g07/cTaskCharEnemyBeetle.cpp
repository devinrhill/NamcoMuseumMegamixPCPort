#include "kingdom/game/g07/cTaskCharEnemyBeetle.hpp"

#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyBeetle::cTaskCharEnemyBeetle()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyBeetle::Construct()
{
	return new cTaskCharEnemyBeetle();
}

// DOL 0x8005097c [audited: per AUDIT_G03_G07 row]
void cTaskCharEnemyBeetle::Init()
{
	nChangeMoveFrame() = 12;
}

void cTaskCharEnemyBeetle::Term()
{
	cTaskCharEnemy::Term();
}

// DOL 0x8005098c [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyBeetle::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_BEETLE");
	string array[3] = {"TSA_CHAR_ENEMY_BEETLE", "CLA_CHAR_ENEMY_BEETLE", "CHA_CHAR_ENEMY_BEETLE"};
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

// DOL 0x80050a70 [audited: per AUDIT_G03_G07 row]
uint cTaskCharEnemyBeetle::Input()
{
	float num = 1.0f - HereticRate();
	ushort num2 = nInputDir();
	if (bTestFlag(1088u))
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
		num2 = nTargetDir(fDx, fDz);
	}
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
	vRotate()[1] += Constants::TSP_DEG2RAD(12.0f);
	return num2;
}

// DOL 0x80050d00 [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyBeetle::ActionBoth()
{
	if (IsFall())
	{
		ChangeFallMove();
	}
	if (!Constants::IS_MASTER())
	{
		vRotate()[1] -= Constants::TSP_DEG2RAD(12.0f);
	}
	return true;
}

// DOL 0x80050d84 [audited: per AUDIT_G03_G07 row]
void cTaskCharEnemyBeetle::SetMMSParam(int para1, int para2, int para3)
{
	nFormPart() = para1;
	if (nFormPart() != 0)
	{
		nFormGroup() = para2;
	}
}

}
