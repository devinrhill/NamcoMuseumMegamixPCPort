#include "kingdom/game/g03/cTaskGhostArea.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharAreaGhost.hpp"
#include "kingdom/game/g03/cTaskCharGhostBlinky.hpp"
#include "kingdom/game/g03/cTaskCharGhostClyde.hpp"
#include "kingdom/game/g03/cTaskCharGhostInky.hpp"
#include "kingdom/game/g03/cTaskCharGhostPinky.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

bool cTaskGhostArea::IsAreaInPacMan()
{
	if (m_nPlayerAreaFlag == 0L)
	{
		return false;
	}
	return true;
}

cTaskCharAreaGhost* cTaskGhostArea::pTaskCharAreaGhost(uint id)
{
	if (id >= 8)
	{
		return nullptr;
	}
	return m_pTaskCharGhost[id];
}

bool cTaskGhostArea::IsNeedBottomCheck()
{
	return m_bBottomCheck;
}

uint cTaskGhostArea::GhostNum()
{
	return m_nGhostNum;
}

bool cTaskGhostArea::IsAreaFusion()
{
	return m_bAreaFusion;
}

cTaskGhostArea::cTaskGhostArea(sEnemyAreaData& data)
{
	SetName("cTaskGhostArea");
	Init();
	RegistData(data);
}

void cTaskGhostArea::Init()
{
	m_pEnemyAreaData = sEnemyAreaData();
	for (int i = 0; i < 8; i++)
	{
		m_pTaskCharGhost[i] = nullptr;
	}
	std::fill(std::begin(m_nAreaPointFlag), std::end(m_nAreaPointFlag), 0u);
	std::fill(std::begin(m_nPointToPointFlag), std::end(m_nPointToPointFlag), 0u);
	std::fill(std::begin(m_nAreaToAreaFlag), std::end(m_nAreaToAreaFlag), 0u);
	m_nGhostNum = 0u;
	m_bAreaFusion = false;
	for (auto& row : m_fAreaInside)
	{
		std::fill(std::begin(row), std::end(row), 0.0f);
	}
	m_nPlayerAreaFlag = 0uL;
	m_nSearchCount = 0u;
	m_bBottomCheck = false;
}

void cTaskGhostArea::Term()
{
	auto& pTaskCharGhost = m_pTaskCharGhost;
	for (int i = 0; i < (int)std::size(pTaskCharGhost); i++)
	{
		if (pTaskCharGhost[i] != nullptr)
		{
			pTaskCharGhost[i]->Term();
		}
	}
}

