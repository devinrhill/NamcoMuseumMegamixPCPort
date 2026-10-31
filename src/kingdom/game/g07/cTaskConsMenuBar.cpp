#include "kingdom/game/g07/cTaskConsMenuBar.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

#include <limits>

#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g07
{

const int cTaskConsMenuBar::ANM_BAR_LIST[3] = {323, 324, 325};
const int cTaskConsMenuBar::ANM_MOTOS_LIST[3] = {51, 52, 53};
const int cTaskConsMenuBar::ANM_NUMBER_LIST[10][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}, {12, 13, 14}, {15, 16, 17}, {18, 19, 20}, {21, 22, 23}, {24, 25, 26}, {27, 28, 29}};
const int cTaskConsMenuBar::ANM_CONMA_LIST[3] = {30, 31, 32};
const int cTaskConsMenuBar::SET_MOTOS_PAT = 13;
const uint cTaskConsMenuBar::SET_MOTOS_LAY[2] = {0u, 1u};
const int cTaskConsMenuBar::SET_SCORE_PAT = 0;
const uint cTaskConsMenuBar::SET_SCORE_LAY[6] = {5u, 4u, 3u, 2u, 1u, 0u};
const int cTaskConsMenuBar::SET_TIME_PAT = 1;
const uint cTaskConsMenuBar::SET_TIME_LAY[6] = {7u, 6u, 4u, 3u, 1u, 0u};
const int cTaskConsMenuBar::SET_CONMA_PAT = 1;
const uint cTaskConsMenuBar::SET_CONMA_LAY[2] = {2u, 5u};

uint& cTaskConsMenuBar::nWorldNo()
{
	return m_nWorldNo;
}

uint& cTaskConsMenuBar::nStageNo()
{
	return m_nStageNo;
}

uint& cTaskConsMenuBar::nScore()
{
	return m_nScore;
}

uint& cTaskConsMenuBar::nTime()
{
	return m_nTime;
}

bool& cTaskConsMenuBar::bScoreNew()
{
	return m_bScoreNew;
}

bool& cTaskConsMenuBar::bTimeNew()
{
	return m_bTimeNew;
}

// DOL 0x801c6254 [audited: bar / motos / number clip ids vs the DOL tables, step logic vs decomp; digit layout loops not re-read line by line]
cTaskConsMenuBar::cTaskConsMenuBar(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(2147483650u);
	nWorldNo() = std::numeric_limits<uint>::max();
	nStageNo() = std::numeric_limits<uint>::max();
	nScore() = 0u;
	nTime() = 0u;
	bScoreNew() = false;
	bTimeNew() = false;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
	pAnita(0u)->SetAnimNo(ANM_BAR_LIST[1]);
	pAnita(0u)->SetState(1u, false);
	uint num = 1u;
	uint num2 = 0u;
	while (num <= 2)
	{
		pAnita(num)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
		pAnita(num)->SetAnimNo(ANM_MOTOS_LIST[1]);
		pAnita(num)->SetState(1u, false);
		pAnita(num)->GetLayerPos(pAnita(num)->vTrans(), SET_MOTOS_LAY[num2], SET_MOTOS_PAT);
		num++;
		num2++;
	}
	uint num3 = 3u;
	uint num4 = 0u;
	while (num3 <= 8)
	{
		pAnita(num3)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
		pAnita(num3)->SetAnimNo(ANM_NUMBER_LIST[0][1]);
		pAnita(num3)->SetState(1u, false);
		pAnita(num3)->GetLayerPos(pAnita(num3)->vTrans(), SET_SCORE_LAY[num4], SET_SCORE_PAT);
		num3++;
		num4++;
	}
	uint num5 = 9u;
	uint num6 = 0u;
	while (num5 <= 14)
	{
		pAnita(num5)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
		pAnita(num5)->SetAnimNo(ANM_NUMBER_LIST[0][1]);
		pAnita(num5)->SetState(1u, false);
		pAnita(num5)->GetLayerPos(pAnita(num5)->vTrans(), SET_TIME_LAY[num6], SET_TIME_PAT);
		num5++;
		num6++;
	}
	uint num7 = 15u;
	uint num8 = 0u;
	while (num7 <= 16)
	{
		pAnita(num7)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
		pAnita(num7)->SetAnimNo(ANM_CONMA_LIST[1]);
		pAnita(num7)->SetState(1u, false);
		pAnita(num7)->GetLayerPos(pAnita(num7)->vTrans(), SET_CONMA_LAY[num8], SET_CONMA_PAT);
		num7++;
		num8++;
	}
}

cTaskConsMenuBar::~cTaskConsMenuBar()
{
}

// DOL 0x801c6678 [audited: bar / motos / number clip ids vs the DOL tables, step logic vs decomp; digit layout loops not re-read line by line]
void cTaskConsMenuBar::SetAnmScore(int nType)
{
	for (uint num = 3u; num <= 8; num++)
	{
		uint num2 = 10u;
		uint num3 = 1u;
		for (uint num4 = 0u; num4 < 8 - num; num4++)
		{
			num2 *= 10;
			num3 *= 10;
		}
		if (8 == num || nScore() >= num3)
		{
			uint num5 = nScore() % num2 / num3;
			pAnita(num)->SetAnimNo(ANM_NUMBER_LIST[num5][nType], false);
			pAnita(num)->SetState(1u);
		}
		else
		{
			pAnita(num)->SetAnimNo(ANM_NUMBER_LIST[0][nType], false);
			pAnita(num)->SetState(1u, false);
		}
	}
	pAnita(1u)->SetAnimNo(ANM_MOTOS_LIST[nType]);
	pAnita(1u)->SetState(1u, bScoreNew());
}

