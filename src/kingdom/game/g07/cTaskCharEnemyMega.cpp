#include "kingdom/game/g07/cTaskCharEnemyMega.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyMega::cTaskCharEnemyMega()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyMega::Construct()
{
	return new cTaskCharEnemyMega();
}

// DOL 0x8005603c [audited: per AUDIT_G03_G07 row]
void cTaskCharEnemyMega::Init()
{
	OnFlag(2048u);
	nChangeMoveFrame() = 12;
}

void cTaskCharEnemyMega::Term()
{
	cTaskCharEnemy::Term();
}

// DOL 0x80056058 [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyMega::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_MEGA");
	string array[3] = {"TSA_CHAR_ENEMY_MEGA", "CLA_CHAR_ENEMY_MEGA", "CHA_CHAR_ENEMY_MEGA"};
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

// DOL 0x8005613c [audited: per AUDIT_G03_G07 row]
uint cTaskCharEnemyMega::Input()
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
	if (nHamariTimer() > (int)fExParam(0) /* DOL 0x8005613c: no / 10 (the port divided) */ && bTestFlag(2048u) && IsFall(vMTrans()) && !CheckFall(vMTrans()))
	{
		num2 = nHamariRecoverDir(vMTrans(), v);
	}
	else
	{
		if ((long)Constants::get_GAME_COUNTER() % (long)nChangeMoveFrame() == 0L)
		{
			if (nFallPassDir() != 0)
			{
				num2 = (ushort)nFallPassDir();
				nFallPassDir() = 0;
			}
			else
			{
				num2 = nTargetDir(fDx, fDz);
			}
		}
		ushort num3 = nFallPassDirTarget(num2, v, vSpeed(), fAccelSpeed() * num);
		if (num3 != num2)
		{
			num2 = num3;
			nFallPassDir() = (short)nTargetDir2(fDx, fDz);
			::Constants::jetGeoVecUnit(vSpeed());
		}
	}
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, Constants::motosDir2Vector(num2));
	Constants::VEC3_MUL(v2, v2, fAccelSpeed() * num);
	Constants::VEC3_ADD(vSpeed(), vSpeed(), v2);
	AdjustMaxSpeed();
	RollMatrix(pMatrix(), 0, "sphere36");
	return num2;
}

// DOL 0x800564a8 [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyMega::ActionBoth()
{
	if (IsFall())
	{
		ChangeFallMove();
	}
	if (!Constants::IS_MASTER())
	{
		vRotate()[1] -= Constants::TSP_DEG2RAD(3.0f);
	}
	Vector4 _out;
	Constants::VEC3_SUB(_out, vMTrans(), KingdomGlobal::g_pMotosGame->pPlayers(Constants::MY_ID())->vMTrans());
	float num = ::Constants::jetGeoVecLength(_out);
	if (num < KingdomGlobal::g_pMotosGame->fComingSELength())
	{
		KingdomGlobal::g_pMotosGame->fComingSELength() = num;
	}
	return true;
}

// DOL 0x800565e8 [audited: part = p1, group = p2 only when part != 0, vs disassembly]
void cTaskCharEnemyMega::SetMMSParam(int para1, int para2, int para3)
{
	nFormPart() = para1;
	if (CustomKingdomUtil::IF(nFormPart()))
	{
		nFormGroup() = para2;
	}
}

}