bool cTaskGhostArea::Execute(cJetTask* pParam)
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	m_nPlayerAreaFlag = 0uL;
	if (!cTaskCharPacman2->IsState(16u) && m_fAreaInside[0][0] > cTaskCharPacman2->vTrans()[0] && m_fAreaInside[0][1] < cTaskCharPacman2->vTrans()[0] && m_fAreaInside[1][0] > cTaskCharPacman2->vTrans()[1] && m_fAreaInside[1][1] < cTaskCharPacman2->vTrans()[1] && m_fAreaInside[2][0] > cTaskCharPacman2->vTrans()[2] && m_fAreaInside[2][1] < cTaskCharPacman2->vTrans()[2])
	{
		for (int i = 0; i < m_pEnemyAreaData.nAreaNum; i++)
		{
			if (IsIntoArea(cTaskCharPacman2->vTrans(), i))
			{
				m_nPlayerAreaFlag |= (uint)(1 << i);
			}
		}
	}
	if (m_nPlayerAreaFlag != 0L)
	{
		m_nSearchCount++;
		if (m_nSearchCount > 8)
		{
			m_nSearchCount = 0u;
		}
	}
	else
	{
		m_nSearchCount = 0u;
	}
	for (int j = 0; j < 8; j++)
	{
		if (m_pTaskCharGhost[j] == nullptr || m_pTaskCharGhost[j]->nStat() >= 8)
		{
			continue;
		}
		switch (m_pTaskCharGhost[j]->nMoveStatus())
		{
		case 1:
			{
				int num4 = 0;
				m_pTaskCharGhost[j]->nTargetPointID();
				do
				{
					num4 = Random::Range(0, m_pEnemyAreaData.nPointNum);
				}
				while (m_pTaskCharGhost[j]->nTargetPointID() == num4);
				m_pTaskCharGhost[j]->nTargetPointID() = num4;
				::Constants::jetGeoVecCopy(m_pTaskCharGhost[j]->vTarget(), m_pEnemyAreaData.Point[num4].point);
				::Constants::jetGeoVecCopy(m_pTaskCharGhost[j]->vMoveTarget(), m_pEnemyAreaData.Point[num4].point);
				ulong num5 = 0uL;
				ulong num6 = 0uL;
				bool flag = false;
				for (int m = 0; m < m_pEnemyAreaData.nAreaNum; m++)
				{
					int num7 = 0;
					if (IsIntoArea(m_pTaskCharGhost[j]->vTrans(), m))
					{
						num5 |= (uint)(1 << m);
						num7++;
					}
					if (IsIntoArea(m_pTaskCharGhost[j]->vTarget(), m))
					{
						num6 |= (uint)(1 << m);
						num7++;
					}
					if (num7 == 2)
					{
						flag = true;
						break;
					}
				}
				if (flag)
				{
					break;
				}
				int num8 = -1;
				int num9 = -1;
				int next_area_id = -1;
				int num10 = 13;
				for (int n = 0; n < m_pEnemyAreaData.nAreaNum; n++)
				{
					if (CustomKingdomUtil::IF(num5 & (uint)(1 << n)))
					{
						int num11 = SearchRoute(num10, next_area_id, n, num6, 0uL);
						if (num11 > -1 && num10 > num11)
						{
							num10 = num11;
							num8 = n;
							num9 = next_area_id;
						}
					}
				}
				if (num8 == -1)
				{
					break;
				}
				for (int num12 = 0; num12 < m_pEnemyAreaData.nPointNum; num12++)
				{
					if (CustomKingdomUtil::IF(m_nAreaPointFlag[num8] & (uint)(1 << num12)) && CustomKingdomUtil::IF(m_nAreaPointFlag[num9] & (uint)(1 << num12)))
					{
						::Constants::jetGeoVecCopy(m_pTaskCharGhost[j]->vMoveTarget(), m_pEnemyAreaData.Point[num12].point);
						break;
					}
				}
				break;
			}
		case 2:
			{
				m_pTaskCharGhost[j]->nTargetPointID();
				int num13 = m_pTaskCharGhost[j]->nTargetPointID();
				::Constants::jetGeoVecCopy(m_pTaskCharGhost[j]->vTarget(), m_pEnemyAreaData.Point[num13].point);
				::Constants::jetGeoVecCopy(m_pTaskCharGhost[j]->vMoveTarget(), m_pEnemyAreaData.Point[num13].point);
				ulong num14 = 0uL;
				ulong num15 = 0uL;
				bool flag2 = false;
				for (int num16 = 0; num16 < m_pEnemyAreaData.nAreaNum; num16++)
				{
					int num17 = 0;
					if (IsIntoArea(m_pTaskCharGhost[j]->vTrans(), num16))
					{
						num14 |= (uint)(1 << num16);
						num17++;
					}
					if (IsIntoArea(m_pTaskCharGhost[j]->vTarget(), num16))
					{
						num15 |= (uint)(1 << num16);
						num17++;
					}
					if (num17 == 2)
					{
						flag2 = true;
						break;
					}
				}
				if (flag2)
				{
					break;
				}
				int num18 = 13;
				int next_area_id2 = -1;
				int num19 = -1;
				int num20 = -1;
				for (int num21 = 0; num21 < m_pEnemyAreaData.nAreaNum; num21++)
				{
					if (CustomKingdomUtil::IF(num14 & (uint)(1 << num21)))
					{
						int num22 = SearchRoute(num18, next_area_id2, num21, num15, 0uL);
						if (num22 > -1 && num18 > num22)
						{
							num18 = num22;
							num19 = num21;
							num20 = next_area_id2;
						}
					}
				}
				if (num19 == -1)
				{
					break;
				}
				for (int num23 = 0; num23 < m_pEnemyAreaData.nPointNum; num23++)
				{
					if (CustomKingdomUtil::IF(m_nAreaPointFlag[num19] & (uint)(1 << num23)) && CustomKingdomUtil::IF(m_nAreaPointFlag[num20] & (uint)(1 << num23)))
					{
						::Constants::jetGeoVecCopy(m_pTaskCharGhost[j]->vMoveTarget(), m_pEnemyAreaData.Point[num23].point);
						break;
					}
				}
				break;
			}
		case 3:
			{
				if (!CustomKingdomUtil::IF(m_nPlayerAreaFlag) || (long)m_nSearchCount != (long)j)
				{
					break;
				}
				::Constants::jetGeoVecCopy(m_pTaskCharGhost[j]->vTarget(), cTaskCharPacman2->vTrans());
				::Constants::jetGeoVecCopy(m_pTaskCharGhost[j]->vMoveTarget(), cTaskCharPacman2->vTrans());
				ulong num24 = 0uL;
				bool flag3 = false;
				for (int num25 = 0; num25 < m_pEnemyAreaData.nAreaNum; num25++)
				{
					if (IsIntoArea(m_pTaskCharGhost[j]->vTrans(), num25))
					{
						num24 |= (uint)(1 << num25);
						if (CustomKingdomUtil::IF(m_nPlayerAreaFlag & (uint)(1 << num25)))
						{
							flag3 = true;
							break;
						}
					}
				}
				if (flag3)
				{
					break;
				}
				int num26 = 13;
				int num27 = -1;
				int num28 = -1;
				int next_area_id3 = -1;
				for (int num29 = 0; num29 < m_pEnemyAreaData.nAreaNum; num29++)
				{
					if (CustomKingdomUtil::IF(num24 & (uint)(1 << num29)))
					{
						int num30 = SearchRoute(num26, next_area_id3, num29, m_nPlayerAreaFlag, 0uL);
						if (num30 > -1 && num26 > num30)
						{
							num26 = num30;
							num27 = num29;
							num28 = next_area_id3;
						}
					}
				}
				if (num27 == -1)
				{
					break;
				}
				for (int num31 = 0; num31 < m_pEnemyAreaData.nPointNum; num31++)
				{
					if (CustomKingdomUtil::IF(m_nAreaPointFlag[num27] & (uint)(1 << num31)) && CustomKingdomUtil::IF(m_nAreaPointFlag[num28] & (uint)(1 << num31)))
					{
						::Constants::jetGeoVecCopy(m_pTaskCharGhost[j]->vMoveTarget(), m_pEnemyAreaData.Point[num31].point);
						break;
					}
				}
				break;
			}
		case 4:
			{
				if (!CustomKingdomUtil::IF(m_nPlayerAreaFlag) || (long)m_nSearchCount != (long)j)
				{
					break;
				}
				::Constants::jetGeoVecDistance(cTaskCharPacman2->vTrans(), m_pTaskCharGhost[j]->vTrans());
				float num = 0.0f;
				int num2 = -1;
				float array[2] = {m_pTaskCharGhost[j]->vTrans()[0] - cTaskCharPacman2->vTrans()[0], m_pTaskCharGhost[j]->vTrans()[2] - cTaskCharPacman2->vTrans()[2]};
				for (int k = 0; k < m_pEnemyAreaData.nAreaNum; k++)
				{
					if (!IsIntoArea(m_pTaskCharGhost[j]->vTrans(), k))
					{
						continue;
					}
					for (int l = 0; l < m_pEnemyAreaData.nPointNum; l++)
					{
						if (!CustomKingdomUtil::IF(m_nAreaPointFlag[k] & (uint)(1 << l)))
						{
							continue;
						}
						float array2[2] = {m_pEnemyAreaData.Point[l].point[0] - cTaskCharPacman2->vTrans()[0], m_pEnemyAreaData.Point[l].point[2] - cTaskCharPacman2->vTrans()[2]};
						float x = array[0] * array2[0] + array[1] * array2[1];
						float y = array[0] * array2[1] - array[1] * array2[0];
						if (57.29578f * Mathf::Atan2(y, x) < 180.0f)
						{
							float num3 = ::Constants::jetGeoVecDistance(cTaskCharPacman2->vTrans(), m_pEnemyAreaData.Point[l].point);
							if (num == 0.0f)
							{
								num = num3;
								num2 = l;
							}
							else if (num < num3)
							{
								num = num3;
								num2 = l;
							}
						}
					}
				}
				if (num2 != -1)
				{
					::Constants::jetGeoVecCopy(m_pTaskCharGhost[j]->vMoveTarget(), m_pEnemyAreaData.Point[num2].point);
				}
				break;
			}
		}
	}
	return true;
}

