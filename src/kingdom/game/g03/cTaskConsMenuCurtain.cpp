#include "kingdom/game/g03/cTaskConsMenuCurtain.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

const int cTaskConsMenuCurtain::ANM_LIST[2] = {310, 311};

cTaskConsMenuCurtain::cTaskConsMenuCurtain(cTaskConsMenuManBase* pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(65539u);
	string fileName = "BIN_ANT_MENU_SELECT01";
	pAnita(0u)->RegistData(fileName);
	pAnita(0u)->SetAnimNo(ANM_LIST[1]);
	pAnita(0u)->SetState(1u, false);
}

cTaskConsMenuCurtain::~cTaskConsMenuCurtain()
{
}

void cTaskConsMenuCurtain::WaitExec()
{
	pAnita(0u)->SetAnimNo(ANM_LIST[1], false);
}

bool cTaskConsMenuCurtain::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(ANM_LIST[0]);
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
