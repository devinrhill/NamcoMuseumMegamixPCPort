#include "kingdom/game/g02/cTaskConsMultiMenuConsoles.hpp"

#include <cstdio>

#include "KingdomGlobal.hpp"
#include "gfx/FontWriter.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/menu/sTutorialData.hpp"
#include "kingdom/game/menu/sTutorialPage.hpp"

// Rally-X Remix (g02) multiplayer menu consoles (job 200), ported 1:1 from main.dol (no C# exists for Rally-X). Conventions as in
// cTaskConsMenuConsoles.cpp: the DOL hides / shows an anita by clearing / setting bit 1 of its state word (SetState(1u, false / true)); "the
// animation ended" is state bit 0x10000 (IsState(65536u)); every SetAnimNo passes frame 0.0 (the 4th argument of the DOL is the reset flag).

namespace kingdom::game::g02
{

namespace
{
const char* const LAYOUT = "ANITA_MENU_SELECT_02";   // crc 0xb43de549 (g02menu.lzs)

void Regist(kingdom::cKingdomAnita* a, const char* name)
{
	a->RegistData(ResourceDatabase::GetPath(name));
}

// the two-digit number anims of the flag / time consoles: DOL 0x803775e8 / 0x80377ac8 [digit][appear, wait, delete]
int DigitAnm(int digit, int nCol)
{
	return 0x24 + digit * 3 + nCol;
}
}

// ---- cTaskConsMultiMenuArrow ------------------------------------------------------------------------------------------------------------

const int cTaskConsMultiMenuArrow::s_AnmTbl[6][4] = {{0x97, 0x98, 0x99, 0x9a}, {0x9b, 0x9c, 0x9d, 0x9e}, {0x85, 0x86, 0x87, 0x88},
                                                     {0x85, 0x86, 0x87, 0x88}, {0x89, 0x8a, 0x8b, 0x8c}, {0x89, 0x8a, 0x8b, 0x8c}};
const int cTaskConsMultiMenuArrow::s_HitPat[6] = {0x21, 0x22, 0x1d, 0x1d, 0x1d, 0x1d};
const int cTaskConsMultiMenuArrow::s_LayPat[6] = {0x11, 0x12, 8, 8, 9, 9};
const int cTaskConsMultiMenuArrow::s_LayLayer[6] = {0, 0, 1, 0, 1, 0};

// DOL 0x8010b7b4: priority 0x80000005, wait anim (reset), hidden, position = layer s_LayLayer of pattern s_LayPat, hit pattern s_HitPat
cTaskConsMultiMenuArrow::cTaskConsMultiMenuArrow(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nIndex, bool bActive)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x80000005u);
	m_nIndex = nIndex;
	m_bActive = bActive;
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(s_AnmTbl[m_nIndex][1], true);
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), (uint)s_LayLayer[m_nIndex], s_LayPat[m_nIndex]);
	m_nHitPat = s_HitPat[m_nIndex];
}

// DOL 0x8010b914
cTaskConsMultiMenuArrow::~cTaskConsMultiMenuArrow()
{
}

// DOL 0x8010b96c: an inactive arrow never hits
bool cTaskConsMultiMenuArrow::IsDpdHit(float x, float y)
{
	if (!m_bActive) return false;
	return DpdHitPat(x, y, m_nHitPat);
}

void cTaskConsMultiMenuArrow::WaitExec() { pAnita(0u)->SetAnimNo(s_AnmTbl[m_nIndex][1], false); }    // DOL 0x8010bac4
void cTaskConsMultiMenuArrow::PointExec() { pAnita(0u)->SetAnimNo(s_AnmTbl[m_nIndex][2], false); }   // DOL 0x8010bb1c

// DOL 0x8010bb74: shown only when active
bool cTaskConsMultiMenuArrow::AppearExec()
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

// DOL 0x8010bc34
bool cTaskConsMultiMenuArrow::DecideExec()
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

// ---- cTaskConsMultiMenuBack -------------------------------------------------------------------------------------------------------------

cTaskConsMultiMenuBack::cTaskConsMultiMenuBack(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBack(pConsMan)
{
}

cTaskConsMultiMenuBack::~cTaskConsMultiMenuBack()
{
}

// DOL 0x8010bd78: priority 0x8000000b, hit pattern 0x24, wait anim (reset), hidden, position = layer 0 of pattern 2
void cTaskConsMultiMenuBack::Init()
{
	nDrawPriority(0x8000000bu);
	m_nHitPat = 0x24;
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(GetAnm(1), true);
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), 0u, 2);
}

