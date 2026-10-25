#include "kingdom/game/menu/cTaskConsInfoWnd.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::menu
{

cTaskConsInfoWnd::~cTaskConsInfoWnd()
{
}

cTaskConsInfoWnd::cTaskConsInfoWnd(int nId, string Key)
	: cTaskConsMatching(nId)
{
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetAnimNo(nGetWndAnmNo(2u));
	pAnita(0u)->SetState(1u, false);
}

int cTaskConsInfoWnd::nGetWndAnmNo(uint nState)
{
	int obj[3] = {60, 61, 62};
	CustomKingdomUtil::Assert(0 <= nState && nState < 3, "error");
	return obj[nState];
}

bool cTaskConsInfoWnd::AppearProc()
{
	if (nStep() == 0)
	{
		if (pAnita(0u)->GetAnimNo() == nGetWndAnmNo(2u))
		{
			pAnita(0u)->SetAnimNo(nGetWndAnmNo(0u));
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

void cTaskConsInfoWnd::WaitProc()
{
	pAnita(0u)->SetAnimNo(nGetWndAnmNo(1u), false);
}

bool cTaskConsInfoWnd::DeleteProc()
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
