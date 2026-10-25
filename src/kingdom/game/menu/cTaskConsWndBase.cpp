#include "kingdom/game/menu/cTaskConsWndBase.hpp"

#include "MotosUIMotosMultiHome.hpp"

namespace kingdom::game::menu
{

cTaskConsWndBase::~cTaskConsWndBase()
{
}

cTaskConsWndBase::cTaskConsWndBase(int nId)
	: cTaskConsMatching(nId)
{
}

int cTaskConsWndBase::nGetWndAnmNo(uint nState)
{
	return 0;
}

bool cTaskConsWndBase::AppearProc()
{
	bool bit = nullptr == UnityUI;
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetWndAnmNo(0u));
		pAnita(0u)->SetState(1u, bit);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsWndBase::WaitProc()
{
	pAnita(0u)->SetAnimNo(nGetWndAnmNo(1u), false);
}

bool cTaskConsWndBase::DeleteProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetWndAnmNo(2u));
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