// DOL 0x8010be88 (table 0x80377430)
int cTaskConsMultiMenuBack::GetAnm(int n)
{
	static const int s_Anm[4] = {0x9f, 0xa0, 0xa1, 0xa2};
	return s_Anm[n];
}

// ---- cTaskConsMultiMenuBg ---------------------------------------------------------------------------------------------------------------

cTaskConsMultiMenuBg::cTaskConsMultiMenuBg(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nStageKey)
	: cTaskConsMenuBg(pConsMan, nStageKey)
{
}

cTaskConsMultiMenuBg::~cTaskConsMultiMenuBg()
{
}

// DOL 0x8010bf68: priority 0x10001, ANITA_MENU_BG, the key's anim (reset); the same as cTaskConsMenuBg::SetData (0x801072e8)
void cTaskConsMultiMenuBg::Init()
{
	SetData();
}

// ---- cTaskConsMultiMenuBoard ------------------------------------------------------------------------------------------------------------

// DOL 0x8010c034: priority 0x10003, anim 0x92 (no reset), hidden
cTaskConsMultiMenuBoard::cTaskConsMultiMenuBoard(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x10003u);
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(0x92, false);
	pAnita(0u)->SetState(1u, false);
}

cTaskConsMultiMenuBoard::~cTaskConsMultiMenuBoard()
{
}

// DOL 0x8010c16c: the board folds out (0x91), then stays (0x92)
bool cTaskConsMultiMenuBoard::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(0x91, false);
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		pAnita(0u)->SetAnimNo(0x92, false);
		return true;
	}
	return false;
}

// ---- cTaskConsMultiMenuController -------------------------------------------------------------------------------------------------------

const int cTaskConsMultiMenuController::s_HitPat[4] = {0x16, 0x17, 0x18, 0x19};
const int cTaskConsMultiMenuController::s_Anm[2][4][4] = {
	{{0x62, 0x63, 0x64, 0x65}, {0x6c, 0x6d, 0x6e, 0x6f}, {0x76, 0x77, 0x78, 0x79}, {0x80, 0x81, 0x82, 0x83}},   // type 1 (remote only)
	{{0x5d, 0x5e, 0x5f, 0x60}, {0x67, 0x68, 0x69, 0x6a}, {0x71, 0x72, 0x73, 0x74}, {0x7b, 0x7c, 0x7d, 0x7e}},   // type 2 (Nunchuk)
};

// DOL 0x8010c234
cTaskConsMultiMenuController::cTaskConsMultiMenuController(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nType, int nPlayer)
	: cTaskConsMenuController(pConsMan, nType)
{
	m_nPlayer = nPlayer;
}

cTaskConsMultiMenuController::~cTaskConsMultiMenuController()
{
}

