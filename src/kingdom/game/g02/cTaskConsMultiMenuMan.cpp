#include "kingdom/game/g02/cTaskConsMultiMenuMan.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cTaskConsMenuMan.hpp"
#include "kingdom/game/g02/cTaskConsMultiMenuConsoles.hpp"

// Rally-X Remix (g02) multiplayer menu console manager, ported 1:1 from main.dol (no C# exists for Rally-X).

namespace kingdom::game::g02
{

namespace
{
using GamePad::PAD_UP, GamePad::PAD_DOWN, GamePad::PAD_LEFT, GamePad::PAD_RIGHT, GamePad::PAD_2;

// DOL 0x803776d0 (copied to the stack by KeySelect 0x8010d260): {focus, UP target, DOWN target}, terminated by focus -1
const int s_KeyTbl[10][3] = {{0, -1, -1}, {1, 9, 0xc}, {2, 4, 0x11}, {3, 0xc, 4}, {8, 0x12, 9},
                             {9, 8, 1},   {4, 0xc, 2}, {0xc, 1, 4},  {0x11, 2, 0x12}, {0x12, 0x11, 8}};
// DOL 0x80377758 (copied by DecideCheck 0x8010d748): {focus, state}, terminated by focus -1
const int s_DecideTbl[13][2] = {{0, -1}, {1, 2}, {2, 5}, {3, 6}, {8, 4}, {9, 3}, {4, 0xe}, {0xa, 0xc}, {0xb, 0xd}, {0xd, 7}, {0xf, 8}, {0xe, 9}, {0x10, 0xa}};
}

// DOL 0x8010cadc. Coming back from the game (0x0c300000) the settings of the last round are kept, otherwise stage 0, 3:00 (time 1), 10 flags (flag 2).
cTaskConsMultiMenuMan::cTaskConsMultiMenuMan(kingdom::game::cAppGameBaseMenu* pApp)
	: cTaskConsMenuManBase(pApp)
{
	SetName("g02::cTaskConsMultiMenuMan");
	cTaskConsMenuManBase* pConsMan = this;
	kingdom::cTaskKingdom* app = KingdomGlobal::KingSys->pAppMain();   // KingSys +0xa40: every console is its child
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	if (KingdomGlobal::SysCore->GetLastJobNo() == job::RALLYX_GAME)
	{
		m_nStageKey = sys->nStageIdx() - 0x19;
		m_nTimeIdx = sys->nTimeIdx();
		m_nFlagIdx = sys->nFlagIdx();
	}
	else
	{
		m_nStageKey = 0;
		m_nTimeIdx = 1;
		sys->nTimeIdx(m_nTimeIdx);
		m_nFlagIdx = 2;
		sys->nFlagIdx(m_nFlagIdx);
	}
	InitStageNo();
	m_nPlayer = cG02RallyXSysExt::nPlayer;
	for (int i = 0; i < m_nPlayer; i++)
	{
		m_nState[i] = 0;
		m_nStep[i] = 0;
		m_nSelectOld[i] = i + 4;
	}
	m_nSelectNow[0] = 8;
	m_nSelectOld[0] = 8;

	m_pBg = new cTaskConsMultiMenuBg(pConsMan, m_nStageKey);
	m_pBg->Init();
	app->ConnectChild(m_pBg);
	m_pBack = new cTaskConsMultiMenuBack(pConsMan);
	m_pBack->Init();
	app->ConnectChild(m_pBack);
	m_pStart = new cTaskConsMultiMenuStart(pConsMan);
	app->ConnectChild(m_pStart);
	m_pRule = new cTaskConsMultiMenuRule(pConsMan);
	m_pRule->RegistData();
	app->ConnectChild(m_pRule);
	for (int i = 0; i < 6; i++)
	{
		m_pArrow[i] = new cTaskConsMultiMenuArrow(pConsMan, i, m_nMaxKey != 0);
		app->ConnectChild(m_pArrow[i]);
	}
	m_pBoard = new cTaskConsMultiMenuBoard(pConsMan);
	app->ConnectChild(m_pBoard);
	m_pTime = new cTaskConsMultiMenuTime(pConsMan);
	m_pTime->nValue() = sys->nTimeMs();   // sys 0x800c5978
	app->ConnectChild(m_pTime);
	m_pFlag = new cTaskConsMultiMenuFlag(pConsMan);
	m_pFlag->nValue() = sys->nFlagCount();   // sys 0x800c5950
	app->ConnectChild(m_pFlag);
	for (int i = 0; i < m_nPlayer; i++)
	{
		m_pPlayer[i] = new cTaskConsMultiMenuPlayer(pConsMan, i, cG02RallyXSysExt::szMatchName[i]);   // sys +0x68 + 0x54 i (the matching info)
		app->ConnectChild(m_pPlayer[i]);
	}
	m_pStage = new cTaskConsMultiMenuStage(pConsMan);
	m_pStage->nStageKey() = m_nStageKey;
	app->ConnectChild(m_pStage);
	m_pSingle = new cTaskConsMultiMenuSingle(pConsMan);
	app->ConnectChild(m_pSingle);
	m_pMap = new cTaskMultiMenuMap(pConsMan, m_nStageKey);
	m_pMap->RegistData();
	app->ConnectChild(m_pMap);
	m_pCurtain = new cTaskConsMultiMenuCurtain(pConsMan, m_nStageKey);
	m_pCurtain->Init();
	app->ConnectChild(m_pCurtain);
	AddHitArray();
}

// DOL 0x8010cff4
cTaskConsMultiMenuMan::~cTaskConsMultiMenuMan()
{
}

// DOL 0x8010d05c. Coming from the matching (0x0c700000) the stage index restarts at 25 (the first multiplayer stage); all four stages are open.
void cTaskConsMultiMenuMan::InitStageNo()
{
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	if (KingdomGlobal::SysCore->GetLastJobNo() == job::RALLYX_MATCHING) sys->nStageIdx(0x19);   // sys 0x800c5850
	m_nMaxKey = 3;
	m_nStageKey = cG02RallyXSysExt::StageInWorldOf(sys->nStageIdx()) - 1;   // sys 0x800c5808(NULL)
}

// DOL 0x8010d0c0: the pointable consoles by focus number (the controllers of players 1..3 are not pointable)
void cTaskConsMultiMenuMan::AddHitArray()
{
	for (auto& p : m_pHitArray) p = nullptr;
	m_pHitArray[1] = m_pBack;
	m_pHitArray[2] = m_pRule;
	m_pHitArray[3] = m_pMap;
	m_pHitArray[4] = m_nPlayer > 0 ? m_pPlayer[0]->pController() : nullptr;
	m_pHitArray[8] = m_pStart;
	m_pHitArray[9] = m_pSingle;
	m_pHitArray[10] = m_pArrow[0];
	m_pHitArray[11] = m_pArrow[1];
	m_pHitArray[13] = m_pArrow[2];
	m_pHitArray[14] = m_pArrow[3];
	m_pHitArray[15] = m_pArrow[4];
	m_pHitArray[16] = m_pArrow[5];
}

// DOL 0x8010d164 (the Bg, the board, the time and the flag consoles are not touched)
void cTaskConsMultiMenuMan::AllWaitState()
{
	m_pBack->WaitState();
	m_pStart->WaitState();
	m_pSingle->WaitState();
	m_pRule->WaitState();
	m_pStage->WaitState();
	m_pCurtain->WaitState();
	m_pMap->WaitState();
	if (m_nPlayer > 0) m_pPlayer[0]->WaitState();
	for (int i = 0; i < 6; i++) m_pArrow[i]->WaitState();
}

// DOL 0x8010d99c
void cTaskConsMultiMenuMan::Reset()
{
	AllWaitState();
	m_nState[0] = 0;
	m_nStep[0] = 0;
}

// DOL 0x8010d260. The arrows count as their row (0xc stage, 0x11 flags, 0x12 time); UP / DOWN follow the table (first match; the DOL has no
// skip loop here).
void cTaskConsMultiMenuMan::KeySelect(uint trig)
{
	int& sel = m_nSelectNow[0];
	if ((uint)(sel - 10) < 2u)
		sel = 0xc;
	else if (sel == 0xf || sel == 0xd)
		sel = 0x11;
	else if (sel == 0x10 || sel == 0xe)
		sel = 0x12;
	int column = 0;
	if (trig & PAD_UP)
		column = 1;
	else if (trig & PAD_DOWN)
		column = 2;
	if (column == 0) return;
	for (int i = 0; i < 10; i++)
	{
		if (s_KeyTbl[i][0] == sel)
		{
			sel = s_KeyTbl[i][column];
			return;
		}
	}
}

// DOL 0x8010d388. TWO focuses Back. The move sound is skipped for the pair sums 0x16 / 0x17 / 0x1e / 0x20 / 0x22 (moving between the arrows of a row).
void cTaskConsMultiMenuMan::PointUpdate(uint trig)
{
	int& sel = m_nSelectNow[0];
	if (trig & PAD_2) sel = 1;
	if (sel == 0) return;
	if (sel != m_nSelectOld[0])
	{
		const int sum = sel + m_nSelectOld[0];
		if (sum != 0x16 && sum != 0x17 && sum != 0x1e && sum != 0x20 && sum != 0x22) PlaySndSe("m00se_01idou", 0);   // PTR 0x8047ba50
		AllWaitState();
		if (sel == 0xc)
		{
			m_pHitArray[10]->RestartAnim();
			m_pHitArray[11]->RestartAnim();
		}
		else if (sel == 0x11)
		{
			m_pHitArray[13]->RestartAnim();
			m_pHitArray[15]->RestartAnim();
		}
		else if (sel == 0x12)
		{
			m_pHitArray[14]->RestartAnim();
			m_pHitArray[16]->RestartAnim();
		}
		else if (sel == 3)
		{
			m_pHitArray[3]->RestartAnim();
			m_pStart->RestartAnim();
		}
	}
	if (sel == 0xc)
	{
		m_pHitArray[10]->PointState();
		m_pHitArray[11]->PointState();
	}
	else if (sel == 0x11)
	{
		m_pHitArray[13]->PointState();
		m_pHitArray[15]->PointState();
	}
	else if (sel == 0x12)
	{
		m_pHitArray[14]->PointState();
		m_pHitArray[16]->PointState();
	}
	else if (sel == 3)
	{
		m_pHitArray[3]->PointState();
		m_pStart->PointState();
	}
	else if ((uint)sel < 19u && m_pHitArray[sel] != nullptr)
	{
		m_pHitArray[sel]->PointState();
	}
}

// DOL 0x8010d748 (the table loop does not stop at a match, as in the DOL)
void cTaskConsMultiMenuMan::DecideCheck(uint trig)
{
	const int sel = m_nSelectNow[0];
	if (sel == 0) return;
	if ((trig & PAD_2) != 0 && sel == m_nSelectOld[0])
	{
		m_nState[0] = 2;
	}
	else if ((trig & 0x860u) != 0)   // A / B / Z
	{
		for (int i = 0; i < 13; i++)
			if (s_DecideTbl[i][0] == sel) m_nState[0] = s_DecideTbl[i][1];
	}
	else if (trig & PAD_LEFT)
	{
		if (sel == 0xc)
			m_nState[0] = 0xc;
		else if (sel == 0x11)
			m_nState[0] = 7;
		else if (sel == 0x12)
			m_nState[0] = 9;
	}
	else if (trig & PAD_RIGHT)
	{
		if (sel == 0xc)
			m_nState[0] = 0xd;
		else if (sel == 0x11)
			m_nState[0] = 8;
		else if (sel == 0x12)
			m_nState[0] = 10;
	}
}

// DOL 0x8010d86c: pad 0's pointer over a console focuses it (the last hit wins), otherwise the keys move; then players 1.. switch their own
// control type with A / B / Z of their pad
void cTaskConsMultiMenuMan::Select()
{
	if (m_nState[0] == 0)
	{
		const GamePad::State& pad = GamePad::Get(0);
		m_nSelectNow[0] = m_nSelectOld[0];
		bool hit = false;
		for (int i = 0; i < 19; i++)
		{
			// UNVERIFIED as in the other menus: the DOL tests the pointer floats (pad +0x48 / +0x4c) even when the remote points away
			if (m_pHitArray[i] != nullptr && pad.pointerCount > 0 && m_pHitArray[i]->IsDpdHit(pad.pointerX, pad.pointerY))
			{
				m_nSelectNow[0] = i;
				hit = true;
			}
		}
		if (!hit) KeySelect(pad.trig);
		PointUpdate(pad.trig);
		DecideCheck(pad.trig);
		m_nSelectOld[0] = m_nSelectNow[0];
	}
	for (int p = 1; p < m_nPlayer; p++)
	{
		if (m_nState[p] == 0 && (GamePad::Get(cG02RallyXSysExt::nPadNo[p]).trig & 0x860u) != 0) m_nState[p] = p + 0xe;
	}
}

// DOL 0x8010daac. Nothing while the main app is paused (KingSys app +0x1b4).
bool cTaskConsMultiMenuMan::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (KingdomGlobal::KingSys->pAppMain()->IsPause()) return true;
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	int& state = m_nState[0];
	int& step = m_nStep[0];
	switch (state)
	{
	case 2:   // Back (TWO = the cancel sound)
		if (step == 0)
		{
			m_pBack->DecideState();
			if ((GamePad::Get(cG02RallyXSysExt::nPadNo[0]).trig & PAD_2) == 0)
				PlaySndSe("m00se_02Kettei", 0);   // PTR 0x8047ba54
			else
				PlaySndSe("m00se_03Cancel", 0);   // PTR 0x8047ba58
			step++;
		}
		break;
	case 3:   // Single player
		if (step == 0)
		{
			m_pSingle->DecideState();
			PlaySndSe("m00se_02Kettei", 0);
			step++;
		}
		break;
	case 4:   // Start (the button)
		if (step == 0)
		{
			m_pStart->DecideState();
			m_pMap->DecideState();
			PlaySndSe("m00se_02Kettei", 0);
			step++;
		}
		break;
	case 5:   // Rule
		if (step == 0)
		{
			m_pRule->DecideState();
			PlaySndSe("m00se_02Kettei", 0);
			step++;
		}
		break;
	case 6:   // Start (the map)
		if (step == 0)
		{
			m_pMap->DecideState();
			m_pStart->DecideState();
			PlaySndSe("m00se_02Kettei", 0);
			step++;
		}
		break;
	case 7:    // flag count left
	case 8:    // flag count right
	{
		const bool left = state == 7;
		cTaskConsMultiMenuArrow* pDecided = m_pArrow[left ? 2 : 4];
		m_pArrow[left ? 4 : 2]->WaitState();   // the other arrow, every frame
		if (step == 0)
		{
			if (left)
			{
				if (--m_nFlagIdx < 0) m_nFlagIdx = 4;
			}
			else
			{
				if (4 < ++m_nFlagIdx) m_nFlagIdx = 0;
			}
			sys->nFlagIdx(m_nFlagIdx);   // sys 0x800c5970
			pDecided->DecideState();
			m_pFlag->DeleteState();
			PlaySndSe("m00se_02Kettei", 0);
			step++;
		}
		else if (step == 1)
		{
			if (m_pFlag->IsDeleteEndState())
			{
				m_pFlag->nValue() = sys->nFlagCount();
				m_pFlag->AppearState();
				step++;
			}
		}
		else if (step == 2 && m_pFlag->IsAppearEndState())
		{
			m_pFlag->WaitState();
			pDecided->WaitState();
			step = 0;
			state = 0;
		}
		break;
	}
	case 9:    // time left
	case 10:   // time right
	{
		const bool left = state == 9;
		cTaskConsMultiMenuArrow* pDecided = m_pArrow[left ? 3 : 5];
		m_pArrow[left ? 5 : 3]->WaitState();
		if (step == 0)
		{
			if (left)
			{
				if (--m_nTimeIdx < 0) m_nTimeIdx = 2;
			}
			else
			{
				if (2 < ++m_nTimeIdx) m_nTimeIdx = 0;
			}
			sys->nTimeIdx(m_nTimeIdx);   // sys 0x800c5998
			pDecided->DecideState();
			m_pTime->DeleteState();
			PlaySndSe("m00se_02Kettei", 0);
			step++;
		}
		else if (step == 1)
		{
			if (m_pTime->IsDeleteEndState())
			{
				m_pTime->nValue() = sys->nTimeMs();
				m_pTime->AppearState();
				step++;
			}
		}
		else if (step == 2 && m_pTime->IsAppearEndState())
		{
			m_pTime->WaitState();
			pDecided->WaitState();
			step = 0;
			state = 0;
		}
		break;
	}
	case 0xb:
		PlaySndSe("m00se_02Kettei", 0);
		state = 0;
		break;
	case 0xc:   // stage left
	case 0xd:   // stage right
	{
		const bool right = state == 0xd;
		cTaskConsMultiMenuArrow* pDecided = m_pArrow[right ? 1 : 0];
		m_pArrow[right ? 0 : 1]->WaitState();
		if (step == 0)
		{
			if (right)
			{
				do
				{
					m_nStageKey = m_nStageKey == 3 ? 0 : m_nStageKey + 1;
				} while ((uint)m_nMaxKey < (uint)m_nStageKey);
			}
			else
			{
				do
				{
					m_nStageKey = m_nStageKey == 0 ? 3 : m_nStageKey - 1;
				} while ((uint)m_nMaxKey < (uint)m_nStageKey);
			}
			m_pStage->DeleteState();
			pDecided->DecideState();
			PlaySndSe("m00se_07select", 0);   // PTR 0x8047ba5c
			step++;
		}
		else if (step == 1)
		{
			if (m_pStage->IsDeleteEndState())
			{
				m_pMap->ChangeData(m_nStageKey);
				m_pStage->nStageKey() = m_nStageKey;
				m_pBg->nWorldNo() = m_nStageKey;
				m_pCurtain->SetWorld(m_nStageKey);
				m_pBg->AppearState();
				m_pStage->AppearState();
				step++;
			}
		}
		else if (step == 2)
		{
			if (pDecided->IsDecideEndState() && m_pStage->IsAppearEndState())
			{
				state = 0;
				step = 0;
			}
		}
		break;
	}
	case 0xe:   // player 0's control type
		if (step == 0)
		{
			m_pPlayer[0]->pController()->NextState();
			PlaySndSe("m00se_02Kettei", 0);
			step++;
		}
		else if (m_pPlayer[0]->pController()->IsNextEndState())
		{
			state = 0;
			step = 0;
		}
		break;
	default:
		break;
	}
	// the other players' control types (the sound goes to their remote: ctrl = the player)
	for (int p = 1; p < m_nPlayer; p++)
	{
		if (p + 0xe != m_nState[p]) continue;
		cTaskConsMultiMenuController* ctl = m_pPlayer[p]->pController();
		if (m_nStep[p] == 0)
		{
			ctl->NextState();
			PlaySndSe("m00se_02Kettei", p);
			m_nStep[p]++;
		}
		else if (ctl->IsNextEndState())
		{
			m_nState[p] = 0;
			m_nStep[p] = 0;
			ctl->WaitState();
		}
	}
	return true;
}

