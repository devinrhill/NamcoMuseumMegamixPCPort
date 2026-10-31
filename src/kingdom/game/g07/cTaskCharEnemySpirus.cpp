#include "kingdom/game/g07/cTaskCharEnemySpirus.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemySpirus::cTaskCharEnemySpirus()
{
	Init();
}

cTaskCharEnemy* cTaskCharEnemySpirus::Construct()
{
	return new cTaskCharEnemySpirus();
}

// DOL 0x8005925c [audited: per AUDIT_G03_G07 row]
void cTaskCharEnemySpirus::Init()
{
	nChangeMoveFrame() = 16;
	m_bHit = false;
	ForceMode(1);
}

void cTaskCharEnemySpirus::Term()
{
	cTaskCharEnemy::Term();
}

// DOL 0x80059280 [audited: per AUDIT_G03_G07 row]
bool cTaskCharEnemySpirus::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_ENEMY_SPIRUS");
	string array[3] = {"TSA_CHAR_ENEMY_SPIRUS", "CLA_CHAR_ENEMY_SPIRUS", "CHA_CHAR_ENEMY_SPIRUS"};
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

// DOL 0x80059364 [audited: per AUDIT_G03_G07 row] (every branch compared 2026-10-30)
uint cTaskCharEnemySpirus::Input()
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
	float num3 = v[0] - vMTrans()[0];
	float num4 = v[2] - vMTrans()[2];
	float num5 = Constants::MOTOS_P_ABS(num3);
	float num6 = Constants::MOTOS_P_ABS(num4);
	if ((long)Constants::get_GAME_COUNTER() % (long)nChangeMoveFrame() == 0L)
	{
		num2 = 0;
		if (Constants::MOTOS_P_ABS(num5 - num6) < 0.5f && GetMode() == 1)
		{
			ForceMode(2);
			m_nExDir = 0;
			if (num3 < 0.0f)
			{
				m_nExDir |= 4;
			}
			if (num3 >= 0.0f)
			{
				m_nExDir |= 8;
			}
			if (num4 < 0.0f)
			{
				m_nExDir |= 1;
			}
			if (num4 >= 0.0f)
			{
				m_nExDir |= 2;
			}
			num2 = ((!(num5 > num6)) ? ((ushort)(m_nExDir & -13)) : ((ushort)(m_nExDir & -4)));
		}
		if (GetMode() == 2)
		{
			ushort num7 = (ushort)(m_nExDir & -4);
			ushort num8 = (ushort)(m_nExDir & -13);
			if (nInputDir() == num7)
			{
				num2 = num8;
			}
			else if (nInputDir() == num8)
			{
				num2 = num7;
			}
			else
			{
				ForceMode(1);
			}
			if ((num8 == 1 && num4 > 0.0f) || (num8 == 2 && num4 < 0.0f) || (num7 == 4 && num3 > 0.0f) || (num7 == 8 && num3 < 0.0f))
			{
				ForceMode(1);
			}
		}
		else if (num5 > num6)
		{
			if (num3 < 0.0f)
			{
				num2 |= 4;
			}
			if (num3 > 0.0f)
			{
				num2 |= 8;
			}
			if (num4 < 0.0f)
			{
				num2 |= 1;
			}
			if (num4 > 0.0f)
			{
				num2 |= 2;
			}
			if (num2 == nInputDir())
			{
				int num9 = -13;
				num2 &= (ushort)num9;
			}
		}
		else
		{
			if (num4 < 0.0f)
			{
				num2 |= 1;
			}
			if (num4 > 0.0f)
			{
				num2 |= 2;
			}
			if (num3 < 0.0f)
			{
				num2 |= 4;
			}
			if (num3 > 0.0f)
			{
				num2 |= 8;
			}
			if (num2 == nInputDir())
			{
				int num10 = -4;
				num2 &= (ushort)num10;
			}
		}
	}
	float num11 = fAccelSpeed() * num;
	Vector4 _out;
	Constants::VEC3_ADD(_out, vMTrans(), vSpeed());
	ushort num12 = 0;
	if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(_out[0], _out[1], _out[2] - num11))
	{
		num12 |= 1;
	}
	else if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(_out[0], _out[1], _out[2] + num11))
	{
		num12 |= 2;
	}
	if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(_out[0] - num11, _out[1], _out[2]))
	{
		num12 |= 4;
	}
	else if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(_out[0] + num11, _out[1], _out[2]))
	{
		num12 |= 8;
	}
	if (num12 != 0)
	{
		ushort dir = num2;
		num2 = 0;
		ushort num13 = 0;
		switch (num12)
		{
		case 1:
			num2 = 2;
			if (vMTrans()[0] > v[0])
			{
				num13 = (ushort)(4 & ~num12);
			}
			else if (vMTrans()[0] < v[0])
			{
				num13 = (ushort)(8 & ~num12);
			}
			break;
		case 2:
			num2 = 1;
			if (vMTrans()[0] > v[0])
			{
				num13 = (ushort)(4 & ~num12);
			}
			else if (vMTrans()[0] < v[0])
			{
				num13 = (ushort)(8 & ~num12);
			}
			break;
		case 4:
			num2 = 8;
			if (vMTrans()[2] > v[2])
			{
				num13 = (ushort)(1 & ~num12);
			}
			else if (vMTrans()[2] < v[2])
			{
				num13 = (ushort)(2 & ~num12);
			}
			break;
		case 8:
			num2 = 4;
			if (vMTrans()[2] > v[2])
			{
				num13 = (ushort)(1 & ~num12);
			}
			else if (vMTrans()[2] < v[2])
			{
				num13 = (ushort)(2 & ~num12);
			}
			break;
		default:
			num2 = nReverseDir(dir);
			break;
		}
		ForceMode(1);
		if (!m_bHit || nReverseDir((ushort)(num2 | num13)) != nInputDir())
		{
			num2 |= num13;
		}
		m_bHit = true;
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

// DOL 0x80059b04 [audited: part = p1, group = p2 only when part != 0, vs disassembly]
void cTaskCharEnemySpirus::SetMMSParam(int para1, int para2, int para3)
{
	nFormPart() = para1;
	if (CustomKingdomUtil::IF(nFormPart()))
	{
		nFormGroup() = para2;
	}
}

}
