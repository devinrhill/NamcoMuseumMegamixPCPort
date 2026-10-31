#include "kingdom/MslRand.hpp"
#include "kingdom/game/g07/cTaskCharEnemyBomb.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemyBomb::cTaskCharEnemyBomb()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemyBomb::Construct()
{
	return new cTaskCharEnemyBomb();
}

float& cTaskCharEnemyBomb::fShootRange()
{
	return m_fShootRange;
}

float& cTaskCharEnemyBomb::fNoHitRange()
{
	return m_fNoHitRange;
}

Vector4& cTaskCharEnemyBomb::vSpawnPos()
{
	return m_vSpawnPos;
}

bool cTaskCharEnemyBomb::IsFall(float x, float y, float z)
{
	return IsFall(Vector4(x, y, z, 0.0f));
}

// DOL 0x80050eac [audited: per AUDIT_G03_G07 row]
void cTaskCharEnemyBomb::Init()
{
	m_nRotDir = 0;
	m_fGensoku = 0.0f;
	m_fShootRange = 0.0f;
	m_fNoHitRange = 0.0f;
	Constants::VEC3_SET(m_vSpawnPos, 0.0f, 0.0f, 0.0f);
	nChangeMoveFrame() = 12;
	SetMode(1);
	Constants::tspVuMtxUnit(m_mRoll);
}

void cTaskCharEnemyBomb::Term()
{
	cTaskCharEnemy::Term();
}

// DOL 0x80050f2c [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyBomb::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_BOMB");
	string array[3] = {"TSA_CHAR_ENEMY_BOMB", "CLA_CHAR_ENEMY_BOMB", "CHA_CHAR_ENEMY_BOMB"};
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

// DOL (inlined in ModeFunc 0x8005155c mode 2)
uint cTaskCharEnemyBomb::InputLaunch()
{
	float num = 1.0f - HereticRate();
	ushort num2 = nInputDir();
	if (bTestFlag(64u))
	{
		return 0u;
	}
	float fDx = vFTarget()[0] - vMTrans()[0];
	float fDz = vFTarget()[2] - vMTrans()[2];
	nTargetDir(fDx, fDz);
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::motosDir2Vector(num2));
	Constants::VEC3_MUL(v, v, fAccelSpeed() * num);
	Constants::VEC3_ADD(vSpeed(), vSpeed(), v);
	AdjustMaxSpeed();
	m_nRotDir = num2;
	return num2;
}

bool cTaskCharEnemyBomb::RATE(int rand, int rate)
{
	if (rand >= rate)
	{
		return false;
	}
	return true;
}

// DOL 0x80051010 [audited: described by the 43rd-session pass] (audited 44th: matches; the rand() % 100 is drawn every frame, before the change-frame test)
uint cTaskCharEnemyBomb::Input()
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
	Vector4 v = Vector4();
	cTaskCharPlayer* cTaskCharPlayer2 = SearchNearPlayer();
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
	int rand = kingdom::MslRandRange(0, 100);
	if ((long)Constants::get_GAME_COUNTER() % (long)nChangeMoveFrame() == 0L)
	{
		num2 = nTargetDir(fDx, fDz);
		if (nInputDir() == num2)
		{
			if (cTaskCharEnemyBomb::RATE(rand, 15))
			{
				num2 = nReverseDir(num2);
			}
			else if (cTaskCharEnemyBomb::RATE(rand, 30))
			{
				num2 = 0;
			}
		}
		else if (cTaskCharEnemyBomb::RATE(rand, 25))
		{
			num2 = 0;
		}
	}
	ushort num3 = nFallPassDirRandom(num2, vSpeed(), fAccelSpeed() * num);
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
	m_nRotDir = num2;
	return num2;
}

