#include "platform/devtools.hpp"
#include "platform/devtools_g01edit.hpp"
#include "kingdom/game/g01/cAppG01GalagaGame.hpp"
#include "kingdom/game/g01/cAppG01GalagaBoss.hpp"
#include "kingdom/game/cTaskConsRetry.hpp"
#include "kingdom/cTaskCursor.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomModelBase.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/game/g01/cDemoMan.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskCamera.hpp"
#include "kingdom/game/g01/cTaskCollisionMan.hpp"
#include "kingdom/game/g01/cTaskConsoleLife.hpp"
#include "kingdom/game/g01/cTaskConsoleReadyString.hpp"
#include "kingdom/game/g01/cTaskConsoleResultSingle.hpp"
#include "kingdom/game/g01/cTaskConsoleMultiPlayer.hpp"
#include "kingdom/game/g01/cTaskConsolePlayer.hpp"
#include "kingdom/game/g01/cTaskConsoleResultMulti.hpp"
#include "kingdom/game/g01/cTaskEffect.hpp"
#include "kingdom/game/g01/cTaskEnemy.hpp"
#include "kingdom/game/g01/cTaskPacman.hpp"
#include "kingdom/game/g01/cTaskPause.hpp"
#include "GamePad.hpp"
#include "kingdom/game/g01/cTaskScene.hpp"
#include "kingdom/game/g01/cTaskStage.hpp"
#include <EBGMKind.hpp>
#include <SoundManager.hpp>
#include "snd/Mixer.hpp"

