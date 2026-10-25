#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include <cmath>
#include <iterator>
#include <numbers>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharEnemy::cTaskCharEnemy()
{
	Init();
}

cTaskCharEnemy::cTaskCharEnemy(const string& key, uint option)
	: cTaskChar(key, option)
{
	Init();
}

cTaskCharEnemy::cTaskCharEnemy(Object* data, uint option)
	: cTaskChar(data, option)
{
	Init();
}

bool cTaskCharEnemy::Create(uint no)
{
	return false;
}

short& cTaskCharEnemy::nFallPassDir()
{
	return m_nFallPassDir;
}

int& cTaskCharEnemy::nLTimer()
{
	return m_nLTimer;
}

int& cTaskCharEnemy::nHamariTimer()
{
	return m_nHamariTimer;
}

int& cTaskCharEnemy::nNoInputTimer()
{
	return m_nNoInputTimer;
}

int& cTaskCharEnemy::nRebirthTimer()
{
	return m_nRebirthTimer;
}

int& cTaskCharEnemy::nChangeMoveFrame()
{
	return m_nChangeMoveFrame;
}

Constants::FORMATION_INFO*& cTaskCharEnemy::pFormInfo()
{
	return m_FormInfo;
}

int& cTaskCharEnemy::nFormPart()
{
	return m_nFormPart;
}

int& cTaskCharEnemy::nFormGroup()
{
	return m_nFormGroup;
}

int& cTaskCharEnemy::nFormNum()
{
	return m_nFormNum;
}

Vector4& cTaskCharEnemy::vFTarget()
{
	return m_vFTarget;
}

std::vector<Constants::FORM_GROUP_INFO*>& cTaskCharEnemy::pFormGroupInfo()
{
	return m_pFormGroupInfo;
}

Constants::FORM_GROUP_INFO*& cTaskCharEnemy::pFormNow()
{
	return m_pFormNow;
}

void cTaskCharEnemy::Init()
{
	KingdomGlobal::g_pMotosGame->RegistEnemys(this);
	OnFlag(4u);
	m_nChangeMoveFrame = 0;
	m_nNoInputTimer = 0;
	m_nRebirthTimer = 0;
	m_nLTimer = 0;
	m_nFallPassDir = 0;
	m_vFTarget = Vector4();
	m_FormInfo = new Constants::FORMATION_INFO();
	m_nFormPart = 0;
	m_pFormGroupInfo.clear();
	m_pFormNow = nullptr;
	m_nFormNum = 0;
	m_nHamariTimer = 0;
}

void cTaskCharEnemy::Term()
{
	m_pFormGroupInfo.clear();
	cTaskChar::Term();
}

bool cTaskCharEnemy::ActionBoth()
{
	if (IsFall())
	{
		ChangeFallMove();
	}
	if (!Constants::IS_MASTER() && nKind() != 14)
	{
		Vector4 _out;
		Constants::VEC3_SUB(_out, vMTrans(), vMTransOLD());
		nModelDir() = (nInputDir() = (ushort)Constants::motosVector2Dir(_out));
		Dir2Rotate(nModelDir());
	}
	return true;
}

bool cTaskCharEnemy::ActionLocal()
{
	if (nFormPart() != 0)
	{
		return ExecModeFunc() != 0;
	}
	nInputDir() = (ushort)Input();
	return true;
}

bool cTaskCharEnemy::ActionOther()
{
	vMTransOLD() = vMTrans();
	return true;
}

bool cTaskCharEnemy::ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene)
{
	if (nFormPart() == 1)
	{
		return Formation_Parent(nMes, pListC, pScene);
	}
	if (nFormPart() == 2)
	{
		return Formation_Child(nMes, pListC, pScene);
	}
	nFormPart() = 0;
	return true;
}

void cTaskCharEnemy::PresetEnv()
{
}

void cTaskCharEnemy::RegistPacket()
{
}

void cTaskCharEnemy::SyncPacket()
{
}

cTaskCharEnemy* cTaskCharEnemy::Spawn(int kind, Vector4 pPos, float rad)
{
	return nullptr;
}

int cTaskCharEnemy::Mms2Kind(int mmsKind)
{
	return 0;
}

int cTaskCharEnemy::Kind2ChipColor(int nKind)
{
	int obj[27] = {7, 7, 5, 8, 8, 8, 8, 8, 8, 8, 8, 8, 5, 7, 8, 8, 8, 8, 8, 8, 5, 8, 5, 7, 5, 8, 8};
	int num = (int)std::size(obj);
	CustomKingdomUtil::Assert(nKind <= num, "error");
	return obj[nKind];
}

bool cTaskCharEnemy::IsDistFallDir(ushort nDir, Vector4 pvSp, float fAccel)
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::motosDir2Vector(nDir));
	Constants::VEC3_MUL(v, v, fAccel);
	Constants::VEC3_ADD(v, v, pvSp);
	AdjustMaxSpeed(v);
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, v);
	Constants::VEC3_ADD(v, vMTrans(), v);
	if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(v) || CustomKingdomUtil::IF(KingdomGlobal::g_pMotosGame->pMap()->HitCheck(vMTrans(), v2, fHitSize())) || Constants::SEARCH_GIMMICK(1, v[0], v[2]))
	{
		return true;
	}
	return false;
}

ushort cTaskCharEnemy::nSearchFallDir(Vector4 pvSpeed, float fAccel)
{
	ushort num = 0;
	if (IsDistFallDir(1, pvSpeed, fAccel))
	{
		num |= 1;
	}
	if (IsDistFallDir(2, pvSpeed, fAccel))
	{
		num |= 2;
	}
	if (IsDistFallDir(4, pvSpeed, fAccel))
	{
		num |= 4;
	}
	if (IsDistFallDir(8, pvSpeed, fAccel))
	{
		num |= 8;
	}
	return num;
}

bool cTaskCharEnemy::IsDistFallVec(Vector4 pvSp)
{
	Vector4 _out;
	Constants::VEC3_ADD(_out, vMTrans(), pvSp);
	if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(_out) || CustomKingdomUtil::IF(KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->HitCheck(vMTrans(), pvSp, fHitSize())) || Constants::SEARCH_GIMMICK(1, _out[0], _out[2]))
	{
		return true;
	}
	return false;
}

bool cTaskCharEnemy::IsDistBar(ushort nDir, Vector4 pvSp, float fAccel)
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::motosDir2Vector(nDir));
	Constants::VEC3_MUL(v, v, fAccel);
	Constants::VEC3_ADD(v, v, pvSp);
	AdjustMaxSpeed(v);
	cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pParent;
	if (cTaskCharEnemy2 != nullptr)
	{
		while (cTaskCharEnemy2 != nullptr)
		{
			if (cTaskCharEnemy2 != this && KingdomGlobal::g_pMotosGame->pHitMan()->HitCheckSimple(this, cTaskCharEnemy2, v, cTaskCharEnemy2->vSpeed()))
			{
				return true;
			}
			cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild;
		}
	}
	return false;
}

cTaskCharEnemy* cTaskCharEnemy::pDistBarChar(ushort nDir, Vector4 pvSp, float fAccel)
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::motosDir2Vector(nDir));
	Constants::VEC3_MUL(v, v, fAccel);
	Constants::VEC3_ADD(v, v, pvSp);
	AdjustMaxSpeed(v);
	cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pParent;
	if (cTaskCharEnemy2 != nullptr)
	{
		while (cTaskCharEnemy2 != nullptr)
		{
			if (cTaskCharEnemy2 != this && KingdomGlobal::g_pMotosGame->pHitMan()->HitCheckSimple(this, cTaskCharEnemy2, vSpeed(), cTaskCharEnemy2->vSpeed()) && (pFormInfo()->m_ID > cTaskCharEnemy2->pFormInfo()->m_ID || !cTaskCharEnemy2->bTestFlag(212992u)))
			{
				return cTaskCharEnemy2;
			}
			cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild;
		}
	}
	return nullptr;
}

