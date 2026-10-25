#include "kingdom/game/g03/cTaskConsMenuArrow.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

const int cTaskConsMenuArrow::ANM_LIST[2][4] = {{298, 299, 300, 301}, {302, 303, 304, 305}};
const int cTaskConsMenuArrow::HIT_PAT[2] = {19, 20};

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

bool cTaskConsMenuArrow::IsActive()
{
	return bActive();
}

cTaskConsMenuArrow::cTaskConsMenuArrow(cTaskConsMenuManBase* pConsMan, uint nType, bool bActive)
	: cTaskConsMenuBase(pConsMan)
{
	CustomKingdomUtil::Assert(0 <= nType && nType < 2, "ArrowType Over");
	nDrawPriority(2147483650u);
	nArrowType() = nType;
	this->bActive() = bActive;
	string fileName = "BIN_ANT_MENU_SELECT01";
	pAnita(0u)->RegistData(fileName);
	pAnita(0u)->SetAnimNo(ANM_LIST[nArrowType()][1]);
	pAnita(0u)->SetState(1u, false);
	nHitPat() = HIT_PAT[nArrowType()];
}

cTaskConsMenuArrow::~cTaskConsMenuArrow()
{
}

// DOL 0x8005a258: only while active (byte +0x1d8), pattern nHitPat (+0x1e0) of anita 0
bool cTaskConsMenuArrow::IsDpdHit(float x, float y)
{
	return bActive() && DpdHitPat(x, y, nHitPat());
}

void cTaskConsMenuArrow::WaitExec()
{
	pAnita(0u)->SetAnimNo(ANM_LIST[nArrowType()][1], false);
}

void cTaskConsMenuArrow::PointExec()
{
	pAnita(0u)->SetAnimNo(ANM_LIST[nArrowType()][2], false);
}

bool cTaskConsMenuArrow::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(ANM_LIST[nArrowType()][0]);
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
		pAnita(0u)->SetAnimNo(ANM_LIST[nArrowType()][3]);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
