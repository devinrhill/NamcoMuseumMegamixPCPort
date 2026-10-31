#include "kingdom/game/g02/cTaskConsMenuConsoles.hpp"

#include "KingdomGlobal.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/game/menu/sTutorialData.hpp"
#include "kingdom/game/menu/sTutorialPage.hpp"

// Rally-X Remix (g02) stage select consoles, ported 1:1 from main.dol (no C# exists for Rally-X). The DOL hides / shows an anita by clearing /
// setting bit 1 of its state word (anita +4): SetState(1u, false / true) here; "the animation ended" is state bit 0x10000 (IsState(65536u)).
// IsDpdHit of every console is the shared body (pointer -> layout pixels, cKingdomAnita::IsHit on anita 0 with the console's pattern):
// cTaskConsMenuBase::DpdHitPat.

namespace kingdom::game::g02
{

namespace
{
const char* const LAYOUT = "ANITA_MENU_SELECT_01";   // crc 0x2d34b4f3 (g02menu.lzs)

// FUN_80141708 (RegistData) of the menu layout on anita n, loaded hidden unless told otherwise
void Regist(kingdom::cKingdomAnita* a, const char* name)
{
	a->RegistData(ResourceDatabase::GetPath(name));
}
}

// ---- cTaskConsMenuArrow ---------------------------------------------------------------------------------------------------------------

// DOL 0x803768d0 [audited] [index][appear, wait, point, decide]
const int cTaskConsMenuArrow::s_AnmTbl[2][4] = {{0x101, 0x102, 0x103, 0x104}, {0x105, 0x106, 0x107, 0x108}};

namespace
{
const int s_ArrowHitPat[2] = {0x1a, 0x1b};   // DOL 0x8047b8e0 [audited]
const int s_ArrowLayPat[2] = {4, 5};         // DOL 0x8047b8e8 [audited]
}

// DOL 0x80105aa4 [audited]: priority 0x80000007, wait anim (reset), position = layer 0 of pattern s_ArrowLayPat[index] (FUN_80014408 into anita +0x400
// = vTrans), hidden
cTaskConsMenuArrow::cTaskConsMenuArrow(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nIndex, bool bActive)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x80000007u);
	m_nIndex = nIndex;
	m_bActive = bActive;
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(s_AnmTbl[m_nIndex][1], true);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), 0u, s_ArrowLayPat[nIndex]);
	pAnita(0u)->SetState(1u, false);
	m_nHitPat = s_ArrowHitPat[m_nIndex];
}

// DOL 0x80105bf8 [audited]
cTaskConsMenuArrow::~cTaskConsMenuArrow()
{
}

// DOL 0x80105c50 [audited]: an inactive arrow never hits
bool cTaskConsMenuArrow::IsDpdHit(float x, float y)
{
	if (!m_bActive) return false;
	return DpdHitPat(x, y, m_nHitPat);
}

// DOL 0x80105da8 [audited]
void cTaskConsMenuArrow::WaitExec()
{
	pAnita(0u)->SetAnimNo(s_AnmTbl[m_nIndex][1], false);
}

// DOL 0x80105e58 [audited]: shown only when active
bool cTaskConsMenuArrow::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(s_AnmTbl[m_nIndex][0], true);
		if (m_bActive) pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// DOL 0x80105e00 [audited]
void cTaskConsMenuArrow::PointExec()
{
	pAnita(0u)->SetAnimNo(s_AnmTbl[m_nIndex][2], false);
}

// DOL 0x80105f18 [audited]
bool cTaskConsMenuArrow::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(s_AnmTbl[m_nIndex][3], true);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// ---- the shared four-column button pattern (Back, Multi, Rule: identical bodies in the DOL) ---------------------------------------------
// WaitExec: GetAnm(1) (no reset); AppearExec: step 0 GetAnm(0) (reset) + show, step 1 at the end GetAnm(1) (reset) and done; PointExec:
// GetAnm(2) (no reset); DecideExec: step 0 GetAnm(3) (reset), step 1 done at the end.
namespace
{
template <class T> bool ButtonAppear(T* p, int& step)
{
	if (step == 0)
	{
		p->pAnita(0u)->SetAnimNo(p->GetAnm(0), true);
		p->pAnita(0u)->SetState(1u);
		step++;
	}
	else if (step == 1 && p->pAnita(0u)->IsState(65536u))
	{
		p->pAnita(0u)->SetAnimNo(p->GetAnm(1), true);
		return true;
	}
	return false;
}
template <class T> bool ButtonDecide(T* p, int& step)
{
	if (step == 0)
	{
		p->pAnita(0u)->SetAnimNo(p->GetAnm(3), true);
		step++;
	}
	else if (step == 1 && p->pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}
}

// ---- cTaskConsMenuBack ------------------------------------------------------------------------------------------------------------------

// DOL 0x80105fc0 [audited] (the base ctor only; SetData follows)
cTaskConsMenuBack::cTaskConsMenuBack(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

// DOL 0x80106004 [audited]
cTaskConsMenuBack::~cTaskConsMenuBack()
{
}

// DOL 0x8010605c [audited]: hit pattern 0x1d, priority 0x80000009, wait anim (reset), hidden, position = layer 0 of pattern 2
void cTaskConsMenuBack::SetData()
{
	m_nHitPat = 0x1d;
	nDrawPriority(0x80000009u);
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(GetAnm(1), true);
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), 0u, 2);
}

