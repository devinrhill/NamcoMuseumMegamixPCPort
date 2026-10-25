#include "kingdom/game/g03/cTaskGhostMan.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomSys.hpp"

#include <iterator>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharGhost.hpp"
#include "kingdom/game/g03/cTaskCharGhostBomb.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/cTaskCharRouteGhost.hpp"
#include "kingdom/game/g03/cTaskGhostArea.hpp"
#include "kingdom/game/g03/cTaskGhostRoute.hpp"
#include "KingdomGlobal.hpp"
#include <ReadFileUtil.hpp>

namespace kingdom::game::g03
{

void cTaskGhostMan::IjikeStart()
{
	m_bIjike = true;
}

void cTaskGhostMan::IjikeEnd()
{
	m_bIjike = false;
}

bool cTaskGhostMan::IsIjike()
{
	return m_bIjike;
}

uint cTaskGhostMan::GetGhostNum()
{
	return m_nAllGhostNum;
}

void cTaskGhostMan::SetGondolaAnim(float frame)
{
	m_fGondolaAnim = frame;
}

float cTaskGhostMan::GetGondolaAnim()
{
	return m_fGondolaAnim;
}

cTaskGhostMan::cTaskGhostMan()
{
	SetName("cTaskGhostMan");
	Init();
	string array[8] = {"BIN_GHOST_AREA_0_SET_DAT", "BIN_GHOST_AREA_1_SET_DAT", "BIN_GHOST_AREA_2_SET_DAT", "BIN_GHOST_AREA_3_SET_DAT", "BIN_GHOST_AREA_4_SET_DAT", "BIN_GHOST_AREA_5_SET_DAT", "BIN_GHOST_AREA_6_SET_DAT", "BIN_GHOST_AREA_7_SET_DAT"};
	int outParamSize;
	for (int i = 0; i < (int)std::size(array); i++)
	{
		sKingdomData* textAsset = KingdomGlobal::KingSys->SearchDataEx(kingdom::ResourceDatabase::GetPath(array[i]));   // the stage archive entry (Unity: "<key>_are" TextAsset)
		if (textAsset != nullptr && 0 < textAsset->nSize)
		{
			std::vector<sEnemyAreaData*> outParam;
			ReadFileUtil::Convert<sEnemyAreaData>(textAsset->pBin, outParam, outParamSize, outParamSize);
			if (!outParam.empty()) ConnectChild(m_pGhostArea[i] = new cTaskGhostArea(*outParam[0]));
		}
	}
	string array2[6] = {"BIN_GHOST_ROUTE_0_SET_DAT", "BIN_GHOST_ROUTE_1_SET_DAT", "BIN_GHOST_ROUTE_2_SET_DAT", "BIN_GHOST_ROUTE_3_SET_DAT", "BIN_GHOST_ROUTE_4_SET_DAT", "BIN_GHOST_ROUTE_5_SET_DAT"};
	for (int j = 0; j < (int)std::size(array2); j++)
	{
		sKingdomData* textAsset2 = KingdomGlobal::KingSys->SearchDataEx(kingdom::ResourceDatabase::GetPath(array2[j]));   // Unity: "<key>_rot"
		if (textAsset2 != nullptr && 0 < textAsset2->nSize)
		{
			std::vector<sEnemyRouteData*> outParam2;
			ReadFileUtil::Convert<sEnemyRouteData>(textAsset2->pBin, outParam2, outParamSize, outParamSize);
			if (!outParam2.empty()) ConnectChild(m_pGhostRoute[j] = new cTaskGhostRoute(*outParam2[0]));
		}
	}
	if (KingdomGlobal::PacnRollSys->IsRetry())
	{
		std::vector<cG03PacnRollSys::sRetryGhost*>& vecGhost = KingdomGlobal::PacnRollSys->pRetryData()->vecGhost;
		if (0 < (int)vecGhost.size())
		{
			cG03PacnRollSys::sRetryGhost* sRetryGhost = vecGhost[0];
			for (uint num = 0u; num < 8; num++)
			{
				if (m_pGhostArea[num] == nullptr)
				{
					continue;
				}
				for (int k = 0; k < 8; k++)
				{
					if (CustomKingdomUtil::IF(sRetryGhost->nAreaGhostFlag[num] & (uint)(1 << k)))
					{
						m_pGhostArea[num]->DeadAreaGhost((uint)k);
					}
				}
			}
			for (uint num2 = 0u; num2 < 6; num2++)
			{
				if (m_pGhostRoute[num2] != nullptr && sRetryGhost->nRouteGhostFlag[num2] != 0)
				{
					m_pGhostRoute[num2]->DeadRouteGhost();
				}
			}
		}
	}
	if (KingdomGlobal::PacnRollSys->nRound() == 13 || KingdomGlobal::PacnRollSys->nRound() == 25)
	{
		for (uint num3 = 0u; num3 < 8; num3++)
		{
			if (m_pGhostArea[num3] != nullptr)
			{
				for (uint num4 = 0u; num4 < 8; num4++)
				{
					m_pGhostArea[num3]->DeadAreaGhost(num4);
				}
			}
		}
		for (uint num5 = 0u; num5 < 6; num5++)
		{
			if (m_pGhostRoute[num5] != nullptr)
			{
				m_pGhostRoute[num5]->DeadRouteGhost();
			}
		}
	}
	if (KingdomGlobal::PacnRollSys->nRound() == 14 && m_pGhostArea[2] != nullptr && m_pGhostArea[2]->pTaskCharAreaGhost(0u) != nullptr)
	{
		m_pGhostArea[2]->pTaskCharAreaGhost(0u)->SetHydeItem(true);
	}
	for (uint num6 = 0u; num6 < 8; num6++)
	{
		if (m_pGhostArea[num6] != nullptr)
		{
			m_pGhostArea[num6]->SetOption();
		}
	}
	for (uint num7 = 0u; num7 < 6; num7++)
	{
		if (m_pGhostRoute[num7] != nullptr)
		{
			m_pGhostRoute[num7]->SetOption();
		}
	}
	m_nEatedGhost = DeadGhostNum();
	cTaskCharGhost* cTaskCharGhost2 = nullptr;
	for (uint num8 = 0u; num8 < 8; num8++)
	{
		if (m_pGhostArea[num8] == nullptr)
		{
			continue;
		}
		for (uint num9 = 0u; num9 < 8; num9++)
		{
			cTaskCharGhost2 = m_pGhostArea[num8]->pTaskCharAreaGhost(num9);
			if (cTaskCharGhost2 != nullptr)
			{
				CustomKingdomUtil::Assert(m_nAllGhostNum < 32, "error");
				m_pTaskCharGhost[m_nAllGhostNum] = cTaskCharGhost2;
				m_nAllGhostNum++;
			}
		}
	}
	for (uint num10 = 0u; num10 < 6; num10++)
	{
		if (m_pGhostRoute[num10] != nullptr)
		{
			cTaskCharGhost2 = m_pGhostRoute[num10]->pTaskCharRouteGhost();
			if (cTaskCharGhost2 != nullptr)
			{
				CustomKingdomUtil::Assert(m_nAllGhostNum < 32, "error");
				m_pTaskCharGhost[m_nAllGhostNum] = cTaskCharGhost2;
				m_nAllGhostNum++;
			}
		}
	}
	for (uint num11 = 0u; num11 < 3; num11++)
	{
		ConnectChild(m_pTaskCharGhostBomb[num11] = new cTaskCharGhostBomb(num11));
	}
}

void cTaskGhostMan::Init()
{
	for (int i = 0; i < 8; i++)
	{
		m_pGhostArea[i] = nullptr;
	}
	for (int j = 0; j < 6; j++)
	{
		m_pGhostRoute[j] = nullptr;
	}
	m_nEatedGhost = 0u;
	m_nEatCombo = 0u;
	m_nAllGhostNum = 0u;
	m_bIjike = false;
	m_bIjikeOld = false;
	m_nChangeArea = 0u;
	m_fGondolaAnim = 0.0f;
	for (int k = 0; k < 32; k++)
	{
		m_pTaskCharGhost[k] = nullptr;
	}
}

void cTaskGhostMan::Term()
{
	auto& pGhostArea = m_pGhostArea;
	for (int i = 0; i < (int)std::size(pGhostArea); i++)
	{
		if (pGhostArea[i] != nullptr)
		{
			pGhostArea[i]->Term();
		}
	}
	for (int i = 0; i < (int)std::size(pGhostArea); i++)
	{
		if (pGhostArea[i] != nullptr)
		{
			pGhostArea[i]->Term();
		}
	}
	auto& pTaskCharGhost = m_pTaskCharGhost;
	for (int i = 0; i < (int)std::size(pTaskCharGhost); i++)
	{
		if (pTaskCharGhost[i] != nullptr)
		{
			pTaskCharGhost[i]->Term();
		}
	}
	auto& pTaskCharGhostBomb = m_pTaskCharGhostBomb;
	for (int i = 0; i < (int)std::size(pTaskCharGhostBomb); i++)
	{
		if (pTaskCharGhostBomb[i] != nullptr)
		{
			pTaskCharGhostBomb[i]->Term();
		}
	}
}

bool cTaskGhostMan::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	cTaskCharPacman* cTaskCharPacman2 = cAppG03PacnRollGame2->pTaskCharPacman();
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	uint num = DeadGhostNum();
	if (m_nEatedGhost != num)
	{
		if (m_nEatedGhost <= num)
		{
			m_nEatCombo++;
			if (m_nEatCombo > 99)
			{
				m_nEatCombo = 99u;
			}
			cTaskCharPacman2->nEatComboCnt() = m_nEatCombo;
			if (m_nEatCombo == 2)
			{
				cTaskCharPacman2->RecoverHp(1u);
			}
			else if (m_nEatCombo == 3)
			{
				cTaskCharPacman2->RecoverHp(2u);
			}
			else if (m_nEatCombo == 4)
			{
				cTaskCharPacman2->RecoverHp(4u);
			}
			else if (m_nEatCombo > 4)
			{
				cTaskCharPacman2->GainLife(1u);
			}
		}
		m_nEatedGhost = num;
	}
	cTaskCharPacman2->nEatEnemy() = m_nEatedGhost;
	if (!m_bIjikeOld && m_bIjike)
	{
		m_nEatCombo = 0u;
	}
	m_bIjikeOld = m_bIjike;
	if (KingdomGlobal::PacnRollSys->nRound() == 14 && cAppG03PacnRollGame2->pTaskBgGimmickSwitch() != nullptr && cAppG03PacnRollGame2->pTaskBgGimmickSwitch()->IsBoot() && m_nChangeArea == 0)
	{
		m_nChangeArea = 1u;
		sKingdomData* textAsset = KingdomGlobal::KingSys->SearchDataEx(kingdom::ResourceDatabase::GetPath("BIN_GHOST_AREA_CHANGE_0_DAT"));
		int outParamSize;
		std::vector<sEnemyAreaData*> outParam;
		if (textAsset != nullptr) ReadFileUtil::Convert<sEnemyAreaData>(textAsset->pBin, outParam, outParamSize, outParamSize);
		if (!outParam.empty())
		{
			m_pGhostArea[1]->ChangeAreaData(*outParam[0]);
			m_pGhostArea[2]->ChangeAreaData(*outParam[0]);
			if (m_pGhostArea[2]->pTaskCharAreaGhost(0u) != nullptr)
			{
				m_pGhostArea[2]->pTaskCharAreaGhost(0u)->SetHydeItem(false);
			}
		}
		textAsset = KingdomGlobal::KingSys->SearchDataEx(kingdom::ResourceDatabase::GetPath("BIN_GHOST_AREA_CHANGE_1_DAT"));
		std::vector<sEnemyAreaData*> outParam2;
		if (textAsset != nullptr) ReadFileUtil::Convert<sEnemyAreaData>(textAsset->pBin, outParam2, outParamSize, outParamSize);
		if (!outParam2.empty())
		{
			m_pGhostArea[3]->ChangeAreaData(*outParam2[0]);
			m_pGhostArea[4]->ChangeAreaData(*outParam2[0]);
		}
		m_nChangeArea = 1u;
	}
	return true;
}