// DOL 0x8005133c [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyBomb::ActionBoth()
{
	if (nLTimer() < 180)
	{
		(void)((0.5f + 0.036111113f * (float)(180 - nLTimer())) / 60.0f);
	}
	if (IsFall())
	{
		ChangeFallMove();
	}
	else if (--nLTimer() <= 0)
	{
		SetMode(5);
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
	return true;
}

// DOL 0x800514c4 [audited: ExecModeFunc != 0 vs decomp]
bool cTaskCharEnemyBomb::ActionLocal()
{
	return ExecModeFunc() != 0;
}

// DOL 0x800514f8 [audited: per AUDIT_G03_G07 row]
void cTaskCharEnemyBomb::ChangeTaiki()
{
	cTaskChar::ChangeTaiki();
	::Constants::jetGeoVecUnit(vSpeed());
	fMaxSpeed() = fDefMaxSpeed();
	fHitSize() = fDefHitSize();
	m_fGensoku = 0.0f;
	SetMode(1);
}

// DOL 0x8005155c [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyBomb::ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene)
{
	if (GetMode() != 0)
	{
		if (1 == GetMode())
		{
			if (2 == nMes)
			{
				fMaxSpeed() = Constants::tspVector3Length(vSpeed());
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				if (vMTrans()[2] < 50.0f && !IsFall())
				{
					m_fGensoku = 0.025f;
					SetMode(2);
				}
				OffFlag(131136u);
				return true;
			}
		}
		else if (2 == GetMode())
		{
			if (2 == nMes)
			{
				OffFlag(16384u);
				Constants::tspVuMtxUnit(m_mRoll);
				m_nRotDir = 0;
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				float num = Constants::tspVector3Length(vSpeed());
				if (num > fDefMaxSpeed())
				{
					fMaxSpeed() = num;
				}
				nInputDir() = (ushort)InputLaunch();
				Vector4 _out;
				Constants::VEC3_SET(_out, vFTarget()[0] - vMTrans()[0], 0.0f, vFTarget()[2] - vMTrans()[2]);
				float num2 = Constants::tspVector3Length(_out);
				Constants::VEC3_SET(_out, vSpawnPos()[0] - vMTrans()[0], 0.0f, vSpawnPos()[2] - vMTrans()[2]);
				float num3 = Constants::tspVector3Length(_out);
				if (fNoHitRange() <= num3)
				{
					OffFlag(32u);
				}
				Constants::VEC3_ADD(_out, vMTrans(), vSpeed());
				if (num2 < 0.5f || num2 < num || num3 >= fShootRange() || bTestFlag(1024u) || IsFall(vMTrans()))
				{
					fSlopePower() = fDefSlopePower();
					OffFlag(32u);
					SetMode(3);
				}
				else
				{
					num = Constants::tspVector3Length(vSpeed());
					if (CustomKingdomUtil::IF(num))
					{
						Constants::tspVector3Normalize(_out, vSpeed());
						if (num >= fDefMaxSpeed() * 1.3f + m_fGensoku)
						{
							Constants::VEC3_MUL(_out, _out, 0.0f - m_fGensoku);
							Constants::VEC3_ADD(vSpeed(), vSpeed(), _out);
						}
						else
						{
							Constants::VEC3_MUL(vSpeed(), _out, fDefMaxSpeed() * 1.3f);
						}
					}
				}
				OffFlag(131136u);
				return true;
			}
		}
		else if (3 == GetMode())
		{
			if (2 == nMes)
			{
				return true;
			}
			if (3 == nMes)
			{
				fMaxSpeed() = fDefMaxSpeed();
				return true;
			}
			if (1 == nMes)
			{
				float num4 = Constants::tspVector3Length(vSpeed());
				if (num4 > fDefMaxSpeed())
				{
					fMaxSpeed() = num4;
				}
				nInputDir() = (ushort)Input();
				num4 = Constants::tspVector3Length(vSpeed());
				if (CustomKingdomUtil::IF(num4))
				{
					Vector4 v;
					Constants::tspVector3Normalize(v, vSpeed());
					if (num4 >= fDefMaxSpeed() + m_fGensoku)
					{
						Constants::VEC3_MUL(v, v, 0.0f - m_fGensoku);
						Constants::VEC3_ADD(vSpeed(), vSpeed(), v);
					}
					else
					{
						Constants::VEC3_MUL(vSpeed(), v, fDefMaxSpeed());
					}
				}
				if (num4 <= fDefMaxSpeed() + m_fGensoku)
				{
					SetMode(4);
				}
				return true;
			}
		}
		else if (4 == GetMode())
		{
			if (2 == nMes)
			{
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				nInputDir() = (ushort)Input();
				return true;
			}
		}
		else if (5 == GetMode())
		{
			if (2 == nMes)
			{
				::Constants::jetGeoVecUnit(vSpeed());
				nLTimer() = 30;
				fHitSize() = 2.5f;
				OnFlag(1u);
				Constants::SpawnEffect(19, vMTrans(), nullptr, 0, true);   // DOL 0x80051cc4 [audited: described by the 43rd-session pass]: (0x13, pos, 0, 0, 1) - the C# drew a random 0..4 frame delay
				KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->StrikePanel((int)ID(), 1, 0.4f, vMTrans());
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				if (--nLTimer() <= 0)
				{
					fHitSize() = 0.0f;
					ChangeTaiki();
				}
				return true;
			}
		}
	}
	return true;
}

// DOL 0x80051d94 [audited: per AUDIT_G03_G07 row]
void cTaskCharEnemyBomb::RefAttacked(int nParts, int nGrobalID)
{
	cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)nGrobalID);
	if (cTaskChar2 == nullptr || GetMode() == 1)
	{
		return;
	}
	if (GetMode() == 2)
	{
		OffFlag(32u);
		SetMode(3);
	}
	else if (GetMode() != 5)
	{
		if (cTaskChar2->bTestFlag(4u) && cTaskChar2->bTestFlag(1u) && cTaskChar2->nKind() == 25)
		{
			SetMode(5);
		}
	}
	else if (!cTaskChar2->bTestFlag(4u) || cTaskChar2->nKind() != 25)
	{
		nLTimer() = 0;
	}
}

// DOL 0x80051e74 [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemyBomb::IsFall(Nullable<Vector4> pVec)
{
	if (GetMode() == 1 || GetMode() == 2)
	{
		return false;
	}
	return cTaskChar::IsFall(pVec);
}

}
