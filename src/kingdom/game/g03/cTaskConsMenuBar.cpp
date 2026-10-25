#include "kingdom/game/g03/cTaskConsMenuBar.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include <SSaveDataPacnRoll.hpp>

namespace kingdom::game::g03
{

const int cTaskConsMenuBar::ANM_BAR_LIST[3] = {283, 284, 285};
const int cTaskConsMenuBar::ANM_CROWN_LIST[3] = {51, 52, 53};
const int cTaskConsMenuBar::ANM_NUMBER_LIST[10][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}, {12, 13, 14}, {15, 16, 17}, {18, 19, 20}, {21, 22, 23}, {24, 25, 26}, {27, 28, 29}};
const int cTaskConsMenuBar::ANM_SLASH_LIST[3] = {33, 34, 35};
const int cTaskConsMenuBar::ANM_CONMA_LIST[3] = {30, 31, 32};
const int cTaskConsMenuBar::SET_CROWN_PAT = 13;
const uint cTaskConsMenuBar::SET_CROWN_LAY[3] = {2u, 1u, 0u};
const int cTaskConsMenuBar::SET_GHOST_PAT = 2;
const uint cTaskConsMenuBar::SET_GHOST_LAY[2] = {1u, 0u};
const int cTaskConsMenuBar::SET_COOKIE_PAT = 0;
const uint cTaskConsMenuBar::SET_COOKIE_LAY[3] = {6u, 5u, 4u};
const int cTaskConsMenuBar::SET_COOKIE_MAX_PAT = 0;
const uint cTaskConsMenuBar::SET_COOKIE_MAX_LAY[3] = {2u, 1u, 0u};
const int cTaskConsMenuBar::SET_TIME_PAT = 1;
const uint cTaskConsMenuBar::SET_TIME_LAY[5] = {6u, 4u, 3u, 1u, 0u};
const int cTaskConsMenuBar::SET_SLASH_PAT = 0;
const uint cTaskConsMenuBar::SET_SLASH_LAY = 3u;
const int cTaskConsMenuBar::SET_CONMA_PAT = 1;
const uint cTaskConsMenuBar::SET_CONMA_LAY[2] = {5u, 2u};

uint& cTaskConsMenuBar::nCashWorld()
{
	return m_nCashWorld;
}

uint& cTaskConsMenuBar::nCashStage()
{
	return m_nCashStage;
}

uint& cTaskConsMenuBar::nCookie()
{
	return m_nCookie;
}

uint& cTaskConsMenuBar::nCookieMax()
{
	return m_nCookieMax;
}

uint& cTaskConsMenuBar::nGhost()
{
	return m_nGhost;
}

uint& cTaskConsMenuBar::nTime()
{
	return m_nTime;
}

bool& cTaskConsMenuBar::bCookieNew()
{
	return m_bCookieNew;
}

bool& cTaskConsMenuBar::bGhostNew()
{
	return m_bGhostNew;
}

bool& cTaskConsMenuBar::bTimeNew()
{
	return m_bTimeNew;
}

bool& cTaskConsMenuBar::bUpdate()
{
	return m_bUpdate;
}