bool cTaskGhostArea::RegistData(sEnemyAreaData& data)
{
	m_pEnemyAreaData = data;
	for (int i = 0; i < m_pEnemyAreaData.nEnmeyNum; i++)
	{
		cTaskCharAreaGhost* cTaskCharAreaGhost2 = nullptr;
		switch (m_pEnemyAreaData.Enemy[i].type)
		{
		case 0:
			ConnectChild(cTaskCharAreaGhost2 = new cTaskCharGhostClyde());
			break;
		case 1:
			ConnectChild(cTaskCharAreaGhost2 = new cTaskCharGhostPinky());
			break;
		case 2:
			ConnectChild(cTaskCharAreaGhost2 = new cTaskCharGhostBlinky());
			break;
		case 3:
			ConnectChild(cTaskCharAreaGhost2 = new cTaskCharGhostInky());
			break;
		}
		if (cTaskCharAreaGhost2 != nullptr && i < 8)
		{
			m_pTaskCharGhost[i] = cTaskCharAreaGhost2;
			::Constants::jetGeoVecCopy(cTaskCharAreaGhost2->vTrans(), m_pEnemyAreaData.Enemy[i].pos);
			::Constants::jetGeoVecCopy(cTaskCharAreaGhost2->vStartPos(), m_pEnemyAreaData.Enemy[i].pos);
			m_nGhostNum++;
		}
	}
	for (int j = 0; j < m_pEnemyAreaData.nAreaNum; j++)
	{
		for (int k = 0; k < m_pEnemyAreaData.nPointNum; k++)
		{
			if (IsIntoArea(m_pEnemyAreaData.Point[k].point, j))
			{
				m_nAreaPointFlag[j] |= (uint)(1 << k);
			}
		}
	}
	for (int l = 0; l < m_pEnemyAreaData.nAreaNum; l++)
	{
		for (int m = 0; m < m_pEnemyAreaData.nPointNum; m++)
		{
			if (!CustomKingdomUtil::IF(m_nAreaPointFlag[l] & (uint)(1 << m)))
			{
				continue;
			}
			for (int n = 0; n < m_pEnemyAreaData.nPointNum; n++)
			{
				if (m != n && CustomKingdomUtil::IF(m_nAreaPointFlag[l] & (uint)(1 << n)))
				{
					m_nPointToPointFlag[m] |= (uint)(1 << n);
				}
			}
		}
	}
	for (int num = 0; num < m_pEnemyAreaData.nAreaNum; num++)
	{
		for (int num2 = 0; num2 < m_pEnemyAreaData.nPointNum; num2++)
		{
			if (!CustomKingdomUtil::IF(m_nAreaPointFlag[num] & (uint)(1 << num2)))
			{
				continue;
			}
			for (int num3 = 0; num3 < m_pEnemyAreaData.nAreaNum; num3++)
			{
				if (num != num3 && CustomKingdomUtil::IF(m_nAreaPointFlag[num3] & (uint)(1 << num2)))
				{
					m_nAreaToAreaFlag[num] |= (uint)(1 << num3);
				}
			}
		}
	}
	for (int num4 = 0; num4 < 3; num4++)
	{
		m_fAreaInside[num4][0] = m_pEnemyAreaData.Area[0].pos[0][num4];
		m_fAreaInside[num4][1] = m_pEnemyAreaData.Area[0].pos[0][num4];
	}
	for (int num5 = 0; num5 < m_pEnemyAreaData.nAreaNum; num5++)
	{
		int num6 = 0;
		num6 = ((m_pEnemyAreaData.Area[num5].kind != 0) ? 4 : 3);
		for (int num7 = 0; num7 < num6; num7++)
		{
			for (int num8 = 0; num8 < 3; num8++)
			{
				if (m_fAreaInside[num8][0] < m_pEnemyAreaData.Area[num5].pos[num7][num8])
				{
					m_fAreaInside[num8][0] = m_pEnemyAreaData.Area[num5].pos[num7][num8];
				}
				if (m_fAreaInside[num8][1] > m_pEnemyAreaData.Area[num5].pos[num7][num8])
				{
					m_fAreaInside[num8][1] = m_pEnemyAreaData.Area[num5].pos[num7][num8];
				}
			}
		}
	}
	if (Mathf::Abs(m_fAreaInside[1][0] - m_fAreaInside[1][1]) > 0.2f)
	{
		m_bBottomCheck = true;
	}
	else
	{
		m_bBottomCheck = false;
	}
	m_fAreaInside[1][0] += 0.5f;
	return true;
}

