#include "kingdom/game/g01/cTaskConsMenuRule.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

// Galaga stage select, rule console. Ported 1:1 from main.dol (no C# exists for Galaga).
// Every SetAnimNo of this class passes reset = 0 (also Appear / Decide, unlike g07).

namespace kingdom::game::g01
{

// DOL 0x80370400 [audited]: appear, wait, point, decide
const int cTaskConsMenuRule::ANM_LIST[eANM_MAX] = {0x5a, 0x5b, 0x5c, 0x5d};

// DOL 0x800bef54 [audited]: no draw priority; layout ANT_MENU loaded hidden
cTaskConsMenuRule::cTaskConsMenuRule(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	m_n1d8 = 0;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU"));
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x800bf004 [audited]
cTaskConsMenuRule::~cTaskConsMenuRule()
{
}

// DOL 0x800bf05c [audited]: hit test of anita 0, pattern 10
bool cTaskConsMenuRule::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, HIT_PAT);
}

// DOL 0x800bf194 [audited]
void cTaskConsMenuRule::WaitExec()
{
	pAnita(0u)->SetAnimNo(ANM_LIST[eANM_WAIT], false);
}

// DOL 0x800bf1e0 [audited]
bool cTaskConsMenuRule::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetState(1u);
		pAnita(0u)->SetAnimNo(ANM_LIST[eANM_APPEAR], false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(0x10000u))
	{
		return true;
	}
	return false;
}

// DOL 0x800bf294 [audited]
void cTaskConsMenuRule::PointExec()
{
	pAnita(0u)->SetAnimNo(ANM_LIST[eANM_POINT], false);
}

// DOL 0x800bf2e0 [audited]
bool cTaskConsMenuRule::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(ANM_LIST[eANM_DECIDE], false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(0x10000u))
	{
		return true;
	}
	return false;
}

}
