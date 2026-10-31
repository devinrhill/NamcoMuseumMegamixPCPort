#include "kingdom/game/g07/cTaskConsMultiMenuTime.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

const int cTaskConsMultiMenuTime::SET_TWO_PAT = 10;
const uint cTaskConsMultiMenuTime::SET_TWO_LAY[3] = {0u, 2u, 3u};
const int cTaskConsMultiMenuTime::SET_CONMA_PAT = 10;
const uint cTaskConsMultiMenuTime::SET_CONMA_LAY = 1u;

uint& cTaskConsMultiMenuTime::nTime()
{
	return m_nTime;
}

cTaskConsMultiMenuTime::~cTaskConsMultiMenuTime()
{
}

void cTaskConsMultiMenuTime::nSetTime(uint nNo)
{
	nTime() = nNo;
}

uint cTaskConsMultiMenuTime::nGetTime()
{
	return nTime();
}

cTaskConsMultiMenuTime::cTaskConsMultiMenuTime(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

// DOL 0x801cef28 [audited: number anim table (identity 3 per digit), priorities, digit layout and step logic vs decomp]
int cTaskConsMultiMenuTime::nGetNumberAnmNo(uint nNo, int nAnmState)
{
	int obj[10][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}, {12, 13, 14}, {15, 16, 17}, {18, 19, 20}, {21, 22, 23}, {24, 25, 26}, {27, 28, 29}};
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 10, "Number Over");
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 3, "AnmState Over");
	return obj[nNo][nAnmState];
}

int cTaskConsMultiMenuTime::nGetConmaAnmNo(int nAnmState)
{
	int obj[3] = {30, 31, 32};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 3, "AnmState Over");
	return obj[nAnmState];
}

// DOL 0x801cefa8 [audited: number anim table (identity 3 per digit), priorities, digit layout and step logic vs decomp]
void cTaskConsMultiMenuTime::SetData(uint nTime)
{
	nDrawPriority(2147483652u);
	this->nTime() = nTime;
	for (uint num = 0u; num <= 2; num++)
	{
		pAnita(num)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
		pAnita(num)->SetAnimNo(0);
		pAnita(num)->SetState(1u, false);
		pAnita(num)->GetLayerPos(pAnita(num)->vTrans(), SET_TWO_LAY[num], SET_TWO_PAT);
	}
	pAnita(3u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
	pAnita(3u)->SetAnimNo(0);
	pAnita(3u)->SetState(1u, false);
	pAnita(3u)->GetLayerPos(pAnita(3u)->vTrans(), SET_CONMA_LAY, SET_CONMA_PAT);
}

// DOL 0x801cf14c [audited: number anim table (identity 3 per digit), priorities, digit layout and step logic vs decomp]
void cTaskConsMultiMenuTime::SetTimeAnm(int nAnmState)
{
	uint nNo = nTime() / 1000 / 60;
	pAnita(0u)->SetAnimNo(nGetNumberAnmNo(nNo, nAnmState));
	pAnita(0u)->SetState(1u);
	uint num = nTime() % 60000 / 1000;
	for (uint num2 = 1u; num2 <= 2; num2++)
	{
		uint num3 = 10u;
		uint num4 = 1u;
		for (uint num5 = 0u; num5 < 2 - num2; num5++)
		{
			num3 *= 10;
			num4 *= 10;
		}
		uint nNo2 = num % num3 / num4;
		pAnita(num2)->SetAnimNo(nGetNumberAnmNo(nNo2, nAnmState));
		pAnita(num2)->SetState(1u);
	}
	pAnita(3u)->SetAnimNo(nGetConmaAnmNo(nAnmState));
	pAnita(3u)->SetState(1u);
}

// DOL 0x801cf350 [audited: number anim table (identity 3 per digit), priorities, digit layout and step logic vs decomp]
void cTaskConsMultiMenuTime::WaitExec()
{
	SetTimeAnm(1);
}

// DOL 0x801cf364 [audited: number anim table (identity 3 per digit), priorities, digit layout and step logic vs decomp]
bool cTaskConsMultiMenuTime::AppearExec()
{
	if (nStep() == 0)
	{
		SetTimeAnm(0);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(2u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// DOL 0x801cf3e8 [audited: number anim table (identity 3 per digit), priorities, digit layout and step logic vs decomp]
bool cTaskConsMultiMenuTime::DeleteExec()
{
	if (nStep() == 0)
	{
		SetTimeAnm(2);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(2u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