// DOL 0x8010c2e0: priority 0x80000003, hit pattern of the player, wait anim (reset), hidden
void cTaskConsMultiMenuController::Init()
{
	nDrawPriority(0x80000003u);
	m_nHitPat = s_HitPat[m_nPlayer];
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(GetAnm(1), true);
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x8010c3f4
int cTaskConsMultiMenuController::GetAnm(int n)
{
	return s_Anm[m_nType - 1][m_nPlayer][n];
}

// ---- cTaskConsMultiMenuCurtain ----------------------------------------------------------------------------------------------------------

cTaskConsMultiMenuCurtain::cTaskConsMultiMenuCurtain(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nStageKey)
	: cTaskConsMenuCurtain(pConsMan, nStageKey)
{
}

cTaskConsMultiMenuCurtain::~cTaskConsMultiMenuCurtain()
{
}

// DOL 0x8010c4f8: priority 0x10002, wait anim (reset), hidden
void cTaskConsMultiMenuCurtain::Init()
{
	nDrawPriority(0x10002u);
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(GetAnm(1), true);
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x8010c5d4 (table 0x80377598 [key][appear, wait])
int cTaskConsMultiMenuCurtain::GetAnm(int n)
{
	static const int s_Anm[4][2] = {{0xb3, 0xb4}, {0xb5, 0xb6}, {0xb7, 0xb8}, {0xb9, 0xba}};
	return s_Anm[m_nWorldNo][n];
}

// ---- cTaskConsMultiMenuFlag -------------------------------------------------------------------------------------------------------------

// DOL 0x8010c644: priority 0x80000007; anitas 0 (tens) and 1 (ones): digit 0 anim (no reset), hidden, at layer {1, 0} of pattern 0xb
cTaskConsMultiMenuFlag::cTaskConsMultiMenuFlag(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x80000007u);
	static const int s_Layer[2] = {1, 0};   // DOL 0x8047ba40
	for (uint i = 0; i < 2; i++)
	{
		kingdom::cKingdomAnita* a = pAnita(i);
		Regist(a, LAYOUT);
		a->SetAnimNo(0x24, false);
		a->SetState(1u, false);
		a->GetLayerPos(a->vTrans(), (uint)s_Layer[i], 0xb);   // DAT_80477b84
	}
}

cTaskConsMultiMenuFlag::~cTaskConsMultiMenuFlag()
{
}

// DOL 0x8010c7b8: the tens digit shows only from 10 up (a leading zero hides); the ones digit moves to the centre (pattern 0xc) below 10
void cTaskConsMultiMenuFlag::SetNumber(int nCol)
{
	const u32 v = (u32)m_nValue;
	for (uint i = 0; i < 2; i++)
	{
		const u32 pow = i == 0 ? 10u : 1u;
		const u32 mod = pow * 10u;
		kingdom::cKingdomAnita* a = pAnita(i);
		if (i == 1 || pow <= v)
		{
			a->SetAnimNo(DigitAnm((int)((v % mod) / pow), nCol), true);
			a->SetState(1u);
		}
		else
		{
			a->SetAnimNo(DigitAnm(0, nCol), true);
			a->SetState(1u, false);
		}
	}
	pAnita(1u)->GetLayerPos(pAnita(1u)->vTrans(), 0u, m_nValue < 10 ? 0xc : 0xb);   // DAT_80477b80 / DAT_80477b84
}

void cTaskConsMultiMenuFlag::WaitExec() { SetNumber(1); }   // DOL 0x8010ca38

// DOL 0x8010c978: the last digit decides the end
bool cTaskConsMultiMenuFlag::AppearExec()
{
	if (nStep() == 0)
	{
		for (uint i = 0; i < 2; i++) pAnita(i)->SetState(1u);
		SetNumber(0);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(1u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// DOL 0x8010ca40
bool cTaskConsMultiMenuFlag::DeleteExec()
{
	if (nStep() == 0)
	{
		SetNumber(2);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(1u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// ---- cTaskConsMultiMenuPlayer::cTaskPlayerNumber ----------------------------------------------------------------------------------------

namespace
{
const int s_NumAnmA[4][2] = {{0x51, 0x52}, {0x54, 0x55}, {0x57, 0x58}, {0x5a, 0x5b}};   // DOL 0x80377818 [player][appear, wait]
const int s_NumAnmB[4][2] = {{0xbf, 0xc0}, {0xc2, 0xc3}, {0xc5, 0xc6}, {0xc8, 0xc9}};   // DOL 0x80377838
const int s_NumLayerA[4] = {3, 2, 1, 0};                                                  // DOL 0x80377858
const int s_NumLayerC[4] = {0, 1, 2, 3};                                                  // DOL 0x80377878
}

// DOL 0x8010e7e8: priority 0x80000001; anita 0 / 1 = the plate (wait anims, reset), anita 2 = the mark (0xcc); all hidden. The mark's flag is
// the sys' player mask bit (0x800c58b0)
cTaskConsMultiMenuPlayer::cTaskPlayerNumber::cTaskPlayerNumber(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nPlayer)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x80000001u);
	m_nPlayer = nPlayer;
	m_bActive = false;
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(s_NumAnmA[m_nPlayer][1], true);
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), (uint)s_NumLayerA[m_nPlayer], 4);
	Regist(pAnita(1u), LAYOUT);
	pAnita(1u)->SetAnimNo(s_NumAnmB[m_nPlayer][1], true);
	pAnita(1u)->SetState(1u, false);
	Regist(pAnita(2u), LAYOUT);
	pAnita(2u)->SetAnimNo(0xcc, true);
	pAnita(2u)->SetState(1u, false);
	pAnita(2u)->GetLayerPos(pAnita(2u)->vTrans(), (uint)s_NumLayerC[m_nPlayer], 0x28);
	m_bActive = KingdomGlobal::RallyXSys->IsPlayerActive(nPlayer);
}

cTaskConsMultiMenuPlayer::cTaskPlayerNumber::~cTaskPlayerNumber()
{
}

// DOL 0x8010eaa8
bool cTaskConsMultiMenuPlayer::cTaskPlayerNumber::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(s_NumAnmA[m_nPlayer][0], true);
		pAnita(1u)->SetAnimNo(s_NumAnmB[m_nPlayer][0], true);
		pAnita(2u)->SetAnimNo(0xcb, true);
		pAnita(0u)->SetState(1u);
		pAnita(1u)->SetState(1u);
		pAnita(2u)->SetState(1u, m_bActive);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u) && pAnita(1u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// DOL 0x8010ec08 (the mark's reset flag is the player mask bit, as in the DOL)
void cTaskConsMultiMenuPlayer::cTaskPlayerNumber::WaitExec()
{
	pAnita(0u)->SetAnimNo(s_NumAnmA[m_nPlayer][1], false);
	pAnita(1u)->SetAnimNo(s_NumAnmB[m_nPlayer][1], false);
	pAnita(2u)->SetAnimNo(0xcc, m_bActive);
}

// ---- cTaskConsMultiMenuPlayer -----------------------------------------------------------------------------------------------------------

const int cTaskConsMultiMenuPlayer::s_AnmTbl[4][2] = {{0xa7, 0xa8}, {0xaa, 0xab}, {0xad, 0xae}, {0xb0, 0xb1}};
const int cTaskConsMultiMenuPlayer::s_LayLayer[4] = {3, 2, 1, 0};

// DOL 0x8010ecac: priority 0x10005, wait anim (reset), hidden, the name's position = layer s_LayLayer of pattern 5 (+ half the 640 x 480 screen,
// y flipped), the number plate (+0x1e0), the control button (+0x1e4; the sys' type of the player), the car (+0x1e8) and the font writer, all
// connected to the main app. The name is drawn in Draw (the DOL's cTaskFontWriter at priority 0x80000002).
cTaskConsMultiMenuPlayer::cTaskConsMultiMenuPlayer(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nPlayer, const string& name)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x10005u);
	m_nPlayer = nPlayer;
	m_Name = name;
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(s_AnmTbl[m_nPlayer][1], true);
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(m_vPos, (uint)s_LayLayer[m_nPlayer], 5);
	m_vPos.x = 0.5f * 640.0f + m_vPos.x;
	m_vPos.y = -(0.5f * 480.0f - m_vPos.y);
	kingdom::cTaskKingdom* app = KingdomGlobal::KingSys->pAppMain();
	m_pNumber = new cTaskPlayerNumber(pConsMan, nPlayer);
	app->ConnectChild(m_pNumber);
	m_pController = new cTaskConsMultiMenuController(pConsMan, cG02RallyXSysExt::nCtrlType[nPlayer], nPlayer);   // sys 0x800c5b88
	m_pController->Init();
	app->ConnectChild(m_pController);
	m_pCar = new cTaskMultiMenuCar(nPlayer);
	m_pCar->Init();   // vtable +0x44
	app->ConnectChild(m_pCar);
	// the font writer (0x26c bytes, priority 0x80000002) draws above the car: a task of its own
	class cTaskName : public cJetTask
	{
	public:
		explicit cTaskName(const cTaskConsMultiMenuPlayer* p) : m_p(p) { nDrawPriority(0x80000002u); SetName("g02::cTaskFontWriter"); }
		void Draw(int, cJetTask*) override { m_p->DrawName(); }

	private:
		const cTaskConsMultiMenuPlayer* m_p;
	};
	app->ConnectChild(new cTaskName(this));
	m_bShowName = false;
}

cTaskConsMultiMenuPlayer::~cTaskConsMultiMenuPlayer()
{
}

// DOL 0x8010f148 (vtable +0x64): the state wait, then the children wait (the car resets)
void cTaskConsMultiMenuPlayer::WaitState()
{
	cTaskConsMenuBase::WaitState();
	m_pNumber->WaitState();
	m_pController->WaitState();
	m_pCar->Reset();
}

void cTaskConsMultiMenuPlayer::WaitExec()   // DOL 0x8010f30c
{
	pAnita(0u)->SetAnimNo(s_AnmTbl[m_nPlayer][1], false);
}

// DOL 0x8010f1bc. Step 1 tests anita 0's SHOWN bit (state & 1), not the end bit, as the DOL does.
bool cTaskConsMultiMenuPlayer::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(s_AnmTbl[m_nPlayer][0], true);
		pAnita(0u)->SetState(1u);
		m_pNumber->AppearState();
		m_pController->AppearState();
		m_pCar->AppearState();
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool b = pAnita(0u)->IsState(1u);
		if (b) b = m_pNumber->IsAppearEndState();
		if (b) b = m_pController->IsAppearEndState();
		if (b) b = m_pCar->IsAppearEnd();
		if (b)
		{
			m_bShowName = true;
			return true;
		}
	}
	return false;
}

