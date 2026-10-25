#include "kingdom/game/g03/cTaskCharGolvis01.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include <Constants.hpp>
#include <ReadFileUtil.hpp>

namespace kingdom::game::g03
{

cTaskCharGolvis01::LINE_POINT* cTaskCharGolvis01::LINE_POINT::Clone()
{
	// FIXME(cs2cpp): unresolved identifier MemberwiseClone
	// TODO(unity): return (LINE_POINT)MemberwiseClone();
	return new LINE_POINT(*this);   // MemberwiseClone
}

cTaskCharGolvis01::cTaskCharGolvis01(cJetTask* pParent)
	: cTaskCharGolvis(pParent)
{
	SetName("cTaskCharGolvis01");
	Init();
}

void cTaskCharGolvis01::Init()
{
	float num = 0.0f;
	Vector4 v = Vector4();
	sKingdomData* textAsset = KingdomGlobal::KingSys->SearchDataEx(kingdom::ResourceDatabase::GetPath("BIN_BOSS_AREA_SET_DAT"));
	std::vector<sEnemyAreaData*> outParam;
	if (textAsset != nullptr)
	{
		int outSize = 0;
		ReadFileUtil::Convert<sEnemyAreaData>(textAsset->pBin, outParam, outSize, outSize);
	}
	if (!outParam.empty())
	{
		const sEnemyAreaData& area = *outParam[0];
		uint nPointNum = (uint)area.nPointNum;
		cTaskCharGolvis01::LINE_POINT* lINE_POINT = new cTaskCharGolvis01::LINE_POINT();
		for (uint num2 = 0u; num2 <= nPointNum; num2++)
		{
			cTaskCharGolvis01::LINE_POINT* lINE_POINT2 = new cTaskCharGolvis01::LINE_POINT();
			if (num2 != nPointNum)
			{
				::Constants::jetGeoVecCopy(lINE_POINT2->m_vPos, area.Point[num2].point);
			}
			else
			{
				::Constants::jetGeoVecCopy(lINE_POINT2->m_vPos, area.Point[0].point);
			}
			lINE_POINT2->m_vPos[1] += 10.5f;
			float _discard1;
			Constants::sFace* _discard2;
			cTaskBgHit* _discard3;
			if (!Constants::PACN_APP()->pTaskCollision()->CheckBottom(lINE_POINT2->m_vPos, _discard1, lINE_POINT2->m_vPos, _discard2, _discard3))
			{
				lINE_POINT2->m_vPos[1] -= 10.5f;
			}
			if (num2 == 0)
			{
				::Constants::jetGeoVecCopy(vTrans(), lINE_POINT2->m_vPos);
				num = 0.0f;
			}
			else
			{
				num += ::Constants::jetGeoVecDistance(lINE_POINT2->m_vPos, lINE_POINT->m_vPos);
			}
			lINE_POINT2->m_fDist = num;
			lINE_POINT = lINE_POINT2->Clone();
			m_vecLinePoint.push_back(lINE_POINT2);
			::Constants::jetGeoVecAdd(v, v, lINE_POINT2->m_vPos);
		}
		::Constants::jetGeoVecScale(m_vRailCenter, v, 1.0f / (float)(nPointNum + 1));
	}
	Setting(false);
}

void cTaskCharGolvis01::Term()
{
	cTaskCharGolvis::Term();
}

bool cTaskCharGolvis01::IsNearAttack()
{
	float num = ::Constants::fabs(Constants::PACMAN_OBJ()->vTrans(0) - vTrans(0));
	float num2 = ::Constants::fabs(Constants::PACMAN_OBJ()->vTrans(1) - vTrans(1));
	float num3 = ::Constants::fabs(Constants::PACMAN_OBJ()->vTrans(2) - vTrans(2));
	if ((num < 0.2f && num3 < 0.2f) || (num < 4.0f && num3 < 4.0f && num2 > 1.3f))
	{
		return true;
	}
	return false;
}

bool cTaskCharGolvis01::IsForceAttack()
{
	return IsNearAttack();
}

bool cTaskCharGolvis01::ChangeToAttack(int iAttackKind)
{
	if (IsNearAttack())
	{
		return cTaskCharGolvis::ChangeToAttack(6);
	}
	return cTaskCharGolvis::ChangeToAttack(iAttackKind);
}

bool cTaskCharGolvis01::Execute(cJetTask* pParam)
{
	if (!Constants::IS_PAC_PAUSE() && !Constants::PACMAN_OBJ()->IsMiss())
	{
		pHitChar(1)->FlagOff(17u);
		MonsterFrameMove();
	}
	return cTaskCharGolvis::Execute(pParam);
}

void cTaskCharGolvis01::Draw(int nStatus, cJetTask* pParam)
{
	cTaskCharGolvis::Draw(nStatus, pParam);
}

void cTaskCharGolvis01::Setting(bool bRelocate)
{
	cTaskCharGolvis::Setting(bRelocate);
	m_iState = cTaskCharGolvis01::gmEMonsterStateGolvisRail::STATE_GOLVISRAIL_PURSUIT;
	m_fMoveSpeedNow = m_fMoveSpeed;
	m_iForwardDir = 0;
	m_iDirRefreshTime = 0;
	m_iMoveStopTime = 0;
	InitStateFirst();
	m_fLinePosition = 0.0f;
}

void cTaskCharGolvis01::InitStateFirst()
{
	::Constants::jetGeoVecCopy(vTrans(), m_vecLinePoint[0]->m_vPos);
	::Constants::jetGeoVecCopy(vTransOld(), vTrans());
	ChangeToAppear();
	m_bDisp = true;
}

void cTaskCharGolvis01::InitStateStart()
{
	m_iAIMode = 2;
	m_iState = cTaskCharGolvis01::gmEMonsterStateGolvisRail::STATE_GOLVISRAIL_PURSUIT;
}

void cTaskCharGolvis01::ChangeToRevive()
{
	cTaskCharGolvis::ChangeToRevive();
	::Constants::jetGeoVecCopy(vTrans(), m_vecLinePoint[0]->m_vPos);
	::Constants::jetGeoVecCopy(vTransOld(), vTrans());
	m_fLinePosition = 0.0f;
}

void cTaskCharGolvis01::SelectForwardDir()
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::PACN_APP()->pTaskCharPacman()->vTrans());
	if (::Constants::bizMathNearDir(Mathf::Atan2(vTrans()[0] - m_vRailCenter[0], vTrans()[2] - m_vRailCenter[2]), Mathf::Atan2(v[0] - m_vRailCenter[0], v[2] - m_vRailCenter[2])) <= 0)
	{
		m_iForwardDir = 0;
	}
	else
	{
		m_iForwardDir = 1;
	}
}

