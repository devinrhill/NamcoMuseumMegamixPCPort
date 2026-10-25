#include "kingdom/game/menu/cTaskConsOk.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::menu
{

cTaskConsOk::~cTaskConsOk()
{
}

cTaskConsOk::cTaskConsOk(int nId, string Key)
	: cTaskConsMatching(nId)
{
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetState(1u, false);
}

int cTaskConsOk::nGetOkAnmNo(uint nState)
{
	int obj[5] = {90, 91, 92, 93, 94};
	CustomKingdomUtil::Assert(0 <= nState && nState < 5, "error");
	return obj[nState];
}

bool cTaskConsOk::IsDpdHit(float x, float y)
{
	return false;
}

bool cTaskConsOk::AppearProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetOkAnmNo(0u));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsOk::WaitProc()
{
	pAnita(0u)->SetAnimNo(nGetOkAnmNo(1u), false);
}

void cTaskConsOk::PointProc()
{
	pAnita(0u)->SetAnimNo(nGetOkAnmNo(2u), false);
}

bool cTaskConsOk::DecideProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetOkAnmNo(3u), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

bool cTaskConsOk::DeleteProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetOkAnmNo(4u), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
