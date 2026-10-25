#include "kingdom/game/g07/cTaskHitMan.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include <iterator>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include "kingdom/game/g07/cTaskGimmickBase.hpp"
#include "kingdom/game/g07/cTaskGimmickSwitch.hpp"
#include "kingdom/game/g07/NET_RF_PACKET.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

uint cTaskHitMan::GetRefID()
{
	if (!CustomKingdomUtil::IF(++m_nRefID))
	{
		return (uint)(m_nRefID + 1);
	}
	return m_nRefID;
}

NET_RF_PACKET* cTaskHitMan::RefInfo(int n)
{
	return m_nRefInfos[n];
}

cTaskHitMan::cTaskHitMan()
{
	Init();
}

cTaskHitMan::~cTaskHitMan()
{
	Free();
}

void cTaskHitMan::Init()
{
	m_nRefID = 0;
	m_nRefIndex = 0;
	for (int i = 0; i < (int)std::size(m_nRefInfos); i++)
	{
		m_nRefInfos[i] = new NET_RF_PACKET();
	}
}

void cTaskHitMan::Free()
{
}

bool cTaskHitMan::Execute(cJetTask* pParam)
{
	if (Constants::MOTOS_IS_PAUSE() || Constants::IS_CLEARPAUSE())
	{
		return true;
	}
	for (uint num = 0u; num < 36; num++)
	{
		cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars(num);
		if (cTaskChar2 != nullptr)
		{
			if (cTaskChar2->RefChar() == -1 || cTaskChar2->RefChar() != cTaskChar2->RefCharOLD())
			{
				cTaskChar2->RefCounter() = 0;
			}
			else
			{
				cTaskChar2->RefCounter()++;
			}
			cTaskChar2->RefFlag() = 0u;
			cTaskChar2->RefCharOLD() = cTaskChar2->RefChar();
			cTaskChar2->RefChar() = -1;
			cTaskChar2->RefPartsOLD() = cTaskChar2->RefParts();
			cTaskChar2->RefParts() = -1;
		}
	}
	for (uint num2 = 0u; num2 < 35; num2++)
	{
		cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars(num2);
		if (cTaskChar2 == nullptr || cTaskChar2->bTestFlag(278624u))
		{
			continue;
		}
		for (uint num3 = num2 + 1; num3 < 36; num3++)
		{
			cTaskChar* cTaskChar3 = KingdomGlobal::g_pMotosGame->pChars(num3);
			if (cTaskChar3 == nullptr || HitCheckThrough(cTaskChar2, cTaskChar3))
			{
				continue;
			}
			bool flag = true;
			int num4 = HitCheck(cTaskChar2, cTaskChar3, cTaskChar2->vSpeed(), cTaskChar3->vSpeed());
			if (0 > num4)
			{
				continue;
			}
			cTaskChar2->AdjustSpeedByHit(cTaskChar3);
			cTaskChar3->AdjustSpeedByHit(cTaskChar2);
			if (!Constants::IS_MASTER() || (cTaskChar2->bRefFlag() && cTaskChar2->RefChar() != cTaskChar2->RefCharOLD()) || (cTaskChar3->bRefFlag() && cTaskChar3->RefChar() != cTaskChar3->RefCharOLD()))
			{
				continue;
			}
			cTaskChar2->RefFlag() = Constants::get_MO_HITFLAG_REFLECT();
			cTaskChar3->RefFlag() = Constants::get_MO_HITFLAG_REFLECT();
			cTaskChar2->RefChar() = (int)num3;
			cTaskChar3->RefChar() = (int)num2;
			if ((long)cTaskChar2->RefCharOLD() == (long)num3)
			{
				if (cTaskChar2->RefCounter() < 5 && (!flag || num4 == cTaskChar2->RefAxisOLD()) && cTaskChar2->bRefPartsOLD() && cTaskChar2->bRefParts() && cTaskChar3->bRefPartsOLD() && cTaskChar3->bRefParts())
				{
					continue;
				}
				if (cTaskChar2->RefPartsOLD() == cTaskChar2->RefParts() && cTaskChar3->RefPartsOLD() == cTaskChar3->RefParts())
				{
					num4 = cTaskChar2->RefAxisOLD();
				}
				else if (num4 == Constants::motosAxisReverse(cTaskChar2->RefAxisOLD()))
				{
					num4 = (num4 + 2) % 7;
				}
			}
			RegistReflect((int)cTaskChar2->GrobalID(), (int)cTaskChar3->GrobalID(), num4);
			cTaskChar2->RefAxisOLD() = num4;
			cTaskChar3->RefAxisOLD() = num4;
			cTaskChar2->RefCounter() = 0;
			cTaskChar3->RefCounter() = 0;
			if (!Constants::IS_LANPLAY())
			{
				continue;
			}
			if (cTaskChar2->bTestFlag(2u) && cTaskChar3->bTestFlag(2u))
			{
				KingdomGlobal::g_pMotosGame->nPoints((int)cTaskChar2->ID()) += Constants::get_MO_VS_MOTOS_TOUCH();
				KingdomGlobal::g_pMotosGame->nPoints((int)cTaskChar3->ID()) += Constants::get_MO_VS_MOTOS_TOUCH();
				if (KingdomGlobal::g_pMotosGame->nPoints((int)cTaskChar2->ID()) > 999)
				{
					KingdomGlobal::g_pMotosGame->nPoints((int)cTaskChar2->ID()) = 999;
				}
				if (KingdomGlobal::g_pMotosGame->nPoints((int)cTaskChar3->ID()) > 999)
				{
					KingdomGlobal::g_pMotosGame->nPoints((int)cTaskChar3->ID()) = 999;
				}
			}
			if (cTaskChar2->bTestFlag(2u) && cTaskChar3->bTestFlag(4u))
			{
				KingdomGlobal::g_pMotosGame->nPoints((int)cTaskChar2->ID()) += Constants::get_MO_VS_ENEMY_TOUCH();
				if (KingdomGlobal::g_pMotosGame->nPoints((int)cTaskChar2->ID()) > 999)
				{
					KingdomGlobal::g_pMotosGame->nPoints((int)cTaskChar2->ID()) = 999;
				}
			}
			if (cTaskChar2->bTestFlag(4u) && cTaskChar3->bTestFlag(2u))
			{
				KingdomGlobal::g_pMotosGame->nPoints((int)cTaskChar3->ID()) += Constants::get_MO_VS_ENEMY_TOUCH();
				if (KingdomGlobal::g_pMotosGame->nPoints((int)cTaskChar3->ID()) > 999)
				{
					KingdomGlobal::g_pMotosGame->nPoints((int)cTaskChar3->ID()) = 999;
				}
			}
		}
	}
	if (Constants::IS_MASTER())
	{
		ExecHitGimmick();
		RegistPacket();
	}
	else
	{
		SyncPacket();
	}
	auto& nRefInfos = m_nRefInfos;
	for (int i = 0; i < 32; i++)
	{
		if (nRefInfos[i]->m_nRefID != 0 && (int)Constants::GAME_COUNTER_FROMBYTE(nRefInfos[i]->m_nCounter) < (int)(Constants::get_GAME_COUNTER() - 15))
		{
			nRefInfos[i]->m_nRefID = 0;
		}
	}
	auto& nRefInfos2 = m_nRefInfos;
	for (int j = 0; j < 32; j++)
	{
		if (nRefInfos2[j]->m_nRefID != 0)
		{
			cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars(nRefInfos2[j]->m_nChar[0]);
			cTaskChar* cTaskChar3 = KingdomGlobal::g_pMotosGame->pChars(nRefInfos2[j]->m_nChar[1]);
			if (cTaskChar2 != nullptr && (!Constants::IS_MASTER() || !Constants::IS_LANPLAY() || Constants::GAME_COUNTER_FROMBYTE(nRefInfos2[j]->m_nCounter) < Constants::get_GAME_COUNTER()))
			{
				cTaskChar2->ExecReflect(nRefInfos2[j]);
			}
			if (cTaskChar3 != nullptr && (!Constants::IS_MASTER() || !Constants::IS_LANPLAY() || Constants::GAME_COUNTER_FROMBYTE(nRefInfos2[j]->m_nCounter) < Constants::get_GAME_COUNTER()))
			{
				cTaskChar3->ExecReflect(nRefInfos2[j]);
			}
		}
	}
	return true;
}

