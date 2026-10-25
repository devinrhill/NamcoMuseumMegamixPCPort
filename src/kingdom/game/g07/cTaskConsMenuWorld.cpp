#include "kingdom/game/g07/cTaskConsMenuWorld.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

uint& cTaskConsMenuWorld::nWorldNo()
{
	return m_nWorldNo;
}

cTaskConsMenuWorld::~cTaskConsMenuWorld()
{
}

void cTaskConsMenuWorld::nSetWorldNo(uint nNo)
{
	nWorldNo() = nNo;
}

uint cTaskConsMenuWorld::nGetWorldNo()
{
	return nWorldNo();
}

cTaskConsMenuWorld::cTaskConsMenuWorld(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

void cTaskConsMenuWorld::SetData(uint nWorld)
{
	nDrawPriority(2147483651u);
	nWorldNo() = nWorld;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
	pAnita(0u)->SetAnimNo(nGetWorldAnmNo(1));
	pAnita(0u)->SetState(1u, false);
}

int cTaskConsMenuWorld::nGetWorldAnmNo(int nAnmState)
{
	int obj[5][3] = {{36, 37, 38}, {39, 40, 41}, {42, 43, 44}, {45, 46, 47}, {48, 49, 50}};
	CustomKingdomUtil::Assert(0 <= nWorldNo() && nWorldNo() < 5, "WorldOver");
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 3, "AnmState Over");
	return obj[nWorldNo()][nAnmState];
}

void cTaskConsMenuWorld::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetWorldAnmNo(1), false);
}

bool cTaskConsMenuWorld::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetWorldAnmNo(0), false);
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

bool cTaskConsMenuWorld::DeleteExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetWorldAnmNo(2), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
