#include "kingdom/game/g01/cTaskConsMenuMan.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskConsMenuArrow.hpp"
#include "kingdom/game/g01/cTaskConsMenuBack.hpp"
#include "kingdom/game/g01/cTaskConsMenuBar.hpp"
#include "kingdom/game/g01/cTaskConsMenuBg.hpp"
#include "kingdom/game/g01/cTaskConsMenuMap.hpp"
#include "kingdom/game/g01/cTaskConsMenuMulti.hpp"
#include "kingdom/game/g01/cTaskConsMenuRule.hpp"
#include "kingdom/game/g01/cTaskConsMenuStage.hpp"
#include "kingdom/game/g01/cTaskConsMenuWorld.hpp"
#include "kingdom/game/g01/cTaskConsMultiMenuPlayer.hpp"

// Galaga stage select console manager, ported 1:1 from main.dol (no C# exists for Galaga). DOL vtable slots of the consoles used here
// (task vtable +0x60..): RestartAnim, WaitState, IsWaitState, AppearState, AppearEndState, IsAppearEndState, PointState, DecideState,
// DecideEndState, IsDecideEndState, DeleteState, DeleteEndState, IsDeleteEndState. Every console is a child of this task (ConnectChild on +0x50).

namespace kingdom::game::g01
{

namespace
{
using GamePad::PAD_UP, GamePad::PAD_DOWN, GamePad::PAD_LEFT, GamePad::PAD_RIGHT, GamePad::PAD_2;

// DOL 0x803701a0 (copied to the stack by KeySelect 0x800bdac8): {focus, UP target, DOWN target}, terminated by focus -1
const int s_KeyTbl[10][3] = {{0, -1, -1}, {1, 4, 10}, {2, 1, 3}, {3, 10, 5}, {4, 5, 1}, {5, 3, 4}, {6, 3, 4}, {7, 3, 4}, {10, 1, 3}, {-1, -1, -1}};
// DOL 0x80370218 (copied by DecideCheck 0x800be028): {focus, state}, terminated by focus -1
const int s_DecideTbl[11][2] = {{0, -1}, {1, 4}, {2, 2}, {3, 5}, {4, 6}, {5, 3}, {6, 3}, {7, 3}, {8, 8}, {9, 7}, {-1, -1}};
}

// The DOL indexes Stage[man +0x210] without a bound; when the newest open stage is the boss (nOpenStage 10: nStageInWorld = 4) InitStageNo
// can set stage 3, which in the DOL reads the next member (Back, +0x238). UNVERIFIED whether that is reachable; clamped here to stay in the array.
static inline int StageSlot(int n) { return n < 0 ? 0 : (n > 2 ? 2 : n); }

// DOL 0x800bc114
cTaskConsMenuMan::cTaskConsMenuMan(kingdom::game::cAppGameBaseMenu* pApp, bool bMulti)
	: cTaskConsMenuManBase(pApp)
{
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	cTaskConsMenuManBase* pConsMan = this;
	m_nPlayer = sys->nPlayer();
	m_bMulti = bMulti;
	m_nStageNo = 0;
	m_nStageMax = 0;
	m_nWorldNo = 0;
	m_nWorldMax = 0;
	InitStageNo();
	m_pPlayer[0] = nullptr;
	m_nSelectOld = m_nSelectNow = m_nStageNo + 5;
	m_pPlayer[1] = m_pPlayer[2] = m_pPlayer[3] = nullptr;

	m_pMulti = new cTaskConsMenuMulti(pConsMan, m_bMulti);
	m_pMulti->nDrawPriority(0x80000001u);
	ConnectChild(m_pMulti);
	for (int i = 0; i < 3; i++)
	{
		m_pStage[i] = new cTaskConsMenuStage(pConsMan, (uint)m_nWorldNo, (uint)i, m_bMulti);
		m_pStage[i]->nDrawPriority(0x80000001u);
		m_pStage[i]->LoadStageData((uint)m_nWorldNo);
		ConnectChild(m_pStage[i]);
	}
	m_pBack = new cTaskConsMenuBack(pConsMan);
	m_pBack->nDrawPriority(0x80000001u);
	ConnectChild(m_pBack);
	m_pWorld = new cTaskConsMenuWorld(pConsMan);
	m_pWorld->SetWorldNo(m_nWorldNo);
	m_pWorld->nDrawPriority(0x80000001u);
	ConnectChild(m_pWorld);
	for (int i = 0; i < 2; i++)
	{
		m_pArrow[i] = new cTaskConsMenuArrow(pConsMan, i, m_nWorldMax != 0);
		m_pArrow[i]->nDrawPriority(0x80000001u);
		ConnectChild(m_pArrow[i]);
	}
	m_pRule = new cTaskConsMenuRule(pConsMan);
	m_pRule->nDrawPriority(0x80000001u);
	ConnectChild(m_pRule);
	if (!m_bMulti)
	{
		m_pBar = new cTaskConsMenuBar(pConsMan);
		m_pBar->nDrawPriority(0x80000001u);
		m_pBar->SetStage(m_nWorldNo, m_nStageNo);
		ConnectChild(m_pBar);
	}
	else
	{
		// no priority call here in the DOL (the panel's ctor sets its own)
		for (uint i = 0; i < (uint)m_nPlayer; i++)
		{
			m_pPlayer[i] = new cTaskConsMultiMenuPlayer(pConsMan, (int)i, sys->pName((int)i));
			m_pPlayer[i]->SetRecord(m_nWorldNo, m_nStageNo);
			ConnectChild(m_pPlayer[i]);
		}
	}
	m_pBg = new cTaskConsMenuBg(pConsMan, m_nWorldNo);
	m_pBg->nDrawPriority(0x10001u);
	ConnectChild(m_pBg);
	m_pMap = new cTaskConsMenuMap(pConsMan, (uint)m_nWorldNo, (uint)m_nStageNo);
	m_pMap->nDrawPriority(0x10002u);
	ConnectChild(m_pMap);
	AddHitArray();
}

// DOL 0x800bc774
cTaskConsMenuMan::~cTaskConsMenuMan()
{
	if (KingdomGlobal::GalagaSys != nullptr) KingdomGlobal::GalagaSys->bNewStageApear(false);
}

// DOL 0x800bc838
void cTaskConsMenuMan::InitStageNo()
{
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	const int open = sys->nOpenStage();
	m_nWorldMax = sys->nWorld(open) - 1;
	m_nStageMax = sys->nStageInWorld(open) - 1;
	if (sys->nStage() == 10) sys->nStage(9);
	const uint last = KingdomGlobal::SysCore->GetLastJobNo();
	if (last == job::SAVE || last == job::GALAGA_GAME)
	{
		if (!sys->bNewStageApear())
		{
			m_nWorldNo = sys->nWorld() - 1;
			m_nStageNo = sys->nStageInWorld() - 1;
		}
		else
		{
			m_nWorldNo = m_nWorldMax;
			m_nStageNo = m_nStageMax;
		}
	}
	else if (!sys->bAllOpen())
	{
		m_nWorldNo = m_nWorldMax;
		m_nStageNo = m_nStageMax;
	}
	else
	{
		sys->nStage(1);
		m_nWorldNo = sys->nWorld() - 1;
		m_nStageNo = sys->nStageInWorld() - 1;
	}
}

// DOL 0x800bc988. The Bar / player panels get IsAppearEndState (vtable +0x74), not WaitState, in the DOL: kept (it has no side effect).
void cTaskConsMenuMan::AllWaitState()
{
	m_pMulti->WaitState();
	for (int i = 0; i < 3; i++) m_pStage[i]->WaitState();
	m_pBack->WaitState();
	m_pWorld->WaitState();
	for (int i = 0; i < 2; i++) m_pArrow[i]->WaitState();
	if (!m_bMulti)
		(void)m_pBar->IsAppearEndState();
	else
		for (uint i = 0; i < (uint)m_nPlayer; i++) (void)m_pPlayer[i]->IsAppearEndState();
	m_pMap->WaitState();
	m_pRule->WaitState();
}

bool cTaskConsMenuMan::AllBoardDeleteEnd()
{
	bool b = true;
	for (int i = 0; i < 3 && b; i++) b = m_pStage[i]->IsDeleteEndState();
	if (!m_bMulti)
	{
		if (b) b = m_pBar->IsDeleteEndState();
	}
	else
	{
		for (uint i = 0; i < (uint)m_nPlayer && b; i++) b = m_pPlayer[i]->IsDeleteEndState();
	}
	return b;
}

bool cTaskConsMenuMan::AllBoardAppearEnd()
{
	bool b = true;
	if (!m_bMulti)
	{
		if (b) b = m_pBar->IsAppearEndState();
	}
	else
	{
		for (uint i = 0; i < (uint)m_nPlayer && b; i++) b = m_pPlayer[i]->IsAppearEndState();
	}
	for (int i = 0; i < 3 && b; i++) b = m_pStage[i]->IsAppearEndState();
	return b;
}

// Execute states 7 / 8, step 1: the boards are gone -> change the world (wrapping), clamp to the newest open stage, set everything up again
void cTaskConsMenuMan::ChangeWorld(int step)
{
	if (step > 0)
	{
		if (m_nWorldMax < ++m_nWorldNo) m_nWorldNo = 0;
	}
	else
	{
		if (--m_nWorldNo < 0) m_nWorldNo = m_nWorldMax;
	}
	if (m_nWorldMax <= m_nWorldNo)
	{
		m_nWorldNo = m_nWorldMax;
		if (m_nStageMax <= m_nStageNo) m_nStageNo = m_nStageMax;
	}
	m_pBg->nWorldNo() = m_nWorldNo;
	m_pWorld->SetWorldNo(m_nWorldNo);
	m_pWorld->AppearState();
	if (!m_bMulti)
	{
		m_pBar->SetStage(m_nWorldNo, m_nStageNo);
		m_pBar->AppearState();
	}
	else
	{
		for (uint i = 0; i < (uint)m_nPlayer; i++)
		{
			m_pPlayer[i]->SetRecord(m_nWorldNo, m_nStageNo);
			m_pPlayer[i]->AppearState();
		}
	}
	m_pMap->ChangeData((uint)m_nWorldNo, (uint)m_nStageNo);
	m_pMap->AppearState();
	for (int i = 0; i < 3; i++)
	{
		m_pStage[i]->LoadStageData((uint)m_nWorldNo);
		m_pStage[i]->AppearState();
	}
}

// DOL 0x800bcad4 (through the task thunk 0x800be38c). Nothing while the main app is paused (KingSys app +0x1b4).
bool cTaskConsMenuMan::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (KingdomGlobal::KingSys->pAppMain()->IsPause()) return true;
	switch (m_nState)
	{
	case 1:   // appear: wait for every console, then all of them wait (Rule and Map keep their state)
	{
		bool b = m_pMulti->IsAppearEndState();
		if (b) b = m_pBack->IsAppearEndState();
		if (b) b = m_pWorld->IsAppearEndState();
		if (!m_bMulti)
		{
			if (b) b = m_pBar->IsAppearEndState();
		}
		else
		{
			for (uint i = 0; i < (uint)m_nPlayer; i++)
				if (b) b = m_pPlayer[i]->IsAppearEndState();
		}
		if (b) b = m_pMap->IsAppearEndState();
		for (int i = 0; i < 3 && b; i++) b = m_pStage[i]->IsAppearEndState();
		for (int i = 0; i < 2 && b; i++) b = m_pArrow[i]->IsAppearEndState();
		if (b)
		{
			m_pMulti->WaitState();
			m_pBack->WaitState();
			m_pWorld->WaitState();
			if (!m_bMulti)
				m_pBar->WaitState();
			else
				for (uint i = 0; i < (uint)m_nPlayer; i++) m_pPlayer[i]->WaitState();
			for (int i = 0; i < 3; i++) m_pStage[i]->WaitState();
			for (int i = 0; i < 2; i++) m_pArrow[i]->WaitState();
			m_nState = 0;
		}
		break;
	}
	case 2:   // Map decided
		if (m_nStep == 0)
		{
			PlaySndSe("m00se_02Kettei", 0);
			m_pMap->DecideState();
			m_pStage[StageSlot(m_nStageNo)]->DecideState();
			m_nStep++;
		}
		break;
	case 3:   // a stage decided (locked: NG sound, back to choosing)
		if (m_nStep == 0)
		{
			if (!m_pStage[StageSlot(m_nStageNo)]->IsOpen())
			{
				PlaySndSe("m00se_08NG", 0);
				m_nState = 0;
				m_nStep = 0;
			}
			else
			{
				PlaySndSe("m00se_02Kettei", 0);
				m_pMap->DecideState();
				m_pStage[StageSlot(m_nStageNo)]->DecideState();
				m_nStep++;
			}
		}
		break;
	case 4:   // Back
		if (m_nStep == 0)
		{
			if ((GamePad::Get(0).trig & 0x80u) == 0)
				PlaySndSe("m00se_02Kettei", 0);
			else
				PlaySndSe("m00se_03Cancel", 0);
			m_pBack->DecideState();
			m_nStep++;
		}
		break;
	case 5:   // Rule
		if (m_nStep == 0)
		{
			PlaySndSe("m00se_02Kettei", 0);
			m_pRule->DecideState();
			m_nStep++;
		}
		break;
	case 6:   // Multi
		if (m_nStep == 0)
		{
			m_pMulti->DecideState();
			PlaySndSe("m00se_02Kettei", 0);
			m_nStep++;
		}
		break;
	case 7:   // next world (right arrow)
	case 8:   // previous world (left arrow)
	{
		const bool next = m_nState == 7;
		cTaskConsMenuArrow* pDecided = m_pArrow[next ? 1 : 0];
		cTaskConsMenuArrow* pOther = m_pArrow[next ? 0 : 1];
		if (m_nStep == 0)
		{
			PlaySndSe("m00se_07select", 0);
			pDecided->DecideState();
			pOther->WaitState();
			if (!next) m_pMap->DeleteState();   // state 8 deletes the Map first, state 7 last
			if (!m_bMulti)
				m_pBar->DeleteState();
			else
				for (uint i = 0; i < (uint)m_nPlayer; i++) m_pPlayer[i]->DeleteState();
			for (int i = 0; i < 3; i++) m_pStage[i]->DeleteState();
			if (next) m_pMap->DeleteState();
			m_nStep++;
		}
		else if (m_nStep == 1)
		{
			if (AllBoardDeleteEnd())
			{
				ChangeWorld(next ? 1 : -1);
				m_nStep++;
			}
		}
		else if (m_nStep == 2)
		{
			bool b = m_pWorld->IsAppearEndState();
			if (b) b = pDecided->IsDecideEndState();
			if (next && b) b = m_pMap->IsAppearEndState();   // only state 7 waits for the Map
			if (b) b = AllBoardAppearEnd();
			if (b)
			{
				m_pWorld->WaitState();
				pDecided->WaitState();
				if (!m_bMulti)
					m_pBar->WaitState();
				else
					for (uint i = 0; i < (uint)m_nPlayer; i++) m_pPlayer[i]->WaitState();
				for (int i = 0; i < 3; i++) m_pStage[i]->WaitState();
				m_nState = 0;
				m_nStep = 0;
			}
		}
		break;
	}
	default:
		break;
	}
	return true;
}