int cTaskHitMan::HitCheck(cTaskChar* pChar1, cTaskChar* pChar2, Nullable<Vector4> pSpeed1, Nullable<Vector4> pSpeed2)
{
	int pAxis = -1;
	Vector4 v1_ptr = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
	Vector4 v;
	if (pSpeed1.has_value())
	{
		Constants::VEC3_ADD(v, pChar1->vMTrans(), pSpeed1.value());
	}
	else
	{
		::Constants::jetGeoVecCopy(v, pChar1->vMTrans());
	}
	Vector4 v2;
	if (pSpeed2.has_value())
	{
		Constants::VEC3_ADD(v2, pChar2->vMTrans(), pSpeed2.value());
	}
	else
	{
		::Constants::jetGeoVecCopy(v2, pChar2->vMTrans());
	}
	if (0.0f != pChar1->fHitSize() && 0.0f != pChar2->fHitSize() && CustomKingdomUtil::IF(pChar1->nHitFlag() & pChar2->nHitFlag()))
	{
		Vector4 _out;
		Constants::VEC3_SUB(_out, v2, v);
		float num = Constants::VEC3_LENGTH_SQUARE(_out[0], _out[1], _out[2]);
		float num2 = pChar1->fHitSize() + pChar2->fHitSize();
		num2 *= num2;
		if (num <= num2)
		{
			Constants::VEC3_SUB(_out, pChar1->vMTrans(), pChar2->vMTrans());
			pAxis = Constants::motosRad2Axis(Constants::Motos_P_vecGetRad(_out, v1_ptr));
			pAxis = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->AvoidPingBall(pChar1, pChar2, pAxis);
			pChar1->RefParts() = 0;
			pChar2->RefParts() = 0;
		}
	}
	if (pAxis < 0)
	{
		float fSize = pChar2->fHitSize();
		int num3 = m_HitCheckExSub(pAxis, pChar1, pChar2, v, v2, fSize, (int)pChar2->nHitFlag());
		if (num3 != -1)
		{
			pChar1->RefParts() = num3 + 1;
			pChar2->RefParts() = 0;
		}
	}
	if (pAxis < 0)
	{
		float fSize2 = pChar1->fHitSize();
		int num3 = m_HitCheckExSub(pAxis, pChar2, pChar1, v2, v, fSize2, (int)pChar1->nHitFlag());
		if (num3 != -1)
		{
			pChar1->RefParts() = 0;
			pChar2->RefParts() = num3 + 1;
			pAxis = (pAxis + 4) & 7;
		}
	}
	return pAxis;
}