ushort cTaskCharEnemy::nReverseDir(ushort dir)
{
	ushort result = 0;
	switch (dir)
	{
	case 8:
		result = 4;
		break;
	case 4:
		result = 8;
		break;
	case 1:
		result = 2;
		break;
	case 2:
		result = 1;
		break;
	case 5:
		result = 10;
		break;
	case 9:
		result = 6;
		break;
	case 6:
		result = 9;
		break;
	case 10:
		result = 5;
		break;
	}
	return result;
}

ushort cTaskCharEnemy::nSlantDir(ushort dir)
{
	ushort array[8] = {1, 9, 8, 10, 2, 6, 4, 5};
	int idx;
	switch (dir)
	{
	case 1: idx = 0; break;
	case 9: idx = 1; break;
	case 8: idx = 2; break;
	case 10: idx = 3; break;
	case 2: idx = 4; break;
	case 6: idx = 5; break;
	case 4: idx = 6; break;
	case 5: idx = 7; break;
	default: idx = Random::Range(0, 8); break;
	}
	return array[(idx + Random::Range(0, 4) * 2 + 1) % 8];
}

ushort cTaskCharEnemy::nRandomDir()
{
	switch (Random::Range(0, 8))     // evaluated once (the C# switch expression)
	{
	case 0: return 4;
	case 1: return 8;
	case 2: return 2;
	case 3: return 1;
	case 4: return 10;
	case 5: return 6;
	case 6: return 9;
	case 7: return 5;
	default: return 0;
	}
}

ushort cTaskCharEnemy::nTargetDir(float fDx, float fDz, float fDistAngle)
{
	ushort result = 0;
	float num = Constants::MOTOS_P_ABS(fDx);
	float num2 = Constants::MOTOS_P_ABS(fDz);
	if (!(Constants::MOTOS_P_ABS(num - num2) < fDistAngle))
	{
		result = (ushort)((num > num2) ? ((!(fDx < 0.0f)) ? 8 : 4) : ((fDz < 0.0f) ? 1 : 2));
	}
	else if (fDx < 0.0f && fDz < 0.0f)
	{
		result = 5;
	}
	else if (fDx < 0.0f && fDz > 0.0f)
	{
		result = 6;
	}
	else if (fDx > 0.0f && fDz < 0.0f)
	{
		result = 9;
	}
	else if (fDx > 0.0f && fDz > 0.0f)
	{
		result = 10;
	}
	return result;
}

ushort cTaskCharEnemy::nTargetDir2(float fDx, float fDz, float fDistAngle)
{
	ushort num = 0;
	float num2 = Constants::MOTOS_P_ABS(fDx);
	float num3 = Constants::MOTOS_P_ABS(fDz);
	float fAccel = fAccelSpeed() * (1.0f - HereticRate());
	if (Constants::MOTOS_P_ABS(num2 - num3) < fDistAngle)
	{
		num = (ushort)((fDx < 0.0f && fDz < 0.0f) ? 5 : ((fDx < 0.0f && fDz > 0.0f) ? 6 : ((!(fDx > 0.0f) || !(fDz < 0.0f)) ? 10 : 9)));
		int num4 = num;
		Vector4 v = Vector4::zero;
		::Constants::jetGeoVecUnit(v);
		if (IsDistFallDir((ushort)(num & 3), v, fAccel))
		{
			num4 &= -4;
		}
		::Constants::jetGeoVecUnit(v);
		if (IsDistFallDir((ushort)(num & 0xC), v, fAccel))
		{
			num4 &= -13;
		}
		if (num4 == num)
		{
			num4 = ((num2 > num3) ? ((!(fDx < 0.0f)) ? 8 : 4) : ((fDz < 0.0f) ? 1 : 2));
		}
		if (num4 != 0)
		{
			num = (ushort)num4;
		}
	}
	else if (num2 > num3)
	{
		num = (ushort)((!(fDx < 0.0f)) ? 8 : 4);
		if (IsDistFallDir(num, vSpeed(), fAccel))
		{
			num = (ushort)((fDz < 0.0f) ? 1 : 2);
		}
	}
	else
	{
		num = (ushort)((fDz < 0.0f) ? 1 : 2);
		if (IsDistFallDir(num, vSpeed(), fAccel))
		{
			num = (ushort)((!(fDx < 0.0f)) ? 8 : 4);
		}
	}
	return num;
}

ushort cTaskCharEnemy::nTargetDirMeet(float fDx, float fDz, float fDistAngle)
{
	ushort num = 0;
	float num2 = Constants::MOTOS_P_ABS(fDx);
	float num3 = Constants::MOTOS_P_ABS(fDz);
	float fAccel = fAccelSpeed() * (1.0f - HereticRate());
	if (num2 > num3)
	{
		num = (ushort)((!(fDx < 0.0f)) ? 8 : 4);
		if (num == nReverseDir(nInputDir()))
		{
			num = ((!(fDz < 0.0f)) ? ((ushort)(num | 1)) : ((ushort)(num | 2)));
		}
		cTaskCharEnemy* cTaskCharEnemy2 = pDistBarChar(num, vSpeed(), fAccel);
		if (cTaskCharEnemy2 != nullptr)
		{
			short num4 = (short)num;
			Vector4 _out;
			Constants::VEC3_SUB(_out, cTaskCharEnemy2->vMTrans(), vMTrans());
			Vector4 _out2;
			Constants::VEC3_SUB(_out2, vFTarget(), vMTrans());
			if (CustomKingdomUtil::IF(Constants::tspVector3Length(_out)))
			{
				Constants::tspVector3Normalize(_out, _out);
			}
			else
			{
				::Constants::jetGeoVecUnit(_out);
			}
			if (CustomKingdomUtil::IF(Constants::tspVector3Length(_out2)))
			{
				Constants::tspVector3Normalize(_out2, _out2);
			}
			else
			{
				::Constants::jetGeoVecUnit(_out2);
			}
			num = (ushort)((_out2[2] - _out[2] < 0.0f) ? 1 : 2);
			cTaskCharEnemy* cTaskCharEnemy3 = pDistBarChar(num, vSpeed(), fAccel);
			if (cTaskCharEnemy3 != nullptr && cTaskCharEnemy3 == cTaskCharEnemy2)
			{
				num |= nReverseDir((ushort)num4);
			}
		}
	}
	else
	{
		num = (ushort)((fDz < 0.0f) ? 1 : 2);
		if (num == nReverseDir(nInputDir()))
		{
			num = ((!(fDx < 0.0f)) ? ((ushort)(num | 4)) : ((ushort)(num | 8)));
		}
		cTaskCharEnemy* cTaskCharEnemy4 = pDistBarChar(num, vSpeed(), fAccel);
		if (cTaskCharEnemy4 != nullptr)
		{
			short num5 = (short)num;
			Vector4 _out3;
			Constants::VEC3_SUB(_out3, cTaskCharEnemy4->vMTrans(), vMTrans());
			Vector4 _out4;
			Constants::VEC3_SUB(_out4, vFTarget(), vMTrans());
			if (CustomKingdomUtil::IF(Constants::tspVector3Length(_out3)))
			{
				Constants::tspVector3Normalize(_out3, _out3);
			}
			else
			{
				::Constants::jetGeoVecUnit(_out3);
			}
			if (CustomKingdomUtil::IF(Constants::tspVector3Length(_out4)))
			{
				Constants::tspVector3Normalize(_out4, _out4);
			}
			else
			{
				::Constants::jetGeoVecUnit(_out4);
			}
			num = (ushort)((!(_out4[0] - _out3[0] < 0.0f)) ? 8 : 4);
			cTaskCharEnemy* cTaskCharEnemy5 = pDistBarChar(num, vSpeed(), fAccel);
			if (cTaskCharEnemy5 != nullptr && cTaskCharEnemy5 == cTaskCharEnemy4)
			{
				num |= nReverseDir((ushort)num5);
			}
		}
	}
	return num;
}

