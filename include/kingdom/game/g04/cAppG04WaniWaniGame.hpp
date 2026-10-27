#pragma once

#include "common.hpp"
#include "WiiSnd.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/game/cTaskGameBasePause.hpp"

namespace kingdom { class cTaskFade; }

// Gator Panic Remix (g04) game = job 227 (0x0e300000): kingdom::game::g04::cAppG04WaniWaniGame (0x330 bytes, vtable 0x80394114; job init
// FUN_8019a13c, Init 0x80090fb8, Execute 0x8009219c, Draw 0x800931bc (empty), dtor 0x80094480). Members are read through one tiny accessor
// each (tools/dol/accessors.py resolves them):
//   +0x1b8 scene root, +0x280 tortoise, +0x284 BG, +0x288 gator manager, +0x28c collision, +0x290 fade, +0x294 console, +0x298 Pac-Man
//   manager, +0x29c effect, +0x2a0 timer, +0x2a4 camera, +0x2a8 pause, +0x2ac lens flare, +0x2b0 Mii icons (multiplayer), +0x2b4 / +0x2b8
//   fog / light managers (sets 0..5 = BG_SCA, CHA_SCA, CHA_A..D_SCA), +0x2bc.. six sound handles, +0x304 new record, +0x308 the next player
//   count, +0x30c a counter, +0x310 the mode slot, +0x314 cParamGame (CSV_GAME), +0x318 the next job, +0x31c / +0x320 the angry time / count,
//   +0x325 "started from the retry".
// Execute states (+0x310):
//   0 fade in -> 1 (a retry starts at 4)   1 the menu (m04bgm_Select): back -> hub (0x0ef00000), start -> 2, how to play -> the tutorial,
//   players -> the matching (0x0e500000 alone) / the Nunchuk card (0x0e700000)   2 the menu closes -> 5   3 the game: every 15 s a new gator
//   pattern; at ANGREE_TIME seconds left with ANGREE_COUNT hits the angry event (m04bgm_Game2, the zoom camera, CAMERA_EVENT_TIME s, the timer
//   stops); the timer's end -> 6   4 the retry's fade in -> 5   5 READY (m04se_12start, m04se_01tabetyau, READY at 0, GO + m04bgm_Game1 at 120,
//   the game at 180)   6 time up (m04se_15result) -> 7   7 the result (the tortoise turns, the rank board) -> 8 (alone) / 0xb   8 Play again?
//   -> the game job again with sys +0x164 (yes) or not (no)   9 fade out, save the record, the next job   10 / 0xb the multiplayer end.
// NOT PORTED: the Mii icons (+0x2b0) and the Mii listeners (out of scope). The remote speaker plays through WiiSnd::PlayRemote.
namespace kingdom::game::g04
{
class cTaskBG;
class cTaskCamera;
class cTaskCollision;
class cTaskConsTimer;
class cTaskEffect;
class cTaskEnemyManager;
class cTaskGameConsole;
class cTaskPacmanManager;
class cTaskTortoise;

struct cParamGame   // CSV_GAME (0xa0 bytes, ctor 0x80091d60)
{
	s32 nAngryTime{30};         // +0x20 row 0 (seconds left)
	s32 nAngryCount{55};        // +0x24 row 1 (hits)
	s32 nEventTime{2};          // +0x28 row 2 (seconds)
	Vector4 vEventEnemyPos;     // +0x60 row 3
	u32 nNameColor[3][4]{};     // +0x2c / +0x3c / +0x4c rows 4..15, RGBA packed from columns 1..4 (set 0 is the player name colour 0x80094378)
	s32 nNameSize[3]{};         // +0x8c / +0x90 / +0x94 rows 16 / 17 / 18: the name font sizes (menu + score boards / result winners / result others)
	s32 nPlayTime{120};         // +0x70 row 19 (seconds)
	s32 nMultiTime[3]{};        // +0x74 rows 20 / 22 / 24
	s32 nMultiCount[3]{};       // +0x80 rows 21 / 23 / 25
	void Load();
};

// kingdom::game::g04::cTaskPause (0x1f0, vtable 0x80394198; ctor 0x80091af8): the shared pause with COMMON_ANITA_PAUSE / COMMON_ANITA_POINTER,
// menu job 0x0e300000 (0x80094a58)
class cTaskPause : public kingdom::game::cTaskGameBasePause
{
public:
	cTaskPause();
	~cTaskPause() override;

private:
	// the DOL hands the base pause cG04WaniWaniSys +4 (sWaniMatching) in multiplayer; natively the converted sMatchingData built from it
	kingdom::sMatchingData m_Matching;
	kingdom::sMatchingData::InfoData m_Info[4];
};

class cAppG04WaniWaniGame : public kingdom::cAppKingdom
{
public:
	static void JobInit();             // DOL 0x8019a13c (job 227)
	cAppG04WaniWaniGame();
	~cAppG04WaniWaniGame() override;
	void Init();                       // DOL 0x80090fb8
	void Free() override;              // DOL 0x80094480 / 0x80092018
	bool Execute(cJetTask* pParam) override;   // DOL 0x8009219c

