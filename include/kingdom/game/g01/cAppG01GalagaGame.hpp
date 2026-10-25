#pragma once

#include "common.hpp"
#include "kingdom/cAppKingdom.hpp"

namespace kingdom { class cTaskFade; }
namespace kingdom::game { class cTaskConsRetry; }

// Galaga Remix (g01) stage = job 179 (0x0b300000): kingdom::game::g01::cAppG01GalagaGame (DOL vtable 0x8044aa68, 0x2d0 bytes; stage 10 uses
// the derived cAppG01GalagaBoss, 0x2d8 bytes, cAppG01GalagaBoss.hpp). Ported from main.dol with the old hand-reversed tree (reversing/dol/g01_ghidra/
// cAppG01GalagaGame.cpp) as the guide.
namespace kingdom::game::g01
{
class cTaskConsoleLife;
class cTaskConsoleReadyString;
class cTaskPause;
struct sStageRecord;
struct sMultiRecord;
class cTaskConsoleResultSingle;
class cTaskConsoleResultMulti;
class cTaskConsolePlayer;
class cTaskEnemyMan;
class cTaskPacman;
class cTaskScene;
class cTaskStage;

class cAppG01GalagaGame : public kingdom::cAppKingdom
{
public:
	enum EProc
	{
		ePROC_START = 0,   // wait for the fade-in
		ePROC_DEMO = 1,    // opening demo (cDemoMan)
		ePROC_READY = 2,   // start camera, "ready"
		ePROC_GAME = 4,
		ePROC_CLEAR = 5,   // result console
		ePROC_MISS = 6,
		ePROC_GOAL = 7,
		ePROC_RETRY = 8,
		ePROC_EXIT = 9,
	};

	static void JobInit();             // DOL 0x800b0ed4 (job 179)
	cAppG01GalagaGame();               // DOL 0x801bb1a0
	void Init();                       // DOL 0x801bb258 (vtable slot 14)
	void Free() override;              // DOL 0x801bbdb8 Term (from the dtor 0x800b1158)
	virtual void SaveRecord();         // DOL 0x801bbf5c (vtable slot 13)
	bool Execute(cJetTask* pParam) override;   // DOL 0x801bc0fc

protected:
	// the scene / Pac-Man the Init builds (cAppG01GalagaBoss::Init 0x801b88cc builds cTaskBossScene / cTaskBossPacman instead)
	virtual cTaskScene* CreateScene();
	virtual cTaskPacman* CreatePacman();
	virtual cTaskEnemyMan* CreateEnemyMan();   // cTaskEnemyMan (0x801bb9fc); the boss app's cTaskBossEnemyMan (0x801b9404)
	virtual void CreateBoss() {}               // the boss app: cTaskBoss + cTaskBossLife after Pac-Man (0x801b88cc)
	void StoreResult(sStageRecord* rec);                 // console manager vtable 32 (0x800ec2f8) on the player 0
	void StoreMulti(sMultiRecord* rec);                  // console manager vtable 34 (0x800ec610)
	void PauseCheck();                 // the pad loop of READY / GAME / MISS (0x801bc3b4): HOME / MINUS triggered -> the pause starts
	void PauseFlag();                  // the tail of both apps' Execute: DAT_80479118 (cG01GalagaSys::IsPause)
	int m_nStage{};                    // +0x29c
	int m_nWorld{};                    // +0x2a0
	bool m_bMulti{};                   // +0x2a4 (DOL "IsSingle" = is multiplayer, see cG01GalagaSys::IsMulti)
	uint m_nNextJob{};                 // +0x2a8
	bool m_bRetry{};                   // +0x2ac
	kingdom::cTaskFade* m_pTaskFade{}; // +0x280 (cAppKingdom area)
	cTaskScene* m_pTaskScene{};        // +0x2b8
	cTaskStage* m_pTaskStage{};        // +0x2c0
	cTaskPacman* m_pTaskPacman{};      // +0x2bc
	cTaskEnemyMan* m_pTaskEnemyMan{};  // +0x2b4
	cTaskConsolePlayer* m_pPlayer[4]{};   // inside cTaskGameConsoleMan (+0x2c4) in the DOL
	cTaskConsoleLife* m_pTaskLife{};     // cTaskGameConsoleMan +0x228
	cTaskConsoleReadyString* m_pTaskReady{};   // cTaskGameConsoleMan: the first console (0x800d7908)
	cTaskConsoleResultSingle* m_pTaskResult{};   // cTaskGameConsoleMan +0x23c (alone)
	cTaskConsoleResultMulti* m_pTaskResultMulti{};   // cTaskGameConsoleMan +0x23c (more than one player)
	int m_nJingle{-1};                 // the jingle handle +0x290 (a voice natively)
	kingdom::game::cTaskConsRetry* m_pTaskRetry{};   // +0x2c8 (cTaskRetry)
	cTaskPause* m_pTaskPause{};        // +0x2cc
	u32 m_nFlow{};
	int m_nDbgFrame{};   // MEGAMIX_G01DBG print interval
	uint m_nDbgMode{};
};
}