void cTaskGhostMan::SetRetryData()
{
	cG03PacnRollSys::sRetryGhost* sRetryGhost = new cG03PacnRollSys::sRetryGhost();
	for (uint num = 0u; num < 8; num++)
	{
		if (m_pGhostArea[num] == nullptr)
		{
			continue;
		}
		for (int i = 0; i < 8; i++)
		{
			if (!m_pGhostArea[num]->IsAreaGhostDeadOrAlive((uint)i))
			{
				sRetryGhost->nAreaGhostFlag[num] |= (uint)(1 << i);
			}
		}
	}
	for (uint num2 = 0u; num2 < 6; num2++)
	{
		if (m_pGhostRoute[num2] != nullptr && !m_pGhostRoute[num2]->IsRouteGhostAlive())
		{
			sRetryGhost->nRouteGhostFlag[num2] = 1u;
		}
	}
	KingdomGlobal::PacnRollSys->pRetryData()->vecGhost.push_back(sRetryGhost);
}

uint cTaskGhostMan::AllGhostNum()
{
	return m_nAllGhostNum;
}

uint cTaskGhostMan::DeadGhostNum()
{
	uint num = 0u;
	for (uint num2 = 0u; num2 < 8; num2++)
	{
		if (m_pGhostArea[num2] == nullptr)
		{
			continue;
		}
		for (uint num3 = 0u; num3 < 8; num3++)
		{
			if (!m_pGhostArea[num2]->IsAreaGhostDeadOrAlive(num3))
			{
				num++;
			}
		}
	}
	for (uint num4 = 0u; num4 < 6; num4++)
	{
		if (m_pGhostRoute[num4] != nullptr && !m_pGhostRoute[num4]->IsRouteGhostAlive())
		{
			num++;
		}
	}
	return num;
}