// DOL 0x8010616c [audited] (table 0x80376918)
int cTaskConsMenuBack::GetAnm(int n)
{
	static const int s_Anm[4] = {0x109, 0x10a, 0x10b, 0x10c};
	return s_Anm[n];
}

// DOL 0x801061a8 [audited]
bool cTaskConsMenuBack::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, m_nHitPat);
}

void cTaskConsMenuBack::WaitExec() { pAnita(0u)->SetAnimNo(GetAnm(1), false); }    // DOL 0x801062ec [audited]
bool cTaskConsMenuBack::AppearExec() { return ButtonAppear(this, nStep()); }        // DOL 0x80106350 [audited]
void cTaskConsMenuBack::PointExec() { pAnita(0u)->SetAnimNo(GetAnm(2), false); }   // DOL 0x8010644c [audited]
bool cTaskConsMenuBack::DecideExec() { return ButtonDecide(this, nStep()); }        // DOL 0x801064b0 [audited]

// ---- cTaskConsMenuBar -------------------------------------------------------------------------------------------------------------------

namespace
{
const int s_BarFrameAnm[4][3] = {{0xf5, 0xf6, 0xf7}, {0xf8, 0xf9, 0xfa}, {0xfb, 0xfc, 0xfd}, {0xfe, 0xff, 0x100}};   // DOL 0x80376950 [audited] [world]
const int s_BarNewAnm[3] = {0x3c, 0x3d, 0x3e};        // DOL 0x80376980 [audited] [col]: the "new record" mark
const int s_BarDigitAnm[10][3] = {{0x00, 0x01, 0x02}, {0x03, 0x04, 0x05}, {0x06, 0x07, 0x08}, {0x09, 0x0a, 0x0b}, {0x0c, 0x0d, 0x0e},
                                  {0x0f, 0x10, 0x11}, {0x12, 0x13, 0x14}, {0x15, 0x16, 0x17}, {0x18, 0x19, 0x1a}, {0x1b, 0x1c, 0x1d}};   // DOL 0x80376990 [audited]
const int s_BarNoScoreAnm[3] = {0x11d, 0x11e, 0x11f};   // DOL 0x80376a08 [audited] [col]: no score (negative)
const int s_BarColonAnm[3] = {0x1e, 0x1f, 0x20};        // DOL 0x80376a14 [audited] [col]: anitas 15 / 16
const int s_BarLayNew[2] = {1, 0};                      // DOL 0x8047b930 [audited]: anitas 1, 2 (pattern 0x10, DAT_80477ae0)
const int s_BarLayScore[6] = {5, 4, 3, 2, 1, 0};        // DOL 0x80376a20 [audited]: anitas 3..8 (pattern 0x21, DAT_80477ae4)
const int s_BarLayTime[6] = {7, 6, 4, 3, 1, 0};         // DOL 0x80376a38 [audited]: anitas 9..14 (pattern 0x22, DAT_80477ae8)
const int s_BarLayColon[2] = {5, 2};                    // DOL 0x8047b938 [audited]: anitas 15, 16 (pattern 0x22, DAT_80477aec)

// the digit loops: 10^pos (pos = digits right of this one)
u32 Pow10(u32 pos)
{
	u32 p = 1;
	for (u32 i = 0; i < pos; i++) p *= 10;
	return p;
}
}