// DOL 0x8010f440: the name's position is set every frame (Draw), then the state machine unless the main app is paused
bool cTaskConsMultiMenuPlayer::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::KingSys->pAppMain()->IsPause()) return true;
	return cTaskConsMenuBase::Execute(pParam);
}

// DOL 0x8010f364 (an orthographic setup around the anita draw: the same projection as the default 2D pass) + the font writer's text.
// UNVERIFIED: the name is drawn centred on the pane position, RESIDENT_FONT_MII, size 28 (the writer's settings are not traced).
void cTaskConsMultiMenuPlayer::Draw(int nStatus, cJetTask* pParam)
{
	cTaskAnita::Draw(nStatus, pParam);
}

void cTaskConsMultiMenuPlayer::DrawName() const
{
	if (!m_bShowName || m_Name.empty()) return;
	gfx::TextStyle st;
	st.font = "RESIDENT_FONT_MII";
	const int height = gfx::FontHeight(st.font);
	st.scaleX = st.scaleY = 28.0f / (float)(height > 0 ? height : 32);
	st.vorigin = gfx::TextStyle::VOrigin::Middle;
	gfx::DrawText((float)(int)m_vPos.x, (float)(int)-m_vPos.y, m_Name, st, gfx::TextAlign::Center);
}

// ---- cTaskConsMultiMenuRule -------------------------------------------------------------------------------------------------------------

