#include "platform/devtools.hpp"
#include "kingdom/game/g01/cTaskConsMenuStage.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"

// Galaga stage select, stage button console. Ported 1:1 from main.dol (no C# exists for Galaga).
// Every SetAnimNo of this class passes reset = 0. Priority 0x80000001 while waiting, 0x80000004 while pointed (WaitState / PointState).

namespace kingdom::game::g01
{

// DOL 0x80370438 [audited]: [open, closed][world][stage][appear, wait, point, decide, delete]
const int cTaskConsMenuStage::ANM_LIST[2][3][3][eANM_MAX] = {
	{
		{{0x5e, 0x5f, 0x60, 0x61, 0x62}, {0x66, 0x67, 0x68, 0x69, 0x6a}, {0x6e, 0x6f, 0x70, 0x71, 0x72}},
		{{0x82, 0x83, 0x84, 0x85, 0x86}, {0x8a, 0x8b, 0x8c, 0x8d, 0x8e}, {0x92, 0x93, 0x94, 0x95, 0x96}},
		{{0xa9, 0xaa, 0xab, 0xac, 0xad}, {0xb1, 0xb2, 0xb3, 0xb4, 0xb5}, {0xb9, 0xba, 0xbb, 0xbc, 0xbd}},
	},
	{
		{{0x63, 0x64, -1, -1, 0x65}, {0x6b, 0x6c, -1, -1, 0x6d}, {0x73, 0x74, -1, -1, 0x75}},
		{{0x87, 0x88, -1, -1, 0x89}, {0x8f, 0x90, -1, -1, 0x91}, {0x97, 0x98, -1, -1, 0x99}},
		{{0xae, 0xaf, -1, -1, 0xb0}, {0xb6, 0xb7, -1, -1, 0xb8}, {0xbe, 0xbf, -1, -1, 0xc0}},
	},
};

uint& cTaskConsMenuStage::nWorldNo()
{
	return m_nWorldNo;
}

uint& cTaskConsMenuStage::nStageNo()
{
	return m_nStageNo;
}

bool& cTaskConsMenuStage::bActive()
{
	return m_bActive;
}

// table read inlined in every *Exec: row = cntlzw(byte +0x1e0) >> 5 (= !bActive), no range checks
int cTaskConsMenuStage::nGetStageAnm(int nAnm)
{
	return ANM_LIST[bActive() ? 0 : 1][nWorldNo()][nStageNo()][nAnm];
}

// DOL 0x800bf388 [audited]: priority 0x80000001, layout ANT_MENU loaded hidden, vTrans = position of layer nStageNo in pattern 1
// (FUN_80014408 into a stack vector, then copied to anita +0x400 = vTrans; the DOL copies even when the lookup fails)
cTaskConsMenuStage::cTaskConsMenuStage(cTaskConsMenuManBase* const& pConsMan, uint nWorldNo, uint nStageNo, uint bMulti)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x80000001u);
	this->nWorldNo() = nWorldNo;
	this->nStageNo() = nStageNo;
	bActive() = false;
	m_bMulti = bMulti;
	m_n1e8 = 0;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU"));
	pAnita(0u)->SetState(1u, false);
	Vector4 pos(0.0f, 0.0f, 0.0f, 1.0f);   // UNVERIFIED: uninitialised stack vector in the DOL (only matters if the lookup fails)
	pAnita(0u)->GetLayerPos(pos, nStageNo, SET_STAGE_PAT);
	pAnita(0u)->vTrans() = pos;
}

// DOL 0x800bf4c8 [audited]
cTaskConsMenuStage::~cTaskConsMenuStage()
{
}

// DOL 0x800bf520 [audited]: world store, then bActive = (GalagaSys->pStageRecord(world, nStageNo)->bOpen == 1)
void cTaskConsMenuStage::LoadStageData(uint nWorldNo)
{
	this->nWorldNo() = nWorldNo;
	bActive() = KingdomGlobal::GalagaSys->pStageRecord((int)nWorldNo, (int)nStageNo())->bOpen == 1;
	if (devtools::PreviewAll()) bActive() = true; // devtools: --preview-all (not 1:1)
}

// DOL 0x800bf57c [audited]: only while active, hit test of anita 0, pattern 11
bool cTaskConsMenuStage::IsDpdHit(float x, float y)
{
	if (!bActive())
	{
		return false;
	}
	return DpdHitPat(x, y, HIT_PAT);
}

// DOL 0x800bfaf4 [audited]
void cTaskConsMenuStage::WaitState()
{
	nDrawPriority(0x80000001u);
	nState() = eSTATE_WAIT;
	nStep() = 0;
}

// DOL 0x800bfb44 [audited]
void cTaskConsMenuStage::PointState()
{
	nDrawPriority(0x80000004u);
	nState() = eSTATE_POINT;
	nStep() = 0;
}

// DOL 0x800bf6c8 [audited]
void cTaskConsMenuStage::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetStageAnm(eANM_WAIT), false);
}

// DOL 0x800bf76c [audited]
bool cTaskConsMenuStage::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetState(1u);
		pAnita(0u)->SetAnimNo(nGetStageAnm(eANM_APPEAR), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(0x10000u))
	{
		return true;
	}
	return false;
}

// DOL 0x800bf870 [audited]: only while active (a closed stage has no point anim)
void cTaskConsMenuStage::PointExec()
{
	if (bActive())
	{
		pAnita(0u)->SetAnimNo(nGetStageAnm(eANM_POINT), false);
	}
}

// DOL 0x800bf91c [audited] (no bActive check: a closed stage would play anim -1, UNVERIFIED what cJetAnita does with it; the manager
// presumably never decides a closed stage)
bool cTaskConsMenuStage::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetStageAnm(eANM_DECIDE), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(0x10000u))
	{
		return true;
	}
	return false;
}

// DOL 0x800bfa08 [audited]
bool cTaskConsMenuStage::DeleteExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetStageAnm(eANM_DELETE), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(0x10000u))
	{
		return true;
	}
	return false;
}

}