// DOL 0x80106564 [audited]: priority 0x80000001; anita 0 = the frame (world 0's wait anim, reset), hidden; anitas 1..16 load the layout too, each
// with its start anim (reset), hidden, at a layer position: 1..2 new marks (anim 0x3d), 3..8 score digits, 9..14 time digits (anim 1),
// 15..16 colons (anim 0x1f)
cTaskConsMenuBar::cTaskConsMenuBar(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x80000001u);
	m_nWorldNo = 0;
	m_nStageIdx = -1;
	m_nTime = 0u;
	m_nScore = 0;
	m_bNewTime = false;
	m_bNewScore = false;
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(s_BarFrameAnm[m_nWorldNo][1], true);
	pAnita(0u)->SetState(1u, false);
	auto part = [this](uint i, int anm, int lay, int pat) {
		kingdom::cKingdomAnita* a = pAnita(i);
		Regist(a, LAYOUT);
		a->SetAnimNo(anm, true);
		a->SetState(1u, false);
		a->GetLayerPos(a->vTrans(), (uint)lay, pat);
	};
	for (uint i = 1; i < 3; i++) part(i, 0x3d, s_BarLayNew[i - 1], 0x10);
	for (uint i = 3; i < 9; i++) part(i, 1, s_BarLayScore[i - 3], 0x21);
	for (uint i = 9; i < 15; i++) part(i, 1, s_BarLayTime[i - 9], 0x22);
	for (uint i = 15; i < 17; i++) part(i, 0x1f, s_BarLayColon[i - 15], 0x22);
}

// DOL 0x80106940 [audited]
cTaskConsMenuBar::~cTaskConsMenuBar()
{
}

// DOL 0x80106998 [audited]: anitas 3..8 = the six score digits (8 = ones). No score (negative): the "no score" anim (reset), shown. Otherwise a digit
// shows its number (no reset) when it is the ones digit or 10^pos <= score; leading zeros play digit 0 (reset) and are hidden. Anita 1 = the
// new mark, shown when the score is new.
void cTaskConsMenuBar::SetScoreAnm(int nCol)
{
	for (uint i = 3; i < 9; i++)
	{
		kingdom::cKingdomAnita* a = pAnita(i);
		if (m_nScore < 0)
		{
			a->SetAnimNo(s_BarNoScoreAnm[nCol], true);
			a->SetState(1u);
			continue;
		}
		const u32 v = (u32)m_nScore;
		const u32 pow = Pow10(8 - i);
		const u32 mod = pow * 10u;
		if (i == 8 || pow <= v)
		{
			a->SetAnimNo(s_BarDigitAnm[(v % mod) / pow][nCol], false);
			a->SetState(1u);
		}
		else
		{
			a->SetAnimNo(s_BarDigitAnm[0][nCol], true);
			a->SetState(1u, false);
		}
	}
	pAnita(1u)->SetAnimNo(s_BarNewAnm[nCol], true);
	pAnita(1u)->SetState(1u, m_bNewScore);
}

// DOL 0x80106b88 [audited]: minutes (anitas 9..10, a leading zero hidden), seconds (11..12) and hundredths (13..14) of the time in ms; anita 2 = the
// new mark; 15 / 16 = the separators
void cTaskConsMenuBar::SetTimeAnm(int nCol)
{
	const u32 min = m_nTime / 60000u;
	const u32 sec = (m_nTime % 60000u) / 1000u;
	const u32 cs = (m_nTime % 1000u) / 10u;
	for (uint i = 9; i < 11; i++)
	{
		const u32 pow = Pow10(10 - i);
		const u32 mod = pow * 10u;
		if (i == 10 || pow <= min)
		{
			pAnita(i)->SetAnimNo(s_BarDigitAnm[(min % mod) / pow][nCol], false);
			pAnita(i)->SetState(1u);
		}
		else
		{
			pAnita(i)->SetAnimNo(s_BarDigitAnm[0][nCol], true);
			pAnita(i)->SetState(1u, false);
		}
	}
	for (uint i = 11; i < 13; i++)
	{
		const u32 pow = Pow10(12 - i);
		pAnita(i)->SetAnimNo(s_BarDigitAnm[(sec % (pow * 10u)) / pow][nCol], false);
	}
	for (uint i = 13; i < 15; i++)
	{
		const u32 pow = Pow10(14 - i);
		pAnita(i)->SetAnimNo(s_BarDigitAnm[(cs % (pow * 10u)) / pow][nCol], false);
	}
	pAnita(2u)->SetAnimNo(s_BarNewAnm[nCol], true);
	pAnita(2u)->SetState(1u, m_bNewTime);
	pAnita(15u)->SetAnimNo(s_BarColonAnm[nCol], true);
	pAnita(16u)->SetAnimNo(s_BarColonAnm[nCol], true);
}

// DOL 0x80106f24 [audited] (the frame's wait anim is set with reset every frame, as in the DOL)
void cTaskConsMenuBar::WaitExec()
{
	pAnita(0u)->SetAnimNo(s_BarFrameAnm[m_nWorldNo][1], true);
	SetScoreAnm(1);
	SetTimeAnm(1);
}

