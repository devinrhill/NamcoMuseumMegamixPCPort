#include "kingdom/game/g01/cTaskConsMenuBar.hpp"

#include "KingdomGlobal.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/ResourceDatabase.hpp"

// Galaga stage select record bar. Every SetAnimNo of the DOL uses frame 0.0 (r2-0x6ae8); reset = 0 unless noted.
// Point / Decide / IsDpdHit are the base ones (vtable 0x801456d0 / 0x801456d4 / 0x801456bc).

namespace kingdom::game::g01
{

// DOL 0x8036fd90 (14 and 15 are swapped in the DOL table)
const int cTaskConsMenuBar::s_LayTbl[23] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 14, 16, 17, 18, 19, 20, 21, 22};

// DOL 0x8036fe50
const int cTaskConsMenuBar::s_NumberAnmTbl[10][5] = {
	{0x00, 0x01, -1, -1, 0x02}, {0x03, 0x04, -1, -1, 0x05}, {0x06, 0x07, -1, -1, 0x08}, {0x09, 0x0a, -1, -1, 0x0b},
	{0x0c, 0x0d, -1, -1, 0x0e}, {0x0f, 0x10, -1, -1, 0x11}, {0x12, 0x13, -1, -1, 0x14}, {0x15, 0x16, -1, -1, 0x17},
	{0x18, 0x19, -1, -1, 0x1a}, {0x1b, 0x1c, -1, -1, 0x1d}};

// DOL 0x8036ff18
const int cTaskConsMenuBar::s_MarkAnmTbl[5] = {0x4f, 0x50, -1, -1, 0x51};

// DOL 0x8036ff30
const int cTaskConsMenuBar::s_RankAnmTbl[8][5] = {
	{0x31, 0x32, -1, -1, 0x33}, {0x34, 0x35, -1, -1, 0x36}, {0x37, 0x38, -1, -1, 0x39}, {0x3a, 0x3b, -1, -1, 0x3c},
	{0x3d, 0x3e, -1, -1, 0x3f}, {0x40, 0x41, -1, -1, 0x42}, {0x43, 0x44, -1, -1, 0x45}, {0x46, 0x47, -1, -1, 0x48}};

// DOL 0x8036ffd0 / 0x8036ffe4 / 0x8036fff8
const int cTaskConsMenuBar::s_Anita14AnmTbl[5] = {0x1e, 0x1f, -1, -1, 0x20};
const int cTaskConsMenuBar::s_Anita10AnmTbl[5] = {0x21, 0x22, -1, -1, 0x23};
const int cTaskConsMenuBar::s_Anita15AnmTbl[5] = {0x24, 0x25, -1, -1, 0x26};

// DOL 0x8036fdec
const int cTaskConsMenuBar::s_FrameAnmTbl[5] = {0x49, 0x4a, -1, -1, 0x4b};

// DOL 0x800bb090: anitas 0 and 24 get ANT_MENU (crc 0xd3a909fe) hidden; anitas 1..23 get ANT_MENU, SetAnimNo(s_NumberAnmTbl[0][1],
// reset = 1), hidden, and their position from layer s_LayTbl[i - 1] of pattern 4 (FUN_80014408 into anita +0x400).
cTaskConsMenuBar::cTaskConsMenuBar(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	m_nWorldNo = -1;
	m_nStageNo = -1;
	m_u1e0 = 0;
	m_u1e4 = 0;
	m_u1e8 = 0;
	m_u1ec = 0;
	m_n1f0 = 0;
	m_b1f4 = false;
	m_b1f6 = false;
	m_b1f5 = false;
	m_b1f7 = false;
	m_n1f8 = 0;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU"));
	pAnita(0u)->SetState(1u, false);
	pAnita(24u)->RegistData(ResourceDatabase::GetPath("ANT_MENU"));
	pAnita(24u)->SetState(1u, false);
	const int nAnm = s_NumberAnmTbl[0][eANM_WAIT];
	for (uint i = 1; i < 24; i++)
	{
		pAnita(i)->RegistData(ResourceDatabase::GetPath("ANT_MENU"));
		pAnita(i)->SetAnimNo(nAnm, true);
		pAnita(i)->SetState(1u, false);
		// UNVERIFIED: anita +0x400 taken as vTrans (the layout offset, as cTaskConsMultiMenuItem in g07 does it)
		pAnita(i)->GetLayerPos(pAnita(i)->vTrans(), (uint)s_LayTbl[i - 1], 4);
	}
}

// DOL 0x800bb2d0
cTaskConsMenuBar::~cTaskConsMenuBar()
{
}