bool cTaskGhostArea::ChangeAreaData(sEnemyAreaData& data)
{
	std::fill(std::begin(m_nAreaToAreaFlag), std::end(m_nAreaToAreaFlag), 0u);
	std::fill(std::begin(m_nPointToPointFlag), std::end(m_nPointToPointFlag), 0u);
	std::fill(std::begin(m_nAreaToAreaFlag), std::end(m_nAreaToAreaFlag), 0u);
	m_pEnemyAreaData = data;
	m_bAreaFusion = true;
	for (uint num = 0u; num < 8; num++)
	{
		if (m_pTaskCharGhost[num] != nullptr && IsAreaGhostDeadOrAlive(num))
		{
			m_pTaskCharGhost[num]->ChangeArea();
		}
	}
	for (int i = 0; i < m_pEnemyAreaData.nAreaNum; i++)
	{
		for (int j = 0; j < m_pEnemyAreaData.nPointNum; j++)
		{
			if (IsIntoArea(m_pEnemyAreaData.Point[j].point, i))
			{
				m_nAreaPointFlag[i] |= (uint)(1 << j);
			}
		}
	}
	for (int k = 0; k < m_pEnemyAreaData.nAreaNum; k++)
	{
		for (int l = 0; l < m_pEnemyAreaData.nPointNum; l++)
		{
			if (!CustomKingdomUtil::IF(m_nAreaPointFlag[k] & (uint)(1 << l)))
			{
				continue;
			}
			for (int m = 0; m < m_pEnemyAreaData.nPointNum; m++)
			{
				if (l != m && CustomKingdomUtil::IF(m_nAreaPointFlag[k] & (uint)(1 << m)))
				{
					m_nPointToPointFlag[l] |= (uint)(1 << m);
				}
			}
		}
	}
	for (int n = 0; n < m_pEnemyAreaData.nAreaNum; n++)
	{
		for (int num2 = 0; num2 < m_pEnemyAreaData.nPointNum; num2++)
		{
			if (!CustomKingdomUtil::IF(m_nAreaPointFlag[n] & (uint)(1 << num2)))
			{
				continue;
			}
			for (int num3 = 0; num3 < m_pEnemyAreaData.nAreaNum; num3++)
			{
				if (n != num3 && CustomKingdomUtil::IF(m_nAreaPointFlag[num3] & (uint)(1 << num2)))
				{
					m_nAreaToAreaFlag[n] |= (uint)(1 << num3);
				}
			}
		}
	}
	for (int num4 = 0; num4 < 3; num4++)
	{
		m_fAreaInside[num4][0] = m_pEnemyAreaData.Area[0].pos[0][num4];
		m_fAreaInside[num4][1] = m_pEnemyAreaData.Area[0].pos[0][num4];
	}
	for (int num5 = 0; num5 < m_pEnemyAreaData.nAreaNum; num5++)
	{
		int num6 = 0;
		num6 = ((m_pEnemyAreaData.Area[num5].kind != 0) ? 4 : 3);
		for (int num7 = 0; num7 < num6; num7++)
		{
			for (int num8 = 0; num8 < 3; num8++)
			{
				if (m_fAreaInside[num8][0] < m_pEnemyAreaData.Area[num5].pos[num7][num8])
				{
					m_fAreaInside[num8][0] = m_pEnemyAreaData.Area[num5].pos[num7][num8];
				}
				if (m_fAreaInside[num8][1] > m_pEnemyAreaData.Area[num5].pos[num7][num8])
				{
					m_fAreaInside[num8][1] = m_pEnemyAreaData.Area[num5].pos[num7][num8];
				}
			}
		}
	}
	if (Mathf::Abs(m_fAreaInside[1][0] - m_fAreaInside[1][1]) > 0.2f)
	{
		m_bBottomCheck = true;
	}
	else
	{
		m_bBottomCheck = false;
	}
	m_fAreaInside[1][0] += 0.5f;
	data = m_pEnemyAreaData;
	return true;
}

