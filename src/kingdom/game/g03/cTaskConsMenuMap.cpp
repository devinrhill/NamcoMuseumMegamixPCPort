#include "kingdom/game/g03/cTaskConsMenuMap.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

const int cTaskConsMenuMap::HIT_MAP_PAT{};

uint& cTaskConsMenuMap::nWorldNo()
{
	return m_nWorldNo;
}

uint& cTaskConsMenuMap::nStageNo()
{
	return m_nStageNo;
}

cTaskConsMenuMap::~cTaskConsMenuMap()
{
}

void cTaskConsMenuMap::ChangeData(uint nWorldNo, uint nStageNo)
{
	CustomKingdomUtil::Assert(nWorldNo < 5, "error");
	CustomKingdomUtil::Assert(nStageNo < 6, "error");
	this->nWorldNo() = nWorldNo;
	this->nStageNo() = nStageNo;
}

cTaskConsMenuMap::cTaskConsMenuMap(cTaskConsMenuManBase* const& pConsMan, uint nWorldNo, uint nStageNo)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(65538u);
	ChangeData(nWorldNo, nStageNo);
	string fileName = "BIN_ANT_MENU_STAGE";
	pAnita(0u)->RegistData(fileName);
	pAnita(0u)->SetState(1u, false);
}

int cTaskConsMenuMap::nGetMapAnm(uint nState)
{
	// the C# table is consecutive: world * 30 + stage * 5 + state (worlds have 6 stage slots, 5 states each)
	return (int)((nWorldNo() * 6 + nStageNo()) * 5 + nState);
}

// DOL 0x8005dda0 [audited: described by the 43rd-session pass]: pattern 0 of anita 0
bool cTaskConsMenuMap::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, 0);
}

bool cTaskConsMenuMap::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetMapAnm(0u));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsMenuMap::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetMapAnm(1u), false);
}

void cTaskConsMenuMap::PointExec()
{
	pAnita(0u)->SetAnimNo(nGetMapAnm(2u), false);
}

bool cTaskConsMenuMap::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetMapAnm(3u));
		nStep()++;
	}
	else if (pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

bool cTaskConsMenuMap::DeleteExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetMapAnm(4u));
		nStep()++;
	}
	else if (pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