bool cTaskHitMan::HitCheckSimple(cTaskChar* pChar1, cTaskChar* pChar2, Vector4 pSpeed1, Vector4 pSpeed2)
{
	Constants::MOTOS_CAUTION("HitCheckSimple");
	Vector4 v;
	if (CustomKingdomUtil::IF(pSpeed1))
	{
		Constants::VEC3_ADD(v, pChar1->vMTrans(), pSpeed1);
	}
	else
	{
		::Constants::jetGeoVecCopy(v, pChar1->vMTrans());
	}
	Vector4 v2;
	if (CustomKingdomUtil::IF(pSpeed2))
	{
		Constants::VEC3_ADD(v2, pChar2->vMTrans(), pSpeed2);
	}
	else
	{
		::Constants::jetGeoVecCopy(v2, pChar2->vMTrans());
	}
	Vector4 _out;
	Constants::VEC3_SUB(_out, v2, v);
	float num = Constants::VEC3_LENGTH_SQUARE(_out[0], _out[1], _out[2]);
	float num2 = pChar1->fHitSize() + pChar2->fHitSize();
	num2 *= num2;
	if (!(num <= num2))
	{
		return false;
	}
	return true;
}

int cTaskHitMan::m_HitCheckExSub(int& pAxis, cTaskChar* pChar1, cTaskChar* pChar2, Vector4 pPos1, Vector4 pPos2, float fSize, int nHigh)
{
	int num = 0;
	Vector4 vector = Vector4();
	if (pChar1->bnExHitNum())
	{
		bool flag = false;
		Vector4 _out;
		Constants::VEC3_SUB(_out, pPos2, pPos1);
		Constants::Motos_P_vecRotateVector(_out, _out, pChar1->vRotate()[1]);
		Vector4 _out2;
		Constants::VEC3_SUB(_out2, pChar2->vMTrans(), pChar1->vMTrans());
		Constants::Motos_P_vecRotateVector(_out2, _out2, pChar1->vRotate()[1]);
		int num2 = 0;
		Constants::EX_HIT_PARAM* eX_HIT_PARAM;
		while ((eX_HIT_PARAM = pChar1->ExHitParam(num2)) != nullptr)
		{
			if (CustomKingdomUtil::IF(nHigh & eX_HIT_PARAM->m_nFlag))
			{
				if (eX_HIT_PARAM->m_nKind == 0)
				{
					bool flag2 = ((Constants::MOTOS_P_ABS(eX_HIT_PARAM->m_square.m_vPoint0[0] - eX_HIT_PARAM->m_square.m_vPoint1[0]) < Constants::MOTOS_P_ABS(eX_HIT_PARAM->m_square.m_vPoint0[2] - eX_HIT_PARAM->m_square.m_vPoint1[2])) ? true : false);
					if (!(_out[0] < eX_HIT_PARAM->m_square.m_vPoint0[0] - fSize) && !(_out[0] > eX_HIT_PARAM->m_square.m_vPoint1[0] + fSize) && !(_out[2] < eX_HIT_PARAM->m_square.m_vPoint0[2] - fSize) && !(_out[2] > eX_HIT_PARAM->m_square.m_vPoint1[2] + fSize))
					{
						vector[0] = (eX_HIT_PARAM->m_square.m_vPoint0[0] + eX_HIT_PARAM->m_square.m_vPoint1[0]) / 2.0f;
						vector[1] = 0.0f;
						vector[2] = (eX_HIT_PARAM->m_square.m_vPoint0[2] + eX_HIT_PARAM->m_square.m_vPoint1[2]) / 2.0f;
						num = ((_out2[2] > eX_HIT_PARAM->m_square.m_vPoint1[2]) ? ((_out2[0] < eX_HIT_PARAM->m_square.m_vPoint0[0]) ? 7 : ((!(_out2[0] < eX_HIT_PARAM->m_square.m_vPoint1[0])) ? 1 : 0)) : ((_out2[2] < eX_HIT_PARAM->m_square.m_vPoint0[2]) ? ((_out2[0] < eX_HIT_PARAM->m_square.m_vPoint0[0]) ? 5 : ((!(_out2[0] < eX_HIT_PARAM->m_square.m_vPoint1[0])) ? 3 : 4)) : ((_out2[0] < eX_HIT_PARAM->m_square.m_vPoint0[0]) ? 6 : ((_out2[0] > eX_HIT_PARAM->m_square.m_vPoint1[0]) ? 2 : (flag2 ? ((!(_out2[0] < vector[0])) ? 2 : 6) : ((_out2[2] < vector[2]) ? 4 : 0))))));
						if (CustomKingdomUtil::IF(eX_HIT_PARAM->m_nFlag & 8))
						{
							num = (flag2 ? ((_out2[2] < vector[2]) ? 4 : 0) : ((!(_out2[0] > vector[0])) ? 6 : 2));
						}
						else if (CustomKingdomUtil::IF(eX_HIT_PARAM->m_nFlag & 0x10))
						{
							num = (flag2 ? ((_out2[2] < vector[2]) ? 4 : 0) : ((!(_out2[0] > vector[0])) ? 6 : 2));
						}
						flag = true;
						break;
					}
				}
				else if (eX_HIT_PARAM->m_nKind == 1)
				{
					Vector4 v1_ptr = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
					Vector4 _out3;
					Constants::VEC3_SUB(_out3, _out, eX_HIT_PARAM->m_circle.m_vOrigin);
					float num3 = Constants::VEC3_LENGTH_SQUARE(_out3[0], _out3[1], _out3[2]);
					float num4 = fSize + eX_HIT_PARAM->m_circle.m_fRange;
					if (!(num4 * num4 < num3))
					{
						Constants::VEC3_SUB(_out3, _out, eX_HIT_PARAM->m_circle.m_vOrigin);
						num = Constants::motosRad2Axis(Constants::Motos_P_vecGetRad(_out3, v1_ptr));
						flag = true;
						break;
					}
				}
			}
			num2++;
		}
		if (flag)
		{
			pAxis = (num + 4) & 7;
			pAxis = Constants::motosAxisRotate(pAxis, pChar1->vRotate()[1]);
			return num2;
		}
	}
	return -1;
}