// DOL 0x80106f94 [audited]: step 0: the frame appears only when it is hidden; anitas 9..16 are shown; step 1: done when anitas 3..16 all ended
bool cTaskConsMenuBar::AppearExec()
{
	if (nStep() == 0)
	{
		if (!pAnita(0u)->IsState(1u))
		{
			pAnita(0u)->SetAnimNo(s_BarFrameAnm[m_nWorldNo][0], true);
			pAnita(0u)->SetState(1u);
		}
		for (uint i = 9; i < 17; i++) pAnita(i)->SetState(1u);
		SetScoreAnm(0);
		SetTimeAnm(0);
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool end = true;
		for (uint i = 3; i < 17 && end; i++) end = pAnita(i)->IsState(65536u);
		if (end) return true;
	}
	return false;
}

// DOL 0x801070c0 [audited]: the digits play their delete column (the frame stays)
bool cTaskConsMenuBar::DeleteExec()
{
	if (nStep() == 0)
	{
		SetScoreAnm(2);
		SetTimeAnm(2);
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool end = true;
		for (uint i = 3; i < 17 && end; i++) end = pAnita(i)->IsState(65536u);
		if (end) return true;
	}
	return false;
}

// DOL 0x80107164 [audited]: the record of stage index idx (0x800c62a8 score, 0x800c62c0 time, 0x800c62d8 / 0x800c62fc new flags)
void cTaskConsMenuBar::SetStage(int nStageIdx)
{
	if (nStageIdx == m_nStageIdx) return;
	m_nStageIdx = nStageIdx;
	const cG02RallyXRecord& rec = cG02RallyXRecord::Get();
	m_nScore = (s32)rec.GetScore(nStageIdx);
	m_nTime = rec.GetTime(nStageIdx);
	m_bNewScore = rec.IsNewScore(nStageIdx);
	m_bNewTime = rec.IsNewTime(nStageIdx);
}

// DOL 0x801071d8 [audited]
void cTaskConsMenuBar::SetWorld(int nWorldNo)
{
	m_nWorldNo = nWorldNo;
	pAnita(0u)->SetAnimNo(s_BarFrameAnm[m_nWorldNo][1], true);
}

// ---- cTaskConsMenuBg --------------------------------------------------------------------------------------------------------------------

// DOL 0x8010723c [audited]
cTaskConsMenuBg::cTaskConsMenuBg(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nWorldNo)
	: cTaskConsMenuBase(pConsMan)
{
	m_nWorldNo = nWorldNo;
}

// DOL 0x80107290 [audited]
cTaskConsMenuBg::~cTaskConsMenuBg()
{
}

// DOL 0x801072e8 [audited]: priority 0x10001, ANITA_MENU_BG (crc 0xc2669bd3), the world's anim (reset); not hidden
void cTaskConsMenuBg::SetData()
{
	nDrawPriority(0x10001u);
	Regist(pAnita(0u), "ANITA_MENU_BG");
	pAnita(0u)->SetAnimNo(GetAnm(m_nWorldNo), true);
}

// DOL 0x801073ac [audited] (table 0x80376b10; called with the world number)
int cTaskConsMenuBg::GetAnm(int n)
{
	static const int s_Anm[4] = {0, 1, 2, 3};
	return s_Anm[n];
}

// DOL 0x801073e8 [audited]: the new world's anim (no reset), done at once
bool cTaskConsMenuBg::AppearExec()
{
	pAnita(0u)->SetAnimNo(GetAnm(m_nWorldNo), false);
	return true;
}

// ---- cTaskConsMenuController ------------------------------------------------------------------------------------------------------------

// DOL 0x80107458 [audited]
cTaskConsMenuController::cTaskConsMenuController(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nType)
	: cTaskConsMenuBase(pConsMan)
{
	m_nType = nType;
}

// DOL 0x801074ac [audited]
cTaskConsMenuController::~cTaskConsMenuController()
{
}

// DOL 0x80107504 [audited]: priority 0x80000002, hit pattern 0x16, wait anim (reset), hidden (no layer position)
void cTaskConsMenuController::SetData()
{
	nDrawPriority(0x80000002u);
	m_nHitPat = 0x16;
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(GetAnm(1), true);
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x801075e8 [audited] (table 0x80376b48 [type - 1][n])
int cTaskConsMenuController::GetAnm(int n)
{
	static const int s_Anm[2][4] = {{0x44, 0x45, 0x46, 0x47}, {0x3f, 0x40, 0x41, 0x42}};
	return s_Anm[m_nType - 1][n];
}

// DOL 0x80107654 [audited]
bool cTaskConsMenuController::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, m_nHitPat);
}

// DOL 0x80107798 [audited] (no step-1 test: any later frame checks the end)
bool cTaskConsMenuController::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(GetAnm(0), true);
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsMenuController::WaitExec() { pAnita(0u)->SetAnimNo(GetAnm(1), false); }    // DOL 0x80107854 [audited]
void cTaskConsMenuController::PointExec() { pAnita(0u)->SetAnimNo(GetAnm(2), false); }   // DOL 0x801078b8 [audited]

