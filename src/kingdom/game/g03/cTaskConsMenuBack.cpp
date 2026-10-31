#include "kingdom/game/g03/cTaskConsMenuBack.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

const int cTaskConsMenuBack::ANM_LIST[4] = {306, 307, 308, 309};

cTaskConsMenuBack::cTaskConsMenuBack(cTaskConsMenuManBase* pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(2147483654u);
	string fileName = "BIN_ANT_MENU_SELECT01";
	pAnita(0u)->RegistData(fileName);
	pAnita(0u)->SetAnimNo(ANM_LIST[1]);
	pAnita(0u)->SetState(1u, false);
}

cTaskConsMenuBack::~cTaskConsMenuBack()
{
}

// DOL 0x8005a7f0 [audited: described by the 43rd-session pass]: pattern 0x16 of anita 0
bool cTaskConsMenuBack::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, 22);
}

void cTaskConsMenuBack::WaitExec()
{
	pAnita(0u)->SetAnimNo(ANM_LIST[1], false);
}

bool cTaskConsMenuBack::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(ANM_LIST[0]);
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		pAnita(0u)->SetAnimNo(ANM_LIST[1]);
		return true;
	}
	return false;
}

void cTaskConsMenuBack::PointExec()
{
	pAnita(0u)->SetAnimNo(ANM_LIST[2], false);
}

bool cTaskConsMenuBack::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(ANM_LIST[3]);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