	static cAppG04WaniWaniGame* Instance() { return s_pApp; }   // the DOL reaches the app through KingSys +0xa40 / the Execute parameter
	bool IsPause() const { return m_bPause; }   // +0x1b4 (the pause's +0x1d4)
	static bool IsHome();                       // get_pad1(0)->hold & 0x400 (the HBM is open)
	const cParamGame& ParamGame() const { return m_Param; }
	cTaskPacmanManager* pPacmanManager() { return m_pPacmanManager; }
	cTaskEnemyManager* pEnemyManager() { return m_pEnemyManager; }
	uint TimerFrames() const;                   // the timer's +0x1d0
	uint Score() const;                         // DOL 0x801b1a1c (the console's hit count)
	void TortoiseReact(int state);              // DOL 0x801b7c7c(+0x280, state)
	void CameraShake(int mode);                 // DOL 0x8019d724(+0x2a4, mode)

private:
	void BuildEffect();                // DOL 0x80093ae0
	void BuildTortoise();              // DOL 0x80093260
	void BuildEnemyManager();          // DOL 0x800933f4
	void BuildPacmanManager();         // DOL 0x80093624
	void BuildCollision();             // DOL 0x80093920
	void BuildConsole();               // DOL 0x800937bc
	void BuildTimer();                 // DOL 0x800940b8
	void SetAngry();                   // DOL 0x800943cc
	void PauseCheck();

	static cAppG04WaniWaniGame* s_pApp;
	bool m_bPause{};                   // +0x1b4
	cTaskTortoise* m_pTortoise{};      // +0x280
	cTaskBG* m_pTaskBG{};              // +0x284
	cTaskEnemyManager* m_pEnemyManager{};   // +0x288
	cTaskCollision* m_pCollision{};    // +0x28c
	kingdom::cTaskFade* m_pTaskFade{}; // +0x290
	cTaskGameConsole* m_pConsole{};    // +0x294
	cTaskPacmanManager* m_pPacmanManager{};   // +0x298
	cTaskEffect* m_pEffect{};          // +0x29c
	class cTaskEffectRensFlear* m_pFlare{};   // +0x2ac
	cTaskConsTimer* m_pTimer{};        // +0x2a0
	cTaskCamera* m_pTaskCamera{};      // +0x2a4
	cTaskPause* m_pPause{};            // +0x2a8
	WiiSnd::Handle m_SndMenu;          // +0x2bc
	WiiSnd::Handle m_SndGame;          // +0x2c8
	WiiSnd::Handle m_SndStart;         // +0x2d4
	WiiSnd::Handle m_SndEat;           // +0x2ec
	WiiSnd::Handle m_SndResult;        // +0x2f8
	bool m_bNewRecord{};               // +0x304
	int m_nNextPlayers{1};             // +0x308
	uint m_nCounter{};                 // +0x30c
	u32 m_nFlow{};                     // +0x310 (the mode slot)
	cParamGame m_Param;                // +0x314
	u32 m_nNextJob{};                  // +0x318
	int m_nAngryTime{};                // +0x31c
	uint m_nAngryCount{};              // +0x320
	bool m_bFromRetry{};               // +0x325
	uint m_nDbgMode{~0u};
};
}