// DOL 0x8010791c [audited]: the switch anim (column 3), then the next type (1 -> 2 -> 1)
bool cTaskConsMenuController::NextExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(GetAnm(3), true);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		if (2 < ++m_nType) m_nType = 1;
		return true;
	}
	return false;
}

// DOL 0x801079e4 [audited]: the previous type (nothing calls PrevState in this build)
bool cTaskConsMenuController::PrevExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(GetAnm(3), true);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		if (--m_nType < 1) m_nType = 2;
		return true;
	}
	return false;
}

void cTaskConsMenuController::NextState() { nState() = 11; nStep() = 0; }        // DOL 0x80107cd0 [audited]
void cTaskConsMenuController::NextEndState() { nState() = 12; nStep() = 0; }     // DOL 0x80107c6c [audited]
bool cTaskConsMenuController::IsNextEndState() { return nState() == 12; }        // DOL 0x80107cbc [audited]
void cTaskConsMenuController::PrevState() { nState() = 9; nStep() = 0; }         // DOL 0x80107ca8 [audited]
void cTaskConsMenuController::PrevEndState() { nState() = 10; nStep() = 0; }     // DOL 0x80107c80 [audited]
bool cTaskConsMenuController::IsPrevEndState() { return nState() == 10; }        // DOL 0x80107c94 [audited]

// DOL 0x80107aac [audited]: the base state machine (cTaskConsMenuBase::Execute 0x80145740) plus 11 (next) and 9 (previous); unknown states do nothing
// (the base asserts)
bool cTaskConsMenuController::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::KingSys->pAppMain()->IsPause()) return true;
	cTaskAnita::Execute(pParam);   // FUN_80014d58 (the anita vector)
	switch (nState())
	{
	case 1: WaitExec(); break;
	case 2:
		if (AppearExec()) AppearEndState();
		break;
	case 4: PointExec(); break;
	case 5:
		if (DecideExec()) DecideEndState();
		break;
	case 7:
		if (DeleteExec()) DeleteEndState();
		break;
	case 11:
		if (NextExec()) NextEndState();
		break;
	case 9:
		if (PrevExec()) PrevEndState();
		break;
	default: break;
	}
	return true;
}

// ---- cTaskConsMenuCurtain ---------------------------------------------------------------------------------------------------------------

// DOL 0x80107cec [audited]
cTaskConsMenuCurtain::cTaskConsMenuCurtain(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nWorldNo)
	: cTaskConsMenuBase(pConsMan)
{
	m_nWorldNo = nWorldNo;
}

// DOL 0x80107d40 [audited]
cTaskConsMenuCurtain::~cTaskConsMenuCurtain()
{
}

// DOL 0x80107f28 [audited]: priority 0x10002, wait anim (reset), hidden
void cTaskConsMenuCurtain::SetData()
{
	nDrawPriority(0x10002u);
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(GetAnm(1), true);
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x80107d98 [audited] (table 0x80376b98 [world][appear, wait])
int cTaskConsMenuCurtain::GetAnm(int n)
{
	static const int s_Anm[4][2] = {{0x111, 0x112}, {0x113, 0x114}, {0x115, 0x116}, {0x117, 0x118}};
	return s_Anm[m_nWorldNo][n];
}

void cTaskConsMenuCurtain::WaitExec() { pAnita(0u)->SetAnimNo(GetAnm(1), false); }   // DOL 0x80107e00 [audited]

// DOL 0x80107e64 [audited]
bool cTaskConsMenuCurtain::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(GetAnm(0), true);
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// DOL 0x80108004 [audited]
void cTaskConsMenuCurtain::SetWorld(int nWorldNo)
{
	m_nWorldNo = nWorldNo;
	pAnita(0u)->SetAnimNo(GetAnm(1), true);
}

// ---- cTaskConsMenuMulti -----------------------------------------------------------------------------------------------------------------

// DOL 0x80109dd8 [audited] (the base ctor only; SetData follows)
cTaskConsMenuMulti::cTaskConsMenuMulti(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

// DOL 0x80109e1c [audited]
cTaskConsMenuMulti::~cTaskConsMenuMulti()
{
}

// DOL 0x80109e74 [audited]: priority 0x80000008, hit pattern 0x1f, wait anim (reset), hidden, position = layer 0 of pattern 0x12
void cTaskConsMenuMulti::SetData()
{
	nDrawPriority(0x80000008u);
	m_nHitPat = 0x1f;
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(GetAnm(1), true);
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), 0u, 0x12);
}

// DOL 0x80109f84 [audited] (table 0x80376d80)
int cTaskConsMenuMulti::GetAnm(int n)
{
	static const int s_Anm[4] = {0x10d, 0x10e, 0x10f, 0x110};
	return s_Anm[n];
}

