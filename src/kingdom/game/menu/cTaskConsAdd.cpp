#include "kingdom/game/menu/cTaskConsAdd.hpp"

#include "CustomKingdomUtil.hpp"
#include "MotosUIMotosMultiHome.hpp"

namespace kingdom::game::menu
{

cTaskConsAdd::~cTaskConsAdd()
{
}

cTaskConsAdd::cTaskConsAdd(int nId, string Key)
	: cTaskConsMatching(nId)
{
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetState(1u, false);
}

int cTaskConsAdd::nGetAddAnmNo(uint nState)
{
	int obj[5] = {55, 56, 57, 58, 59};
	CustomKingdomUtil::Assert(0 <= nState && nState < 5, "error");
	return obj[nState];
}

bool cTaskConsAdd::IsDpdHit(float x, float y)
{
	return false;
}

bool cTaskConsAdd::AppearProc()
{
	bool bit = nullptr == UnityUI;
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetAddAnmNo(0u));
		pAnita(0u)->SetState(1u, bit);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsAdd::WaitProc()
{
	pAnita(0u)->SetAnimNo(nGetAddAnmNo(1u), false);
}

void cTaskConsAdd::PointProc()
{
	pAnita(0u)->SetAnimNo(nGetAddAnmNo(2u), false);
}

bool cTaskConsAdd::DecideProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetAddAnmNo(3u), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

bool cTaskConsAdd::DeleteProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetAddAnmNo(4u), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