cTaskConsMultiMenuRule::cTaskConsMultiMenuRule(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuRule(pConsMan)
{
}

cTaskConsMultiMenuRule::~cTaskConsMultiMenuRule()
{
}

// DOL 0x8010f528: priority 0x80000009, hit pattern 0x1b, wait anim (reset), hidden (no layer position)
void cTaskConsMultiMenuRule::RegistData()
{
	nDrawPriority(0x80000009u);
	m_nHitPat = 0x1b;
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(GetAnm(1), true);
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x8010f60c (table 0x803779c8)
int cTaskConsMultiMenuRule::GetAnm(int n)
{
	static const int s_Anm[4] = {0x93, 0x94, 0x95, 0x96};
	return s_Anm[n];
}

// ---- cTaskConsMultiMenuSingle -----------------------------------------------------------------------------------------------------------

// DOL 0x8010f6a8: priority 0x8000000a, anim 0xa4 (reset), hidden, position = layer 0 of pattern 0x14 (DAT_80479080 = 0, DAT_80477bb4)
cTaskConsMultiMenuSingle::cTaskConsMultiMenuSingle(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x8000000au);
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(0xa4, true);
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), 0u, 0x14);
}

cTaskConsMultiMenuSingle::~cTaskConsMultiMenuSingle()
{
}

// DOL 0x8010f80c (hit pattern DAT_80477bb0 = 0x26)
bool cTaskConsMultiMenuSingle::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, 0x26);
}

void cTaskConsMultiMenuSingle::WaitExec() { pAnita(0u)->SetAnimNo(0xa4, false); }    // DOL 0x8010f950
void cTaskConsMultiMenuSingle::PointExec() { pAnita(0u)->SetAnimNo(0xa5, false); }   // DOL 0x8010fa48

// DOL 0x8010f988
bool cTaskConsMultiMenuSingle::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(0xa3, true);
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		pAnita(0u)->SetAnimNo(0xa4, true);
		return true;
	}
	return false;
}

// DOL 0x8010fa80
bool cTaskConsMultiMenuSingle::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(0xa6, true);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// ---- cTaskConsMultiMenuStage ------------------------------------------------------------------------------------------------------------

