#include "kingdom/game/g01/cTaskConsMenuMulti.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

// Galaga stage select, single / multi player console. Ported 1:1 from main.dol (no C# exists for Galaga).
// Every SetAnimNo of this class passes reset = 0 (also Appear / Decide, unlike g07).

namespace kingdom::game::g01
{

// DOL 0x803703b8 [audited]: [single, multi][appear, wait, point, decide]
const int cTaskConsMenuMulti::ANM_LIST[2][eANM_MAX] = {
	{0x9a, 0x9b, 0x9c, 0x9d},
	{0x9e, 0x9f, 0xa0, 0xa1},
};

// DOL 0x800be9b0 [audited]: no draw priority; WaitState (vtable +0x64) before the layout is loaded hidden
cTaskConsMenuMulti::cTaskConsMenuMulti(cTaskConsMenuManBase* const& pConsMan, uint bMulti)
	: cTaskConsMenuBase(pConsMan)
{
	m_bMulti = bMulti;
	m_n1dc = 0;
	WaitState();
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU"));
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x800bea7c [audited]
cTaskConsMenuMulti::~cTaskConsMenuMulti()
{
}

// table read inlined in 0x800bec0c / 0x800beca8 / 0x800bedb0 / 0x800bee4c: row = (-b | b) >> 31 (b != 0)
int cTaskConsMenuMulti::nGetAnm(int nAnm)
{
	return ANM_LIST[(m_bMulti != 0u) ? 1 : 0][nAnm];
}

// DOL 0x800bead4 [audited]: hit test of anita 0, pattern 12
bool cTaskConsMenuMulti::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, HIT_PAT);
}

// DOL 0x800bec0c [audited]
void cTaskConsMenuMulti::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetAnm(eANM_WAIT), false);
}

// DOL 0x800beca8 [audited]
bool cTaskConsMenuMulti::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetState(1u);
		pAnita(0u)->SetAnimNo(nGetAnm(eANM_APPEAR), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(0x10000u))
	{
		return true;
	}
	return false;
}

// DOL 0x800bedb0 [audited]
void cTaskConsMenuMulti::PointExec()
{
	pAnita(0u)->SetAnimNo(nGetAnm(eANM_POINT), false);
}

// DOL 0x800bee4c [audited]
bool cTaskConsMenuMulti::DecideExec()
{
	if (nStep() == 0)
	{
		m_n1dc = 0;
		pAnita(0u)->SetAnimNo(nGetAnm(eANM_DECIDE), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(0x10000u))
	{
		return true;
	}
	return false;
}

}
