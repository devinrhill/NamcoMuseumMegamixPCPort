#include "kingdom/game/g03/cGolvisAIArea.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/AREA_POINT.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskCharGolvis.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

// DOL 0x80113064 [audited: per AUDIT_G03_G07 row]
cGolvisAIArea::cGolvisAIArea(cTaskCharGolvis* pParent, uint nStage)
{
	m_nAreaStep = -1;
	m_nAreaMax = -1;
	m_nStage = nStage;
	m_nMode = 0;
	m_nModeSub = 0;
	::Constants::jetGeoVecSet(m_vPacPos, 0.0f, 0.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecSet(m_vTarget, 0.0f, 0.0f, 0.0f, 1.0f);
	std::fill(std::begin(m_aAreas), std::end(m_aAreas), nullptr);   // memset(+8, 0, 0x40): the entries point into the stage's area table once a search ran
	m_nPacArea = -1;
	m_nPrevPacArea = -1;
	m_nEscapeArea = -1;
	m_pGolvis = pParent;
	m_nMyPrevArea = -1;
	m_nMyNextArea = -1;
	m_nMyArea = -1;
	m_nRefTime = 0;
	m_nRefCounter = 0;
	m_nGiveUpTime = 0;
	m_nGiveUpCounter = 0;
}

// DOL 0x80113138 [audited: class row done in the 43rd session; function not individually re-read]
cGolvisAIArea::~cGolvisAIArea()
{
}

// inlined in Think 0x80113188: AreaStep = 0, AreaMax = areaSearch (0x80112ef0) with the 64-bit block mask (hi 0, lo nParam)
bool cGolvisAIArea::SearchRoute(int nSrc, int nDst, uint nParam)
{
	m_nAreaStep = 0;
	m_nAreaMax = Constants::areaSearch(m_nStage, m_aAreas, 16u, nSrc, nDst, (ulong)nParam);
	if (m_nAreaMax != -1)
	{
		return true;
	}
	return false;
}

// inlined in Think 0x80113188 (areaGetInfo 0x80112bbc, then a scan of the route)
bool cGolvisAIArea::IsOnRoute(int nPos)
{
	AREA_POINT* aREA_POINT = Constants::areaGetInfo(m_nStage, nPos);
	for (int i = 0; i < m_nAreaMax; i++)
	{
		if (aREA_POINT == m_aAreas[i])
		{
			return true;
		}
	}
	return false;
}

// DOL 0x80113178 [audited: per AUDIT_G03_G07 row]
void cGolvisAIArea::SetKind(int nMode, int nRefTime, int nGiveUpTime)
{
	m_nMode = nMode;
	m_nRefTime = nRefTime;
	m_nGiveUpTime = nGiveUpTime;
}

// DOL 0x80113188 [audited: per AUDIT_G03_G07 row] (equal, every branch + the four areaSearch mask arguments checked; 3D-squared distances 0x800a3c28, XZ-squared 0x800a3c74; y window FLOAT_8047bb60 = 1.3)
void cGolvisAIArea::Think(const Vector4& vPos, uint nParam)
{
	bool flag = false;
	if (m_nRefCounter != 0)
	{
		m_nRefCounter--;
	}
	else
	{
		flag = true;
		m_nRefCounter = m_nRefTime;
	}
	if (m_nGiveUpCounter != 0 && !CustomKingdomUtil::IF(--m_nGiveUpCounter))
	{
		m_nMode = 1;
	}
	switch (m_nMode)
	{
	case 2:
		{
			Constants::sFace* outFace;
			cTaskBgHit* outObj;
			if (m_nModeSub == 2)
			{
				float num = ::Constants::bizGeoVecDistance2(vPos, m_pGolvis->vTrans());
				Vector4 v;
				::Constants::jetGeoVecSub(v, vPos, m_pGolvis->vTrans());
				Vector4 outCross = vPos;
				float outDist;
				if (::Constants::JET_ABS(outCross[1] - m_pGolvis->vTrans()[1]) < 1.3f && !Constants::PACN_APP()->pTaskCollision()->CheckVector(0u, m_pGolvis->vTrans(), v, outDist, outCross, outFace, outObj) && num > outDist)
				{
					::Constants::jetGeoVecCopy(m_vTarget, vPos);
					break;
				}
				m_nRefCounter = 0;
				m_nModeSub = 1;
			}
			else if (flag)
			{
				m_nMyArea = Constants::areaGetNo(m_nStage, m_pGolvis->vTrans());
				m_nPacArea = Constants::areaGetNo(m_nStage, vPos);
				if (m_nMyPrevArea == -1)
				{
					m_nMyPrevArea = m_nMyArea;
				}
				if (Constants::areaCheckArrive(m_nStage, m_nMyArea, m_pGolvis->vTrans()))
				{
					m_nMyPrevArea = m_nMyArea;
				}
				if (SearchRoute(m_nMyPrevArea, m_nPacArea, nParam))
				{
					m_nModeSub = 1;
					if (m_nAreaMax < 1)
					{
						float num2 = ::Constants::bizGeoVecDistance2(vPos, m_pGolvis->vTrans());
						Vector4 v2;
						::Constants::jetGeoVecSub(v2, vPos, m_pGolvis->vTrans());
						Vector4 outCross = vPos;
						float outDist2;
						if (::Constants::JET_ABS(outCross[1] - m_pGolvis->vTrans()[1]) < 1.3f && !Constants::PACN_APP()->pTaskCollision()->CheckVector(0u, m_pGolvis->vTrans(), v2, outDist2, outCross, outFace, outObj) && num2 > outDist2)
						{
							m_nModeSub = 2;
							::Constants::jetGeoVecCopy(m_vTarget, vPos);
						}
					}
					if (m_nModeSub == 1)
					{
						if (!IsOnRoute(m_nMyArea))
						{
							m_nMyPrevArea = m_nMyArea;
						}
						::Constants::jetGeoVecCopy(m_vTarget, m_aAreas[m_nAreaStep]->m_vPos);
					}
				}
				else if (SearchRoute(m_nMyPrevArea, m_nPrevPacArea, nParam))
				{
					m_nModeSub = 3;
					m_nGiveUpCounter = m_nGiveUpTime;
					::Constants::jetGeoVecCopy(m_vTarget, m_aAreas[m_nAreaStep]->m_vPos);
				}
				else
				{
					m_nModeSub = 3;
					m_nMode = 1;
				}
				if (m_nMode != 2 && m_aAreas[m_nAreaStep] != nullptr)
				{
					::Constants::jetGeoVecCopy(m_vTarget, m_aAreas[m_nAreaStep]->m_vPos);
				}
			}
			else if (m_aAreas[m_nAreaStep] != nullptr && m_aAreas[m_nAreaStep]->m_fRangeSQ >= ::Constants::bizGeoVecDistance2XZ(m_vTarget, m_pGolvis->vTrans()) && m_nAreaStep < m_nAreaMax - 1)
			{
				::Constants::jetGeoVecCopy(m_vTarget, m_aAreas[++m_nAreaStep]->m_vPos);
			}
			break;
		}
	case 3:
		if (!flag || m_nModeSub == 3)
		{
			break;
		}
		m_nMyArea = Constants::areaGetNo(m_nStage, m_pGolvis->vTrans());
		m_nPacArea = Constants::areaGetNo(m_nStage, vPos);
		m_nEscapeArea = Constants::areaGetFarNo(m_nStage, vPos);
		if (m_nMyArea != m_nPacArea && SearchRoute(m_nMyArea, m_nEscapeArea, (uint)(1 << m_nPacArea)))
		{
			m_nModeSub = 1;
			::Constants::jetGeoVecCopy(m_vTarget, m_aAreas[m_nAreaStep]->m_vPos);
			break;
		}
		m_nModeSub = 3;
		m_nEscapeArea = Constants::areaGetRevNo(m_nStage, m_pGolvis->vTrans(), vPos);
		if (SearchRoute(m_nMyArea, m_nEscapeArea, (uint)(1 << m_nPacArea)))
		{
			m_nModeSub = 1;
			::Constants::jetGeoVecCopy(m_vTarget, m_aAreas[m_nAreaStep]->m_vPos);
		}
		break;
	case 0:
	case 1:
		break;
	}
}

// DOL 0x801137cc [audited: per AUDIT_G03_G07 row] (returns &m_vTarget)
Vector4 cGolvisAIArea::vGoalPos()
{
	return m_vTarget;
}

}
