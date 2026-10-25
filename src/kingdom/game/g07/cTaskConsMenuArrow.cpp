#include "kingdom/game/g07/cTaskConsMenuArrow.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

bool& cTaskConsMenuArrow::bActive()
{
	return m_bActive;
}

uint& cTaskConsMenuArrow::nArrowType()
{
	return m_nArrowType;
}

int& cTaskConsMenuArrow::nHitPat()
{
	return m_nHitPat;
}

cTaskConsMenuArrow::cTaskConsMenuArrow(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

cTaskConsMenuArrow::~cTaskConsMenuArrow()
{
}

bool cTaskConsMenuArrow::IsActive()
{
	return bActive();
}

void cTaskConsMenuArrow::SetData(uint nType, bool bActive)
{
	CustomKingdomUtil::Assert(0 <= nType && nType < 2, "ArrowType Over");
	nDrawPriority(2147483652u);
	nArrowType() = nType;
	this->bActive() = bActive;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
	pAnita(0u)->SetAnimNo(nGetArrowAnmNo(1));
	pAnita(0u)->SetState(1u, false);
	int array[2] = {19, 20};
	nHitPat() = array[nArrowType()];
}

int cTaskConsMenuArrow::nGetArrowAnmNo(int nAnmState)
{
	int obj[2][4] = {{326, 327, 328, 329}, {330, 331, 332, 333}};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 4, "State Over");
	CustomKingdomUtil::Assert(0 <= nArrowType() && nArrowType() < 2, "ArrowType Over");
	return obj[nArrowType()][nAnmState];
}

// DOL 0x801c5924: hit test of anita 0, only while active (+0x1d8), pattern +0x1e0 (the C# returned false: Unity used colliders)
bool cTaskConsMenuArrow::IsDpdHit(float x, float y)
{
	return bActive() && DpdHitPat(x, y, nHitPat());
}

void cTaskConsMenuArrow::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetArrowAnmNo(1), false);
}

void cTaskConsMenuArrow::PointExec()
{
	pAnita(0u)->SetAnimNo(nGetArrowAnmNo(2), false);
}

bool cTaskConsMenuArrow::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetArrowAnmNo(0));
		if (bActive())
		{
			pAnita(0u)->SetState(1u);
		}
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

bool cTaskConsMenuArrow::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetArrowAnmNo(3));
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