bool cTaskGhostArea::SetOption()
{
	for (int i = 0; i < 8; i++)
	{
		if (m_pTaskCharGhost[i] != nullptr && m_pEnemyAreaData.Enemy[i].option != 0)
		{
			m_pTaskCharGhost[i]->SetOptionItem(m_pEnemyAreaData.Enemy[i].option);
		}
	}
	return true;
}

void cTaskGhostArea::ReSetGhost()
{
	for (int i = 0; i < 8; i++)
	{
		if (m_pTaskCharGhost[i] != nullptr && m_pTaskCharGhost[i]->nStat() == 9)
		{
			m_pTaskCharGhost[i]->nStat() = 10;
		}
	}
}

bool cTaskGhostArea::IsAreaGhostDeadOrAlive(uint id)
{
	if (id < 8 && m_pTaskCharGhost[id] != nullptr && m_pTaskCharGhost[id]->nStat() >= 8)
	{
		return false;
	}
	return true;
}

void cTaskGhostArea::DeadAreaGhost(uint id)
{
	if (id < 8 && m_pTaskCharGhost[id] != nullptr)
	{
		m_pTaskCharGhost[id]->FastDead();
	}
}

bool cTaskGhostArea::IsIntoAreaPos(Vector4 pos)
{
	for (int i = 0; i < m_pEnemyAreaData.nAreaNum; i++)
	{
		if (IsIntoArea(pos, i))
		{
			return true;
		}
	}
	return false;
}

