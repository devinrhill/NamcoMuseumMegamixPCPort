#include "kingdom/game/g07/cTaskCharEnemyFireBug.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyFireBug::cTaskCharEnemyFireBug()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyFireBug::Construct()
{
	return new cTaskCharEnemyFireBug();
}

// DOL 0x80054d40 [audited: per AUDIT_G03_G07 row]
void cTaskCharEnemyFireBug::Init()
{
	nChangeMoveFrame() = 30;
	ForceMode(1);
}

void cTaskCharEnemyFireBug::Term()
{
	cTaskCharEnemy::Term();
}

// DOL 0x80054d5c [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyFireBug::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_FIRE_BUG");
	string array[3] = {"TSA_CHAR_ENEMY_FIRE_BUG", "CLA_CHAR_ENEMY_FIRE_BUG", "CHA_CHAR_ENEMY_FIRE_BUG"};
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

// DOL 0x80054e40 [audited: per AUDIT_G03_G07 row]
uint cTaskCharEnemyFireBug::Input()
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
	float num3 = v[0] - vMTrans()[0];
	float num4 = v[2] - vMTrans()[2];
	Vector2 v2 = Vector2(num3, num4);
	if (GetMode() != 2 && Constants::tspVector2Length(v2) < 1.5f)
	{
		switch (nInputDir())
		{
		case 1:
			num2 = 4;
			break;
		case 2:
			num2 = 8;
			break;
		case 4:
			num2 = 2;
			break;
		case 8:
			num2 = 1;
			break;
		case 5:
			num2 = 6;
			break;
		case 9:
			num2 = 5;
			break;
		case 6:
			num2 = 10;
			break;
		case 10:
			num2 = 9;
			break;
		}
		ForceMode(2);
	}
	else if ((long)Constants::get_GAME_COUNTER() % (long)nChangeMoveFrame() == 0L)
	{
		ForceMode(1);
		num2 = nTargetDir(num3, num4);
	}
	ushort num5 = nFallPassDirRegular(num2, vSpeed(), fAccelSpeed() * num);
	if (num5 != num2)
	{
		num2 = num5;
		::Constants::jetGeoVecUnit(vSpeed());
	}
	Vector4 v3;
	::Constants::jetGeoVecCopy(v3, Constants::motosDir2Vector(num2));
	Constants::VEC3_MUL(v3, v3, fAccelSpeed() * num);
	Constants::VEC3_ADD(vSpeed(), vSpeed(), v3);
	AdjustMaxSpeed();
	if (num2 != 0)
	{
		nModelDir() = num2;
	}
	Dir2Rotate(nModelDir());
	return num2;
}

bool cTaskCharEnemyFireBug::ActionBoth()
{
	return cTaskCharEnemy::ActionBoth();
}

// DOL 0x8005518c [audited: part = p1, group = p2 only when part != 0, vs disassembly]
void cTaskCharEnemyFireBug::SetMMSParam(int para1, int para2, int para3)
{
	nFormPart() = para1;
	if (CustomKingdomUtil::IF(nFormPart()))
	{
		nFormGroup() = para2;
	}
}

}