// DOL 0x80109fc0 [audited]
bool cTaskConsMenuMulti::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, m_nHitPat);
}

void cTaskConsMenuMulti::WaitExec() { pAnita(0u)->SetAnimNo(GetAnm(1), false); }    // DOL 0x8010a104 [audited]
bool cTaskConsMenuMulti::AppearExec() { return ButtonAppear(this, nStep()); }        // DOL 0x8010a168 [audited]
void cTaskConsMenuMulti::PointExec() { pAnita(0u)->SetAnimNo(GetAnm(2), false); }   // DOL 0x8010a264 [audited]
bool cTaskConsMenuMulti::DecideExec() { return ButtonDecide(this, nStep()); }        // DOL 0x8010a2c8 [audited]

// ---- cTaskConsMenuRule ------------------------------------------------------------------------------------------------------------------

// DOL 0x8010a37c [audited] (the base ctor only; RegistData follows through the vtable)
cTaskConsMenuRule::cTaskConsMenuRule(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

// DOL 0x8010a4a4 [audited]
cTaskConsMenuRule::~cTaskConsMenuRule()
{
}

// DOL 0x8010a3c0 [audited]: priority 0x80000004, hit pattern 0x18, wait anim (reset), hidden (no layer position)
void cTaskConsMenuRule::RegistData()
{
	nDrawPriority(0x80000004u);
	m_nHitPat = 0x18;
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(GetAnm(1), true);
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x8010a4fc [audited] (table 0x80376db8)
int cTaskConsMenuRule::GetAnm(int n)
{
	static const int s_Anm[4] = {0xf1, 0xf2, 0xf3, 0xf4};
	return s_Anm[n];
}

// DOL 0x8010a538 [audited]
bool cTaskConsMenuRule::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, m_nHitPat);
}

void cTaskConsMenuRule::WaitExec() { pAnita(0u)->SetAnimNo(GetAnm(1), false); }    // DOL 0x8010a67c [audited]
bool cTaskConsMenuRule::AppearExec() { return ButtonAppear(this, nStep()); }        // DOL 0x8010a6e0 [audited]
void cTaskConsMenuRule::PointExec() { pAnita(0u)->SetAnimNo(GetAnm(2), false); }   // DOL 0x8010a7dc [audited]
bool cTaskConsMenuRule::DecideExec() { return ButtonDecide(this, nStep()); }        // DOL 0x8010a840 [audited]

// ---- cTaskConsMenuStage -----------------------------------------------------------------------------------------------------------------

// DOL 0x80376df0 [audited] [closed][world][stage][appear, wait, point, decide, delete]; -1 = no anim (SetAnimNo refuses it)
const int cTaskConsMenuStage::s_AnmTbl[2][4][6][5] = {
	{
		{{0x49, 0x4a, 0x4b, 0x4c, 0x4d}, {0x51, 0x52, 0x53, 0x54, 0x55}, {0x59, 0x5a, 0x5b, 0x5c, 0x5d}, {0x61, 0x62, 0x63, 0x64, 0x65}, {0x69, 0x6a, 0x6b, 0x6c, 0x6d}, {0x6e, 0x6f, 0x70, 0x71, 0x72}},
		{{0x73, 0x74, 0x75, 0x76, 0x77}, {0x7b, 0x7c, 0x7d, 0x7e, 0x7f}, {0x83, 0x84, 0x85, 0x86, 0x87}, {0x8b, 0x8c, 0x8d, 0x8e, 0x8f}, {0x93, 0x94, 0x95, 0x96, 0x97}, {0x98, 0x99, 0x9a, 0x9b, 0x9c}},
		{{0x9d, 0x9e, 0x9f, 0xa0, 0xa1}, {0xa5, 0xa6, 0xa7, 0xa8, 0xa9}, {0xad, 0xae, 0xaf, 0xb0, 0xb1}, {0xb5, 0xb6, 0xb7, 0xb8, 0xb9}, {0xbd, 0xbe, 0xbf, 0xc0, 0xc1}, {0xc2, 0xc3, 0xc4, 0xc5, 0xc6}},
		{{0xc7, 0xc8, 0xc9, 0xca, 0xcb}, {0xcf, 0xd0, 0xd1, 0xd2, 0xd3}, {0xd7, 0xd8, 0xd9, 0xda, 0xdb}, {0xdf, 0xe0, 0xe1, 0xe2, 0xe3}, {0xe7, 0xe8, 0xe9, 0xea, 0xeb}, {0xec, 0xed, 0xee, 0xef, 0xf0}},
	},
	{
		{{-1, -1, -1, -1, -1}, {0x4e, 0x4f, 0x4f, -1, 0x50}, {0x56, 0x57, 0x57, -1, 0x58}, {0x5e, 0x5f, 0x5f, -1, 0x60}, {0x66, 0x67, 0x67, -1, 0x68}, {0x66, 0x67, 0x67, -1, 0x68}},
		{{-1, -1, -1, -1, -1}, {0x4e, 0x4f, 0x4f, -1, 0x50}, {0x56, 0x57, 0x57, -1, 0x58}, {0x5e, 0x5f, 0x5f, -1, 0x60}, {0x66, 0x67, 0x67, -1, 0x68}, {0x66, 0x67, 0x67, -1, 0x68}},
		{{-1, -1, -1, -1, -1}, {0x4e, 0x4f, 0x4f, -1, 0x50}, {0x56, 0x57, 0x57, -1, 0x58}, {0x5e, 0x5f, 0x5f, -1, 0x60}, {0x66, 0x67, 0x67, -1, 0x68}, {0x66, 0x67, 0x67, -1, 0x68}},
		{{-1, -1, -1, -1, -1}, {0x4e, 0x4f, 0x4f, -1, 0x50}, {0x56, 0x57, 0x57, -1, 0x58}, {0x5e, 0x5f, 0x5f, -1, 0x60}, {0x66, 0x67, 0x67, -1, 0x68}, {0x66, 0x67, 0x67, -1, 0x68}},
	},
};