bool cTaskGhostArea::IsIntoArea(Vector4 pos, int area_id)
{
	if (m_pEnemyAreaData.Area[area_id].kind == 0)
	{
		int array[3] = {};
		Vector2 pos2 = Vector2();
		Vector2 array2[2] = {};
		pos2[0] = pos[0];
		pos2[1] = pos[2];
		for (uint num = 0u; num < 3; num++)
		{
			array2[0][0] = m_pEnemyAreaData.Area[area_id].pos[num][0];
			array2[0][1] = m_pEnemyAreaData.Area[area_id].pos[num][2];
			array2[1][0] = m_pEnemyAreaData.Area[area_id].pos[(num + 1) % 3][0];
			array2[1][1] = m_pEnemyAreaData.Area[area_id].pos[(num + 1) % 3][2];
			array[num] = PointLineSide(pos2, array2[0], array2[1]);
			if (array[num] == 0)
			{
				return false;
			}
		}
		if (array[0] == array[1] && array[1] == array[2])
		{
			return true;
		}
	}
	else if (m_pEnemyAreaData.Area[area_id].kind == 1)
	{
		int array3[4] = {};
		Vector2 pos3 = Vector2();
		Vector2 array4[2] = {};
		pos3[0] = pos[0];
		pos3[1] = pos[2];
		for (uint num2 = 0u; num2 < 4; num2++)
		{
			array4[0][0] = m_pEnemyAreaData.Area[area_id].pos[num2][0];
			array4[0][1] = m_pEnemyAreaData.Area[area_id].pos[num2][2];
			array4[1][0] = m_pEnemyAreaData.Area[area_id].pos[(num2 + 1) % 4][0];
			array4[1][1] = m_pEnemyAreaData.Area[area_id].pos[(num2 + 1) % 4][2];
			array3[num2] = PointLineSide(pos3, array4[0], array4[1]);
			if (array3[num2] == 0)
			{
				return false;
			}
		}
		if (array3[0] == array3[1] && array3[1] == array3[2] && array3[2] == array3[3])
		{
			return true;
		}
	}
	return false;
}