// DOL 0x8010e3d8 (the Bg is not appeared here)
void cTaskConsMultiMenuMan::AppearStart()
{
	m_pBack->AppearState();
	m_pStart->AppearState();
	m_pSingle->AppearState();
	m_pRule->AppearState();
	m_pStage->AppearState();
	m_pCurtain->AppearState();
	m_pBoard->AppearState();
	m_pTime->AppearState();
	m_pFlag->AppearState();
	m_pMap->AppearState();
	for (int i = 0; i < m_nPlayer; i++) m_pPlayer[i]->AppearState();
	for (int i = 0; i < 6; i++) m_pArrow[i]->AppearState();
}

// DOL 0x8010e534
bool cTaskConsMultiMenuMan::IsAppearEnd()
{
	bool b = m_pBack->IsAppearEndState();
	if (b) b = m_pStart->IsAppearEndState();
	if (b) b = m_pSingle->IsAppearEndState();
	if (b) b = m_pRule->IsAppearEndState();
	if (b) b = m_pStage->IsAppearEndState();
	if (b) b = m_pCurtain->IsAppearEndState();
	if (b) b = m_pBoard->IsAppearEndState();
	if (b) b = m_pTime->IsAppearEndState();
	if (b) b = m_pFlag->IsAppearEndState();
	if (b) b = m_pMap->IsAppearEndState();
	for (int i = 0; i < m_nPlayer && b; i++) b = m_pPlayer[i]->IsAppearEndState();
	for (int i = 0; i < 6 && b; i++) b = m_pArrow[i]->IsAppearEndState();
	return b;
}