ushort cTaskCharEnemy::nFallPassDirReverse(ushort nDir, Vector4 pvSp, float fAccel)
{
	ushort result = nDir;
	if (IsDistFallDir(nDir, pvSp, fAccel))
	{
		int num = 0;
		Vector4 v = Vector4::zero;
		::Constants::jetGeoVecUnit(v);
		ushort num2 = nSearchFallDir(v, fAccel);
		num2 |= nDir;
		num = nReverseDir(nDir);
		num &= ~num2;
		if (num == 0)
		{
			num = (ushort)(0xF & ~num2);
		}
		result = (ushort)num;
	}
	return result;
}

ushort cTaskCharEnemy::nFallPassDirRegular(ushort nDir, Vector4 pvSp, float fAccel)
{
	ushort result = nDir;
	if (IsDistFallDir(nDir, pvSp, fAccel))
	{
		Vector4 v = Vector4::zero;
		::Constants::jetGeoVecUnit(v);
		ushort num = nSearchFallDir(v, fAccel);
		num |= nDir;
		int num2 = 0;
		switch (nDir)
		{
		case 8:
			num2 = 6;
			break;
		case 4:
			num2 = 9;
			break;
		case 1:
			num2 = 10;
			break;
		case 2:
			num2 = 5;
			break;
		case 5:
			num2 = 8;
			break;
		case 9:
			num2 = 2;
			break;
		case 6:
			num2 = 1;
			break;
		case 10:
			num2 = 4;
			break;
		}
		num2 &= ~num;
		if (num2 == 0)
		{
			num2 = (ushort)(0xF & ~num);
		}
		result = (ushort)num2;
	}
	return result;
}

ushort cTaskCharEnemy::nFallPassDirRandom(ushort nDir, Vector4 pvSp, float fAccel)
{
	ushort result = nDir;
	if (IsDistFallDir(nDir, pvSp, fAccel))
	{
		Vector4 v = Vector4::zero;
		::Constants::jetGeoVecUnit(v);
		ushort num = nSearchFallDir(v, fAccel);
		num |= nDir;
		int num2 = nRandomDir();
		num2 &= ~num;
		if (num2 == 0)
		{
			num2 = (ushort)(0xF & ~num);
		}
		result = (ushort)num2;
	}
	return result;
}

ushort cTaskCharEnemy::nFallPassDirTarget(ushort nDir, Vector4 pvTarget, Vector4 pvSp, float fAccel)
{
	ushort result = nDir;
	if (IsDistFallDir(nDir, pvSp, fAccel))
	{
		Vector4 _discard3;
		::Constants::jetGeoVecCopy(_discard3, vMTrans());
		Vector4 v2 = Vector4::zero;
		::Constants::jetGeoVecUnit(v2);
		ushort num = nSearchFallDir(v2, fAccel);
		num |= nDir;
		int num2 = 0;
		if (vMTrans()[0] > pvTarget[0])
		{
			num2 |= 4;
		}
		else if (vMTrans()[0] < pvTarget[0])
		{
			num2 |= 8;
		}
		if (vMTrans()[2] > pvTarget[2])
		{
			num2 |= 1;
		}
		else if (vMTrans()[2] < pvTarget[2])
		{
			num2 |= 2;
		}
		ushort num3 = (ushort)(num2 & 3);
		ushort num4 = (ushort)(num2 & 0xC);
		num2 = 0;
		switch (nDir)
		{
		case 1:
			num2 |= 2 | num4;
			break;
		case 2:
			num2 |= 1 | num4;
			break;
		case 4:
			num2 |= 8 | num3;
			break;
		case 8:
			num2 |= 4 | num3;
			break;
		default:
			if (nDir == (num & nDir))
			{
				num2 = nReverseDir(nDir);
				break;
			}
			if (CustomKingdomUtil::IF(nDir & 1))
			{
				if (CustomKingdomUtil::IF(1 & num))
				{
					num2 |= 2;
				}
			}
			else if (CustomKingdomUtil::IF(nDir & 2) && CustomKingdomUtil::IF(2 & num))
			{
				num2 |= 1;
			}
			if (CustomKingdomUtil::IF(nDir & 4))
			{
				if (CustomKingdomUtil::IF(4 & num))
				{
					num2 |= 8;
				}
			}
			else if (CustomKingdomUtil::IF(nDir & 8) && CustomKingdomUtil::IF(8 & num))
			{
				num2 |= 4;
			}
			break;
		}
		num2 &= ~num;
		if (num2 == 0)
		{
			num2 = (ushort)(0xF & ~num);
		}
		result = (ushort)num2;
	}
	return result;
}

int cTaskCharEnemy::FormNormal_Parent()
{
	nInputDir() = (ushort)Input();
	if (--nLTimer() < 0)
	{
		return 3;
	}
	return 0;
}

int cTaskCharEnemy::FormNormal_Child()
{
	nInputDir() = (ushort)Input();
	return 0;
}

int cTaskCharEnemy::FormMeetSnake_Parent()
{
	if (bTestFlag(2097152u))
	{
		nInputDir() = 0;
	}
	else
	{
		bool flag = true;
		int num = 0;
		cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pChild;
		Vector4 v = Vector4();
		Constants::MOTOS_VEC4_SET(v, 8.0f, 0.0f, 8.0f);
		if (KingdomGlobal::g_pMotosGame->pPlayers(0u) != nullptr && !KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16448u))
		{
			::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans());
		}
		v[0] = vMTrans()[0] - v[0];
		v[1] = 0.0f;
		v[2] = vMTrans()[2] - v[2];
		if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(v)))
		{
			Constants::Motos_P_vecNormalize(v, v);
		}
		else
		{
			Constants::MOTOS_VEC4_SET(v, 0.0f, 0.0f, 1.0f);
		}
		while (cTaskCharEnemy2 != nullptr)
		{
			if (!cTaskCharEnemy2->bTestFlag(16448u))
			{
				Vector4 v2;
				Constants::tspVector3Scale(v2, v, 1.1f * (float)cTaskCharEnemy2->pFormInfo()->m_ID);
				Constants::VEC3_ADD(cTaskCharEnemy2->vFTarget(), vMTrans(), v2);
				if (cTaskCharEnemy2->bTestFlag(65536u))
				{
					flag = false;
				}
				num++;
			}
			cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild;
		}
		nInputDir() = 0;
		::Constants::jetGeoVecUnit(vSpeed());
		if (num == 0)
		{
			return 1;
		}
		if (flag)
		{
			return 2;
		}
	}
	if (--nLTimer() < 0)
	{
		return 2;
	}
	return 0;
}

