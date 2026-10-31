#include "kingdom/game/g07/cTaskCharEnemyPeta.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include <numbers>

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyPeta::cTaskCharEnemyPeta()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyPeta::Construct()
{
	return new cTaskCharEnemyPeta();
}

bool& cTaskCharEnemyPeta::bStop()
{
	return m_bStop;
}

// DOL 0x80057944 [audited: per AUDIT_G03_G07 row]
void cTaskCharEnemyPeta::Init()
{
	OnFlag(2048u);
	nHitFlag() = 5u;
	nChangeMoveFrame() = 24;
	Constants::tspVuMtxUnit(m_mRoll);
	m_nRotDir = 0;
}

void cTaskCharEnemyPeta::Term()
{
	cTaskCharEnemy::Term();
}

// DOL 0x800579b8 [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyPeta::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	CustomKingdomUtil::Assert(no == 24 || no == 23, "error!");
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_PETA");
	string array[3] = {"TSA_CHAR_ENEMY_PETA", "CLA_CHAR_ENEMY_PETA", "CHA_CHAR_ENEMY_PETA"};
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

// DOL 0x80057a9c [audited: per AUDIT_G03_G07 row]
uint cTaskCharEnemyPeta::Input()
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
	bStop() = false;
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
	RollMatrix(pMatrix(), 0, "sphere33");
	m_nRotDir = num2;
	return num2;
}

// DOL 0x80057e18 [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyPeta::FallMove()
{
	::Constants::jetGeoVecCopy(vMTransOLD(), vMTrans());
	switch (Step())
	{
	case 0:
		vSpeed()[0] = 0.0f;
		vSpeed()[2] = 0.0f;
		Step()++;
		Step(1) = 0;
		vSpeed()[1] = 0.0f;
		break;
	case 1:
		{
			Vector4 v;
			::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->pGravVector());
			Constants::VEC3_MUL(v, v, 0.01f);
			Constants::VEC3_ADD(vSpeed(), vSpeed(), v);
			if (vRotate()[1] > (float)std::numbers::pi)
			{
				vRotate()[1] += 0.1f;
			}
			else
			{
				vRotate()[1] -= 0.1f;
			}
			break;
		}
	}
	Constants::VEC3_ADD(vMTrans(), vMTrans(), vSpeed());
	if (Constants::IS_MYCHAR(this) && vMTrans()[1] < -40.0f)
	{
		vMTrans()[2] = 50.0f;
	}
	return false;
}

// DOL 0x8005800c [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyPeta::ActionBoth()
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
	float num = Constants::tspVector3Length(_out2);
	if (num < KingdomGlobal::g_pMotosGame->fComingSELength())
	{
		KingdomGlobal::g_pMotosGame->fComingSELength() = num;
	}
	return true;
}

// DOL 0x800581f8 [audited: part = p1, group = p2 only when part != 0, vs disassembly]
void cTaskCharEnemyPeta::SetMMSParam(int para1, int para2, int para3)
{
	nFormPart() = para1;
	if (CustomKingdomUtil::IF(nFormPart()))
	{
		nFormGroup() = para2;
	}
}

}