// DOL 0x800bd984
void cTaskConsMenuMan::Reset()
{
	AllWaitState();
	m_nState = 0;
	m_nStep = 0;
}

// DOL 0x800bd9bc
void cTaskConsMenuMan::AddHitArray()
{
	m_pHitArray[0] = nullptr;
	m_pHitArray[10] = nullptr;
	m_pHitArray[1] = m_pBack;
	m_pHitArray[3] = m_pRule;
	m_pHitArray[2] = m_pMap;
	m_pHitArray[4] = m_pMulti;
	m_pHitArray[5] = m_pStage[0];
	m_pHitArray[6] = m_pStage[1];
	m_pHitArray[7] = m_pStage[2];
	m_pHitArray[8] = m_pArrow[0];
	m_pHitArray[9] = m_pArrow[1];
}

// DOL 0x800bda14: a stage button focused -> it becomes the current stage (Map and Bar / panels follow)
void cTaskConsMenuMan::FreeStageCheck()
{
	const uint n = (uint)(m_nSelectNow - 5);
	if (n < 3)
	{
		m_nStageNo = (int)n;
		m_pMap->ChangeData((uint)m_nWorldNo, (uint)m_nStageNo);
		if (!m_bMulti)
			m_pBar->SetStage(m_nWorldNo, m_nStageNo);
		else
			for (uint i = 0; i < (uint)m_nPlayer; i++) m_pPlayer[i]->SetRecord(m_nWorldNo, m_nStageNo);
	}
}