int cTaskCharEnemy::FormMoveSnake_Parent()
{
	if (bTestFlag(2097152u))
	{
		nInputDir() = 0;
	}
	else
	{
		int num = 0;
		nInputDir() = (ushort)Input();
		Vector4 _out;
		Constants::VEC3_ADD(_out, vMTrans(), vSpeed());
		cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pChild;
		Vector4 v = Vector4();
		Constants::MOTOS_VEC4_SET(v, 8.0f, 0.0f, 8.0f);
		if (KingdomGlobal::g_pMotosGame->pPlayers(0u) != nullptr && !KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16448u))
		{
			::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans());
		}
		v[0] = _out[0] - v[0];
		v[1] = 0.0f;
		v[2] = _out[2] - v[2];
		Constants::Motos_P_vecNormalize(v, v);
		while (cTaskCharEnemy2 != nullptr)
		{
			if (!cTaskCharEnemy2->bTestFlag(16448u))
			{
				Vector4 v2;
				Constants::tspVector3Scale(v2, v, 1.1f * (float)cTaskCharEnemy2->pFormInfo()->m_ID);
				Constants::VEC3_ADD(cTaskCharEnemy2->vFTarget(), _out, v2);
				num++;
			}
			cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild;
		}
		if (num == 0)
		{
			return 1;
		}
	}
	if (--nLTimer() < 0)
	{
		return 3;
	}
	return 0;
}

int cTaskCharEnemy::FormMeetCircle_Parent()
{
	if (bTestFlag(2097152u))
	{
		nInputDir() = 0;
	}
	else
	{
		bool flag = true;
		int num = 0;
		Vector4 v;
		::Constants::jetGeoVecCopy(v, vMTrans());
		float num2 = Constants::TSP_DEG2RAD(pFormInfo()->m_nDeg);
		for (cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pChild; cTaskCharEnemy2 != nullptr; cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild)
		{
			if (!cTaskCharEnemy2->bTestFlag(16448u))
			{
				Vector4 v2 = Vector4();
				Constants::MOTOS_VEC4_SET(v2, 1.3f);
				int num3 = ((cTaskCharEnemy2->pFormInfo()->m_ID - 1 >= 0) ? (cTaskCharEnemy2->pFormInfo()->m_ID - 1) : 0);
				float ry = (float)std::numbers::pi * 2.0f / (float)pFormInfo()->m_nChildNum * (float)num3 + num2;
				Constants::tspVector3RotY(v2, v2, ry);
				Constants::VEC3_ADD(cTaskCharEnemy2->vFTarget(), v, v2);
				if (cTaskCharEnemy2->bTestFlag(65536u))
				{
					flag = false;
				}
				num++;
			}
		}
		nInputDir() = 0;
		::Constants::jetGeoVecUnit(vSpeed());
		if (num == 0)
		{
			return 1;
		}
		if (flag)
		{
			return 2;
		}
	}
	if (--nLTimer() < 0)
	{
		return 2;
	}
	return 0;
}

int cTaskCharEnemy::FormMoveCircle_Parent()
{
	if (bTestFlag(2097152u))
	{
		nInputDir() = 0;
	}
	else
	{
		int num = 0;
		nInputDir() = (ushort)Input();
		Vector4 v;
		::Constants::jetGeoVecCopy(v, vMTrans());
		pFormInfo()->m_nDeg += 3;
		pFormInfo()->m_nDeg %= 360;
		float num2 = Constants::TSP_DEG2RAD(pFormInfo()->m_nDeg);
		for (cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pChild; cTaskCharEnemy2 != nullptr; cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild)
		{
			if (!cTaskCharEnemy2->bTestFlag(16448u))
			{
				int num3 = ((cTaskCharEnemy2->pFormInfo()->m_ID - 1 >= 0) ? (cTaskCharEnemy2->pFormInfo()->m_ID - 1) : 0);
				float ry = (float)std::numbers::pi * 2.0f / (float)pFormInfo()->m_nChildNum * (float)num3 + num2;
				Vector4 v2 = Vector4();
				Constants::MOTOS_VEC4_SET(v2, 1.3f);
				Vector4 v3;
				Constants::tspVector3RotY(v3, v2, ry);
				Constants::VEC3_ADD(cTaskCharEnemy2->vFTarget(), v, v3);
				num++;
			}
		}
		if (num == 0)
		{
			return 1;
		}
	}
	if (--nLTimer() < 0)
	{
		return 3;
	}
	return 0;
}

int cTaskCharEnemy::FormMeetArrow_Parent()
{
	bool flag = true;
	int num = 0;
	if (bTestFlag(2097152u))
	{
		nInputDir() = 0;
	}
	else
	{
		cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pChild;
		Vector4 v = Vector4();
		Constants::MOTOS_VEC4_SET(v, 8.0f, 0.0f, 8.0f);
		if (KingdomGlobal::g_pMotosGame->pPlayers(0u) != nullptr && !KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16448u))
		{
			::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans());
		}
		Vector4 v2 = Vector4(v[0] - vMTrans()[0], 0.0f, v[2] - vMTrans()[2], 0.0f);
		if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(v2)))
		{
			Constants::Motos_P_vecNormalize(v2, v2);
		}
		else
		{
			Constants::MOTOS_VEC4_SET(v2, 0.0f, 0.0f, 1.0f);
		}
		Vector4 v3;
		Constants::tspVector3RotY(v3, v2, Constants::TSP_DEG2RAD(90.0f));
		float num2 = 3.0f;
		float num3 = 1.0f;
		while (cTaskCharEnemy2 != nullptr)
		{
			if (!cTaskCharEnemy2->bTestFlag(16448u))
			{
				Vector4 v4;
				Constants::tspVector3Scale(v4, v2, num2 - 1.1f * (float)(cTaskCharEnemy2->pFormInfo()->m_nLine - 1));
				Constants::VEC3_ADD(v, vMTrans(), v4);
				Constants::tspVector3Scale(v4, v3, num3 * (float)(cTaskCharEnemy2->pFormInfo()->m_nLine - 1) * 0.5f - num3 * (float)cTaskCharEnemy2->pFormInfo()->m_nLineID);
				Constants::VEC3_ADD(cTaskCharEnemy2->vFTarget(), v, v4);
				if (cTaskCharEnemy2->bTestFlag(65536u))
				{
					flag = false;
				}
				num++;
			}
			cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild;
		}
		nInputDir() = 0;
		::Constants::jetGeoVecUnit(vSpeed());
		if (num == 0)
		{
			return 1;
		}
		if (flag)
		{
			return 2;
		}
	}
	if (--nLTimer() < 0)
	{
		return 2;
	}
	return 0;
}

