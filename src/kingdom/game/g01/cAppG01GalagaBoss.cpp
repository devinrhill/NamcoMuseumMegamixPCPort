#include "platform/devtools.hpp"
#include "kingdom/game/g01/cAppG01GalagaBoss.hpp"

#include "kingdom/game/g01/cTaskConsoleReadyString.hpp"
#include <cstdio>
#include <cstdlib>

#include "KingdomGlobal.hpp"
#include "SoundManager.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/game/cTaskConsRetry.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskBoss.hpp"
#include "kingdom/game/g01/cTaskBossEnemyMan.hpp"
#include "kingdom/game/g01/cTaskBossLife.hpp"
#include "kingdom/game/g01/cTaskBossPacman.hpp"
#include "kingdom/game/g01/cTaskBossScene.hpp"
#include "kingdom/game/g01/cTaskCollisionMan.hpp"
#include "kingdom/game/g01/cTaskConsoleLife.hpp"
#include "kingdom/game/g01/cTaskConsolePlayer.hpp"
#include "kingdom/game/g01/cTaskConsoleResultSingle.hpp"
#include "kingdom/game/g01/cTaskConsoleResultMulti.hpp"
#include "snd/Mixer.hpp"

namespace kingdom::game::g01
{

// 0x801b9960 (new 0x2c0) + 0x800d1638: two course segments (stage 10 "a" and "b"), speed FLOAT_8047cdf0 (1.0)
cTaskScene* cAppG01GalagaBoss::CreateScene()
{
	return new cTaskBossScene(m_nStage, 2, 1.0f);
}

cTaskPacman* cAppG01GalagaBoss::CreatePacman()
{
	return new cTaskBossPacman(m_nStage, m_pTaskScene);
}

cTaskEnemyMan* cAppG01GalagaBoss::CreateEnemyMan()
{
	return new cTaskBossEnemyMan();
}

// DOL 0x801b88cc: after Pac-Man: new 0x670 (0x801ba078) + 0x800cebbc(boss, 2, scene, Pac-Man), connected; new 0x240 (0x801ba1f4) the gauge
void cAppG01GalagaBoss::CreateBoss()
{
	m_pBoss = new cTaskBoss();
	m_pBoss->Setup(m_pTaskScene, m_pTaskPacman);
	ConnectChild(m_pBoss);
	m_pBossLife = new cTaskBossLife(m_pBoss);
	m_pBoss->SetLife(m_pBossLife);
	ConnectChild(m_pBossLife);
}

// DOL 0x800cfb54
bool cAppG01GalagaBoss::IsBossDefeated() const
{
	return m_pBoss != nullptr && m_pBoss->IsDefeated();
}

// DOL 0x801ba59c. States as cAppG01GalagaGame's: 0 start, 1 demo, 2 ready (at once), 4 game, 5 result, 6 miss, 7 goal (boss demos), 8 retry,
// 9 exit
bool cAppG01GalagaBoss::Execute(cJetTask* pParam)
{
	(void)pParam;
	const uint n = m_nFlow;
	cDemoMan* dm = KingdomGlobal::GalagaSys->pDemoMan();
	static int s_nDbgFrame = 0;
	s_nDbgFrame++;
	if (std::getenv("MEGAMIX_G01DBG") && nModeState(n) != m_nDbgMode) { std::fprintf(stderr, "g01 app mode %u -> %u (app frame %d, frame %d)\n", m_nDbgMode, nModeState(n), s_nDbgFrame, devtools::FrameNo()); m_nDbgMode = nModeState(n); }
	switch (nModeState(n))
	{
	case ePROC_START:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
			nModeStep(n) = 1;
			if (!dm->IsEmpty())
			{
				if (!m_bRetry) dm->Start();   // 0x801becd8
				else dm->Finish();            // 0x801bee48
			}
			[[fallthrough]];
		case 1:
			if (m_pTaskFade->IsEnd()) NextMode(n, (!dm->IsEmpty() && !m_bRetry) ? ePROC_DEMO : ePROC_READY);
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
			dm->Reset();
			[[fallthrough]];
		case 1:
			if (dm->IsAllDone() || dm->IsSkip()) NextMode(n, ePROC_READY);
			dm->Tick();
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
		case 2:
			nModeStep(n) = 0;
			dm->Finish();
			break;
		}
		break;
	case ePROC_READY:
		// console ready (+0x78), the life gauge appears (+0x2d4 vtable +0x80 = 0x800d0a58), console on, m01se_01start, then the game at once
		nOldState(n) = nModeState(n);
		if (m_pBossLife != nullptr) m_pBossLife->Appear();
		for (cTaskConsolePlayer* p : m_pPlayer) if (p != nullptr) { p->Appear(); p->SetEnable(true); }
		m_pTaskLife->Appear();
		if (m_pTaskReady != nullptr) m_pTaskReady->Ready();   // 0x800eb664
		snd::Play("m01se_01start");   // DAT_8047cdb8
		NextMode(n, ePROC_GAME);
		nModeStep(n) = 0;
		break;
	case ePROC_GAME:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
			nModeStep(n) = 1;
			m_pTaskScene->GameStart();    // +0x2b8
			m_pTaskPacman->GameStart();   // vtable +0xf4
			if (m_pBoss != nullptr) m_pBoss->StartFight();   // 0x800cfb2c
			m_bBgm = false;
			[[fallthrough]];
		case 1:
			// 0x800eb680 (the console finished its ready): console game state (+0x7c) and the stage BGM (0x801bbe68: world 3, 4th stage =
			// m01bgm_Boss2). Native: once, at the first game frame (UNVERIFIED timing)
			if (!m_bBgm)
			{
				m_bBgm = true;
				SoundManager::get_BGM()->Stop(0.0f);
				SoundManager::get_BGM()->Play(KingdomGlobal::GalagaSys->nStageInWorld() == 4 ? EBGMKind::GALAGA_SND_BGM06_BOSS2 : EBGMKind::GALAGA_SND_BGM04_WORLD3, 1.0f, true);
			}
			if (IsBossDefeated())
			{
				if (m_pBossLife != nullptr) m_pBossLife->Out();   // +0x2d4 vtable +0x84 = 0x800d0a70
				NextMode(n, ePROC_GOAL);
			}
			if (m_pTaskLife->IsMiss()) NextMode(n, ePROC_MISS);
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
		case 2:
			nModeStep(n) = 0;
			for (cTaskConsolePlayer* p : m_pPlayer) if (p != nullptr) p->SetEnable(false);   // 0x800ec2cc(console, 0)
			break;
		}
		break;
	case ePROC_MISS:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
			nModeStep(n) = 1;
			m_pTaskScene->MissStart();
			m_pTaskPacman->MissStart();   // vtable +0xfc
			KingdomGlobal::GalagaSys->pTaskCollisionMan()->Stop();
			KingdomGlobal::GalagaSys->ResetWork();
			[[fallthrough]];
		case 1:
			if (m_pTaskPacman->IsMissEnd()) NextMode(n, ePROC_RETRY);
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
			nModeStep(n) = 1;
			dm->Reset();
			if (!dm->IsEmpty())
			{
				dm->StartGoal();
				SoundManager::get_BGM()->Stop(0.5f);
			}
			else
			{
				// no goal demo: CLEAR, then on through step 1 (DOL LAB_801bad64), whose tail resets the step for CLEAR's step 0
				NextMode(n, ePROC_CLEAR);
			}
			[[fallthrough]];
		case 1:
			if (dm->IsAllGoalDone() || dm->IsSkip()) NextMode(n, ePROC_CLEAR);
			dm->Tick();
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
		case 2:
			nModeStep(n) = 0;
			break;
		}
		break;
	case ePROC_CLEAR:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
			nModeStep(n) = 1;
			m_pTaskLife->StartResult();
			for (cTaskConsolePlayer* p : m_pPlayer) if (p != nullptr) p->StartResult();
			if (m_pTaskResult != nullptr) m_pTaskResult->Start();
			if (m_pTaskResultMulti != nullptr) m_pTaskResultMulti->Start();
			SoundManager::get_BGM()->SetVolume(0.25f, 0.1f);   // 0x801bab1c: 0x80026394(0.25, BGM +0x284, 100): ducked under the jingle
			snd::Play("m01se_15clear");   // DAT_8047cdbc
			[[fallthrough]];
		case 1:
			if ((m_pTaskResult == nullptr || m_pTaskResult->IsEnd()) && (m_pTaskResultMulti == nullptr || m_pTaskResultMulti->IsEnd()))
			{
				// the staff roll (job 183) when not everything was open yet, else the menu
				m_nNextJob = KingdomGlobal::GalagaSys->bAllOpen() ? job::GALAGA_MENU : job::GALAGA_STAFFROLL;
				NextMode(n, ePROC_EXIT);
			}
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
		case 2:
			nModeStep(n) = 0;
			SaveRecord();   // vtable slot 13 (0x801ba4cc)
			break;
		}
		break;
	case ePROC_RETRY:
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0:
			nModeStep(n) = 1;
			m_pTaskRetry->Start();
			[[fallthrough]];
		case 1:
			if (m_pTaskRetry->IsYes()) { m_nNextJob = job::GALAGA_GAME; NextMode(n, ePROC_EXIT); }
			else if (m_pTaskRetry->IsNo()) { m_nNextJob = job::GALAGA_GAMEOVER; NextMode(n, ePROC_EXIT); }
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
			[[fallthrough]];
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
			// the staff roll is entered without force (0x0b700000), the others with
			if (m_pTaskFade->IsEnd()) Kingdom_SetJobNo(m_nNextJob, m_nNextJob != job::GALAGA_STAFFROLL, false);
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
	PauseFlag();   // 0x801bae98: the same tail as the game app's (the boss app never starts the pause itself: only HOME held sets the flag)
	return true;
}

// DOL 0x801ba4cc: stage 10's result goes into stage record 9 (alone, console vtable 32) or multiplayer record 9 (console vtable 34); then the last record
// is opened, the "new stage" flag set and the save written
void cAppG01GalagaBoss::SaveRecord()
{
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	if (!m_bMulti) StoreResult(sys->pStageRecord(9));
	else StoreMulti(sys->pMultiRecord(9));
	sStageRecord* rec = sys->pStageRecord(9);
	if (rec->bOpen != 1) rec->bOpen = 1;   // +0x24
	sys->bNewStageApear(true);
	sys->SaveData();
}

}