void cTaskCharGolvis01::MonsterFrameMove_Free()
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::PACN_APP()->pTaskCharPacman()->vTrans());
	if (m_iState != cTaskCharGolvis01::gmEMonsterStateGolvisRail::STATE_GOLVISRAIL_PURSUIT)
	{
		(void)(1);
		return;
	}
	if (0 < m_iMoveStopTime)
	{
		m_iMoveStopTime--;
	}
	else
	{
		if (--m_iDirRefreshTime <= 0)
		{
			m_iDirRefreshTime = 60;
			SelectForwardDir();
		}
		float num = ::Constants::bizGeoVecDistance2XZ(v, vTrans());
		float fTarget = ((num < 100.0f) ? m_fMoveSpeed : ((!(num < 400.0f)) ? (m_fMoveSpeed * 3.0f) : (m_fMoveSpeed * 2.0f)));
		::Constants::bizMathNearValue(m_fMoveSpeedNow, m_fMoveSpeedNow, fTarget, 0.01f);
		if (!HitCheckPacman())
		{
			int iForwardDir = m_iForwardDir;
			if (m_iForwardDir == 0)
			{
				LineMove(1, m_fMoveSpeedNow);
			}
			else
			{
				LineMove(-1, m_fMoveSpeedNow);
			}
			SelectForwardDir();
			if (iForwardDir != m_iForwardDir)
			{
				m_iMoveStopTime = 60;
			}
			else
			{
				m_iForwardDir = iForwardDir;
			}
		}
	}
	if (m_iAIMode == 2)
	{
		if (TryChangeToAttack())
		{
			m_fMoveSpeedNow = m_fMoveSpeed;
		}
		if (TryChangeToLaugh())
		{
			m_fMoveSpeedNow = m_fMoveSpeed;
		}
	}
}

void cTaskCharGolvis01::MonsterFrameMove_Escape()
{
	m_iMoveStopTime = 0;
	if (--m_iDirRefreshTime <= 0)
	{
		m_iDirRefreshTime = 60;
		SelectForwardDir();
	}
	::Constants::bizMathNearValue(m_fMoveSpeedNow, m_fMoveSpeedNow, 0.5f, 0.01f);
	if (m_iForwardDir == 0)
	{
		LineMove(-1, m_fBijikeMoveSpeed);
	}
	else
	{
		LineMove(1, m_fBijikeMoveSpeed);
	}
}

