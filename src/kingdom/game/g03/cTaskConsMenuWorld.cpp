#include "kingdom/game/g03/cTaskConsMenuWorld.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

const int cTaskConsMenuWorld::ANM_LIST[5][3] = {{36, 37, 38}, {39, 40, 41}, {42, 43, 44}, {45, 46, 47}, {48, 49, 50}};

uint& cTaskConsMenuWorld::nWorldNo()
{
	return m_nWorldNo;
}

void cTaskConsMenuWorld::nSetWorldNo(uint nNo)
{
	nWorldNo() = nNo;
}

uint cTaskConsMenuWorld::nGetWorldNo()
{
	return nWorldNo();
}

cTaskConsMenuWorld::cTaskConsMenuWorld(cTaskConsMenuManBase* pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(2147483649u);
	nWorldNo() = 0u;
	string fileName = "BIN_ANT_MENU_SELECT01";
	pAnita(0u)->RegistData(fileName);
	pAnita(0u)->SetAnimNo(ANM_LIST[nWorldNo()][1]);
	pAnita(0u)->SetState(1u, false);
}

cTaskConsMenuWorld::~cTaskConsMenuWorld()
{
}

void cTaskConsMenuWorld::WaitExec()
{
	pAnita(0u)->SetAnimNo(ANM_LIST[nWorldNo()][1], false);
}

bool cTaskConsMenuWorld::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(ANM_LIST[nWorldNo()][0], false);
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
		pAnita(0u)->SetAnimNo(ANM_LIST[nWorldNo()][2], false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
