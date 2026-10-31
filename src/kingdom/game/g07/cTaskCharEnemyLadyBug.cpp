#include "kingdom/MslRand.hpp"
#include "kingdom/game/g07/cTaskCharEnemyLadyBug.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyLadyBug::cTaskCharEnemyLadyBug()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyLadyBug::Construct()
{
	return new cTaskCharEnemyLadyBug();
}

// DOL 0x80055a8c [audited: per AUDIT_G03_G07 row]
void cTaskCharEnemyLadyBug::Init()
{
	nChangeMoveFrame() = 20;
	m_bHit = false;
}

void cTaskCharEnemyLadyBug::Term()
{
	cTaskCharEnemy::Term();
}

// DOL 0x80055aa4 [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyLadyBug::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_LADY_BUG");
	string array[3] = {"TSA_CHAR_ENEMY_LADY_BUG", "CLA_CHAR_ENEMY_LADY_BUG", "CHA_CHAR_ENEMY_LADY_BUG"};
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

// DOL 0x80055b88 [audited: per AUDIT_G03_G07 row]
uint cTaskCharEnemyLadyBug::Input()
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
	if (bTestFlag(1024u))
	{
		ForceMode(2);
	}
	if ((long)Constants::get_GAME_COUNTER() % (long)nChangeMoveFrame() == 0L)
	{
		num2 = nTargetDir(fDx, fDz);
		if (GetMode() == 3)
		{
			ForceMode(1);
			nChangeMoveFrame() = 20;
		}
	}
	if (!bTestFlag(1024u) && GetMode() == 2)
	{
		if (kingdom::MslRandRange(0, 2) == 0)
		{
			ForceMode(3);
			switch (num2)
			{
			case 8:
				num2 = 1;
				break;
			case 4:
				num2 = 2;
				break;
			case 1:
				num2 = 8;
				break;
			case 2:
				num2 = 4;
				break;
			case 5:
				num2 = 9;
				break;
			case 9:
				num2 = 10;
				break;
			case 6:
				num2 = 5;
				break;
			case 10:
				num2 = 6;
				break;
			}
			nChangeMoveFrame() = 10;
		}
		else
		{
			ForceMode(1);
		}
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
	if (num2 != 0)
	{
		nModelDir() = num2;
	}
	Dir2Rotate(nModelDir());
	return num2;
}

// DOL 0x80055f14 [audited: part = p1, group = p2 only when part != 0, vs disassembly]
void cTaskCharEnemyLadyBug::SetMMSParam(int para1, int para2, int para3)
{
	nFormPart() = para1;
	if (CustomKingdomUtil::IF(nFormPart()))
	{
		nFormGroup() = para2;
	}
}

}
