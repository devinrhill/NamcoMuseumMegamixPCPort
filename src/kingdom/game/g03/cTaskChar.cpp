#include "kingdom/game/g03/cTaskChar.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cHitChar.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskChar::eKIND& cTaskChar::nKind()
{
	return m_nKind;
}

uint& cTaskChar::nType()
{
	return m_nType;
}

uint& cTaskChar::nState()
{
	return m_nState;
}

void cTaskChar::SetState(uint n)
{
	m_nState = n;
}

bool cTaskChar::IsState(uint n)
{
	if (!CustomKingdomUtil::IF(m_nState & n))
	{
		return false;
	}
	return true;
}

void cTaskChar::OnState(uint n)
{
	m_nState |= n;
}

void cTaskChar::OffState(uint n)
{
	m_nState &= ~n;
}

uint& cTaskChar::nStateOld()
{
	return m_nStateOld;
}

void cTaskChar::SetStateOld(uint n)
{
	m_nStateOld = n;
}

bool cTaskChar::IsStateOld(uint n)
{
	if (!CustomKingdomUtil::IF(m_nStateOld & n))
	{
		return false;
	}
	return true;
}

float& cTaskChar::fRadius()
{
	return m_fRadius;
}

uint& cTaskChar::nPushOutLevel()
{
	return m_nPushOutLevel;
}

uint cTaskChar::GetAnmState()
{
	return 0u;
}

Vector4& cTaskChar::vTrans()
{
	return m_vTrans;
}

float cTaskChar::vTrans(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vTrans[n];
}

Vector4& cTaskChar::vTransOld()
{
	return m_vTransOld;
}

float cTaskChar::vTransOld(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vTransOld[n];
}

Vector4& cTaskChar::vMove()
{
	return m_vMove;
}

float cTaskChar::vMove(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vMove[n];
}

Vector4& cTaskChar::vAccel()
{
	return m_vAccel;
}

float cTaskChar::vAccel(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vAccel[n];
}

Vector4& cTaskChar::vRotate()
{
	return m_vRotate;
}

float cTaskChar::vRotate(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vRotate[n];
}

Vector4& cTaskChar::vScale()
{
	return m_vScale;
}

float cTaskChar::vScale(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vScale[n];
}

cTaskModel::cPBModel* cTaskChar::pCurrentModel()
{
	return pPBModel(nType());
}

std::vector<cHitChar*>& cTaskChar::pvecHitChar()
{
	return m_vecHitChar;
}

cHitChar* cTaskChar::pHitChar(int n)
{
	return m_vecHitChar[n];
}

void cTaskChar::HitPushOut(Vector4 vVec, cTaskChar* pChar)
{
	::Constants::jetGeoVecAdd(vTrans(), vTrans(), vVec);
}

void cTaskChar::IjikeStart()
{
}

void cTaskChar::IjikeEnd()
{
}

cTaskChar::cTaskChar()
{
	SetName("cTaskChar");
	Init();
}

void cTaskChar::Init()
{
	m_nKind = cTaskChar::eKIND::eKIND_PACMAN;
	m_nType = 0u;
	m_nState = 0u;
	m_nStateOld = 0u;
	m_fRadius = 0.0f;
	m_nPushOutLevel = 0u;
	::Constants::jetGeoVecUnit(m_vTrans);
	::Constants::jetGeoVecUnit(m_vTransOld);
	::Constants::jetGeoVecUnit(m_vRotate);
	::Constants::jetGeoVecSet(m_vScale, 1.0f, 1.0f, 1.0f, 1.0f);
	::Constants::jetGeoVecZero(m_vMove);
	::Constants::jetGeoVecZero(m_vAccel);
}

void cTaskChar::Term()
{
	cTaskModel::Term();
}

bool cTaskChar::CheckRepetition(AnimationClip* data, uint idx)
{
	CustomKingdomUtil::Assert(nullptr != data, "nullpo");
	if (nullptr == data)
	{
		return false;
	}
	string strB = pModel(nType())->AnimationDic((int)idx);
	// TODO(unity): return data.name.CompareTo(strB) == 0;
	return false;
}

void cTaskChar::SetMaterialAlpha(uint n, byte alpha)
{
	pModel(n);
}

void cTaskChar::Attacked(cHitChar* pRev, cHitChar* pAtk)
{
}

void cTaskChar::HitUpdate()
{
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoMtxTrans(m, m, vTrans());
	::Constants::jetGeoMtxRotZYX(m, m, vRotate());
	for (cHitChar* item : m_vecHitChar)
	{
		if (item->bFlagTest(1u))
		{
			item->UpdateMatrix(m);
		}
	}
}

void cTaskChar::HitListup(std::vector<cHitChar*>& pVec, const int& nSide, const uint& nFlag)
{
	for (cHitChar* item : m_vecHitChar)
	{
		if ((nSide == -1 || (long)item->nSide() == (long)nSide) && ((nFlag != 0) ? item->FlagAccod(1 | nFlag) : item->bFlagTest(1u)))
		{
			pVec.push_back(item);
		}
	}
}

}
