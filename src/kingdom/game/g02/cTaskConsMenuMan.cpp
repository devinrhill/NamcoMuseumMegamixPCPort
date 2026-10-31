#include "kingdom/game/g02/cTaskConsMenuMan.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cTaskConsMenuConsoles.hpp"
#include "kingdom/game/g02/cTaskMenuCarBase.hpp"
#include "kingdom/game/g02/cTaskMenuMap.hpp"
#include "wii/Anim.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"

// Rally-X Remix (g02) stage select console manager, ported 1:1 from main.dol (no C# exists for Rally-X).

namespace kingdom::game::g02
{

namespace
{
using GamePad::PAD_UP, GamePad::PAD_DOWN, GamePad::PAD_LEFT, GamePad::PAD_RIGHT, GamePad::PAD_2;

// DOL 0x80376c38 [audited] (copied to the stack by KeySelect 0x80108a4c): {focus, UP target, DOWN target}, terminated by focus -1
const int s_KeyTbl[14][3] = {{0, -1, -1}, {1, 4, 14}, {2, 3, 6}, {4, 6, 1},  {3, 14, 2}, {5, 1, 2},  {6, 2, 9},
                             {7, 2, 10},  {8, 2, 11}, {9, 6, 4}, {10, 7, 4}, {11, 8, 4}, {14, 1, 3}, {-1, -1, -1}};
// DOL 0x80376ce0 [audited] (copied by DecideCheck 0x8010912c): {focus, state}, terminated by focus -1
const int s_DecideTbl[15][2] = {{0, -1}, {1, 2}, {4, 3}, {2, 4}, {5, 5}, {6, 8}, {7, 8}, {8, 8}, {9, 8}, {10, 8}, {11, 8}, {3, 9}, {12, 6}, {13, 7}, {-1, -1}};

// the light manager of the ctor (FUN_80018dd0 / FUN_800195c4): the SCN0 light of `sca` into light set `set`
void SetLight(int set, const char* sca)
{
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find(sca, size);
	wii::Brres b;
	wii::ScnAnim a;
	wii::LightResult light;
	if (d != nullptr && b.Load(d, size) && a.Load(b) && a.SampleLight(0, light))
	{
		a.SampleFog(light);
		kingdom::NativeScene::SetLight(set, light);
	}
}
}

// the light manager (+0x1d8: MENU_LIGHT_BG crc 0xf7fa406d set 0, MENU_LIGHT_MODEL crc 0xd04cf948 set 1) and the cTaskSceneCamera (+0x1dc,
// 0x1cc bytes, on ScnRoot camera 0; FUN_80149f74(cam, 0, MENU_LIGHT_MODEL) binds the SCN0, FUN_8014a388(cam, 0) plays it; connected to the
// app). NATIVE: the SCN0 camera 0 sampled once at frame 0 (UNVERIFIED: whether the SCN0 animates the camera after frame 0).
void SetupMenuScene()
{
	SetLight(0, "MENU_LIGHT_BG");
	SetLight(1, "MENU_LIGHT_MODEL");
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find("MENU_LIGHT_MODEL", size);
	wii::Brres b;
	wii::ScnAnim a;
	wii::CameraResult cam;
	if (d != nullptr && b.Load(d, size) && a.Load(b) && a.SampleCamera("", 0.0f, cam))
	{
		float target[3];
		cam.LookAt(target);   // MENU_LIGHT_MODEL's camera is a rotate camera (rotation -4.9, 48.4, 0 degrees), not an aim camera
		kingdom::NativeScene::SetCamera(cam.pos, target, cam.fovy, cam.nearZ, cam.farZ);
	}
}

void cTaskConsMenuMan::SetupScene()
{
	SetupMenuScene();
}

// DOL 0x80108074 [audited]
cTaskConsMenuMan::cTaskConsMenuMan(kingdom::game::cAppGameBaseMenu* pApp)
	: cTaskConsMenuManBase(pApp)
{
	SetName("g02::cTaskConsMenuMan");
	cTaskConsMenuManBase* pConsMan = this;
	kingdom::cTaskKingdom* app = KingdomGlobal::KingSys->pAppMain();   // KingSys +0xa40: every console is its child
	m_nState = 0;
	m_nStep = 0;
	InitStageNo();
	m_nSelectNow = m_nStageNo + 6;
	m_nSelectOld = m_nStageNo + 6;
	SetupScene();

	m_pBg = new cTaskConsMenuBg(pConsMan, m_nWorldNo);
	m_pBg->SetData();
	app->ConnectChild(m_pBg);
	m_pBack = new cTaskConsMenuBack(pConsMan);
	m_pBack->SetData();
	app->ConnectChild(m_pBack);
	m_pRule = new cTaskConsMenuRule(pConsMan);
	m_pRule->RegistData();
	app->ConnectChild(m_pRule);
	for (int i = 0; i < 2; i++)
	{
		m_pArrow[i] = new cTaskConsMenuArrow(pConsMan, i, m_nWorldMax != 0);
		app->ConnectChild(m_pArrow[i]);
	}
	m_pMulti = new cTaskConsMenuMulti(pConsMan);
	m_pMulti->SetData();
	app->ConnectChild(m_pMulti);
	m_pController = new cTaskConsMenuController(pConsMan, cG02RallyXSysExt::nCtrlType[0]);   // sys 0x800c5b88(0)
	m_pController->SetData();
	app->ConnectChild(m_pController);
	for (int i = 0; i < 6; i++)
	{
		m_pStage[i] = new cTaskConsMenuStage(pConsMan, m_nWorldNo, i);
		m_pStage[i]->LoadStageData(m_nWorldNo);
		app->ConnectChild(m_pStage[i]);
	}
	m_pWorld = new cTaskConsMenuWorld(pConsMan);
	m_pWorld->nWorldNo() = m_nWorldNo;
	app->ConnectChild(m_pWorld);
	m_pBar = new cTaskConsMenuBar(pConsMan);
	m_pBar->SetStage(m_nStageNo + m_nWorldNo * 6 + 1);   // the frame keeps world 0's anim (no SetWorld here, as in the DOL)
	app->ConnectChild(m_pBar);
	m_pCurtain = new cTaskConsMenuCurtain(pConsMan, m_nWorldNo);
	m_pCurtain->SetData();
	app->ConnectChild(m_pCurtain);
	m_pMap = new cTaskMenuMap(pConsMan, m_nWorldNo, m_nStageNo);
	m_pMap->RegistData();
	app->ConnectChild(m_pMap);
	m_pPacman = new cTaskMenuPacman();
	m_pPacman->Init();
	app->ConnectChild(m_pPacman);
	AddHitArray();
}

// DOL 0x801085bc [audited]: deletes the light manager (+0x1d8), clears the sys' "new stage" flag (0x800c5c70)
cTaskConsMenuMan::~cTaskConsMenuMan()
{
	cG02RallyXSysExt::bNewStageApear = false;
}

// DOL 0x80108674 [audited]. The newest open stage (record 0x800c6320) gives the max world / stage. Back from the debug menu (0x08600000) or the
// stage (0x0c300000): the current sys stage, or the newest one when a stage was just opened. Otherwise the newest one, unless everything
// was cleared (record 0x800c61d8): then stage 1 (sys 0x800c5850).
void cTaskConsMenuMan::InitStageNo()
{
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	const int nFree = cG02RallyXRecord::Get().GetFreeStage();
	m_nWorldMax = cG02RallyXSysExt::WorldOf(nFree) - 1;
	m_nStageMax = cG02RallyXSysExt::StageInWorldOf(nFree) - 1;
	const uint last = KingdomGlobal::SysCore->GetLastJobNo();
	if (last == job::SAVE || last == job::RALLYX_GAME)
	{
		if (!cG02RallyXSysExt::bNewStageApear)
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
	else if (!cG02RallyXRecord::Get().IsAllClear())
	{
		m_nWorldNo = m_nWorldMax;
		m_nStageNo = m_nStageMax;
	}
	else
	{
		sys->nStageIdx(1);
		m_nWorldNo = sys->nWorld() - 1;
		m_nStageNo = sys->nStageInWorld() - 1;
	}
}

// DOL 0x80108798 [audited]
void cTaskConsMenuMan::AddHitArray()
{
	m_pHitArray[0] = nullptr;
	m_pHitArray[14] = nullptr;
	m_pHitArray[1] = m_pBack;
	m_pHitArray[2] = m_pRule;
	m_pHitArray[5] = m_pMap;
	m_pHitArray[4] = m_pMulti;
	m_pHitArray[3] = m_pController;
	for (int i = 0; i < 6; i++) m_pHitArray[6 + i] = m_pStage[i];
	m_pHitArray[12] = m_pArrow[0];
	m_pHitArray[13] = m_pArrow[1];
}

// DOL 0x80108830 [audited] (the Bg and the car are not touched)
void cTaskConsMenuMan::AllWaitState()
{
	m_pBack->WaitState();
	m_pRule->WaitState();
	m_pWorld->WaitState();
	m_pBar->WaitState();
	m_pCurtain->WaitState();
	m_pMulti->WaitState();
	m_pController->WaitState();
	m_pMap->WaitState();
	for (int i = 0; i < 6; i++) m_pStage[i]->WaitState();
	for (int i = 0; i < 2; i++) m_pArrow[i]->WaitState();
}

// DOL 0x8010895c [audited]: a stage button focused -> it becomes the current stage (the map and the Bar follow)
void cTaskConsMenuMan::FreeStageCheck()
{
	const uint n = (uint)(m_nSelectNow - 6);
	if (n < 6)
	{
		m_nStageNo = (int)n;
		if ((int)n + m_nWorldNo * 6 != m_pMap->nCurIdx()) m_pMap->ChangeData((int)n + m_nWorldNo * 6);
		m_pBar->SetStage(m_nStageNo + m_nWorldNo * 6 + 1);
	}
}

// inlined in KeySelect (0x80108a4c): can the focus rest here? closed stages and inactive arrows are skipped
bool cTaskConsMenuMan::IsSelectable(int focus) const
{
	if ((uint)(focus - 6) < 6) return m_pStage[focus - 6]->IsOpen();
	if (focus == 12) return m_pArrow[0]->bActive();
	if (focus == 13) return m_pArrow[1]->bActive();
	if (focus == 14) return m_pArrow[0]->bActive() && m_pArrow[1]->bActive();
	return true;
}

// DOL 0x80108a4c [audited]. UP / DOWN follow the table; LEFT / RIGHT move inside the stage row of the focus (6..8 or 9..11, wrapping).
void cTaskConsMenuMan::KeySelect(uint trig)
{
	if ((uint)(m_nSelectNow - 12) < 2) m_nSelectNow = 14;
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
	auto row = [this](int step, int lo, int hi) {
		do
		{
			m_nSelectNow += step;
			if (m_nSelectNow < lo) m_nSelectNow = hi;
			if (hi < m_nSelectNow) m_nSelectNow = lo;
		} while (!IsSelectable(m_nSelectNow));
	};
	if (trig & PAD_UP)
		follow(1);
	else if (trig & PAD_DOWN)
		follow(2);
	else if (trig & PAD_LEFT)
	{
		if (6 <= m_nSelectNow && m_nSelectNow <= 8)
			row(-1, 6, 8);
		else if (8 < m_nSelectNow && m_nSelectNow < 12)
			row(-1, 9, 11);
	}
	else if (trig & PAD_RIGHT)
	{
		if (6 <= m_nSelectNow && m_nSelectNow <= 8)
			row(1, 6, 8);
		else if (8 < m_nSelectNow && m_nSelectNow < 12)
			row(1, 9, 11);
	}
}

// DOL 0x80108f74 [audited]
void cTaskConsMenuMan::PointUpdate(uint trig)
{
	if (trig & PAD_2) m_nSelectNow = 1;
	if (m_nSelectNow == 0) return;
	if (m_nSelectNow != m_nSelectOld)
	{
		const int sum = m_nSelectNow + m_nSelectOld;   // 12 + 14 / 13 + 14: moving between an arrow and the arrow row makes no sound
		if (sum != 0x1a && sum != 0x1b) PlaySndSe("m00se_01idou", 0);   // PTR 0x8047b978
	}
	if (m_nSelectNow != m_nSelectOld)
	{
		AllWaitState();
		FreeStageCheck();
		if (m_nSelectNow == 14)
		{
			m_pHitArray[12]->RestartAnim();
			m_pHitArray[13]->RestartAnim();
		}
		else if (m_nSelectNow == 5)
		{
			m_pHitArray[5]->RestartAnim();
			m_pStage[m_nStageNo]->RestartAnim();
		}
	}
	if (m_nSelectNow == 14)
	{
		m_pHitArray[12]->PointState();
		m_pHitArray[13]->PointState();
	}
	else if (m_nSelectNow == 5)
	{
		m_pHitArray[5]->PointState();
		m_pStage[m_nStageNo]->PointState();
	}
	else if ((uint)m_nSelectNow < 15 && m_pHitArray[m_nSelectNow] != nullptr)
	{
		m_pHitArray[m_nSelectNow]->PointState();
	}
}

// DOL 0x8010912c [audited] (the table loop does not stop at a match, as in the DOL)
void cTaskConsMenuMan::DecideCheck(uint trig)
{
	if (m_nSelectNow == 0) return;
	if ((trig & PAD_2) != 0 && m_nSelectNow == m_nSelectOld)
	{
		m_nState = 2;
	}
	else if ((trig & 0x860u) != 0)   // A / B / Z
	{
		for (int i = 0; s_DecideTbl[i][0] != -1; i++)
			if (s_DecideTbl[i][0] == m_nSelectNow) m_nState = s_DecideTbl[i][1];
	}
	else if (trig & PAD_LEFT)
	{
		if (m_nSelectNow == 14) m_nState = 6;
	}
	else if (trig & PAD_RIGHT)
	{
		if (m_nSelectNow == 14) m_nState = 7;
	}
}

// DOL 0x80109200 [audited]: only while choosing (state 0). The pad 0 pointer over a console focuses it (the last hit wins), otherwise the keys move.
void cTaskConsMenuMan::Select()
{
	if (m_nState != 0) return;
	const GamePad::State& pad = GamePad::Get(0);
	m_nSelectNow = m_nSelectOld;
	bool hit = false;
	for (uint i = 0; i < 15; i++)
	{
		// UNVERIFIED as in g01 / g03 / g07: the DOL tests the pointer floats (pad +0x48 / +0x4c) even when the remote points away
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

// DOL 0x801092d8 [audited]
void cTaskConsMenuMan::Reset()
{
	AllWaitState();
	m_nState = 0;
	m_nStep = 0;
}

// DOL 0x80109310 [audited]. Nothing while the main app is paused (KingSys app +0x1b4).
bool cTaskConsMenuMan::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (KingdomGlobal::KingSys->pAppMain()->IsPause()) return true;
	switch (m_nState)
	{
	case 2:   // Back (TWO = the cancel sound)
		if (m_nStep == 0)
		{
			m_pBack->DecideState();
			if ((GamePad::Get(0).trig & PAD_2) == 0)
				PlaySndSe("m00se_02Kettei", 0);
			else
				PlaySndSe("m00se_03Cancel", 0);
			m_nStep++;
		}
		break;
	case 3:   // Multi
		if (m_nStep == 0)
		{
			m_pMulti->DecideState();
			PlaySndSe("m00se_02Kettei", 0);
			m_nStep++;
		}
		break;
	case 4:   // Rule
		if (m_nStep == 0)
		{
			m_pRule->DecideState();
			PlaySndSe("m00se_02Kettei", 0);
			m_nStep++;
		}
		break;
	case 5:   // Map
		if (m_nStep == 0)
		{
			m_pMap->DecideState();
			m_pStage[m_nStageNo]->DecideState();
			PlaySndSe("m00se_02Kettei", 0);
			m_nStep++;
		}
		break;
	case 6:   // previous world (left arrow)
	case 7:   // next world (right arrow)
	{
		const bool next = m_nState == 7;
		m_pArrow[next ? 0 : 1]->WaitState();   // the other arrow, every frame
		cTaskConsMenuArrow* pDecided = m_pArrow[next ? 1 : 0];
		if (m_nStep == 0)
		{
			// the next / previous open world (wrapping over 0..3, skipping worlds past the newest), clamped to the newest stage
			do
			{
				if (next)
					m_nWorldNo = (uint)m_nWorldNo < 3u ? m_nWorldNo + 1 : 0;
				else
					m_nWorldNo = m_nWorldNo == 0 ? 3 : m_nWorldNo - 1;
			} while ((uint)m_nWorldMax < (uint)m_nWorldNo);
			if ((uint)m_nWorldMax <= (uint)m_nWorldNo)
			{
				m_nWorldNo = m_nWorldMax;
				if ((uint)m_nStageMax <= (uint)m_nStageNo) m_nStageNo = m_nStageMax;
			}
			m_pWorld->DeleteState();
			m_pBar->DeleteState();
			for (int i = 0; i < 6; i++) m_pStage[i]->DeleteState();
			pDecided->DecideState();
			PlaySndSe("m00se_07select", 0);
			m_nStep++;
		}
		else if (m_nStep == 1)
		{
			// DOL quirk: the loop meant for the six stages asks the World six times (+0x1f4 instead of +0x208 + 4i)
			bool b = m_pWorld->IsDeleteEndState();
			if (b) b = m_pBar->IsDeleteEndState();
			for (int i = 0; i < 6 && b; i++) b = m_pWorld->IsDeleteEndState();
			if (b)
			{
				m_pWorld->nWorldNo() = m_nWorldNo;
				m_pBg->nWorldNo() = m_nWorldNo;
				m_pBar->SetStage(m_nStageNo + m_nWorldNo * 6 + 1);
				m_pCurtain->SetWorld(m_nWorldNo);
				m_pBar->SetWorld(m_nWorldNo);
				if (m_nStageNo + m_nWorldNo * 6 != m_pMap->nCurIdx()) m_pMap->ChangeData(m_nStageNo + m_nWorldNo * 6);
				for (int i = 0; i < 6; i++) m_pStage[i]->LoadStageData(m_nWorldNo);
				m_pBg->AppearState();
				m_pWorld->AppearState();
				m_pBar->AppearState();
				for (int i = 0; i < 6; i++) m_pStage[i]->AppearState();
				m_nStep++;
			}
		}
		else if (m_nStep == 2)
		{
			// the same quirk (World IsAppearEndState six times instead of the stages)
			bool b = pDecided->IsDecideEndState();
			if (b) b = m_pWorld->IsAppearEndState();
			if (b) b = m_pBar->IsAppearEndState();
			for (int i = 0; i < 6 && b; i++) b = m_pWorld->IsAppearEndState();
			if (b)
			{
				m_nState = 0;
				m_nStep = 0;
			}
		}
		break;
	}
	case 8:   // a stage (closed: NG sound, back to choosing)
		if (m_nStep == 0)
		{
			cTaskConsMenuStage* pStage = m_pStage[m_nStageNo];
			if (!pStage->IsOpen())
			{
				PlaySndSe("m00se_08NG", 0);
				m_nState = 0;
			}
			else
			{
				pStage->DecideState();
				m_pMap->DecideState();
				PlaySndSe("m00se_02Kettei", 0);
				m_nStep++;
			}
		}
		break;
	case 9:   // the control type switches (Controller NextState 11 -> 12)
		if (m_nStep == 0)
		{
			m_pController->NextState();
			PlaySndSe("m00se_02Kettei", 0);
			m_nStep++;
		}
		else if (m_pController->IsNextEndState())
		{
			m_nState = 0;
			m_nStep = 0;
		}
		break;
	default:
		break;
	}
	// a pointed stage button keeps the map on its stage
	for (int i = 0; i < 6; i++)
		if (m_pStage[i]->nCurState() == 4 && i + m_nWorldNo * 6 != m_pMap->nCurIdx()) m_pMap->ChangeData(i + m_nWorldNo * 6);
	return true;
}

// DOL 0x80109a2c [audited] (the Bg is not appeared here; the car through its vtable +0x48)
void cTaskConsMenuMan::AppearStart()
{
	m_pBack->AppearState();
	m_pRule->AppearState();
	m_pWorld->AppearState();
	m_pBar->AppearState();
	m_pCurtain->AppearState();
	m_pMulti->AppearState();
	m_pController->AppearState();
	m_pMap->AppearState();
	m_pPacman->AppearState();
	for (int i = 0; i < 6; i++) m_pStage[i]->AppearState();
	for (int i = 0; i < 2; i++) m_pArrow[i]->AppearState();
}

// DOL 0x80109b6c [audited]
bool cTaskConsMenuMan::IsAppearEnd()
{
	bool b = m_pBack->IsAppearEndState();
	if (b) b = m_pRule->IsAppearEndState();
	if (b) b = m_pWorld->IsAppearEndState();
	if (b) b = m_pBar->IsAppearEndState();
	if (b) b = m_pCurtain->IsAppearEndState();
	if (b) b = m_pMulti->IsAppearEndState();
	if (b) b = m_pController->IsAppearEndState();
	if (b) b = m_pMap->IsAppearEndState();
	if (b) b = m_pPacman->IsAppearEnd();
	for (int i = 0; i < 6 && b; i++) b = m_pStage[i]->IsAppearEndState();
	for (int i = 0; i < 2 && b; i++) b = m_pArrow[i]->IsAppearEndState();
	return b;
}

bool cTaskConsMenuMan::IsBackDecide() { return m_pBack->IsDecideEndState(); }                 // DOL 0x80109d0c [audited]
bool cTaskConsMenuMan::IsMultiDecide() { return m_pMulti->IsDecideEndState(); }               // DOL 0x80109d20 [audited]
bool cTaskConsMenuMan::IsStageDecide() { return m_pStage[m_nStageNo]->IsDecideEndState(); }   // DOL 0x80109d34 [audited]
bool cTaskConsMenuMan::IsRuleDecide() { return m_pRule->IsDecideEndState(); }                 // DOL 0x80109d54 [audited]

// DOL 0x80109d68 [audited]: sys +0x08 = stage index, sys 0x800c5c60(1) (+0x0c players), sys 0x800c5b78(0, type) (+0x14 player 0's control type)
void cTaskConsMenuMan::SetStage()
{
	KingdomGlobal::RallyXSys->nStageIdx(m_nStageNo + m_nWorldNo * 6 + 1);
	cG02RallyXSysExt::nPlayer = 1;
	cG02RallyXSysExt::nCtrlType[0] = m_pController->nType();
}

// DOL 0x80109dcc [audited]
int cTaskConsMenuMan::nCtrlType()
{
	return m_pController->nType();
}

}