// DOL 0x800bb328. Four digit groups (unsigned values, digit n = value % 10^(n+1) / 10^n); a digit is hidden when
// 10^n > value, it is a leading position (n != 0, last group n > 1) and the digit is 0; otherwise it plays its column and is shown.
void cTaskConsMenuBar::SetNumberAnm(int nAnm)
{
	auto digits = [this, nAnm](uint value, uint firstAnita, uint lastAnita, uint minShown)
	{
		uint pos = 0;
		for (uint anita = firstAnita; anita <= lastAnita; anita++, pos++)
		{
			uint mod = 10;
			uint pow = 1;
			for (uint k = 0; k < pos; k++)
			{
				mod *= 10;
				pow *= 10;
			}
			const uint digit = (value % mod) / pow;
			if (pow > value && pos >= minShown && digit == 0)
			{
				pAnita(anita)->SetState(1u, false);
			}
			else
			{
				pAnita(anita)->SetAnimNo(s_NumberAnmTbl[digit][nAnm], false);
				pAnita(anita)->SetState(1u);
			}
		}
	};
	digits(m_u1e0, 1, 6, 1);      // 0x800bb3b0
	digits(m_u1e8, 7, 9, 1);      // 0x800bb4e4
	digits(m_u1e4, 11, 13, 1);    // 0x800bb618
	digits(m_u1ec, 16, 19, 2);    // 0x800bb74c (cmplwi r28, 1)

	// 0x800bb880: anitas 20..23 play the same mark anim, shown by their flag
	const int nMark = s_MarkAnmTbl[nAnm];
	for (uint anita = 20; anita <= 23; anita++)
	{
		pAnita(anita)->SetAnimNo(nMark, false);
		switch (anita)
		{
		case 20: pAnita(anita)->SetState(1u, m_b1f4); break;
		case 21: pAnita(anita)->SetState(1u, m_b1f5); break;
		case 22: pAnita(anita)->SetState(1u, m_b1f6); break;
		case 23: pAnita(anita)->SetState(1u, m_b1f7); break;
		}
	}
	pAnita(14u)->SetAnimNo(s_Anita14AnmTbl[nAnm], false);
	pAnita(10u)->SetAnimNo(s_Anita10AnmTbl[nAnm], false);
	pAnita(15u)->SetAnimNo(s_Anita15AnmTbl[nAnm], false);
}

// DOL 0x800bbad4
void cTaskConsMenuBar::WaitExec()
{
	pAnita(0u)->SetAnimNo(s_FrameAnmTbl[eANM_WAIT], false);
	pAnita(24u)->SetAnimNo(s_RankAnmTbl[m_n1f0][eANM_WAIT], false);
	SetNumberAnm(eANM_WAIT);
}

// DOL 0x800bbb8c: step 0: the frame (anita 0) appears only if it is not shown yet; rank and anitas 1..23 are shown;
// step 1: done when anita 0 and anitas 20..24 all ended.
bool cTaskConsMenuBar::AppearExec()
{
	if (nStep() == 0)
	{
		if (!pAnita(0u)->IsState(1u))
		{
			pAnita(0u)->SetState(1u);
			pAnita(0u)->SetAnimNo(s_FrameAnmTbl[eANM_APPEAR], false);
		}
		pAnita(24u)->SetState(1u);
		pAnita(24u)->SetAnimNo(s_RankAnmTbl[m_n1f0][eANM_APPEAR], false);
		for (uint i = 1; i < 24; i++)
		{
			pAnita(i)->SetState(1u);
		}
		SetNumberAnm(eANM_APPEAR);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		for (uint i = 20; i < 25; i++)
		{
			if (!pAnita(i)->IsState(65536u))
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

// DOL 0x800bbd28: step 0 plays the DELETE column on anitas 1..23 (not on 0 / 24); step 1: done when anitas 20..24 all ended.
// UNVERIFIED: anita 24 keeps its WAIT / APPEAR anim here, so it only ends if that anim does not loop.
bool cTaskConsMenuBar::DeleteExec()
{
	if (nStep() == 0)
	{
		SetNumberAnm(eANM_DELETE);
		nStep()++;
	}
	else if (nStep() == 1)
	{
		for (uint i = 20; i < 25; i++)
		{
			if (!pAnita(i)->IsState(65536u))
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

// DOL 0x800bbdc0. The record is read through a cRecordStageAccess on the stack (vtable 0x803963e8).
void cTaskConsMenuBar::SetStage(int nWorldNo, int nStageNo)
{
	if (nWorldNo == m_nWorldNo && nStageNo == m_nStageNo)
	{
		return;
	}
	m_nWorldNo = nWorldNo;
	m_nStageNo = nStageNo;
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	sStageRecord* pRec = sys->pStageRecord(cG01GalagaSys::nStageIdx(m_nWorldNo, m_nStageNo));
	sScoreNorma* pNorma = sys->pScoreNorma(cG01GalagaSys::nStageIdx(m_nWorldNo, m_nStageNo));
	m_u1e0 = (uint)pRec->n10;
	m_u1ec = (uint)pRec->n08;
	m_u1e4 = (uint)pRec->n00;
	m_u1e8 = (uint)pNorma->nMaxKill1;
	m_n1f0 = pRec->n0c;
	m_b1f4 = pRec->n1c == 1;
	m_b1f6 = pRec->n18 == 1;
	m_b1f5 = pRec->n14 == 1;
	m_b1f7 = pRec->n20 == 1;
}

// DOL 0x800bbedc
void cTaskConsMenuBar::SetCurrentStageRecord()
{
	sStageRecord* pRec = KingdomGlobal::GalagaSys->pStageRecord(cG01GalagaSys::CURRENT);
	pRec->n10 = (s32)m_u1e0;
	pRec->n08 = (s32)m_u1ec;
	pRec->n00 = (s32)m_u1e4;
	pRec->n0c = m_n1f0;
}

}