// inlined in KeySelect (0x800bdac8): can the focus rest here? locked stages and inactive arrows are skipped
bool cTaskConsMenuMan::IsSelectable(int focus) const
{
	if ((uint)(focus - 5) < 3) return m_pStage[focus - 5]->IsOpen();
	if (focus == 8) return m_pArrow[0]->bActive();
	if (focus == 9) return m_pArrow[1]->bActive();
	if (focus == 10) return m_pArrow[0]->bActive() && m_pArrow[1]->bActive();
	return true;
}

// DOL 0x800bdac8
void cTaskConsMenuMan::KeySelect(uint trig)
{
	if ((uint)(m_nSelectNow - 8) < 2) m_nSelectNow = 10;
	auto follow = [this](int column) {
		do
		{
			for (int i = 0; s_KeyTbl[i][0] != -1; i++)
			{
				if (s_KeyTbl[i][0] == m_nSelectNow)
				{
					m_nSelectNow = s_KeyTbl[i][column];
					break;
				}
			}
		} while (!IsSelectable(m_nSelectNow));
	};
	if (trig & PAD_UP)
		follow(1);
	else if (trig & PAD_DOWN)
		follow(2);
	else if (trig & PAD_LEFT)
	{
		if (4 < m_nSelectNow && m_nSelectNow < 8)
		{
			do
			{
				if (--m_nSelectNow < 5) m_nSelectNow = 7;
			} while (!IsSelectable(m_nSelectNow));
		}
	}
	else if (trig & PAD_RIGHT)
	{
		if (4 < m_nSelectNow && m_nSelectNow < 8)
		{
			do
			{
				if (7 < ++m_nSelectNow) m_nSelectNow = 5;
			} while (!IsSelectable(m_nSelectNow));
		}
	}
}

