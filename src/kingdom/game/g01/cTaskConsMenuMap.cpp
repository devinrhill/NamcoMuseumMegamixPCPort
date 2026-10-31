#include "kingdom/game/g01/cTaskConsMenuMap.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

// Galaga stage select, stage map console. Ported 1:1 from main.dol (no C# exists for Galaga).
// The g01 Map has no asserts on world / stage (unlike the g07 one) and no 3D model.

namespace kingdom::game::g01
{

// DOL 0x803702d8 [audited]: [world 0..2][stage 0..2][appear, wait, point, decide, delete]
const int cTaskConsMenuMap::ANM_LIST[3][3][eANM_MAX] = {
	{{0, 1, 2, 3, 4}, {5, 6, 7, 8, 9}, {10, 11, 12, 13, 14}},
	{{15, 16, 17, 18, 19}, {20, 21, 22, 23, 24}, {25, 26, 27, 28, 29}},
	{{30, 31, 32, 33, 34}, {35, 36, 37, 38, 39}, {40, 41, 42, 43, 44}},
};

uint& cTaskConsMenuMap::nWorldNo()
{
	return m_nWorldNo;
}

uint& cTaskConsMenuMap::nStageNo()
{
	return m_nStageNo;
}

// DOL 0x800be39c [audited]: priority 0x10002, layout ANT_STAGE (crc 0xcabf0277) loaded hidden, vTrans = layer 0 position of pattern 0 (FUN_80014408)
cTaskConsMenuMap::cTaskConsMenuMap(cTaskConsMenuManBase* const& pConsMan, uint nWorldNo, uint nStageNo)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x10002u);
	this->nWorldNo() = nWorldNo;
	this->nStageNo() = nStageNo;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_STAGE"));
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), 0u, 0);
}

// DOL 0x800be950 [audited]
cTaskConsMenuMap::~cTaskConsMenuMap()
{
}

// DOL 0x800bd978 [audited]: plain stores, no asserts
void cTaskConsMenuMap::ChangeData(uint nWorldNo, uint nStageNo)
{
	this->nWorldNo() = nWorldNo;
	this->nStageNo() = nStageNo;
}

// DOL 0x800be49c [audited: world 3 stage 3 reads past the DOL's stack copy, -1 natively, Dolphin look pending] (table 0x803702d8 copied to the stack, 45 words; index world * 15 + stage * 5 + state, no range check). Stage 3 (the boss
// slot, set by InitStageNo when stages 1..9 are open but the boss is not cleared) in world 3 reads past the copy: stack garbage in the DOL
// (what the Wii then shows is UNVERIFIED, check in Dolphin). Native: -1 there (SetAnimNo ignores it) instead of reading past the array.
int cTaskConsMenuMap::nGetMapAnm(uint nState)
{
	const uint i = nWorldNo() * 15u + nStageNo() * (uint)eANM_MAX + nState;
	if (i >= 3u * 3u * (uint)eANM_MAX) return -1;
	return (&ANM_LIST[0][0][0])[i];
}

// DOL 0x800be500 [audited]: hit test of anita 0, pattern 1
bool cTaskConsMenuMap::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, HIT_MAP_PAT);
}

// DOL 0x800be638 [audited]
void cTaskConsMenuMap::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetMapAnm(eANM_WAIT), false);
}

// DOL 0x800be69c [audited] (shows the layout first, then the appear anim with reset)
bool cTaskConsMenuMap::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetState(1u);
		pAnita(0u)->SetAnimNo(nGetMapAnm(eANM_APPEAR), true);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(0x10000u))
	{
		return true;
	}
	return false;
}

// DOL 0x800be764 [audited]
void cTaskConsMenuMap::PointExec()
{
	pAnita(0u)->SetAnimNo(nGetMapAnm(eANM_POINT), false);
}

// DOL 0x800be7c8 [audited] (SetAnimNo with reset = 0, li r5, 0 at 0x800be818)
bool cTaskConsMenuMap::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetMapAnm(eANM_DECIDE), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(0x10000u))
	{
		return true;
	}
	return false;
}

// DOL 0x800be874 [audited]: at the end hides the layout and calls DeleteEndState itself (vtable +0x8c) before returning 1
// (the base Execute then calls DeleteEndState once more: harmless, it only stores state 8 / step 0)
bool cTaskConsMenuMap::DeleteExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetMapAnm(eANM_DELETE), true);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(0x10000u))
	{
		pAnita(0u)->SetState(1u, false);
		DeleteEndState();
		return true;
	}
	return false;
}

}
