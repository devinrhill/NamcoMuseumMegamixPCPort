#include "kingdom/game/g01/cTaskConsMenuBack.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

// Galaga stage select RETURN button. Every SetAnimNo of the DOL passes reset = 0 and frame 0.0 (r2-0x6b04).

namespace kingdom::game::g01
{

// DOL 0x8036fd58
const int cTaskConsMenuBack::s_AnmTbl[eANM_MAX] = {0x27, 0x28, 0x29, 0x2a};

// DOL 0x800bac5c: layout crc 0xd3a909fe = ANT_MENU on anita 0, loaded hidden
cTaskConsMenuBack::cTaskConsMenuBack(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	m_n1d8 = 0;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU"));
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x800bad0c
cTaskConsMenuBack::~cTaskConsMenuBack()
{
}

// DOL 0x800bad64: anita-0 hit test, pattern 6
bool cTaskConsMenuBack::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, HIT_PAT);
}

// DOL 0x800bae9c
void cTaskConsMenuBack::WaitExec()
{
	pAnita(0u)->SetAnimNo(s_AnmTbl[eANM_WAIT], false);
}

// DOL 0x800baee8: step 0 shows the layout and plays APPEAR; step 1 waits for the end
bool cTaskConsMenuBack::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetState(1u);
		pAnita(0u)->SetAnimNo(s_AnmTbl[eANM_APPEAR], false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// DOL 0x800baf9c
void cTaskConsMenuBack::PointExec()
{
	pAnita(0u)->SetAnimNo(s_AnmTbl[eANM_POINT], false);
}

// DOL 0x800bafe8
bool cTaskConsMenuBack::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(s_AnmTbl[eANM_DECIDE], false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