// DOL 0x803771b0 [audited]: the layout pattern whose layer 0 places stage button n
const int cTaskConsMenuStage::s_LayPat[6] = {9, 10, 11, 12, 13, 14};

int cTaskConsMenuStage::Anm(int nCol) const
{
	return s_AnmTbl[m_bOpen ? 0 : 1][m_nWorldNo][m_nStageNo][nCol];
}

// DOL 0x8010a8f4 [audited]: priority 0x80000003, open / visible, wait anim (reset; open row), hidden, position = layer 0 of pattern s_LayPat[stage],
// hit pattern 0x14, then the open check of LoadStageData (inlined)
cTaskConsMenuStage::cTaskConsMenuStage(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nWorldNo, int nStageNo)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x80000003u);
	m_nWorldNo = nWorldNo;
	m_nStageNo = nStageNo;
	m_bOpen = true;
	m_bVisible = true;
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(s_AnmTbl[0][m_nWorldNo][m_nStageNo][1], true);
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), 0u, s_LayPat[m_nStageNo]);
	m_nHitPat = 0x14;   // DAT_80477b28
	LoadStageData(nWorldNo);
}

// DOL 0x8010aaac [audited]
cTaskConsMenuStage::~cTaskConsMenuStage()
{
}

// DOL 0x8010b050 [audited]: open = record IsOpen(stage + world * 6 + 1) (0x800c61f4); the boss stage (index 5) also hides while closed
void cTaskConsMenuStage::LoadStageData(int nWorldNo)
{
	m_nWorldNo = nWorldNo;
	const bool bOpen = cG02RallyXRecord::Get().IsOpen(m_nStageNo + nWorldNo * 6 + 1);
	if (m_nStageNo == 5)
	{
		m_bOpen = bOpen;
		m_bVisible = bOpen;
	}
	else
	{
		m_bOpen = bOpen;
	}
}

// DOL 0x8010ab04 [audited]: a closed stage never hits
bool cTaskConsMenuStage::IsDpdHit(float x, float y)
{
	if (!m_bOpen) return false;
	return DpdHitPat(x, y, m_nHitPat);
}

// DOL 0x8010ac5c [audited]: priority back to 0x80000003
void cTaskConsMenuStage::WaitExec()
{
	nDrawPriority(0x80000003u);
	pAnita(0u)->SetAnimNo(Anm(1), false);
}

// DOL 0x8010acf4 [audited]: the pointed button draws above the others (0x80000005)
void cTaskConsMenuStage::PointExec()
{
	nDrawPriority(0x80000005u);
	pAnita(0u)->SetAnimNo(Anm(2), false);
}

// DOL 0x8010ad8c [audited]: shown when visible, hidden otherwise
bool cTaskConsMenuStage::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(Anm(0), true);
		pAnita(0u)->SetState(1u, m_bVisible);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// DOL 0x8010ae90 [audited] (the priority is set every frame)
