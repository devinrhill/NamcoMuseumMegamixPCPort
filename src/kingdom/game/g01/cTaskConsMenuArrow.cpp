#include "kingdom/game/g01/cTaskConsMenuArrow.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

// Galaga stage select arrow. Every SetAnimNo of the DOL passes reset = 0 and frame 0.0 (r2-0x6b24).

namespace kingdom::game::g01
{

// DOL 0x8036fd10 [audited]
const int cTaskConsMenuArrow::s_AnmTbl[2][eANM_MAX] = {{0x52, 0x53, 0x54, 0x55}, {0x56, 0x57, 0x58, 0x59}};

// DOL 0x8047b0d0 [audited] (r2-0x6b30)
const int cTaskConsMenuArrow::s_HitPatTbl[2] = {8, 9};

// DOL 0x800ba6d0 [audited]: layout crc 0xd3a909fe = ANT_MENU on anita 0 (FUN_80141708 with KingSys+0xcc), loaded hidden
cTaskConsMenuArrow::cTaskConsMenuArrow(cTaskConsMenuManBase* const& pConsMan, int nIndex, bool bActive)
	: cTaskConsMenuBase(pConsMan)
{
	m_nIndex = nIndex;
	m_bActive = bActive;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU"));
	pAnita(0u)->SetState(1u, false);
	m_nHitPat = s_HitPatTbl[nIndex];
}

// DOL 0x800ba7a0 [audited]
cTaskConsMenuArrow::~cTaskConsMenuArrow()
{
}

// DOL 0x800ba7f8 [audited]: inactive arrows never hit; otherwise the usual anita-0 hit test with pattern +0x1e0
bool cTaskConsMenuArrow::IsDpdHit(float x, float y)
{
	if (!m_bActive)
	{
		return false;
	}
	return DpdHitPat(x, y, m_nHitPat);
}

// DOL 0x800ba950 [audited]
void cTaskConsMenuArrow::WaitExec()
{
	pAnita(0u)->SetAnimNo(s_AnmTbl[m_nIndex][eANM_WAIT], false);
}

// DOL 0x800ba9e4 [audited]: step 0 shows the layout only when active, plays APPEAR; step 1 waits for the end (no switch back to WAIT here)
bool cTaskConsMenuArrow::AppearExec()
{
	if (nStep() == 0)
	{
		if (m_bActive)
		{
			pAnita(0u)->SetState(1u);
		}
		pAnita(0u)->SetAnimNo(s_AnmTbl[m_nIndex][eANM_APPEAR], false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// DOL 0x800baae4 [audited]
void cTaskConsMenuArrow::PointExec()
{
	pAnita(0u)->SetAnimNo(s_AnmTbl[m_nIndex][eANM_POINT], false);
}

// DOL 0x800bab78 [audited]
bool cTaskConsMenuArrow::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(s_AnmTbl[m_nIndex][eANM_DECIDE], false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