// cTaskGameConsoleMan is not a class natively (the app drives its consoles, see below).
namespace kingdom::game::g01
{

// DOL 0x800b0ed4 [audited]: stage 10 (cG01GalagaSys::nStage) builds cAppG01GalagaBoss, the others cAppG01GalagaGame
void cAppG01GalagaGame::JobInit()
{
	cAppG01GalagaGame* pApp = KingdomGlobal::GalagaSys->nStage() == 10 ? new cAppG01GalagaBoss() : new cAppG01GalagaGame();
	pApp->Init();
	KingdomGlobal::SysCore->RegistMainTask(pApp);
}

// DOL 0x801bb1a0 [audited: 3D sound setup (34000/6000 scale, id counter) not ported]
cAppG01GalagaGame::cAppG01GalagaGame()
{
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	m_nStage = sys->nStage();
	m_nWorld = sys->nWorld();
	m_bMulti = sys->IsMulti();
	m_nNextJob = 0;
	m_bRetry = false;
	sys->bNewStageApear(false);
	sys->bUnk9(false);
	sys->AttachStageArc(m_nWorld, m_nStage);
	kingdom::cKingdomModelBase::ClearAssetCache();
	// The 3D sound setup: FUN_80025714(KingSys +0xa54, FUN_8002571c()) stores 34000 / 6000 (FLOAT_80479ef0 / FUN_8000d834) into the sound
	// manager's +0x2c8 float (no native counterpart: UNVERIFIED what it scales), and FUN_801c16e8 resets DAT_80479110, the id counter of the
	// cGalagaSndActor objects 0x801c1680 makes (natively kingdom::cKingdomSndActor needs no id). (Two calls of FUN_800151a8-like setters with 0 on KingSys come before AttachStageArc; UNVERIFIED.)
}

// DOL 0x801bb258 [audited: scene root/lights/cameras are the native scene's; children compared]
void cAppG01GalagaGame::Init()
{
	// cTaskScnRoot(0x200, 0x200, 0x80, 0x10) + light setup + 4 cameras (eye (0, 0, 100.3), fovy 60, aspect 1.3333, near 0.1, far 32766):
	// natively the scene task draws the models submitted this frame (cTaskModel::Draw -> cTaskScnRoot) with the camera cTaskScene sets
	kingdom::NativeScene::Reset();
	kingdom::NativeScene::SetPerFrame(true);
	// DOL 0x801bb3ec [audited]..0x801bb480 (the boss, 0x801b8968.., is the same): the depth shadow light straight above the origin, radius 25, ortho 20 x 20,
	// near 20, far 40 (41 at the end of Init)
	{
		kingdom::NativeScene::SetShadow(true);
		kingdom::cKingdomShadow& sh = kingdom::NativeScene::Shadow();
		sh.SetMode(0);
		sh.SetTarget(0.0f, 0.0f, 0.0f);
		sh.SetAngles(1.5707964f, 0.0f, 0.0f);
		sh.SetOrtho(20.0f, 20.0f, 20.0f, 41.0f);
		sh.SetRadius(25.0f);
	}
	ConnectChild(new kingdom::cTaskNativeScene());
	m_pTaskFade = new kingdom::cTaskFade();
	m_pTaskFade->Fade(2u, 500);   // in, black
	ConnectChild(m_pTaskFade);
	m_pTaskPause = new cTaskPause();   // 0x801b92cc (+0x2cc), then g01::cTaskEffect (0x801c1ec0) as the app's particle manager
	ConnectChild(m_pTaskPause);
	cG01GalagaSys::s_bPause = false;   // native (the DOL leaves the last value; the app's Execute rewrites it before its children run)
	pTaskPtkMan() = new cTaskEffect();
	ConnectChild(pTaskPtkMan());
	cTaskCamera* pCamera = new cTaskCamera();   // 0x800d3194
	ConnectChild(pCamera);
	KingdomGlobal::GalagaSys->pTaskCamera() = pCamera;
	KingdomGlobal::GalagaSys->pTaskCollisionMan() = new cTaskCollisionMan();
	delete KingdomGlobal::GalagaSys->pDemoMan();
	KingdomGlobal::GalagaSys->pDemoMan() = new cDemoMan();   // the owners below register their demos
	m_pTaskEnemyMan = CreateEnemyMan();
	m_pTaskScene = CreateScene();
	m_pTaskScene->SetEnemyMan(m_pTaskEnemyMan);
	ConnectChild(m_pTaskScene);
	pCamera->Update();   // native: the frame of the start camera before the first spawn
	m_pTaskReady = new cTaskConsoleReadyString();   // 0x800ebab0: the READY sign first, then the life gauge and the players
	ConnectChild(m_pTaskReady);
	// cTaskGameConsoleMan (0x800ebab0, 0x250 bytes: HUD, NOT PORTED) makes a cTaskConsolePlayer per player (pad nWork(player)); natively the
	// players are the app's children
	for (int i = 0; i < KingdomGlobal::GalagaSys->nPlayer() && i < 4; i++)
	{
		if (KingdomGlobal::GalagaSys->nPlayer() > 1)   // 0x800ebab0: more than one player -> cTaskConsoleMultiPlayer
			m_pPlayer[i] = new cTaskConsoleMultiPlayer(i, KingdomGlobal::GalagaSys->nWork(i), KingdomGlobal::GalagaSys->nPlayer());
		else
			m_pPlayer[i] = new cTaskConsolePlayer(i, KingdomGlobal::GalagaSys->nWork(i), KingdomGlobal::GalagaSys->nPlayer());
		ConnectChild(m_pPlayer[i]);
	}
	m_pTaskLife = new cTaskConsoleLife();   // cTaskGameConsoleMan +0x228 (0x800ebab0, not ported as a whole)
	ConnectChild(m_pTaskLife);
	if (KingdomGlobal::GalagaSys->nPlayer() <= 1 && m_pPlayer[0] != nullptr)   // 0x800ebab0: alone -> cTaskConsoleResultSingle (0x800d7c40 / 0x800d7f78)
	{
		m_pTaskResult = new cTaskConsoleResultSingle(m_pPlayer[0], m_pTaskLife);
		ConnectChild(m_pTaskResult);
	}
	else if (KingdomGlobal::GalagaSys->nPlayer() > 1)   // 0x800ebab0: cTaskConsoleResultMulti (0x800dd06c + Init 0x800dd444)
	{
		const int np = KingdomGlobal::GalagaSys->nPlayer() < 4 ? KingdomGlobal::GalagaSys->nPlayer() : 4;
		m_pTaskResultMulti = new cTaskConsoleResultMulti(m_pPlayer, np, m_pTaskLife);
		for (int i = 0; i < np; i++) static_cast<cTaskConsoleMultiPlayer*>(m_pPlayer[i])->SetResult(m_pTaskResultMulti);
		ConnectChild(m_pTaskResultMulti);
	}
	// cTaskRetry (0x801b9d0c: ANT_RETRY 0x2623c0fd, a cursor per player with ANT_MENU_POINTER 0xf63671cb), priority 0x8000000b; natively the
	// shared kingdom::game::cTaskConsRetry it derives from
	m_pTaskRetry = new kingdom::game::cTaskConsRetry("ANT_RETRY", "ANT_MENU_POINTER", (uint)KingdomGlobal::GalagaSys->nPlayer());
	m_pTaskRetry->nDrawPriority(0x8000000bu);
	for (int i = 0; i < KingdomGlobal::GalagaSys->nPlayer() && i < 4; i++)   // 0x801b9d0c: cursor i on the player's work pad (vtable +0x58)
		if (kingdom::cTaskCursor* c = m_pTaskRetry->pCursor((uint)i)) c->SetPadNo(KingdomGlobal::GalagaSys->nWork(i));
	ConnectChild(m_pTaskRetry);
	m_pTaskPacman = CreatePacman();
	m_pTaskPacman->SetLife(m_pTaskLife);   // 0x800f40f0 (enemy manager, console player: not ported)
	ConnectChild(m_pTaskPacman);
	m_pTaskEnemyMan->Init(m_nStage, m_pTaskScene, m_pTaskPacman);   // vtable slot 13 (0x800e6244)
	ConnectChild(m_pTaskEnemyMan);
	CreateBoss();
	m_pTaskStage = new cTaskStage();
	m_pTaskStage->Setup(m_nStage);
	ConnectChild(m_pTaskStage);
	ConnectChild(KingdomGlobal::GalagaSys->pTaskCollisionMan());
	m_nFlow = 0;
	m_bRetry = KingdomGlobal::SysCore->GetLastJobNo() == job::GALAGA_GAME;   // UNVERIFIED (old hand port: Jbc_GetProcNoOld)
}

cTaskScene* cAppG01GalagaGame::CreateScene()
{
	return new cTaskScene(m_nStage, m_pTaskEnemyMan, 1.0f);   // FLOAT_8047ce48 = 1.0
}

cTaskPacman* cAppG01GalagaGame::CreatePacman()
{
	return new cTaskPacman(m_nStage, m_pTaskScene);
}

cTaskEnemyMan* cAppG01GalagaGame::CreateEnemyMan()
{
	return new cTaskEnemyMan();
}

// DOL 0x801bbdb8 [audited]
void cAppG01GalagaGame::Free()
{
	KingdomGlobal::GalagaSys->DetachStageArc(m_nWorld, m_nStage);
	KingdomGlobal::GalagaSys->pTaskCamera() = nullptr;
	KingdomGlobal::GalagaSys->pTaskCollisionMan() = nullptr;   // deleted with the task tree
	delete KingdomGlobal::GalagaSys->pDemoMan();   // 0x801bbdb8
	KingdomGlobal::GalagaSys->pDemoMan() = nullptr;
	kingdom::NativeScene::Reset();
	SoundManager::get_SE()->ClearListener();   // the camera's listener (0x800d3868) goes with the tree
	cAppKingdom::Free();
}

// DOL 0x800ec2f8 [audited] (console manager vtable 32; 33 is the same on the stage work for stage 9): the best of each value goes into the record, with
// the "norma reached" flags (ScoreNorma.csv; stage 10 uses row 9). The life gauge's +0x228 (the life bonus (life + 1) * 1000, set by
// 0x800d54e4 when the result starts) is added to the score; the console player's +0x240 rate bonus (-1 = none) is not ported.
static void StoreResult(sStageRecord* rec, const cTaskConsolePlayer* player, const cTaskConsoleLife* life)
{
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	const sScoreNorma* norma = sys->nStage() == 10 ? sys->pScoreNorma(9) : sys->pScoreNorma();
	const u32 score = (u32)(player->nScore() + life->nBonus());   // shown score +0x244 + the life bonus +0x228 (set at the result start)
	if ((s32)score >= rec->n10) rec->n10 = (s32)score;
	if (norma->nScore <= (s32)score) rec->n1c = 1;
	float rate = 0.0f;   // FLOAT_8047b5c8
	if (player->nShot() != 0) rate = (float)player->nHit() / (float)player->nShot();
	if (player->nRateCarry() != -1) rate = 0.5f * (rate + (float)player->nRateCarry() / 1000.0f);   // stage 10: with stage 9's (FLOAT_8047b5d0)
	const u32 permil = (u32)(1000.0f * rate);   // FLOAT_8047b5cc
	if ((u32)rec->n08 <= permil) rec->n08 = (s32)permil;
	if ((u32)norma->nHitRate <= permil) rec->n18 = 1;
	const u32 kills = (u32)player->nKill();
	if ((u32)rec->n00 <= kills) rec->n00 = (s32)kills;
	if ((u32)norma->nKill <= kills) rec->n14 = 1;
	const u32 lives = (u32)(life->nLife() + 1);
	if ((u32)rec->n0c <= lives) rec->n0c = (s32)lives;
	if ((u32)norma->nLife <= lives) rec->n20 = 1;
}

// The HUD console manager (cTaskGameConsoleMan, vtable 0x8043acf0) is not a class natively: the apps drive its consoles directly. Its object
// slots +0x78 / +0x7c / +0x80 / +0x84 (0x800eb664 / 0x800eb694 / 0x800eb6c4 / 0x800eb6f4: states 1 ready, 3 game, 4 result, 6 retry; 0x800eb64c =
// state 0) set its state; its Execute hands each state to every console: ready 0x800eb76c (the READY sign, the life gauge and each player
// console to state 1, then true when all are in state 2 (0x800eb680)), game 0x800eb87c (life + players to state 3), result 0x800eb914, retry
// 0x800eba28 (the players' +0x84). The multiplayer record 0x800ec610 (vtable 34) is StoreMulti below.
// DOL 0x800ec4fc [audited] (console manager vtable 33, stage 9 alone): this run's values straight into the stage work (no best-of, no norma flags):
// kills +0x00, hit rate per mille +0x08 (with the carried rate as 0x800ec2f8), lives + 1 +0x0c, score + life bonus +0x10. Stage 10 starts
// from them (cTaskConsolePlayer 0x800d65cc).
static void StoreWork(sStageRecord* rec, const cTaskConsolePlayer* player, const cTaskConsoleLife* life)
{
	rec->n10 = player->nScore() + life->nBonus();
	float rate = 0.0f;   // FLOAT_8047b5c8
	if (player->nShot() != 0) rate = (float)player->nHit() / (float)player->nShot();
	if (player->nRateCarry() != -1) rate = 0.5f * (rate + (float)player->nRateCarry() / 1000.0f);   // FLOAT_8047b5d0 / 8047b5cc
	rec->n08 = (s32)(u32)(1000.0f * rate);
	rec->n00 = player->nKill();   // +0x234
	rec->n0c = life->nLife() + 1;   // +0x224 + 1
}

// DOL 0x800ec610 [audited] (console manager vtable 34, called with the stage's multiplayer record, or with the work area on stage 9): this run's numbers
// (no best-of): the winners' mask +0x00 (the result console's +0x244), lives + 1 +0x04, per player kills +0x08.. / hit rate per mille +0x18.. /
// score + the life bonus +0x28..
static void StoreMultiRec(sMultiRecord* rec, cTaskConsolePlayer** players, int count, const cTaskConsoleLife* life, const cTaskConsoleResultMulti* result)
{
	for (int i = 0; i < count && i < 4; i++)
	{
		const cTaskConsolePlayer* p = players[i];
		rec->n[10 + i] = p->nScore() + life->nBonus();
		float rate = 0.0f;
		if (p->nShot() != 0) rate = (float)p->nHit() / (float)p->nShot();
		if (p->nRateCarry() != -1) rate = 0.5f * (rate + (float)p->nRateCarry() / 1000.0f);
		rec->n[6 + i] = (s32)(u32)(1000.0f * rate);
		rec->n[2 + i] = p->nKill();
	}
	rec->n[1] = life->nLife() + 1;
	rec->n[0] = (s32)result->nWinMask();
}

void cAppG01GalagaGame::StoreResult(sStageRecord* rec)
{
	if (m_pPlayer[0] != nullptr) g01::StoreResult(rec, m_pPlayer[0], m_pTaskLife);
}

void cAppG01GalagaGame::StoreMulti(sMultiRecord* rec)
{
	if (m_pTaskResultMulti != nullptr) StoreMultiRec(rec, m_pPlayer, KingdomGlobal::GalagaSys->nPlayer(), m_pTaskLife, m_pTaskResultMulti);
}

// DOL 0x801bbf5c [audited]. The result goes into the record (console vtable 32 / 33 / multiplayer 34); the next stage opens
// and the record is saved.
void cAppG01GalagaGame::SaveRecord()
{
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	if (sys->nStage() == 9)
	{
		if (!m_bMulti)
		{
			sys->pStageWork()->bOpen = 1;
			if (m_pPlayer[0] != nullptr) StoreWork(sys->pStageWork(), m_pPlayer[0], m_pTaskLife);   // vtable 33
		}
		else if (m_pTaskResultMulti != nullptr)
			StoreMulti(sys->pMultiWork());   // vtable 34 on the work area
		return;
	}
	if (!m_bMulti && m_pPlayer[0] != nullptr) StoreResult(sys->pStageRecord());   // vtable 32
	else if (m_bMulti && m_pTaskResultMulti != nullptr)
		StoreMulti(sys->pMultiRecord());   // vtable 34 on the stage's multiplayer record
	sStageRecord* pNext = sys->pStageRecord(sys->nStage() + 1);
	if (pNext->bOpen != 1)
	{
		pNext->bOpen = 1;
		sys->bNewStageApear(true);
	}
	sys->SaveData();
}

// DOL 0x801bc0fc [audited: the START / DEMO / GOAL stage-task tick (+0x2c0 state 0) goes through cDemoMan natively]: the nModeState / nModeStep machine (step 0 enter, 1 run, 2 leave)
bool cAppG01GalagaGame::Execute(cJetTask* pParam)
{
	(void)pParam;
	const uint n = m_nFlow;
	static int dbgFrames = 0;
	dbgFrames++;
	if (std::getenv("MEGAMIX_G01DBG") && nModeState(n) != m_nDbgMode) { std::fprintf(stderr, "g01 app mode %u -> %u (app frame %d, frame %d)\n", m_nDbgMode, nModeState(n), dbgFrames, devtools::FrameNo()); m_nDbgMode = nModeState(n); }
	switch (nModeState(n))
	{
	case ePROC_START:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
		{
			nModeStep(n) = 1;
			cDemoMan* dm = KingdomGlobal::GalagaSys->pDemoMan();
			if (!dm->IsEmpty())
			{
				if (!m_bRetry) dm->Start();   // 0x801becd8
				else dm->Finish();            // 0x801bee48: a retry skips the demo
			}
			[[fallthrough]];
		}
		case 1:
			if (m_pTaskFade->IsEnd())
			{
				const cDemoMan* dm = KingdomGlobal::GalagaSys->pDemoMan();
				NextMode(n, (!dm->IsEmpty() && !m_bRetry) ? ePROC_DEMO : ePROC_READY);
			}
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
		case 2:
			nModeStep(n) = 0;
			break;
		}
		break;
	case ePROC_DEMO:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
			nModeStep(n) = 1;
			KingdomGlobal::GalagaSys->pDemoMan()->Reset();   // 0x801beccc
			m_pTaskStage->SetState(0);
			[[fallthrough]];
		case 1:
		{
			cDemoMan* dm = KingdomGlobal::GalagaSys->pDemoMan();
			if (dm->IsAllDone() || dm->IsSkip()) NextMode(n, ePROC_READY);   // 0x801bedb8 / 0x801bec4c
			dm->Tick();   // 0x801becb8
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
		}
		case 2:
			nModeStep(n) = 0;
			KingdomGlobal::GalagaSys->pDemoMan()->Finish();   // 0x801bee48
			break;
		}
		break;
	case ePROC_READY:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
			nModeStep(n) = 1;
			m_pTaskScene->ReadyStart();   // 0x800f6a80
			snd::Play("m01se_01start");  // DAT_8047ce10 on the BGM handle (+0x294)
			m_pTaskPacman->ReadyStart();
			for (cTaskConsolePlayer* p : m_pPlayer) if (p != nullptr) { p->Appear(); p->SetEnable(true); }   // console vtable 28, 0x800ec2cc(console, 1)
			m_pTaskLife->Appear();
			if (m_pTaskReady != nullptr) m_pTaskReady->Ready();   // every console's READY call (0x800eb664)
			[[fallthrough]];
		case 1:
			PauseCheck();
			if (m_pTaskScene->IsReady() && m_pTaskPacman->IsReady())
			{
				// stage BGM by world (DAT_8047ce00..0c: m01bgm_World1 / World2 / World3, the 4th stage of world 3 m01bgm_Boss2)
				EBGMKind bgm = EBGMKind::GALAGA_SND_BGM02_WORLD1;
				if (m_nWorld == 2) bgm = EBGMKind::GALAGA_SND_BGM03_WORLD2;
				else if (m_nWorld == 3) bgm = KingdomGlobal::GalagaSys->nStageInWorld() == 4 ? EBGMKind::GALAGA_SND_BGM06_BOSS2 : EBGMKind::GALAGA_SND_BGM04_WORLD3;
				SoundManager::get_BGM()->Stop(0.0f);
				SoundManager::get_BGM()->Play(bgm, 1.0f, true);
				NextMode(n, ePROC_GAME);
			}
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
		case 2:
			nModeStep(n) = 0;
			break;
		}
		break;
	case ePROC_GAME:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
			nModeStep(n) = 1;
			m_pTaskLife->SetGame(true);   // console vtable +0x7c: state 3
			m_pTaskStage->SetState(1);
			m_pTaskScene->GameStart();   // 0x800f6aa8
			m_pTaskPacman->GameStart();
			[[fallthrough]];
		case 1:
			if (std::getenv("MEGAMIX_G01DBG") && m_pPlayer[0] != nullptr && (++m_nDbgFrame % 60) == 0)
				std::fprintf(stderr, "g01 player0 score %d kills %d shots %d hits %d life %d\n", m_pPlayer[0]->nScore(), m_pPlayer[0]->nKill(), m_pPlayer[0]->nShot(), m_pPlayer[0]->nHit(), m_pTaskLife->nLife());
			PauseCheck();
			// Pac-Man reached the end (0x800f3ebc) -> GOAL (stage 9) / CLEAR; console miss flag -> MISS
			if (m_pTaskPacman->IsGoal()) NextMode(n, m_nStage == 9 ? ePROC_GOAL : ePROC_CLEAR);
			if (m_pTaskLife->IsMiss()) NextMode(n, ePROC_MISS);   // console +0x228 -> +0x22c
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
		case 2:
			nModeStep(n) = 0;
			for (cTaskConsolePlayer* p : m_pPlayer) if (p != nullptr) p->SetEnable(false);   // 0x800ec2cc(console, 0)
			break;
		}
		break;
	case ePROC_CLEAR:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
			nModeStep(n) = 1;
			// console manager vtable +0x80 (0x800eb914: every console to state 4; the result console 0x800dfcb0 starts its phases), the stage
			// BGM goes to volume 0.25 (FLOAT_8047ce50, 100 ms), the clear jingle DAT_8047ce14 m01se_15clear on the jingle handle (+0x290)
			m_pTaskLife->StartResult();
			for (cTaskConsolePlayer* p : m_pPlayer) if (p != nullptr) p->StartResult();
			if (m_pTaskResult != nullptr) m_pTaskResult->Start();
			if (m_pTaskResultMulti != nullptr) m_pTaskResultMulti->Start();
			SoundManager::get_BGM()->SetVolume(0.25f, 0.1f);   // 0x801bc7f8: 0x80026394(0.25, BGM +0x284, 100): ducked under the jingle
			m_nJingle = snd::Play("m01se_15clear");
			[[fallthrough]];
		case 1:
			// 0x800eb6e0: the consoles reached state 5 (result done); then the jingle stops (0x80026318). Only the result console is
			// waited for natively (the life gauge's is done at once); multiplayer waits for cTaskConsoleResultMulti
			if (m_pTaskResult != nullptr && !m_pTaskResult->IsEnd()) break;
			if (m_pTaskResultMulti != nullptr && !m_pTaskResultMulti->IsEnd()) break;
			if (std::any_of(std::begin(m_pPlayer), std::end(m_pPlayer), [](cTaskConsolePlayer* p) { return p != nullptr && !p->IsResultEnd(); })) break;
			if (m_nJingle >= 0) snd::Stop(m_nJingle);
			m_nJingle = -1;
			NextMode(n, ePROC_GOAL);
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
		case 2:
			nModeStep(n) = 0;
			break;
		}
		break;
	case ePROC_MISS:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
			nModeStep(n) = 1;
			m_pTaskScene->MissStart();      // 0x800f6abc
			m_pTaskPacman->MissStart();     // vtable 61
			KingdomGlobal::GalagaSys->pTaskCollisionMan()->Stop();   // 0x800d44b0
			[[fallthrough]];
		case 1:
			PauseCheck();
			if (m_pTaskPacman->IsMissEnd()) NextMode(n, ePROC_RETRY);
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
		case 2:
			nModeStep(n) = 0;
			break;
		}
		break;
	case ePROC_RETRY:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
			nModeStep(n) = 1;
			m_pTaskLife->SetGame(false);   // console vtable +0x84: state 6
			for (cTaskConsolePlayer* p : m_pPlayer) if (p != nullptr) p->SetEnable(false);   // console vtable 31 (UNVERIFIED: what it does)
			m_pTaskRetry->Start();   // 0x80149b88
			[[fallthrough]];
		case 1:
			if (m_pTaskRetry->IsYes())   // +0x1d0 == 1
			{
				m_nNextJob = job::GALAGA_GAME;
				NextMode(n, ePROC_EXIT);
			}
			else if (m_pTaskRetry->IsNo())   // +0x1d0 == 2
			{
				m_nNextJob = job::GALAGA_GAMEOVER;   // game over (job 181)
				NextMode(n, ePROC_EXIT);
			}
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
		case 2:
			nModeStep(n) = 0;
			break;
		}
		break;
	case ePROC_GOAL:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
		{
			nModeStep(n) = 1;
			SaveRecord();   // vtable +0x3c
			cDemoMan* dm = KingdomGlobal::GalagaSys->pDemoMan();
			dm->Reset();    // 0x801beccc
			if (!dm->IsEmpty())
			{
				// the goal demos (0x801bed48); the stage task (+0x2c0) goes back to state 0 / step 0 first (0x801bc9a4); world 3 fades the BGM out (500 ms)
				m_pTaskStage->SetState(0);
				dm->StartGoal();
				if (m_nWorld == 3) SoundManager::get_BGM()->Stop(0.5f);
			}
			else
			{
				// no goal demo: EXIT to the menu, then on through step 1 (DOL LAB_801bca00: its done test may still pick the boss for
				// stage 9) whose tail resets the step for EXIT's step 0
				m_nNextJob = job::GALAGA_MENU;
				NextMode(n, ePROC_EXIT);
			}
			[[fallthrough]];
		}
		case 1:
		{
			// all goal demos done (0x801bedf8) or skipped (0x801bec4c): stage 9 goes on to stage 10 (the boss, 0x0b300000), the others to the menu
			cDemoMan* dm = KingdomGlobal::GalagaSys->pDemoMan();
			if (dm->IsAllGoalDone() || dm->IsSkip())
			{
				if (m_nStage == 9)
				{
					KingdomGlobal::GalagaSys->nStage(10);   // 0x800afd0c (GXInitTexObjUserData in the map)
					m_nNextJob = job::GALAGA_GAME;
				}
				else
					m_nNextJob = job::GALAGA_MENU;
				NextMode(n, ePROC_EXIT);
			}
			dm->Tick();   // 0x801becb8
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
		}
		case 2:
			nModeStep(n) = 0;
			break;
		}
		break;
	case ePROC_EXIT:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
			nModeStep(n) = 1;
			m_pTaskFade->Fade(0x10002u, 500);
			SoundManager::get_BGM()->Stop(0.5f);
			[[fallthrough]];
		case 1:
			if (m_pTaskFade->IsEnd()) Kingdom_SetJobNo(m_nNextJob, KingdomGlobal::GalagaSys->nStage() != 10, false);
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
		case 2:
			nModeStep(n) = 0;
			break;
		}
		break;
	default:
		break;
	}
	PauseFlag();
	return true;
}

// DOL 0x801bc3b4 [audited] (inlined in READY / GAME / MISS): the first of the players' pads (get_pad1(i), i < nPlayer) that triggers HOME or MINUS
// starts the pause (0x80147c8c)
void cAppG01GalagaGame::PauseCheck()
{
	for (int i = 0; i < KingdomGlobal::GalagaSys->nPlayer(); i++)
		if ((GamePad::Get(i).trig & GamePad::PAD_HOME) != 0 || (GamePad::Get(i).trig & GamePad::PAD_MINUS) != 0)
		{
			m_pTaskPause->Start();
			break;
		}
}

// DOL 0x801bcb7c [audited] (the Execute tail; the boss app's 0x801bae9c is the same): DAT_80479118 = the pause is on (+0x1d4) || pad 0 holds HOME
void cAppG01GalagaGame::PauseFlag()
{
	cG01GalagaSys::s_bPause = m_pTaskPause->IsPause() || (GamePad::Get(0).hold & GamePad::PAD_HOME) != 0;
	if (devtools::G01EditFrozen()) cG01GalagaSys::s_bPause = true;   // devtools: the Galaga editor holds the stage still (not 1:1)
}

}