bool cTaskConsMultiMenuMan::IsBackDecide() { return m_pBack->IsDecideEndState(); }       // DOL 0x8010e6f0
bool cTaskConsMultiMenuMan::IsStartDecide() { return m_pStart->IsDecideEndState(); }     // DOL 0x8010e704
bool cTaskConsMultiMenuMan::IsSingleDecide() { return m_pSingle->IsDecideEndState(); }   // DOL 0x8010e718
bool cTaskConsMultiMenuMan::IsRuleDecide() { return m_pRule->IsDecideEndState(); }       // DOL 0x8010e72c

// DOL 0x8010e740: sys stage index = key + 25, the flag / time choices, every player's control type
void cTaskConsMultiMenuMan::ApplySettings()
{
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	sys->nStageIdx(m_nStageKey + 0x19);
	sys->nFlagIdx(m_nFlagIdx);
	sys->nTimeIdx(m_nTimeIdx);
	for (int i = 0; i < m_nPlayer; i++) cG02RallyXSysExt::nCtrlType[i] = m_pPlayer[i]->pController()->nType();   // sys 0x800c5b78
}

// DOL 0x8010e7d8
int cTaskConsMultiMenuMan::nCtrlType()
{
	return m_nPlayer > 0 ? m_pPlayer[0]->pController()->nType() : 1;
}

}