int cTaskCharEnemy::FormMoveArrow_Parent()
{
	if (bTestFlag(2097152u))
	{
		nInputDir() = 0;
	}
	else
	{
		int num = 0;
		nInputDir() = (ushort)Input();
		cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pChild;
		Vector4 v = Vector4();
		Constants::MOTOS_VEC4_SET(v, 8.0f, 0.0f, 8.0f);
		Vector4 v2 = Vector4();
		if (KingdomGlobal::g_pMotosGame->pPlayers(0u) != nullptr && !KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16448u))
		{
			::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans());
		}
		v2[0] = v[0] - vMTrans()[0];
		v2[1] = 0.0f;
		v2[2] = v[2] - vMTrans()[2];
		if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(v2)))
		{
			Constants::Motos_P_vecNormalize(v2, v2);
		}
		else
		{
			Constants::MOTOS_VEC4_SET(v2, 0.0f, 0.0f, 1.0f);
		}
		Vector4 v3;
		Constants::tspVector3RotY(v3, v2, Constants::TSP_DEG2RAD(90.0f));
		float num2 = 3.0f;
		float num3 = 1.0f;
		while (cTaskCharEnemy2 != nullptr)
		{
			if (!cTaskCharEnemy2->bTestFlag(16448u))
			{
				Vector4 v4;
				Constants::tspVector3Scale(v4, v2, num2 - 1.1f * (float)(cTaskCharEnemy2->pFormInfo()->m_nLine - 1));
				Constants::VEC3_ADD(v, vMTrans(), v4);
				Constants::tspVector3Scale(v4, v3, num3 * (float)(cTaskCharEnemy2->pFormInfo()->m_nLine - 1) * 0.5f - num3 * (float)cTaskCharEnemy2->pFormInfo()->m_nLineID);
				Constants::VEC3_ADD(cTaskCharEnemy2->vFTarget(), v, v4);
				num++;
			}
			cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild;
		}
		if (num == 0)
		{
			return 1;
		}
	}
	if (--nLTimer() < 0)
	{
		return 3;
	}
	return 0;
}

int cTaskCharEnemy::FormMeetFence_Parent()
{
	if (bTestFlag(2097152u))
	{
		nInputDir() = 0;
	}
	else
	{
		bool flag = true;
		int num = 0;
		cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pChild;
		Vector4 v = Vector4(8.0f, 0.0f, 8.0f, 1.0f);
		if (KingdomGlobal::g_pMotosGame->pPlayers(0u) != nullptr && !KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16448u))
		{
			::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans());
		}
		Vector4 v2 = Vector4(v[0] - vMTrans()[0], 0.0f, v[2] - vMTrans()[2], 0.0f);
		if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(v2)))
		{
			Constants::Motos_P_vecNormalize(v2, v2);
		}
		else
		{
			Constants::MOTOS_VEC4_SET(v2, 0.0f, 0.0f, 1.0f);
		}
		Vector4 v3;
		Constants::tspVector3RotY(v3, v2, Constants::TSP_DEG2RAD(90.0f));
		float num2 = 1.0f;
		int num3 = 0;
		int num4 = 0;
		while (cTaskCharEnemy2 != nullptr)
		{
			if (cTaskCharEnemy2->pFormInfo()->m_ID % 2 == 0)
			{
				num3++;
				Constants::tspVector3Scale(v, v3, num2 * (float)num3);
			}
			else
			{
				num4++;
				Constants::tspVector3Scale(v, v3, num2 * (float)(-num4));
			}
			if (!cTaskCharEnemy2->bTestFlag(16448u))
			{
				Constants::VEC3_ADD(cTaskCharEnemy2->vFTarget(), vMTrans(), v);
				if (cTaskCharEnemy2->bTestFlag(65536u))
				{
					flag = false;
				}
				num++;
			}
			cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild;
		}
		nInputDir() = 0;
		::Constants::jetGeoVecUnit(vSpeed());
		if (num == 0)
		{
			return 1;
		}
		if (flag)
		{
			return 2;
		}
	}
	if (--nLTimer() < 0)
	{
		return 2;
	}
	return 0;
}

int cTaskCharEnemy::FormMoveFence_Parent()
{
	if (bTestFlag(2097152u))
	{
		nInputDir() = 0;
	}
	else
	{
		int num = 0;
		nInputDir() = (ushort)Input();
		cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pChild;
		Vector4 v = Vector4(8.0f, 0.0f, 8.0f, 1.0f);
		if (KingdomGlobal::g_pMotosGame->pPlayers(0u) != nullptr && !KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16448u))
		{
			::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans());
		}
		Vector4 v2 = Vector4(v[0] - vMTrans()[0], 0.0f, v[2] - vMTrans()[2], 0.0f);
		if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(v2)))
		{
			Constants::Motos_P_vecNormalize(v2, v2);
		}
		else
		{
			Constants::MOTOS_VEC4_SET(v2, 0.0f, 0.0f, 1.0f);
		}
		Vector4 v3;
		Constants::tspVector3RotY(v3, v2, Constants::TSP_DEG2RAD(90.0f));
		float num2 = 1.0f;
		int num3 = 0;
		int num4 = 0;
		while (cTaskCharEnemy2 != nullptr)
		{
			if (cTaskCharEnemy2->pFormInfo()->m_ID % 2 == 0)
			{
				num3++;
				Constants::tspVector3Scale(v, v3, num2 * (float)num3);
			}
			else
			{
				num4++;
				Constants::tspVector3Scale(v, v3, num2 * (float)(-num4));
			}
			if (!cTaskCharEnemy2->bTestFlag(16448u))
			{
				Constants::VEC3_ADD(cTaskCharEnemy2->vFTarget(), vMTrans(), v);
				num++;
			}
			cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild;
		}
		if (num == 0)
		{
			return 1;
		}
	}
	if (--nLTimer() < 0)
	{
		return 3;
	}
	return 0;
}

int cTaskCharEnemy::FormMeet_Children()
{
	if (bTestFlag(2097152u) || pFormInfo()->m_pParent->bTestFlag(2098176u))
	{
		nInputDir() = (ushort)Input();
	}
	else
	{
		float num = 1.0f - HereticRate();
		float num2 = vFTarget()[0] - vMTrans()[0];
		float num3 = vFTarget()[2] - vMTrans()[2];
		ushort num4;
		switch (pFormInfo()->m_pParent->pFormNow()->m_nType)
		{
		case 4194306:
		case 4194307:
		case 4194309:
			num4 = nTargetDirMeet(num2, num3);
			break;
		case 4194308:
			num4 = ((!(Constants::tspVector2Length(Vector2(num2, num3)) > 1.5f)) ? nTargetDir2(num2, num3) : nTargetDirMeet(num2, num3));
			break;
		default:
			num4 = nTargetDir2(num2, num3);
			break;
		}
		Vector4 v;
		::Constants::jetGeoVecCopy(v, Constants::motosDir2Vector(num4));
		Constants::VEC3_MUL(v, v, fAccelSpeed() * num);
		Constants::VEC3_ADD(vSpeed(), vSpeed(), v);
		::Constants::jetGeoVecCopy(v, vSpeed());
		AdjustMaxSpeed(v);
		Vector2 v2 = Vector2(num2, num3);
		float num5 = Constants::tspVector3Length(v);
		float num6 = Constants::tspVector2Length(v2);
		if (num6 <= num5)
		{
			vSpeed()[0] = num2;
			vSpeed()[2] = num3;
			OffFlag(65536u);
		}
		else if (num6 < 0.05f)
		{
			::Constants::jetGeoVecUnit(vSpeed());
			OffFlag(65536u);
		}
		ushort num7 = nFallPassDirTarget(num4, vFTarget(), vSpeed(), fAccelSpeed() * num);
		if (num7 != num4)
		{
			num4 = num7;
			::Constants::jetGeoVecUnit(vSpeed());
			::Constants::jetGeoVecCopy(v, Constants::motosDir2Vector(num4));
			Constants::VEC3_MUL(v, v, fAccelSpeed() * num);
			Constants::VEC3_ADD(vSpeed(), vSpeed(), v);
		}
		AdjustMaxSpeed();
		if (IsDistFallVec(vSpeed()))
		{
			::Constants::jetGeoVecUnit(vSpeed());
			num4 = 0;
			OnFlag(65536u);
		}
		nInputDir() = num4;
	}
	if (!bTestFlag(65536u))
	{
		return 4;
	}
	return 3;
}