cTaskConsMenuBar::cTaskConsMenuBar(cTaskConsMenuManBase* pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(65540u);
	nCashWorld() = 268435455u;
	nCashStage() = 268435455u;
	nGhost() = 0u;
	nTime() = 0u;
	nCookieMax() = 0u;
	nCookie() = 0u;
	bGhostNew() = false;
	bTimeNew() = false;
	bCookieNew() = false;
	bUpdate() = true;
	string fileName = "BIN_ANT_MENU_SELECT01";
	pAnita(0u)->RegistData(fileName, 5, true);
	pAnita(0u)->SetAnimNo(ANM_BAR_LIST[1]);
	pAnita(0u)->SetState(1u, false);
	uint num = 1u;
	uint num2 = 0u;
	while (num <= 3)
	{
		pAnita(num)->RegistData(fileName);
		pAnita(num)->SetAnimNo(ANM_CROWN_LIST[1]);
		pAnita(num)->SetState(1u, false);
		pAnita(num)->GetLayerPos(pAnita(num)->vTrans(), SET_CROWN_LAY[num2], SET_CROWN_PAT);
		num++;
		num2++;
	}
	uint num3 = 4u;
	uint num4 = 0u;
	while (num3 <= 6)
	{
		pAnita(num3)->RegistData(fileName);
		pAnita(num3)->SetAnimNo(ANM_NUMBER_LIST[0][1]);
		pAnita(num3)->SetState(1u, false);
		pAnita(num3)->GetLayerPos(pAnita(num3)->vTrans(), SET_COOKIE_LAY[num4], SET_COOKIE_PAT);
		num3++;
		num4++;
	}
	pAnita(10u)->RegistData(fileName);
	pAnita(10u)->SetAnimNo(ANM_SLASH_LIST[1]);
	pAnita(10u)->SetState(1u, false);
	pAnita(10u)->GetLayerPos(pAnita(10u)->vTrans(), SET_SLASH_LAY, SET_SLASH_PAT);
	uint num5 = 7u;
	uint num6 = 0u;
	while (num5 <= 9)
	{
		pAnita(num5)->RegistData(fileName);
		pAnita(num5)->SetAnimNo(ANM_NUMBER_LIST[0][1]);
		pAnita(num5)->SetState(1u, false);
		pAnita(num5)->GetLayerPos(pAnita(num5)->vTrans(), SET_COOKIE_MAX_LAY[num6], SET_COOKIE_MAX_PAT);
		num5++;
		num6++;
	}
	uint num7 = 16u;
	uint num8 = 0u;
	while (num7 <= 17)
	{
		pAnita(num7)->RegistData(fileName);
		pAnita(num7)->SetAnimNo(ANM_NUMBER_LIST[0][1]);
		pAnita(num7)->SetState(1u, false);
		pAnita(num7)->GetLayerPos(pAnita(num7)->vTrans(), SET_GHOST_LAY[num8], SET_GHOST_PAT);
		num7++;
		num8++;
	}
	uint num9 = 11u;
	uint num10 = 0u;
	while (num9 <= 15)
	{
		pAnita(num9)->RegistData(fileName);
		pAnita(num9)->SetAnimNo(ANM_NUMBER_LIST[0][1]);
		pAnita(num9)->SetState(1u, false);
		pAnita(num9)->GetLayerPos(pAnita(num9)->vTrans(), SET_TIME_LAY[num10], SET_TIME_PAT);
		num9++;
		num10++;
	}
	uint num11 = 18u;
	uint num12 = 0u;
	while (num11 <= 19)
	{
		pAnita(num11)->RegistData(fileName);
		pAnita(num11)->SetAnimNo(ANM_CONMA_LIST[1]);
		pAnita(num11)->SetState(1u, false);
		pAnita(num11)->GetLayerPos(pAnita(num11)->vTrans(), SET_CONMA_LAY[num12], SET_CONMA_PAT);
		num11++;
		num12++;
	}
}

cTaskConsMenuBar::~cTaskConsMenuBar()
{
}

void cTaskConsMenuBar::SetAnmCookie(int nType)
{
	for (uint num = 4u; num <= 6; num++)
	{
		uint num2 = 10u;
		uint num3 = 1u;
		for (uint num4 = 0u; num4 < 6 - num; num4++)
		{
			num2 *= 10;
			num3 *= 10;
		}
		if (6 == num || nCookie() >= num3)
		{
			uint num5 = nCookie() % num2 / num3;
			pAnita(num)->SetAnimNo(ANM_NUMBER_LIST[num5][nType], false);
			pAnita(num)->SetState(1u);
		}
		else
		{
			pAnita(num)->SetAnimNo(ANM_NUMBER_LIST[0][nType], false);
			pAnita(num)->SetState(1u, false);
		}
	}
	for (uint num6 = 7u; num6 <= 9; num6++)
	{
		uint num7 = 10u;
		uint num8 = 1u;
		for (uint num9 = 0u; num9 < 9 - num6; num9++)
		{
			num7 *= 10;
			num8 *= 10;
		}
		if (9 == num6 || nCookieMax() >= num8)
		{
			uint num10 = nCookieMax() % num7 / num8;
			pAnita(num6)->SetAnimNo(ANM_NUMBER_LIST[num10][nType], false);
			pAnita(num6)->SetState(1u);
		}
		else
		{
			pAnita(num6)->SetAnimNo(ANM_NUMBER_LIST[0][nType], false);
			pAnita(num6)->SetState(1u, false);
		}
	}
	pAnita(10u)->SetAnimNo(ANM_SLASH_LIST[nType]);
	pAnita(10u)->SetState(1u);
	pAnita(1u)->SetAnimNo(ANM_CROWN_LIST[nType]);
	pAnita(1u)->SetState(1u, bCookieNew());
}

