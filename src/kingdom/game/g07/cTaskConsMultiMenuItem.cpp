#include "kingdom/game/g07/cTaskConsMultiMenuItem.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

const int cTaskConsMultiMenuItem::SET_ONE_PAT = 11;
const int cTaskConsMultiMenuItem::SET_TWO_PAT = 12;
const uint cTaskConsMultiMenuItem::SET_TWO_LAY[2] = {0u, 1u};

uint& cTaskConsMultiMenuItem::nItem()
{
	return m_nItem;
}

cTaskConsMultiMenuItem::~cTaskConsMultiMenuItem()
{
}

void cTaskConsMultiMenuItem::nSetItem(uint nNo)
{
	nItem() = nNo;
}

uint cTaskConsMultiMenuItem::nGetItem()
{
	return nItem();
}

cTaskConsMultiMenuItem::cTaskConsMultiMenuItem(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

// DOL 0x801cb54c [audited: number anim table (identity 3 per digit), priorities, digit layout and step logic vs decomp]
int cTaskConsMultiMenuItem::nGetNumberAnmNo(uint nNo, int nAnmState)
{
	int obj[10][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}, {12, 13, 14}, {15, 16, 17}, {18, 19, 20}, {21, 22, 23}, {24, 25, 26}, {27, 28, 29}};
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 10, "Number Over");
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 3, "AnmState Over");
	return obj[nNo][nAnmState];
}

// DOL 0x801cb598 [audited: number anim table (identity 3 per digit), priorities, digit layout and step logic vs decomp]
void cTaskConsMultiMenuItem::SetData(uint nItem)
{
	nDrawPriority(2147483653u);
	this->nItem() = nItem;
	for (uint num = 0u; num < 2; num++)
	{
		pAnita(num)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
		pAnita(num)->SetAnimNo(0);
		pAnita(num)->SetState(1u, false);
		pAnita(num)->GetLayerPos(pAnita(num)->vTrans(), SET_TWO_LAY[num], SET_TWO_PAT);
	}
}

// DOL 0x801cb69c [audited: number anim table (identity 3 per digit), priorities, digit layout and step logic vs decomp]
void cTaskConsMultiMenuItem::SetItemAnm(int nAnmState)
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
		uint nNo = nItem() % num2 / num3;
		if (1 == num || nItem() >= num3)
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
	if (nItem() < 10)
	{
		pAnita(1u)->GetLayerPos(pAnita(1u)->vTrans(), 0u, SET_ONE_PAT);
	}
	else
	{
		pAnita(1u)->GetLayerPos(pAnita(1u)->vTrans(), SET_TWO_LAY[1], SET_TWO_PAT);
	}
}

// DOL 0x801cb880 [audited: number anim table (identity 3 per digit), priorities, digit layout and step logic vs decomp]
void cTaskConsMultiMenuItem::WaitExec()
{
	SetItemAnm(1);
}

// DOL 0x801cb894 [audited: number anim table (identity 3 per digit), priorities, digit layout and step logic vs decomp]
bool cTaskConsMultiMenuItem::AppearExec()
{
	if (nStep() == 0)
	{
		SetItemAnm(0);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(1u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// DOL 0x801cb918 [audited: number anim table (identity 3 per digit), priorities, digit layout and step logic vs decomp]
bool cTaskConsMultiMenuItem::DeleteExec()
{
	if (nStep() == 0)
	{
		SetItemAnm(2);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(1u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