bool cTaskHitMan::HitCheckThrough(cTaskChar* pChar1, cTaskChar* pChar2)
{
	if (pChar1->bTestFlag(278624u))
	{
		return true;
	}
	if (pChar2->bTestFlag(278624u))
	{
		return true;
	}
	if (pChar1->bTestFlag(32768u) && !pChar1->bTestFlag(2098176u) && pChar2->bTestFlag(32768u) && !pChar2->bTestFlag(2098176u) && pChar1->bTestFlag(4u) && pChar2->bTestFlag(4u) && !pChar1->bTestFlag(8u) && !pChar2->bTestFlag(8u) && (static_cast<cTaskCharEnemy*>(pChar1))->nFormGroup() == (static_cast<cTaskCharEnemy*>(pChar2))->nFormGroup())
	{
		return true;
	}
	if (CustomKingdomUtil::IF(pChar1->nHitFlagAll() & pChar2->nHitFlagAll()))
	{
		return false;
	}
	return true;
}

bool cTaskHitMan::ExecHitGimmick()
{
	cTaskGimmickBase* cTaskGimmickBase2 = nullptr;
	if (!KingdomGlobal::g_pMotosGame->bGimmick())
	{
		return false;
	}
	for (int i = 0; i < 36; i++)
	{
		cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)i);
		if (cTaskChar2 == nullptr || cTaskChar2->bTestFlag(278624u) || cTaskChar2->bTestFlag(384u) || cTaskChar2->bRefFlag())
		{
			continue;
		}
		cTaskGimmickBase2 = KingdomGlobal::g_pMotosGame->pMap()->SearchGimmick(cTaskChar2->vMTrans()[0], cTaskChar2->vMTrans()[2]);
		if (cTaskGimmickBase2 == nullptr)
		{
			continue;
		}
		switch (cTaskGimmickBase2->nTaskKind())
		{
		case 1u:
			if (cTaskGimmickBase2->IsAction(cTaskChar2))
			{
				int axis = Constants::motosRad2Axis(cTaskGimmickBase2->vRot()[1]);
				RegistReflect(i, 129, axis);
			}
			break;
		case 3u:
			if (cTaskGimmickBase2->IsStepOnTrig(cTaskChar2))
			{
				(static_cast<cTaskGimmickSwitch*>(cTaskGimmickBase2))->BootSwitch();
			}
			break;
		}
	}
	return true;
}