uint cTaskGhostMan::LiveGhostNum()
{
	uint num = 0u;
	for (uint num2 = 0u; num2 < 8; num2++)
	{
		if (m_pGhostArea[num2] == nullptr)
		{
			continue;
		}
		for (uint num3 = 0u; num3 < 8; num3++)
		{
			if (m_pGhostArea[num2]->IsAreaGhostDeadOrAlive(num3))
			{
				num++;
			}
		}
	}
	for (uint num4 = 0u; num4 < 6; num4++)
	{
		if (m_pGhostRoute[num4] != nullptr && m_pGhostRoute[num4]->IsRouteGhostAlive())
		{
			num++;
		}
	}
	return num;
}

void cTaskGhostMan::ShotBomb(cTaskCharRouteGhost* pParent)
{
	for (uint num = 0u; num < 3; num++)
	{
		if (m_pTaskCharGhostBomb[num] != nullptr && m_pTaskCharGhostBomb[num]->IsWait())
		{
			m_pTaskCharGhostBomb[num]->SetStart(pParent);
			break;
		}
	}
}

bool cTaskGhostMan::GhostIsActive(uint no)
{
	if (no < 32 && m_pTaskCharGhost[no] != nullptr && !m_pTaskCharGhost[no]->IsDead())
	{
		return true;
	}
	return false;
}

bool cTaskGhostMan::GhostRevive(uint no)
{
	if (no < 32 && m_pTaskCharGhost[no] != nullptr && m_pTaskCharGhost[no]->IsDead())
	{
		m_pTaskCharGhost[no]->Revive();
		return true;
	}
	return false;
}

}
