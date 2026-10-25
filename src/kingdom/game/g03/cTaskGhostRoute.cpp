#include "kingdom/game/g03/cTaskGhostRoute.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharGhostBoat.hpp"
#include "kingdom/game/g03/cTaskCharGhostCar.hpp"
#include "kingdom/game/g03/cTaskCharGhostHover.hpp"
#include "kingdom/game/g03/cTaskCharGhostPlane.hpp"
#include "kingdom/game/g03/cTaskCharGhostUfo.hpp"
#include "kingdom/game/g03/cTaskCharNonRideGhost.hpp"
#include "kingdom/game/g03/cTaskCharRouteGhost.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskCharRouteGhost* cTaskGhostRoute::pTaskCharRouteGhost()
{
	return m_pTaskCharRouteGhost;
}

bool cTaskGhostRoute::IsNeedBottomCheck()
{
	return m_bBottomCheck;
}

cTaskGhostRoute::cTaskGhostRoute(sEnemyRouteData& data)
{
	SetName("cTaskGhostRoute");
	Init();
	RegistData(data);
}

void cTaskGhostRoute::Init()
{
	m_pTaskCharRouteGhost = nullptr;
	m_pEnemyRouteData = sEnemyRouteData();
	m_bBottomCheck = false;
}

void cTaskGhostRoute::Term()
{
	if (m_pTaskCharRouteGhost != nullptr)
	{
		m_pTaskCharRouteGhost->Term();
	}
}

bool cTaskGhostRoute::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	if (m_pTaskCharRouteGhost != nullptr && m_pTaskCharRouteGhost->nStat() < 5 && m_pTaskCharRouteGhost->nTargetPointID() < m_pEnemyRouteData.nRouteNum)
	{
		::Constants::jetGeoVecCopy(m_pTaskCharRouteGhost->vMoveTarget(), m_pEnemyRouteData.sEnemyRoute[m_pTaskCharRouteGhost->nTargetPointID()].pos);
		m_pTaskCharRouteGhost->SetTargetSpeed(m_pEnemyRouteData.sEnemyRoute[m_pTaskCharRouteGhost->nTargetPointID()].speed);
	}
	return true;
}

bool cTaskGhostRoute::RegistData(sEnemyRouteData& data)
{
	m_pEnemyRouteData = data;
	uint type = (uint)(m_pEnemyRouteData.sEnemy.type + 1);
	switch (m_pEnemyRouteData.sEnemy.ride)
	{
	case 0:
		ConnectChild(m_pTaskCharRouteGhost = new cTaskCharNonRideGhost(type));
		break;
	case 1:
		ConnectChild(m_pTaskCharRouteGhost = new cTaskCharGhostCar(type));
		break;
	case 2:
		ConnectChild(m_pTaskCharRouteGhost = new cTaskCharGhostUfo(type));
		break;
	case 3:
		ConnectChild(m_pTaskCharRouteGhost = new cTaskCharGhostBoat(type));
		break;
	case 4:
		ConnectChild(m_pTaskCharRouteGhost = new cTaskCharGhostPlane(type));
		break;
	case 5:
		ConnectChild(m_pTaskCharRouteGhost = new cTaskCharGhostHover(type));
		break;
	}
	float num = m_pEnemyRouteData.sEnemyRoute[0].pos[1];
	float num2 = m_pEnemyRouteData.sEnemyRoute[0].pos[1];
	for (uint num3 = 1u; (long)num3 < (long)m_pEnemyRouteData.nRouteNum; num3++)
	{
		if (num < m_pEnemyRouteData.sEnemyRoute[num3].pos[1])
		{
			num = m_pEnemyRouteData.sEnemyRoute[num3].pos[1];
		}
		if (num2 > m_pEnemyRouteData.sEnemyRoute[num3].pos[1])
		{
			num2 = m_pEnemyRouteData.sEnemyRoute[num3].pos[1];
		}
	}
	if (Mathf::Abs(num - num2) > 0.2f)
	{
		m_bBottomCheck = true;
	}
	else
	{
		m_bBottomCheck = false;
	}
	return true;
}

bool cTaskGhostRoute::SetOption()
{
	if (m_pTaskCharRouteGhost != nullptr)
	{
		if (m_pEnemyRouteData.sEnemy.option != 0)
		{
			m_pTaskCharRouteGhost->SetOptionItem(m_pEnemyRouteData.sEnemy.option);
		}
		m_pTaskCharRouteGhost->SetBombType(m_pEnemyRouteData.nBombStatus);
		::Constants::jetGeoVecCopy(m_pTaskCharRouteGhost->vTrans(), m_pEnemyRouteData.sEnemyRoute[0].pos);
		::Constants::jetGeoVecCopy(m_pTaskCharRouteGhost->vStartPos(), m_pTaskCharRouteGhost->vTrans());
		return true;
	}
	return false;
}

bool cTaskGhostRoute::IsRouteGhostAlive()
{
	if (m_pTaskCharRouteGhost != nullptr && m_pTaskCharRouteGhost->nStat() >= 5)
	{
		return false;
	}
	return true;
}

void cTaskGhostRoute::DeadRouteGhost()
{
	if (m_pTaskCharRouteGhost != nullptr)
	{
		m_pTaskCharRouteGhost->FastDead();
	}
}

int cTaskGhostRoute::nRouteNum()
{
	return m_pEnemyRouteData.nRouteNum;
}

}