// DOL 0x800bde70
void cTaskConsMenuMan::PointUpdate(uint trig)
{
	if (trig & PAD_2) m_nSelectNow = 1;
	if (m_nSelectNow == 0) return;
	if (m_nSelectNow != m_nSelectOld)
	{
		const int sum = m_nSelectNow + m_nSelectOld;   // 8 + 10 / 9 + 10: moving between an arrow and the arrow row makes no sound
		if (sum != 0x12 && sum != 0x13) PlaySndSe("m00se_01idou", 0);
	}
	if (m_nSelectNow != m_nSelectOld)
	{
		AllWaitState();
		FreeStageCheck();
		if (m_nSelectNow == 10)
		{
			m_pHitArray[8]->RestartAnim();
			m_pHitArray[9]->RestartAnim();
		}
		else if (m_nSelectNow == 2)
		{
			m_pHitArray[2]->RestartAnim();
			m_pStage[StageSlot(m_nStageNo)]->RestartAnim();
		}
	}
	if (m_nSelectNow == 10)
	{
		m_pHitArray[8]->PointState();
		m_pHitArray[9]->PointState();
	}
	else if (m_nSelectNow == 2)
	{
		m_pHitArray[2]->PointState();
		m_pStage[StageSlot(m_nStageNo)]->PointState();
	}
	else if ((uint)m_nSelectNow < 11 && m_pHitArray[m_nSelectNow] != nullptr)
	{
		m_pHitArray[m_nSelectNow]->PointState();
	}
}