// DOL 0x801c6828 [audited: bar / motos / number clip ids vs the DOL tables, step logic vs decomp; digit layout loops not re-read line by line]
void cTaskConsMenuBar::SetAnmTime(int nType)
{
	uint num = nTime() / 60000;
	uint num2 = nTime() % 60000 / 1000;
	uint num3 = nTime() % 1000 / 10;
	for (uint num4 = 9u; num4 <= 10; num4++)
	{
		uint num5 = 10u;
		uint num6 = 1u;
		for (uint num7 = 0u; num7 < 10 - num4; num7++)
		{
			num5 *= 10;
			num6 *= 10;
		}
		uint num8 = num % num5 / num6;
		pAnita(num4)->SetAnimNo(ANM_NUMBER_LIST[num8][nType], false);
	}
	for (uint num9 = 11u; num9 <= 12; num9++)
	{
		uint num10 = 10u;
		uint num11 = 1u;
		for (uint num12 = 0u; num12 < 12 - num9; num12++)
		{
			num10 *= 10;
			num11 *= 10;
		}
		uint num13 = num2 % num10 / num11;
		pAnita(num9)->SetAnimNo(ANM_NUMBER_LIST[num13][nType], false);
	}
	for (uint num14 = 13u; num14 <= 14; num14++)
	{
		uint num15 = 10u;
		uint num16 = 1u;
		for (uint num17 = 0u; num17 < 14 - num14; num17++)
		{
			num15 *= 10;
			num16 *= 10;
		}
		uint num18 = num3 % num15 / num16;
		pAnita(num14)->SetAnimNo(ANM_NUMBER_LIST[num18][nType], false);
	}
	pAnita(2u)->SetAnimNo(ANM_MOTOS_LIST[nType]);
	pAnita(2u)->SetState(1u, bTimeNew());
	pAnita(15u)->SetAnimNo(ANM_CONMA_LIST[nType]);
	pAnita(16u)->SetAnimNo(ANM_CONMA_LIST[nType]);
}

// DOL 0x801c6b64 [audited: bar / motos / number clip ids vs the DOL tables, step logic vs decomp; digit layout loops not re-read line by line]
void cTaskConsMenuBar::WaitExec()
{
	pAnita(0u)->SetAnimNo(ANM_BAR_LIST[1]);
	SetAnmTime(1);
	SetAnmScore(1);
}

// DOL 0x801c6bc0 [audited: bar / motos / number clip ids vs the DOL tables, step logic vs decomp; digit layout loops not re-read line by line]
bool cTaskConsMenuBar::AppearExec()
{
	if (nStep() == 0)
	{
		if (!pAnita(0u)->IsState(1u))
		{
			pAnita(0u)->SetAnimNo(ANM_BAR_LIST[0]);
			pAnita(0u)->SetState(1u);
		}
		for (uint num = 9u; num <= 16; num++)
		{
			pAnita(num)->SetState(1u);
		}
		SetAnmTime(0);
		for (uint num2 = 3u; num2 <= 8; num2++)
		{
			pAnita(num2)->SetState(1u);
		}
		SetAnmScore(0);
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool flag = true;
		for (uint num3 = 3u; num3 < 17; num3++)
		{
			flag = pAnita(num3)->IsState(65536u);
			if (!flag)
			{
				break;
			}
		}
		if (flag)
		{
			return true;
		}
	}
	return false;
}

// DOL 0x801c6d04 [audited: bar / motos / number clip ids vs the DOL tables, step logic vs decomp; digit layout loops not re-read line by line]
bool cTaskConsMenuBar::DeleteExec()
{
	if (nStep() == 0)
	{
		SetAnmTime(2);
		SetAnmScore(2);
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool flag = true;
		for (uint num = 3u; num < 17; num++)
		{
			flag = pAnita(num)->IsState(65536u);
			if (!flag)
			{
				break;
			}
		}
		if (flag)
		{
			return true;
		}
	}
	return false;
}

// DOL 0x801c6da8 [audited: bar / motos / number clip ids vs the DOL tables, step logic vs decomp; digit layout loops not re-read line by line]
void cTaskConsMenuBar::LoadRecord(uint nWorld, uint nStage)
{
	if (nWorldNo() != nWorld || nStageNo() != nStage)
	{
		nWorldNo() = nWorld;
		nStageNo() = nStage;
		bScoreNew() = KingdomGlobal::MotosSys->GetStageHiScoreNew(nWorldNo() + 1, nStageNo() + 1);
		bTimeNew() = KingdomGlobal::MotosSys->GetStageRecordTimeNew(nWorldNo() + 1, nStageNo() + 1);
		nScore() = KingdomGlobal::MotosSys->GetStageHiScore(nWorldNo() + 1, nStageNo() + 1);
		nTime() = KingdomGlobal::MotosSys->GetStageRecordTime(nWorldNo() + 1, nStageNo() + 1);
	}
}

}
