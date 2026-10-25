#include "kingdom/game/g07/cTaskCharEnemyGiga.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyGiga::cTaskCharEnemyGiga()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyGiga::Construct()
{
	return new cTaskCharEnemyGiga();
}

void cTaskCharEnemyGiga::Init()
{
	nChangeMoveFrame() = 12;
	OnFlag(2048u);
	Constants::tspVuMtxUnit(m_mRoll);
	m_nRotDir = 0;
}

void cTaskCharEnemyGiga::Term()
{
	cTaskCharEnemy::Term();
}

bool cTaskCharEnemyGiga::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_GIGA");
	string array[3] = {"TSA_CHAR_ENEMY_GIGA", "CLA_CHAR_ENEMY_GIGA", "CHA_CHAR_ENEMY_GIGA"};
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

uint cTaskCharEnemyGiga::Input()
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
	if (nHamariTimer() > (int)fExParam(0) && bTestFlag(2048u) && IsFall(vMTrans()) && !CheckFall(vMTrans()))
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
		else
		{
			num2 = nInputDir();
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
	RollMatrix(pMatrix(), 0, "sphere37");
	m_nRotDir = num2;
	return num2;
}

bool cTaskCharEnemyGiga::ActionBoth()
{
	if (IsFall())
	{
		ChangeFallMove();
	}
	if (!Constants::IS_MASTER())
	{
		Vector4 _out;
		Constants::VEC3_SUB(_out, vMTrans(), vMTransOLD());
		if (CustomKingdomUtil::IF(Constants::tspVector3Length(_out)) && Constants::tspVector3Length(_out) > fMaxSpeed())
		{
			Constants::tspVector3Normalize(_out, _out);
			Constants::VEC3_MUL(_out, _out, fMaxSpeed());
		}
	}
	Vector4 _out2;
	Constants::VEC3_SUB(_out2, vMTrans(), KingdomGlobal::g_pMotosGame->pPlayers(Constants::MY_ID())->vMTrans());
	float num = ::Constants::jetGeoVecLength(_out2);
	if (num < KingdomGlobal::g_pMotosGame->fComingSELength())
	{
		KingdomGlobal::g_pMotosGame->fComingSELength() = num;
	}
	return true;
}

void cTaskCharEnemyGiga::SetMMSParam(int para1, int para2, int para3)
{
	nFormPart() = para1;
	if (CustomKingdomUtil::IF(nFormPart()))
	{
		nFormGroup() = para2;
	}
}

}