// DOL 0x800be028
void cTaskConsMenuMan::DecideCheck(uint trig)
{
	if (m_nSelectNow == 0) return;
	if ((trig & PAD_2) != 0 && m_nSelectNow == m_nSelectOld)
	{
		m_nState = 4;
	}
	else if ((trig & 0x860u) != 0)   // A / B / Z
	{
		for (int i = 0; s_DecideTbl[i][0] != -1; i++)
			if (s_DecideTbl[i][0] == m_nSelectNow) m_nState = s_DecideTbl[i][1];
	}
	else if (trig & PAD_LEFT)
	{
		if (m_nSelectNow == 10) m_nState = 8;
	}
	else if (trig & PAD_RIGHT)
	{
		if (m_nSelectNow == 10) m_nState = 7;
	}
}

// DOL 0x800be0fc: only while choosing (state 0). The pad 0 pointer over a console focuses it (the last hit wins), otherwise the keys move.
void cTaskConsMenuMan::Select()
{
	if (m_nState != 0) return;
	const GamePad::State& pad = GamePad::Get(0);
	m_nSelectNow = m_nSelectOld;
	bool hit = false;
	for (uint i = 0; i < 11; i++)
	{
		// UNVERIFIED as in g03 / g07: the DOL tests the pointer floats even when the remote points away; the native pad keeps the last position
		if (m_pHitArray[i] != nullptr && pad.pointerCount > 0 && m_pHitArray[i]->IsDpdHit(pad.pointerX, pad.pointerY))
		{
			m_nSelectNow = (int)i;
			hit = true;
		}
	}
	if (!hit) KeySelect(pad.trig);
	PointUpdate(pad.trig);
	DecideCheck(pad.trig);
	m_nSelectOld = m_nSelectNow;
}

