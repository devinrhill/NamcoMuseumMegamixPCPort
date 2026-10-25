#include "kingdom/game/g07/cTaskConsMultiMenuRound.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

const int cTaskConsMultiMenuRound::SET_ONE_PAT = 8;
const int cTaskConsMultiMenuRound::SET_TWO_PAT = 9;
const uint cTaskConsMultiMenuRound::SET_TWO_LAY[2] = {0u, 1u};

uint& cTaskConsMultiMenuRound::nRound()
{
	return m_nRound;
}

cTaskConsMultiMenuRound::~cTaskConsMultiMenuRound()
{
}

void cTaskConsMultiMenuRound::nSetRound(uint nNo)
{
	nRound() = nNo;
}

uint cTaskConsMultiMenuRound::nGetRound()
{
	return nRound();
}

cTaskConsMultiMenuRound::cTaskConsMultiMenuRound(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

int cTaskConsMultiMenuRound::nGetNumberAnmNo(uint nNo, int nAnmState)
{
	int obj[10][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}, {12, 13, 14}, {15, 16, 17}, {18, 19, 20}, {21, 22, 23}, {24, 25, 26}, {27, 28, 29}};
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 10, "Number Over");
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 3, "AnmState Over");
	return obj[nNo][nAnmState];
}

void cTaskConsMultiMenuRound::SetData(uint nRound)
{
	nDrawPriority(2147483651u);
	this->nRound() = nRound;
	for (uint num = 0u; num < 2; num++)
	{
		pAnita(num)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
		pAnita(num)->SetAnimNo(0);
		pAnita(num)->SetState(1u, false);
		pAnita(num)->GetLayerPos(pAnita(num)->vTrans(), SET_TWO_LAY[num], SET_TWO_PAT);
	}
}

void cTaskConsMultiMenuRound::SetRoundAnm(int nAnmState)
{
	for (uint num = 0u; num < 2; num++)
	{
		uint num2 = 10u;
		uint num3 = 1u;
		for (uint num4 = 0u; num4 < 1 - num; num4++)
		{
			num2 *= 10;
			num3 *= 10;
		}
		uint nNo = nRound() % num2 / num3;
		if (1 == num || nRound() >= num3)
		{
			pAnita(num)->SetAnimNo(nGetNumberAnmNo(nNo, nAnmState));
			pAnita(num)->SetState(1u);
		}
		else
		{
			pAnita(num)->SetAnimNo(nGetNumberAnmNo(nNo, nAnmState));
			pAnita(num)->SetState(1u, false);
		}
	}
	if (nRound() < 10)
	{
		pAnita(1u)->GetLayerPos(pAnita(1u)->vTrans(), 0u, SET_ONE_PAT);
	}
	else
	{
		pAnita(1u)->GetLayerPos(pAnita(1u)->vTrans(), SET_TWO_LAY[1], SET_TWO_PAT);
	}
}

void cTaskConsMultiMenuRound::WaitExec()
{
	SetRoundAnm(1);
}

bool cTaskConsMultiMenuRound::AppearExec()
{
	if (nStep() == 0)
	{
		SetRoundAnm(0);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(1u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

bool cTaskConsMultiMenuRound::DeleteExec()
{
	if (nStep() == 0)
	{
		SetRoundAnm(2);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(1u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