float cTaskCharGolvis01::LineGetPosRot(float& pOut, Vector4& vOut, float fDistance)
{
	pOut = 0.0f;
	vOut = Vector4();
	cTaskCharGolvis01::LINE_POINT* lINE_POINT2;
	cTaskCharGolvis01::LINE_POINT* lINE_POINT = (lINE_POINT2 = nullptr);
	if (fDistance < 0.0f)
	{
		fDistance = m_vecLinePoint[(int)m_vecLinePoint.size() - 1]->m_fDist + fDistance;
		return LineGetPosRot(pOut, vOut, fDistance);
	}
	for (cTaskCharGolvis01::LINE_POINT* item : m_vecLinePoint)
	{
		lINE_POINT = item;
		if (fDistance == lINE_POINT->m_fDist)
		{
			::Constants::jetGeoVecCopy(vOut, lINE_POINT->m_vPos);
			return fDistance;
		}
		if (fDistance < lINE_POINT->m_fDist)
		{
			if (lINE_POINT2 == nullptr)
			{
				::Constants::jetGeoVecCopy(vOut, lINE_POINT->m_vPos);
				return fDistance;
			}
			float num = lINE_POINT->m_fDist - lINE_POINT2->m_fDist;
			float num2 = fDistance - lINE_POINT2->m_fDist;
			Vector4 v;
			::Constants::jetGeoVecSub(v, lINE_POINT->m_vPos, lINE_POINT2->m_vPos);
			::Constants::jetGeoVecScale(v, v, num2 / num);
			::Constants::jetGeoVecAdd(vOut, v, lINE_POINT2->m_vPos);
			pOut = ::Constants::bizGeoVecRadY(lINE_POINT2->m_vPos, lINE_POINT->m_vPos);
			return fDistance;
		}
		lINE_POINT2 = lINE_POINT;
	}
	fDistance -= lINE_POINT->m_fDist;
	return LineGetPosRot(pOut, vOut, fDistance);
}

float cTaskCharGolvis01::LineCalcPosition(const Vector4& vTarget)
{
	int num = (int)LineSearchNearPOINT(vTarget);
	if (num == (int)m_vecLinePoint.size() - 1 || num == 0)
	{
		return m_vecLinePoint[num]->m_fDist;
	}
	Vector4 v;
	::Constants::jetGeoVecSub(v, vTarget, m_vecLinePoint[num]->m_vPos);
	Vector4 v2;
	::Constants::jetGeoVecSub(v2, m_vecLinePoint[num + 1]->m_vPos, m_vecLinePoint[num]->m_vPos);
	Vector4 v3;
	::Constants::jetGeoVecSub(v3, m_vecLinePoint[num - 1]->m_vPos, m_vecLinePoint[num]->m_vPos);
	::Constants::jetGeoVecNormalize(v2, v2);
	::Constants::jetGeoVecNormalize(v3, v3);
	float num2 = ::Constants::jetGeoVecInnerProduct(v2, v);
	float num3 = ::Constants::jetGeoVecInnerProduct(v3, v);
	if (num2 >= 0.0f)
	{
		return m_vecLinePoint[num]->m_fDist + num2;
	}
	if (num3 >= 0.0f)
	{
		return m_vecLinePoint[num]->m_fDist - num3;
	}
	if (num2 < num3)
	{
		return m_vecLinePoint[num]->m_fDist + num2;
	}
	return m_vecLinePoint[num]->m_fDist - num3;
}

uint cTaskCharGolvis01::LineSearchNearPOINT(const Vector4& vTarget)
{
	float num = 6553600.0f;
	uint result;
	uint num2 = (result = 0u);
	for (cTaskCharGolvis01::LINE_POINT* item : m_vecLinePoint)
	{
		Vector4 v;
		::Constants::jetGeoVecSub(v, item->m_vPos, vTarget);
		float num3 = ::Constants::jetGeoVecInnerProduct(v, v);
		if (num > num3)
		{
			num = num3;
			result = num2;
		}
		num2++;
	}
	return result;
}

void cTaskCharGolvis01::LineMove(const int& dir, const float& fSpeed)
{
	if (dir == 1)
	{
		m_fLinePosition += fSpeed;
	}
	if (dir == -1)
	{
		m_fLinePosition -= fSpeed;
	}
	if (dir != 0)
	{
		float _discard4;
		m_fLinePosition = LineGetPosRot(_discard4, vTrans(), m_fLinePosition);
	}
}

Constants::gmSGolvisAttack* cTaskCharGolvis01::GetAttackKind(uint nEatNum, uint nIjikeLevel, int nNum)
{
	return cTaskCharGolvis::GetAttackKind(nEatNum, nIjikeLevel, nNum);
}

}
