#include "kingdom/game/g03/cTaskConsMenuRule.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

const int cTaskConsMenuRule::ANM_LIST[4] = {279, 280, 281, 282};

cTaskConsMenuRule::cTaskConsMenuRule(cTaskConsMenuManBase* pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(2147483652u);
	string fileName = "BIN_ANT_MENU_SELECT01";
	pAnita(0u)->RegistData(fileName);
	pAnita(0u)->SetAnimNo(ANM_LIST[1]);
	pAnita(0u)->SetState(1u, false);
}

cTaskConsMenuRule::~cTaskConsMenuRule()
{
}

// DOL 0x8005e344 [audited: described by the 43rd-session pass]: pattern 0x11 of anita 0
bool cTaskConsMenuRule::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, 17);
}

// DOL 0x8005e47c [audited: anim ids 279..282, step logic checked vs decomp]
void cTaskConsMenuRule::WaitExec()
{
	pAnita(0u)->SetAnimNo(ANM_LIST[1], false);
}

// DOL 0x8005e4b4 [audited: anim ids 279..282, step logic checked vs decomp]
bool cTaskConsMenuRule::AppearExec()
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

// DOL 0x8005e574 [audited: anim ids 279..282, step logic checked vs decomp]
void cTaskConsMenuRule::PointExec()
{
	pAnita(0u)->SetAnimNo(ANM_LIST[2], false);
}

// DOL 0x8005e5ac [audited: anim ids 279..282, step logic checked vs decomp]
bool cTaskConsMenuRule::DecideExec()
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