// DOL 0x800be1d4
void cTaskConsMenuMan::AppearStart()
{
	m_nState = 1;
	m_pMulti->AppearState();
	for (int i = 0; i < 3; i++) m_pStage[i]->AppearState();
	m_pBack->AppearState();
	m_pWorld->AppearState();
	for (int i = 0; i < 2; i++) m_pArrow[i]->AppearState();
	if (!m_bMulti)
		m_pBar->AppearState();
	else
		for (uint i = 0; i < (uint)m_nPlayer; i++) m_pPlayer[i]->AppearState();
	m_pRule->AppearState();
	m_pMap->AppearState();
}

bool cTaskConsMenuMan::IsRuleDecide() { return m_pRule->IsDecideEndState(); }                         // DOL 0x800be328
bool cTaskConsMenuMan::IsStageDecide() { return m_pStage[StageSlot(m_nStageNo)]->IsDecideEndState(); } // DOL 0x800be33c
bool cTaskConsMenuMan::IsMultiDecide() { return m_pMulti->IsDecideEndState(); }                       // DOL 0x800be35c
bool cTaskConsMenuMan::IsBackDecide() { return m_pBack->IsDecideEndState(); }                         // DOL 0x800be370

// DOL 0x800be384 (vtable slot 18)
void cTaskConsMenuMan::SaveStageRecord()
{
	if (m_pBar != nullptr) m_pBar->SetCurrentStageRecord();
}

}