int cTaskCharEnemy::FormMove_Children()
{
	bool flag = false;
	float num = vFTarget()[0] - vMTrans()[0];
	float num2 = vFTarget()[2] - vMTrans()[2];
	if (bTestFlag(2097152u) || pFormInfo()->m_pParent->bTestFlag(2098176u))
	{
		nInputDir() = (ushort)Input();
	}
	else
	{
		vSpeed()[0] = num;
		vSpeed()[2] = num2;
		AdjustMaxSpeed();
		if (IsDistFallVec(vSpeed()))
		{
			::Constants::jetGeoVecUnit(vSpeed());
			flag = true;
		}
	}
	Vector2 v = Vector2(num, num2);
	if ((bTestFlag(2098176u) | flag) || Constants::tspVector2Length(v) > 2.5f)
	{
		return 3;
	}
	return 4;
}

int cTaskCharEnemy::Form_MMS2MeetFunc(int mmsType)
{
	int num = -1;
	switch (mmsType)
	{
	case 4194306:
		num = FormMeetSnake_Parent();
		break;
	case 4194307:
		num = FormMeetCircle_Parent();
		break;
	case 4194308:
		num = FormMeetArrow_Parent();
		break;
	case 4194309:
		num = FormMeetFence_Parent();
		break;
	}
	switch (num)
	{
	case 0:
		return 3;
	case 2:
		OffFlag(65536u);
		return 4;
	case 1:
		return 1;
	default:
		return 1;
	}
}

int cTaskCharEnemy::Form_MMS2MoveFunc(int mmsType)
{
	int num = -1;
	switch (mmsType)
	{
	case 4194306:
		num = FormMoveSnake_Parent();
		break;
	case 4194307:
		num = FormMoveCircle_Parent();
		break;
	case 4194308:
		num = FormMoveArrow_Parent();
		break;
	case 4194309:
		num = FormMoveFence_Parent();
		break;
	}
	switch (num)
	{
	case 0:
		return 4;
	case 1:
		return 1;
	case 3:
		pFormNow() = pFormNow()->m_pNext;
		if (pFormNow() != nullptr)
		{
			if (pFormNow()->m_nType == 4194305)
			{
				return 1;
			}
			return 3;
		}
		return 1;
	default:
		return 1;
	}
}

bool cTaskCharEnemy::Form_MMS2MeetOrder(int mmsType)
{
	pFormInfo()->m_ID = 0;
	pFormInfo()->m_nChildNum = 0;
	pFormInfo()->m_nDeg = 0;
	for (cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pChild; cTaskCharEnemy2 != nullptr; cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild)
	{
		if (!cTaskCharEnemy2->bTestFlag(16448u))
		{
			pFormInfo()->m_nChildNum++;
			cTaskCharEnemy2->OnFlag(98304u);
			cTaskCharEnemy2->SetMode(3);
			cTaskCharEnemy2->pFormInfo()->m_ID = 0;
		}
	}
	if (pFormInfo()->m_nChildNum == 0)
	{
		return false;
	}
	Vector4 v = Vector4();
	cTaskCharEnemy* cTaskCharEnemy3 = nullptr;
	Vector4 v4;
	switch (mmsType)
	{
	case 4194306:
		{
			::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans());
			v[0] = vMTrans()[0] - v[0];
			v[1] = 0.0f;
			v[2] = vMTrans()[2] - v[2];
			if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(v)))
			{
				Constants::Motos_P_vecNormalize(v, v);
			}
			else
			{
				::Constants::jetGeoVecUnit(v);
				v[2] = 1.0f;
			}
			for (int k = 0; k < pFormInfo()->m_nChildNum; k++)
			{
				Constants::tspVector3Scale(v4, v, 1.1f * (float)(k + 1));
				Constants::VEC3_ADD(v4, vMTrans(), v4);
				cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pChild;
				cTaskCharEnemy3 = nullptr;
				while (cTaskCharEnemy2 != nullptr)
				{
					if (!cTaskCharEnemy2->bTestFlag(16448u) && cTaskCharEnemy2->pFormInfo()->m_ID == 0)
					{
						if (cTaskCharEnemy3 == nullptr)
						{
							cTaskCharEnemy2->pFormInfo()->m_ID = k + 1;
							cTaskCharEnemy3 = cTaskCharEnemy2;
						}
						else if (Constants::tspVector3Distance(v4, cTaskCharEnemy2->vMTrans()) < Constants::tspVector3Distance(v4, cTaskCharEnemy3->vMTrans()))
						{
							cTaskCharEnemy3->pFormInfo()->m_ID = 0;
							cTaskCharEnemy2->pFormInfo()->m_ID = k + 1;
							cTaskCharEnemy3 = cTaskCharEnemy2;
						}
					}
					cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild;
				}
			}
			break;
		}
	case 4194307:
		{
			uint num9 = 0u;
			cTaskCharEnemy* cTaskCharEnemy4 = nullptr;
			Constants::MOTOS_VEC4_SET(v, 1.3f);
			for (int l = 0; l < pFormInfo()->m_nChildNum; l++)
			{
				float ry = (float)std::numbers::pi * 2.0f / (float)pFormInfo()->m_nChildNum * (float)l;
				Vector4 v7;
				Constants::tspVector3RotY(v7, v, ry);
				Constants::VEC3_ADD(v7, vMTrans(), v7);
				float num10 = -1.0f;
				int num11 = 0;
				int num12 = 0;
				cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pChild;
				while (cTaskCharEnemy2 != nullptr)
				{
					if (CustomKingdomUtil::IF((long)num9 & (long)(1 << num12)))
					{
						num12++;
						cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild;
						continue;
					}
					float num13 = Constants::tspVector3Distance(v7, cTaskCharEnemy2->vMTrans());
					if (num10 == -1.0f || num10 > num13)
					{
						num10 = num13;
						num11 = num12;
						cTaskCharEnemy4 = cTaskCharEnemy2;
					}
					num12++;
					cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild;
				}
				if (cTaskCharEnemy4 != nullptr)
				{
					cTaskCharEnemy4->pFormInfo()->m_ID = l + 1;
					num9 |= (uint)(1 << num11);
				}
			}
			break;
		}
	case 4194308:
		{
			::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans());
			Vector4 v5 = Vector4(v[0] - vMTrans()[0], 0.0f, v[2] - vMTrans()[2], 0.0f);
			if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(v5)))
			{
				Constants::Motos_P_vecNormalize(v5, v5);
			}
			else
			{
				::Constants::jetGeoVecUnit(v5);
				v5[2] = 1.0f;
			}
			Vector4 v6;
			Constants::tspVector3RotY(v6, v5, Constants::TSP_DEG2RAD(90.0f));
			float num4 = 2.2f;
			float num5 = 1.3f;
			int num6 = 0;
			int num7 = 0;
			int num8 = 0;
			for (int j = 0; j < pFormInfo()->m_nChildNum; j++)
			{
				if (num6 == num8)
				{
					num6++;
					num7++;
					num8 = 0;
				}
				Constants::tspVector3Scale(v4, v5, num4 - 1.1f * (float)num7);
				Constants::VEC3_ADD(v, vMTrans(), v4);
				Constants::tspVector3Scale(v4, v6, num5 * (float)num7 * 0.5f - num5 * (float)num8);
				Constants::VEC3_ADD(v4, v, v4);
				cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pChild;
				cTaskCharEnemy3 = nullptr;
				while (cTaskCharEnemy2 != nullptr)
				{
					if (!cTaskCharEnemy2->bTestFlag(16448u) && cTaskCharEnemy2->pFormInfo()->m_ID == 0)
					{
						if (cTaskCharEnemy3 == nullptr)
						{
							cTaskCharEnemy2->pFormInfo()->m_ID = j + 1;
							cTaskCharEnemy2->pFormInfo()->m_nLine = num7;
							cTaskCharEnemy2->pFormInfo()->m_nLineID = num8;
							cTaskCharEnemy3 = cTaskCharEnemy2;
						}
						else if (Constants::tspVector3Distance(v4, cTaskCharEnemy2->vMTrans()) < Constants::tspVector3Distance(v4, cTaskCharEnemy3->vMTrans()))
						{
							cTaskCharEnemy3->pFormInfo()->m_ID = 0;
							cTaskCharEnemy2->pFormInfo()->m_ID = j + 1;
							cTaskCharEnemy2->pFormInfo()->m_nLine = num7;
							cTaskCharEnemy2->pFormInfo()->m_nLineID = num8;
							cTaskCharEnemy3 = cTaskCharEnemy2;
						}
					}
					cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild;
				}
				num8++;
			}
			break;
		}
	case 4194309:
		{
			::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans());
			Vector4 v2 = Vector4(v[0] - vMTrans()[0], 0.0f, v[2] - vMTrans()[2], 0.0f);
			if (CustomKingdomUtil::IF(::Constants::jetGeoVecLength(v2)))
			{
				Constants::Motos_P_vecNormalize(v2, v2);
			}
			else
			{
				::Constants::jetGeoVecUnit(v2);
				v2[2] = 1.0f;
			}
			Vector4 v3;
			Constants::tspVector3RotY(v3, v2, Constants::TSP_DEG2RAD(90.0f));
			float num = 1.0f;
			int num2 = 0;
			int num3 = 0;
			for (int i = 0; i < pFormInfo()->m_nChildNum; i++)
			{
				if ((i + 1) % 2 == 0)
				{
					num2++;
					Constants::tspVector3Scale(v, v3, num * (float)num2);
				}
				else
				{
					num3++;
					Constants::tspVector3Scale(v, v3, num * (float)(-num3));
				}
				Constants::VEC3_ADD(v4, vMTrans(), v);
				cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pChild;
				cTaskCharEnemy3 = nullptr;
				while (cTaskCharEnemy2 != nullptr)
				{
					if (!cTaskCharEnemy2->bTestFlag(16448u) && cTaskCharEnemy2->pFormInfo()->m_ID == 0)
					{
						if (cTaskCharEnemy3 == nullptr)
						{
							cTaskCharEnemy2->pFormInfo()->m_ID = i + 1;
							cTaskCharEnemy3 = cTaskCharEnemy2;
						}
						else if (Constants::tspVector3Distance(v4, cTaskCharEnemy2->vMTrans()) < Constants::tspVector3Distance(v4, cTaskCharEnemy3->vMTrans()))
						{
							cTaskCharEnemy3->pFormInfo()->m_ID = 0;
							cTaskCharEnemy2->pFormInfo()->m_ID = i + 1;
							cTaskCharEnemy3 = cTaskCharEnemy2;
						}
					}
					cTaskCharEnemy2 = cTaskCharEnemy2->pFormInfo()->m_pChild;
				}
			}
			break;
		}
	default:
		return false;
	}
	return true;
}