// DOL 0x8010fb14: priority 0x80000004, key 0's wait anim (reset), hidden, position = layer 0 of pattern 0 (DAT_8047908c / DAT_80479088, never written)
cTaskConsMultiMenuStage::cTaskConsMultiMenuStage(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x80000004u);
	m_nStageKey = 0;
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(0x45 + m_nStageKey * 3 + 1, true);
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), 0u, 0);
}

cTaskConsMultiMenuStage::~cTaskConsMultiMenuStage()
{
}

void cTaskConsMultiMenuStage::WaitExec() { pAnita(0u)->SetAnimNo(0x45 + m_nStageKey * 3 + 1, false); }   // DOL 0x8010fc94

// DOL 0x8010fcec (table 0x80377a38 [key][appear, wait, delete])
bool cTaskConsMultiMenuStage::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(0x45 + m_nStageKey * 3, false);
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// DOL 0x8010fda0
bool cTaskConsMultiMenuStage::DeleteExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(0x45 + m_nStageKey * 3 + 2, false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// ---- cTaskConsMultiMenuStart ------------------------------------------------------------------------------------------------------------

// DOL 0x8010fe48: priority 0x80000008, anim 0x8e (reset), hidden, position = layer 0 of pattern 0xf (DAT_80479090 = 0, DAT_80477bcc)
cTaskConsMultiMenuStart::cTaskConsMultiMenuStart(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x80000008u);
	Regist(pAnita(0u), LAYOUT);
	pAnita(0u)->SetAnimNo(0x8e, true);
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), 0u, 0xf);
}

cTaskConsMultiMenuStart::~cTaskConsMultiMenuStart()
{
}

// DOL 0x8010ffac (hit pattern DAT_80477bc8 = 0x1f)
bool cTaskConsMultiMenuStart::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, 0x1f);
}

void cTaskConsMultiMenuStart::WaitExec() { pAnita(0u)->SetAnimNo(0x8e, false); }    // DOL 0x801100f0
void cTaskConsMultiMenuStart::PointExec() { pAnita(0u)->SetAnimNo(0x8f, false); }   // DOL 0x801101e8

// DOL 0x80110128
bool cTaskConsMultiMenuStart::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(0x8d, true);
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		pAnita(0u)->SetAnimNo(0x8e, true);
		return true;
	}
	return false;
}

// DOL 0x80110220
bool cTaskConsMultiMenuStart::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(0x90, true);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// ---- cTaskConsMultiMenuTime -------------------------------------------------------------------------------------------------------------

// DOL 0x801102b4: priority 0x80000006; anitas 0..2 (minute, two second digits): digit 0 anim (reset), hidden, at layer {3, 1, 0} of pattern 0xd;
// anita 3 = the colon (0x42), at layer 2 of pattern 0xd
cTaskConsMultiMenuTime::cTaskConsMultiMenuTime(kingdom::game::cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(0x80000006u);
	static const int s_Layer[3] = {3, 1, 0};   // DOL 0x80377b4c
	for (uint i = 0; i < 3; i++)
	{
		kingdom::cKingdomAnita* a = pAnita(i);
		Regist(a, LAYOUT);
		a->SetAnimNo(0x24, true);
		a->SetState(1u, false);
		a->GetLayerPos(a->vTrans(), (uint)s_Layer[i], 0xd);   // DAT_80477bd8
	}
	kingdom::cKingdomAnita* c = pAnita(3u);
	Regist(c, LAYOUT);
	c->SetAnimNo(0x42, true);
	c->SetState(1u, false);
	c->GetLayerPos(c->vTrans(), 2u, 0xd);   // DAT_80477be0 / DAT_80477bdc
}

cTaskConsMultiMenuTime::~cTaskConsMultiMenuTime()
{
}

// DOL 0x801104c8: minutes (anita 0), the two seconds digits (1, 2), the colon (3)
void cTaskConsMultiMenuTime::SetNumber(int nCol)
{
	const u32 v = (u32)m_nValue;
	const u32 min = v / 60000u;
	const u32 sec = (v % 60000u) / 1000u;
	pAnita(0u)->SetAnimNo(DigitAnm((int)min, nCol), true);
	for (uint i = 1; i < 3; i++)
	{
		const u32 pow = i == 1 ? 10u : 1u;
		pAnita(i)->SetAnimNo(DigitAnm((int)((sec % (pow * 10u)) / pow), nCol), true);
	}
	pAnita(3u)->SetAnimNo(0x42 + nCol, true);   // table 0x80377b40
	pAnita(3u)->SetState(1u);
}

