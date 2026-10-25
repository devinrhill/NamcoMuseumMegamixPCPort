#include "kingdom/game/menu/cTaskConsArrow.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::menu
{

uint& cTaskConsArrow::nType()
{
	return m_nType;
}

int& cTaskConsArrow::nHitPat()
{
	return m_nHitPat;
}

cTaskConsArrow::~cTaskConsArrow()
{
}

cTaskConsArrow::cTaskConsArrow(int nId, string Key, uint nType)
	: cTaskConsMatching(nId)
{
	CustomKingdomUtil::Assert(0 <= nType && nType < 2, "error");
	this->nType() = nType;
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetState(1u, false);
	nHitPat() = HIT_PAT[nType];
}

int cTaskConsArrow::nGetArrowAnmNo(uint nState)
{
	int obj[2][5] = {{110, 111, 112, 113, 114}, {115, 116, 117, 118, 119}};
	CustomKingdomUtil::Assert(0 <= nType() && nType() < 2, "error");
	CustomKingdomUtil::Assert(0 <= nState && nState < 5, "error");
	return obj[nType()][nState];
}

bool cTaskConsArrow::IsDpdHit(float x, float y)
{
	return false;
}

bool cTaskConsArrow::AppearProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetArrowAnmNo(0u));
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsArrow::WaitProc()
{
	pAnita(0u)->SetAnimNo(nGetArrowAnmNo(1u), false);
}

void cTaskConsArrow::PointProc()
{
	pAnita(0u)->SetAnimNo(nGetArrowAnmNo(2u), false);
}

bool cTaskConsArrow::DecideProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetArrowAnmNo(3u));
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

bool cTaskConsArrow::DeleteProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetArrowAnmNo(4u));
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsArrow::Visible(bool b)
{
	pAnita(0u)->SetState(1u, b);
}

bool cTaskConsArrow::IsVisible()
{
	return pAnita(0u)->IsState(1u);
}

}