bool cTaskCharEnemy::Formation_Parent(int nMes, cJetTask* pListC, cJetTask* pScene)
{
	if (GetMode() != 0)
	{
		if (2 == GetMode())
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
				if (pFormNow() != nullptr)
				{
					SetMode(3);
				}
				else
				{
					SetMode(1);
				}
				return true;
			}
		}
		else if (3 == GetMode())
		{
			if (2 == nMes)
			{
				OnFlag(98304u);
				if (!Form_MMS2MeetOrder(pFormNow()->m_nType))
				{
					SetMode(1);
				}
				else
				{
					OnFlag(98304u);
					nLTimer() = pFormNow()->m_nMeetTime;
				}
				return true;
			}
			if (3 == nMes)
			{
				nLTimer() = 0;
				return true;
			}
			if (1 == nMes)
			{
				if (bTestFlag(64u))
				{
					return false;
				}
				SetMode(Form_MMS2MeetFunc(pFormNow()->m_nType));
				if (!Constants::IS_LANPLAY() && KingdomGlobal::g_pMotosGame->pPlayers(0u)->IsFall())
				{
					SetMode(1);
				}
				return true;
			}
		}
		else if (4 == GetMode())
		{
			if (2 == nMes)
			{
				nLTimer() = pFormNow()->m_nMoveTime;
				fMaxSpeed() = fDefMaxSpeed() * 0.7f;
				if (nHereticVal() > 4)
				{
					nHereticVal() = 4u;
				}
				return true;
			}
			if (3 == nMes)
			{
				fMaxSpeed() = fDefMaxSpeed();
				nHereticVal() = nDefHereticVal();
				nLTimer() = 0;
				return true;
			}
			if (1 == nMes)
			{
				if (bTestFlag(64u))
				{
					return false;
				}
				SetMode(Form_MMS2MoveFunc(pFormNow()->m_nType));
				if (!Constants::IS_LANPLAY() && KingdomGlobal::g_pMotosGame->pPlayers(0u)->IsFall())
				{
					SetMode(1);
				}
				return true;
			}
		}
		else if (1 == GetMode())
		{
			if (2 == nMes)
			{
				OffFlag(98304u);
				if (pFormNow() != nullptr)
				{
					nLTimer() = pFormNow()->m_nMoveTime;
				}
				return true;
			}
			if (3 == nMes)
			{
				nLTimer() = 0;
				return true;
			}
			if (1 == nMes)
			{
				nInputDir() = (ushort)Input();
				if (pFormNow() != nullptr && --nLTimer() < 0)
				{
					pFormNow() = pFormNow()->m_pNext;
					if (pFormNow() != nullptr)
					{
						SetMode(3);
					}
				}
				if (!Constants::IS_LANPLAY() && KingdomGlobal::g_pMotosGame->pPlayers(0u)->IsFall())
				{
					SetMode(1);
				}
				return true;
			}
		}
	}
	return true;
}

