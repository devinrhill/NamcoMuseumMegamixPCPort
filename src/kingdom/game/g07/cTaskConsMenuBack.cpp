#include "kingdom/game/g07/cTaskConsMenuBack.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

int& cTaskConsMenuBack::nHitPat()
{
	return m_nHitPat;
}

cTaskConsMenuBack::cTaskConsMenuBack(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

cTaskConsMenuBack::~cTaskConsMenuBack()
{
}

int cTaskConsMenuBack::nGetBackAnmNo(int nAnmState)
{
	int obj[4] = {334, 335, 336, 337};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 4, "AnmState Over");
	return obj[nAnmState];
}

void cTaskConsMenuBack::SetData()
{
	nDrawPriority(2147483657u);
	nHitPat() = 22;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
	pAnita(0u)->SetAnimNo(nGetBackAnmNo(1));
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x801c5e40: hit test of anita 0, pattern +0x1d8 (nHitPat) (the C# returned false: Unity used colliders)
bool cTaskConsMenuBack::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, nHitPat());
}

void cTaskConsMenuBack::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetBackAnmNo(1), false);
}

bool cTaskConsMenuBack::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetBackAnmNo(0));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		pAnita(0u)->SetAnimNo(nGetBackAnmNo(1));
		return true;
	}
	return false;
}

void cTaskConsMenuBack::PointExec()
{
	pAnita(0u)->SetAnimNo(nGetBackAnmNo(2), false);
}

bool cTaskConsMenuBack::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetBackAnmNo(3));
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