void cTaskConsMultiMenuTime::WaitExec() { SetNumber(1); }   // DOL 0x80110730

// DOL 0x80110658
bool cTaskConsMultiMenuTime::AppearExec()
{
	if (nStep() == 0)
	{
		for (uint i = 0; i < 4; i++) pAnita(i)->SetState(1u);
		SetNumber(0);
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool b = pAnita(3u)->IsState(65536u);
		if (b) b = pAnita(2u)->IsState(65536u);   // the DOL's loop leaves the last of anitas 0..2
		if (b) return true;
	}
	return false;
}

// DOL 0x80110738
bool cTaskConsMultiMenuTime::DeleteExec()
{
	if (nStep() == 0)
	{
		SetNumber(2);
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool b = pAnita(3u)->IsState(65536u);
		if (b) b = pAnita(2u)->IsState(65536u);
		if (b) return true;
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
const char* const COMMON_TUTORIAL = "COMMON_ANITA_TUTORIAL";      // 0xbc299e43

sTutorialPage s_PageMotion[6] = {   // DOL 0x8043e360
	{-1, 15, ANITA_TUTORIAL_01}, {0, 16, ANITA_TUTORIAL_01}, {4, 17, ANITA_TUTORIAL_01},
	{1, 18, ANITA_TUTORIAL_02}, {-1, 19, ANITA_TUTORIAL_02}, {2, 20, ANITA_TUTORIAL_02},
};
sTutorialPage s_PageCtl[6] = {      // DOL 0x8043e3a8
	{-1, 22, ANITA_TUTORIAL_01}, {0, 23, ANITA_TUTORIAL_01}, {-1, 24, ANITA_TUTORIAL_01},
	{1, 25, ANITA_TUTORIAL_02}, {-1, 26, ANITA_TUTORIAL_02}, {2, 27, ANITA_TUTORIAL_02},
};

sTutorialData* DataMotion()   // DOL 0x80377b88
{
	static sTutorialData s{6u, TEXT_TUTORIAL, {&s_PageMotion[0], &s_PageMotion[1], &s_PageMotion[2], &s_PageMotion[3], &s_PageMotion[4], &s_PageMotion[5]}};
	return &s;
}
sTutorialData* DataCtl()      // DOL 0x80377b94
{
	static sTutorialData s{6u, TEXT_TUTORIAL, {&s_PageCtl[0], &s_PageCtl[1], &s_PageCtl[2], &s_PageCtl[3], &s_PageCtl[4], &s_PageCtl[5]}};
	return &s;
}
}

// DOL 0x801107ec (FUN_80145a9c(this, cursor, COMMON_ANITA_TUTORIAL, 0x80377b88); natively SetCursor + RegistData as the single-player pages)
cTaskConsMultiMenuMotionTutorial::cTaskConsMultiMenuMotionTutorial(kingdom::cTaskCursor* pCursor)
{
	SetCursor(pCursor);
	RegistData(COMMON_TUTORIAL, DataMotion());
}

cTaskConsMultiMenuMotionTutorial::~cTaskConsMultiMenuMotionTutorial()
{
}

// DOL 0x80110840
cTaskConsMultiMenuCtlTutorial::cTaskConsMultiMenuCtlTutorial(kingdom::cTaskCursor* pCursor)
{
	SetCursor(pCursor);
	RegistData(COMMON_TUTORIAL, DataCtl());
}

cTaskConsMultiMenuCtlTutorial::~cTaskConsMultiMenuCtlTutorial()
{
}

// ---- cTaskMultiMenuCar ------------------------------------------------------------------------------------------------------------------

namespace
{
constexpr float DEG2RAD = 0.01745329238474369f;
}

// DOL 0x80111050: cTaskMenuPacman's ctor, then priority 0x10006 + player
cTaskMultiMenuCar::cTaskMultiMenuCar(int nPlayer)
{
	SetName("g02::cTaskMultiMenuCar");
	nDrawPriority(0x10006u + (u32)nPlayer);
	m_nPlayer = nPlayer;
}

// DOL 0x801111a4 (the Mii head object and the CSV_CAR parameters of the DOL are not ported: Mii, out of scope)
cTaskMultiMenuCar::~cTaskMultiMenuCar()
{
}

// DOL 0x80111274: model 1 = MENU_NULL_MODEL (the placement), model 0 = the player's coloured car (blue, green, yellow, white) + CAR_ANIM_NORMAL,
// light set 1, shown, placed at node "set_<p+1>p" of the null model (rotation degrees -> radians), scale 0 until it appears; its node "man_root"
// carries the coloured body + BODY_ANIM_NORMAL. NOT PORTED (Mii, out of scope): the player's Mii head on the body's node "face" and the
// CSV_CAR (cParamCar) values the DOL loads only for its scale.
void cTaskMultiMenuCar::Init()
{
	static const char* const CAR[4] = {"CAR_BLUE", "CAR_GREEN", "CAR_YELLOW", "CAR_WHITE"};
	static const char* const BODY[4] = {"BODY_BLUE", "BODY_GREEN", "BODY_YELLOW", "BODY_WHITE"};
	cKingdomModel* null = CreateModel(1u);
	null->RegistModel("MENU_NULL_MODEL", 0x36fu);
	char node[0x80];
	std::snprintf(node, sizeof(node), "set_%dp", m_nPlayer + 1);   // s_set__dp_80377caa
	cKingdomModel* car = CreateModel(0u);
	car->RegistModel(CAR[m_nPlayer], 0x36fu);
	car->RegistAnm(0u, "CAR_ANIM_NORMAL");
	car->SetLightSetIdx(1);
	car->Visible();
	m_vTrans = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	m_vRotate = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	m_vScale = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	MenuSetNode("MENU_NULL_MODEL", node, m_vTrans, m_vRotate, m_vScale);
	m_vRotate = Vector4(DEG2RAD * m_vRotate.x, DEG2RAD * m_vRotate.y, DEG2RAD * m_vRotate.z, m_vRotate.w);
	car->vTrans() = m_vTrans;
	car->vRotate() = m_vRotate;
	car->vScale() = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	car->RegistExpand(0u, "man_root", BODY[m_nPlayer], 0x36fu);
	if (cKingdomModel* body = car->pExpandMdl(0u)) body->RegistAnm(0u, "BODY_ANIM_NORMAL");
	car->SetLightSetIdx(1);
}

// ---- cTaskMultiMenuMap ------------------------------------------------------------------------------------------------------------------

// DOL 0x80112830: cTaskMenuMap(man, 0, key), priority 0x10004
cTaskMultiMenuMap::cTaskMultiMenuMap(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nStageKey)
	: cTaskMenuMap(pConsMan, 0, nStageKey)
{
	SetName("g02::cTaskMultiMenuMap");
	nDrawPriority(0x10004u);
}

cTaskMultiMenuMap::~cTaskMultiMenuMap()
{
}

// DOL 0x80112898: model 4 = MENU_NULL_MODEL; models 0..3 = MENU_WORLD_1A / 2A / 3B / 4C (table 0x80377db8) + MENU_WORLD_ANIM, light set 0, hidden,
// placed at node "set_bg"; then the hit layout on anita 0
void cTaskMultiMenuMap::RegistData()
{
	static const char* const MODEL[4] = {"MENU_WORLD_1A", "MENU_WORLD_2A", "MENU_WORLD_3B", "MENU_WORLD_4C"};
	cKingdomModel* null = m_pModel->CreateModel(4u);
	null->RegistModel("MENU_NULL_MODEL", 0x36fu);
	for (uint n = 0; n < 4u; n++)
	{
		cKingdomModel* m = m_pModel->CreateModel(n);
		m->RegistModel(MODEL[n], 0x36fu);
		m->RegistAnm(0u, "MENU_WORLD_ANIM");
		m->SetLightSetIdx(0);
		m->Invisible();
		m_vTrans = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		m_vRotate = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		m_vScale = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		MenuSetNode("MENU_NULL_MODEL", "set_bg", m_vTrans, m_vRotate, m_vScale);
		m_vRotate = Vector4(DEG2RAD * m_vRotate.x, DEG2RAD * m_vRotate.y, DEG2RAD * m_vRotate.z, m_vRotate.w);
		m->vTrans() = m_vTrans;
		m->vRotate() = m_vRotate;
		m->vScale() = m_vScale;
	}
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANITA_MENU_SELECT_01"));
}

}