int cTaskGhostArea::PointLineSide(Vector2 pos, Vector2 seg1, Vector2 seg2)
{
	float num = 0.0f;
	Vector2 vector = Vector2();
	Vector2 vector2 = Vector2();
	vector[0] = seg2[0] - seg1[0];
	vector[1] = seg2[1] - seg1[1];
	vector2[0] = pos[0] - seg1[0];
	vector2[1] = pos[1] - seg1[1];
	num = vector[0] * vector2[1] - vector[1] * vector2[0];
	if (num > 0.0f)
	{
		return 1;
	}
	if (num < 0.0f)
	{
		return -1;
	}
	return 0;
}

int cTaskGhostArea::SearchRoute(int result, int& next_area_id, int area_id, ulong target_id_flag, ulong through, int count)
{
	int num = 100;
	bool flag = false;
	int num2 = 0;
	if (count > result || CustomKingdomUtil::IF(through & (uint)(1 << area_id)))
	{
		return -1;
	}
	through |= (uint)(1 << area_id);
	for (num2 = 0; num2 < m_pEnemyAreaData.nAreaNum; num2++)
	{
		if (CustomKingdomUtil::IF(m_nAreaToAreaFlag[area_id] & (uint)(1 << num2)) && CustomKingdomUtil::IF(target_id_flag & (uint)(1 << num2)))
		{
			next_area_id = num2;
			return count;
		}
	}
	for (num2 = 0; num2 < m_pEnemyAreaData.nAreaNum; num2++)
	{
		if (CustomKingdomUtil::IF(m_nAreaToAreaFlag[area_id] & (uint)(1 << num2)) && !CustomKingdomUtil::IF(through & (uint)(1 << num2)))
		{
			int num3 = SearchRoute(result, next_area_id, num2, target_id_flag, through, count + 1);
			if (num3 > 0 && result > num3)
			{
				result = num3;
				num = num2;
				flag = true;
			}
		}
	}
	if (flag)
	{
		if (count == 0)
		{
			next_area_id = num;
		}
		return result;
	}
	return -1;
}

}