void cTaskConsMenuBar::SetAnmGhost(int nType)
{
	for (uint num = 16u; num <= 17; num++)
	{
		uint num2 = 10u;
		uint num3 = 1u;
		for (uint num4 = 0u; num4 < 17 - num; num4++)
		{
			num2 *= 10;
			num3 *= 10;
		}
		if (17 == num || nGhost() >= num3)
		{
			uint num5 = nGhost() % num2 / num3;
			pAnita(num)->SetAnimNo(ANM_NUMBER_LIST[num5][nType], false);
			pAnita(num)->SetState(1u);
		}
		else
		{
			pAnita(num)->SetAnimNo(ANM_NUMBER_LIST[0][nType], false);
			pAnita(num)->SetState(1u, false);
		}
	}
	pAnita(3u)->SetAnimNo(ANM_CROWN_LIST[nType]);
	pAnita(3u)->SetState(1u, bGhostNew());
}

void cTaskConsMenuBar::SetAnmTime(int nType)
{
	uint num = nTime() / 60000;
	uint num2 = nTime() % 60000 / 1000;
	uint num3 = nTime() % 1000 / 10;
	uint num4 = 10u;
	uint num5 = 1u;
	uint num6 = num % num4 / num5;
	pAnita(11u)->SetAnimNo(ANM_NUMBER_LIST[num6][nType], false);
	for (uint num7 = 12u; num7 <= 13; num7++)
	{
		uint num8 = 10u;
		uint num9 = 1u;
		for (uint num10 = 0u; num10 < 13 - num7; num10++)
		{
			num8 *= 10;
			num9 *= 10;
		}
		uint num11 = num2 % num8 / num9;
		pAnita(num7)->SetAnimNo(ANM_NUMBER_LIST[num11][nType], false);
	}
	for (uint num12 = 14u; num12 <= 15; num12++)
	{
		uint num13 = 10u;
		uint num14 = 1u;
		for (uint num15 = 0u; num15 < 15 - num12; num15++)
		{
			num13 *= 10;
			num14 *= 10;
		}
		uint num16 = num3 % num13 / num14;
		pAnita(num12)->SetAnimNo(ANM_NUMBER_LIST[num16][nType], false);
	}
	pAnita(2u)->SetAnimNo(ANM_CROWN_LIST[nType]);
	pAnita(2u)->SetState(1u, bTimeNew());
	pAnita(18u)->SetAnimNo(ANM_CONMA_LIST[nType]);
	pAnita(19u)->SetAnimNo(ANM_CONMA_LIST[nType]);
}

void cTaskConsMenuBar::WaitExec()
{
	if (bUpdate())
	{
		pAnita(0u)->SetAnimNo(ANM_BAR_LIST[1]);
		SetAnmTime(1);
		SetAnmGhost(1);
		SetAnmCookie(1);
		bUpdate() = false;
	}
}

bool cTaskConsMenuBar::AppearExec()
{
	if (nStep() == 0)
	{
		if (!pAnita(0u)->IsState(1u))
		{
			pAnita(0u)->SetAnimNo(ANM_BAR_LIST[0]);
			pAnita(0u)->SetState(1u);
		}
		for (uint num = 11u; num <= 19; num++)
		{
			pAnita(num)->SetState(1u);
		}
		SetAnmTime(0);
		for (uint num2 = 4u; num2 <= 9; num2++)
		{
			pAnita(num2)->SetState(1u);
		}
		SetAnmCookie(0);
		for (uint num3 = 16u; num3 <= 17; num3++)
		{
			pAnita(num3)->SetState(1u);
		}
		SetAnmGhost(0);
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool flag = true;
		for (uint num4 = 4u; num4 < 20; num4++)
		{
			flag = pAnita(num4)->IsState(65536u);
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

bool cTaskConsMenuBar::DeleteExec()
{
	if (nStep() == 0)
	{
		SetAnmTime(2);
		SetAnmGhost(2);
		SetAnmCookie(2);
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool flag = true;
		for (uint num = 4u; num < 20; num++)
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

void cTaskConsMenuBar::LoadRecord(uint nWorldNo, uint nStageNo)
{
	if (nCashWorld() != nWorldNo || nCashStage() != nStageNo)
	{
		nCashWorld() = nWorldNo;
		nCashStage() = nStageNo;
		SSaveDataPacnRoll::sStageRecord _out = SSaveDataPacnRoll::sStageRecord();
		uint round = KingdomGlobal::PacnRollSys->nWorldStage(nWorldNo, nStageNo);
		if (KingdomGlobal::PacnRollSys->pSaveLoadMan()->GetStageRecord(_out, round))
		{
			nCookie() = _out.nCookie;
			nCookieMax() = _out.nCookieMax;
			nGhost() = _out.nGhost;
			nTime() = _out.nTime;
			bCookieNew() = _out.bCookieNew;
			bGhostNew() = _out.bGhostNew;
			bTimeNew() = _out.bTimeNew;
			bUpdate() = true;
		}
	}
}

}