bool cTaskCharEnemy::Formation_Child(int nMes, cJetTask* pListC, cJetTask* pScene)
{
	if (GetMode() != 0)
	{
		if (2 == GetMode())
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
				SetMode(1);
				return true;
			}
		}
		else if (3 == GetMode())
		{
			if (2 == nMes)
			{
				OnFlag(98304u);
				fMaxSpeed() = 0.12f;
				return true;
			}
			if (3 == nMes)
			{
				fMaxSpeed() = fDefMaxSpeed();
				return true;
			}
			if (1 == nMes)
			{
				cTaskCharEnemy* cTaskCharEnemy2 = pFormInfo()->m_pParent;
				if (bTestFlag(2097152u) || pFormInfo()->m_pParent->bTestFlag(2098176u))
				{
					fMaxSpeed() = fDefMaxSpeed();
				}
				else
				{
					fMaxSpeed() = 0.12f;
				}
				if (cTaskCharEnemy2 == nullptr || !cTaskCharEnemy2->bTestFlag(32768u) || cTaskCharEnemy2->pFormNow() == nullptr || cTaskCharEnemy2->bTestFlag(16448u))
				{
					SetMode(1);
				}
				else
				{
					if (bTestFlag(64u))
					{
						return false;
					}
					SetMode(FormMeet_Children());
				}
				return true;
			}
		}
		else if (4 == GetMode())
		{
			if (2 == nMes)
			{
				fMaxSpeed() = 0.23f;
				if (nHereticVal() > 4)
				{
					nHereticVal() = 4u;
				}
				return true;
			}
			if (3 == nMes)
			{
				fMaxSpeed() = fDefMaxSpeed();
				nHereticVal() = nDefHereticVal();
				return true;
			}
			if (1 == nMes)
			{
				cTaskCharEnemy* cTaskCharEnemy3 = pFormInfo()->m_pParent;
				if (bTestFlag(2097152u) || pFormInfo()->m_pParent->bTestFlag(2098176u))
				{
					fMaxSpeed() = fDefMaxSpeed();
					nHereticVal() = nDefHereticVal();
				}
				else
				{
					fMaxSpeed() = 0.23f;
					if (nHereticVal() > 4)
					{
						nHereticVal() = 4u;
					}
				}
				if (cTaskCharEnemy3 == nullptr || !cTaskCharEnemy3->bTestFlag(32768u) || cTaskCharEnemy3->pFormNow() == nullptr || cTaskCharEnemy3->bTestFlag(16448u))
				{
					SetMode(1);
				}
				else
				{
					if (bTestFlag(64u))
					{
						return false;
					}
					SetMode(FormMove_Children());
				}
				return true;
			}
		}
		else if (1 == GetMode())
		{
			if (2 == nMes)
			{
				OffFlag(98304u);
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
	}
	return true;
}

cTaskCharPlayer* cTaskCharEnemy::SearchNearPlayer()
{
	cTaskCharPlayer* cTaskCharPlayer2 = nullptr;
	if (!KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		if (!KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(278624u))
		{
			cTaskCharPlayer2 = KingdomGlobal::g_pMotosGame->pPlayers(0u);
		}
	}
	else
	{
		float num = 0.0f;
		for (int i = 0; i < 4; i++)
		{
			cTaskCharPlayer* cTaskCharPlayer3 = KingdomGlobal::g_pMotosGame->pPlayers((uint)i);
			if (cTaskCharPlayer3 != nullptr && !cTaskCharPlayer3->bTestFlag(16448u))
			{
				Vector4 _out;
				Constants::VEC3_SUB(_out, cTaskCharPlayer3->vMTrans(), vMTrans());
				float num2 = Constants::tspVector3Length(_out);
				if (cTaskCharPlayer2 == nullptr)
				{
					cTaskCharPlayer2 = cTaskCharPlayer3;
					num = num2;
				}
				else if (num2 < num)
				{
					cTaskCharPlayer2 = cTaskCharPlayer3;
					num = num2;
				}
			}
		}
	}
	return cTaskCharPlayer2;
}

bool cTaskCharEnemy::Status()
{
	bool result = cTaskChar::Status();
	if (bTestFlag(2048u) && !bTestFlag(1024u) && IsFall(vMTrans()) && !CheckFall(vMTrans()))
	{
		nHamariTimer()++;
	}
	else
	{
		nHamariTimer() = 0;
	}
	if (bTrigFlag(524288u))
	{
		OnFlag(32u);
		OnFlag(512u);
		vScale()[0] = 0.01f;
		vScale()[2] = 0.01f;
		nNoInputTimer() = 30;
		nRebirthTimer() = 0;
		Constants::SpawnEffect(36, vMTrans());
	}
	if (CustomKingdomUtil::IF(nNoInputTimer()))
	{
		if (!CustomKingdomUtil::IF(--nNoInputTimer()))
		{
			if (Constants::IS_MYCHAR(this))
			{
				OffFlag(524288u);
			}
			OffFlag(32u);
			OffFlag(512u);
			return result;
		}
		vScale()[0] += 0.05f;
		vScale()[2] += 0.05f;
		if (vScale()[0] > 1.0f)
		{
			vScale()[0] = 1.0f;
		}
		if (vScale()[2] > 1.0f)
		{
			vScale()[2] = 1.0f;
		}
	}
	return result;
}

bool cTaskCharEnemy::TaikiAction()
{
	if (Constants::motosIsRebirth() && ++nRebirthTimer() > Constants::motosParamRebirthTime())
	{
		if (!KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(vMBeginTrans()))
		{
			::Constants::jetGeoVecCopy(vMTrans(), vMBeginTrans());
			OnFlag(524288u);
		}
		else
		{
			nRebirthTimer() = 0;
		}
	}
	return false;
}

float cTaskCharEnemy::GetHitPower(int nPart, int nOtherID)
{
	cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)nOtherID);
	if (cTaskChar2->bTestFlag(4u))
	{
		if (bTestFlag(32768u) && cTaskChar2->bTestFlag(32768u) && (static_cast<cTaskCharEnemy*>(cTaskChar2))->nFormGroup() == nFormGroup())
		{
			return fHitPower() * 0.1f;
		}
		if (bTestFlag(2048u) && cTaskChar2->bTestFlag(2048u))
		{
			return fHitPower() * 0.6f;
		}
	}
	return fHitPower();
}

ushort cTaskCharEnemy::nHamariRecoverDir(Vector4 vPos, Vector4 vTarget)
{
	ushort num = 0;
	float fDx = vTarget[0] - vPos[0];
	float fDz = vTarget[2] - vPos[2];
	Vector4 _out;
	Constants::VEC3_SET(_out, Mathf::Floor(vPos[0]) + 0.5f, vPos[1], Mathf::Floor(vPos[2]) + 0.5f);
	ushort num2 = 0;
	if (IsFall(_out[0], _out[1], _out[2] - 1.0f))
	{
		num2 |= 1;
	}
	if (IsFall(_out[0], _out[1], _out[2] + 1.0f))
	{
		num2 |= 2;
	}
	if (IsFall(_out[0] - 1.0f, _out[1], _out[2]))
	{
		num2 |= 4;
	}
	if (IsFall(_out[0] + 1.0f, _out[1], _out[2]))
	{
		num2 |= 8;
	}
	ushort num3 = nTargetDir(fDx, fDz);
	if (!CustomKingdomUtil::IF(num3 & num2))
	{
		return num3;
	}
	if (num3 == 0 || num2 == 0)
	{
		::Constants::jetGeoVecUnit(vSpeed());
		return 0;
	}
	ushort array[8] = {1, 9, 8, 10, 2, 6, 4, 5};
	int num4;
	switch (num3)
	{
	case 1:
		num4 = 0;
		break;
	case 9:
		num4 = 1;
		break;
	case 8:
		num4 = 2;
		break;
	case 10:
		num4 = 3;
		break;
	case 2:
		num4 = 4;
		break;
	case 6:
		num4 = 5;
		break;
	case 4:
		num4 = 6;
		break;
	case 5:
		num4 = 7;
		break;
	default:
		::Constants::jetGeoVecUnit(vSpeed());
		return 0;
	}
	for (int i = 1; i <= 3; i++)
	{
		int num5 = (num4 + i) % 8;
		if (!CustomKingdomUtil::IF(num2 & array[(num4 + i) % 8]))
		{
			num = array[num5];
			break;
		}
		num5 = ((num4 - i < 0) ? 7 : (num4 - i));
		if (!CustomKingdomUtil::IF(num2 & array[num5]))
		{
			num = array[num5];
			break;
		}
	}
	if (num == 0)
	{
		int num5 = (num4 + 4) % 8;
		if (CustomKingdomUtil::IF(num2 & array[num5]))
		{
			::Constants::jetGeoVecUnit(vSpeed());
			return 0;
		}
		num = array[num5];
	}
	return num;
}

}