bool cTaskConsMenuStage::DecideExec()
{
	nDrawPriority(0x80000005u);
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(Anm(3), true);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// DOL 0x8010af70 [audited]
bool cTaskConsMenuStage::DeleteExec()
{
	nDrawPriority(0x80000003u);
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(Anm(4), true);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// ---- cTaskConsMenuWorld -----------------------------------------------------------------------------------------------------------------

namespace
{
const int s_WorldAnm[4][3] = {{0x24, 0x25, 0x26}, {0x27, 0x28, 0x29}, {0x2a, 0x2b, 0x2c}, {0x2d, 0x2e, 0x2f}};   // DOL 0x80377268 [audited] [world][appear, wait, delete]
}

// DOL 0x8010b480 [audited]: priority 0x80000006, world 0's wait anim (reset), position = layer 0 of pattern 0, hidden
cTaskConsMenuWorld::cTaskConsMenuWorld(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x80000006u);
	m_nWorldNo = 0;
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(s_WorldAnm[m_nWorldNo][1], true);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), 0u, 0);
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x8010b5a8 [audited]
cTaskConsMenuWorld::~cTaskConsMenuWorld()
{
}

// DOL 0x8010b600 [audited]
void cTaskConsMenuWorld::WaitExec()
{
	pAnita(0u)->SetAnimNo(s_WorldAnm[m_nWorldNo][1], false);
}

// DOL 0x8010b658 [audited] (no reset on the appear anim, as in the DOL)
bool cTaskConsMenuWorld::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(s_WorldAnm[m_nWorldNo][0], false);
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// DOL 0x8010b70c [audited]
bool cTaskConsMenuWorld::DeleteExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(s_WorldAnm[m_nWorldNo][2], false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// ---- the HOW TO PLAY pages --------------------------------------------------------------------------------------------------------------

namespace
{
using kingdom::game::menu::sTutorialData;
using kingdom::game::menu::sTutorialPage;

const char* const ANITA_TUTORIAL_01 = "ANITA_MENU_TUTORIAL_01";   // 0x7600f2f1
const char* const ANITA_TUTORIAL_02 = "ANITA_MENU_TUTORIAL_02";   // 0xef09a34b
const char* const TEXT_TUTORIAL = "MENU_TEXT_TUTORIAL";           // 0x9e486902
const char* const COMMON_TUTORIAL = "COMMON_ANITA_TUTORIAL";      // 0xbc299e43 (the ctor key)

// sTutorialPage tables (12 bytes each: anim no, text row, layout CRC)
sTutorialPage s_PageMotion[7] = {   // DOL 0x8043d430 [audited]
	{-1, 1, ANITA_TUTORIAL_01}, {0, 2, ANITA_TUTORIAL_01}, {1, 3, ANITA_TUTORIAL_01}, {2, 4, ANITA_TUTORIAL_01},
	{3, 5, ANITA_TUTORIAL_01}, {0, 6, ANITA_TUTORIAL_02}, {-1, 7, ANITA_TUTORIAL_01},
};
sTutorialPage s_PageCtl[5] = {      // DOL 0x8043d484 [audited]
	{-1, 9, ANITA_TUTORIAL_01}, {0, 10, ANITA_TUTORIAL_01}, {-1, 11, ANITA_TUTORIAL_01}, {0, 12, ANITA_TUTORIAL_02}, {-1, 13, ANITA_TUTORIAL_01},
};

sTutorialData* DataMotion()   // DOL 0x803771f0 [audited]
{
	static sTutorialData s{7u, TEXT_TUTORIAL, {&s_PageMotion[0], &s_PageMotion[1], &s_PageMotion[2], &s_PageMotion[3], &s_PageMotion[4],
	                                           &s_PageMotion[5], &s_PageMotion[6]}};
	return &s;
}
sTutorialData* DataCtl()      // DOL 0x803771fc [audited]
{
	static sTutorialData s{5u, TEXT_TUTORIAL, {&s_PageCtl[0], &s_PageCtl[1], &s_PageCtl[2], &s_PageCtl[3], &s_PageCtl[4]}};
	return &s;
}
}

// DOL 0x8010b0f8 [audited]: the base ctor variant FUN_80145a9c(this, cursor, COMMON_ANITA_TUTORIAL, 0x803771f0); natively SetCursor + RegistData
// (as the Galaga tutorial)
cTaskConsMenuMotionTutorial::cTaskConsMenuMotionTutorial(kingdom::cTaskCursor* pCursor)
{
	SetCursor(pCursor);
	RegistData(COMMON_TUTORIAL, DataMotion());
}

// DOL 0x8010b308 [audited]: only the inlined base member destructors
cTaskConsMenuMotionTutorial::~cTaskConsMenuMotionTutorial()
{
}

// DOL 0x8010b14c [audited]: FUN_80145a9c(this, cursor, COMMON_ANITA_TUTORIAL, 0x803771fc)
cTaskConsMenuCtlTutorial::cTaskConsMenuCtlTutorial(kingdom::cTaskCursor* pCursor)
{
	SetCursor(pCursor);
	RegistData(COMMON_TUTORIAL, DataCtl());
}

// DOL 0x8010b1a0 [audited]
cTaskConsMenuCtlTutorial::~cTaskConsMenuCtlTutorial()
{
}

}