bool cTaskHitMan::RegistReflect(int nGrobalID1, int nGrobalID2, int axis)
{
	byte b = 0;
	byte b2 = 0;
	byte b3 = 0;
	byte b4 = 0;
	cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)nGrobalID1);
	if (cTaskChar2 != nullptr)
	{
		b = (byte)cTaskChar2->RefParts();
		if (cTaskChar2->bTestFlag(4096u))
		{
			b3 |= 1;
		}
		if (cTaskChar2->bTestFlag(1048576u))
		{
			b3 |= 2;
		}
	}
	cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)nGrobalID2);
	if (cTaskChar2 != nullptr)
	{
		b2 = (byte)cTaskChar2->RefParts();
		if (cTaskChar2->bTestFlag(4096u))
		{
			b4 |= 1;
		}
		if (cTaskChar2->bTestFlag(1048576u))
		{
			b4 |= 2;
		}
	}
	auto& nRefInfos = m_nRefInfos;
	for (int i = 0; i < 32; i++)
	{
		if (!CustomKingdomUtil::IF(nRefInfos[i]->m_nRefID))
		{
			nRefInfos[i]->m_nRefID = (byte)(GetRefID() & 0xFF);
			nRefInfos[i]->m_nAxis_Status = (byte)(axis | (b3 << 4) | (b4 << 6));
			nRefInfos[i]->m_nParts = (byte)((b & 0xF) | ((b2 << 4) & 0xF0));
			nRefInfos[i]->m_nChar[0] = (byte)nGrobalID1;
			nRefInfos[i]->m_nChar[1] = (byte)nGrobalID2;
			nRefInfos[i]->m_nCounter = (byte)(Constants::get_GAME_COUNTER() & 0xFF);
			return true;
		}
	}
	return false;
}

NET_RF_PACKET* cTaskHitMan::SearchReflect(int nGrobalID1, int nGrobalID2)
{
	auto& nRefInfos = m_nRefInfos;
	for (int i = 0; i < 32; i++)
	{
		if (nRefInfos[i]->m_nRefID != 0 && nRefInfos[i]->m_nChar[0] == nGrobalID1 && nRefInfos[i]->m_nChar[1] == nGrobalID2)
		{
			return nRefInfos[i];
		}
	}
	return nullptr;
}

void cTaskHitMan::DeleteReflect(int nGrobalID1, int nGrobalID2)
{
	auto& nRefInfos = m_nRefInfos;
	for (int i = 0; i < 32; i++)
	{
		if (nRefInfos[i]->m_nRefID != 0 && nRefInfos[i]->m_nChar[0] == nGrobalID1 && nRefInfos[i]->m_nChar[1] == nGrobalID2)
		{
			nRefInfos[i]->m_nRefID = 0;
		}
	}
}

void cTaskHitMan::RegistPacket()
{
}

void cTaskHitMan::SyncPacket()
{
}

}
