#pragma once

#include <random>

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"

namespace kingdom { class cAppKingdom; class cTaskModel; }
namespace kingdom::game::g02 { class cParamGame; }

// Rally-X Remix (g02) the in-game 2D console: kingdom::game::g02::cTaskConsole (0x230 bytes, vtable 0x80441260, parent kingdom::cTaskKingdom;
// built by the app at +0x2a8 (0x80124468 = the inlined constructor + Init 0x8006093c), dtor 0x801275e4). Execute / Draw are cTaskKingdom's:
// the console only owns its parts and answers the app's queries. Ported from main.dol (no C# exists for Rally-X).
//
// Init 0x8006093c (priority 0x80000000): +0x1b4 the app, +0x1b8 a cKingdomRand (0x9cc bytes, seed 0x20061206, 0x80060eb8; the mud splats'
// random order), +0x1bc multiplayer (sys IsMulti), +0x1c4 the player count (sys +0x0c). Single player (ported) builds, in this order:
//   +0x1c8 cTaskConsRadar (0x32c, ANITA_RADAR)      +0x1d0 cTaskConsInfo (0x1e0, score / flags / rest / icon)   +0x1f8 cTaskConsFuel (0x1e8)
//   +0x1cc cTaskConsSignal (0x1d4, the start lamp)   +0x1d4 cTaskConsTime (0x1d4, the play time)                +0x1d8 cTaskConsStage (0x1d0)
//   +0x1dc cTaskConsMessage (0x1d4, CSV_FLAG)        +0x208 cTaskConsMud (0x3bc, ANITA_MUD, player 0)          +0x1e0 cTaskConsResult (0x1d8)
//   +0x1ec cTaskConsLose (0x1d8)                     +0x1f0 cTaskConsBang (0x1d0)                              +0x1f4 cTaskConsWarning (0x1d4)
// NOT PORTED (multiplayer, console +0x1bc = 1): per player cTaskConsFuel / cTaskConsMud / cTaskConsPBMultiNO (0x80064afc, +0x220), the
// radar, cTaskConsInfoMulti (+0x218), Signal, Time, cTaskConsWinner (0x8006546c, +0x1e4), cTaskConsDraw (0x8006579c, +0x1e8) and, for 3
// players, cTaskConsPBMultiBG (+0x21c).
//
// The parts read the app's members (console +0x1b4) and the cars' members directly in the DOL. The port gets them through sAppState (set by
// the app) and the hooks below (set by whoever owns the car / gimmick / save classes); a missing hook gives the documented fallback.
namespace kingdom::game::g02
{
class cTaskConsRadar;
class cTaskConsInfo;
class cTaskConsFuel;
class cTaskConsSignal;
class cTaskConsTime;
class cTaskConsStage;
class cTaskConsMessage;
class cTaskConsMud;
class cTaskConsResult;
class cTaskConsLose;
class cTaskConsBang;
class cTaskConsWarning;
class cTaskConsInfoMulti;
class cTaskConsWinner;
class cTaskConsDraw;
class cTaskConsPBMultiNO;
class cTaskConsPBMultiBG;

// What the console parts read from a car (kingdom::game::g02::cTaskUnit, a cTaskModel; DOL offsets of the unit)
struct sConsCar
{
	u32 nKind{};       // vtable +0x44: unit +0x1cc | 0x10 for the player (0x8007ae2c), | 0x20 for an enemy (0x8006e514)
	float fX{};        // +0x200 (position x)
	float fZ{};        // +0x208 (position z)
	float fWait{};     // +0x1d4 (0 = on the road; the start / respawn wait counts down)
	int nDir{};        // +0x22c (grid direction 0..3)
	int nCrash{};      // +0x260 (non-zero: crashed)
	float fNitro{};    // +0x2a4 (> 0: nitro)
	float fFuel{};     // +0x2ac (0..100)
	float fTimer2bc{}; // +0x2bc (> 0: 0x80088458 true; the fuel gauge plays anim 0xf6 instead of 0xf5)
	float fTimer2c0{}; // +0x2c0 (> 0: 0x80088498 true; the radar / info icon of that state)
	float fMud{};      // +0x2c4 (> 0: in mud, 0x80088664; the remaining mud time 0x8008867c)
	int n2c8{};        // +0x2c8 (== 1: 0x800884c8 true; the radar / info icon of that state)
	int nSmoke{};      // +0x2cc (smoke screens left, 0..5)
	int nFlags{};      // +0x2d0 (flags taken)
	int nNitroState{}; // cTaskUnit::nNitroState 0x80088a40 (0 nitro, 1 normal, 2 brake)
};

class cTaskConsole : public kingdom::cTaskKingdom
{
public:
	// the app's members the parts read (DOL: console +0x1b4 -> app); unset pointers read as 0 / false
	struct sAppState
	{
		const bool* pStageEnd{};   // app +0x284 (stage over: the parts freeze)
		const int* pState{};       // app +0x2e4 (game state: 1 start, 2 / 3 signal, 4 play, 7 clear, 10 lose ...)
		const int* pStep{};        // app +0x2e8 (step of the state)
		const int* pScore{};       // app +0x300
		const float* pPlayTime{};  // app +0x318 (milliseconds, shown m:ss.cc)
		const float* pCount{};     // app +0x31c (frames since the start signal began; 120 / 180 / 240 / 300 / 360 = the lamps)
		const int* pRest{};        // app +0x32c (one digit shown by cTaskConsInfo anita 0x10; UNVERIFIED: the cars left)
		const cParamGame* pParam{};   // app +0x280 (CSV_GAME): cTaskConsResult reads +0x38 (CLEAR_FLAG_WAIT), cTaskConsLose +0x44 (LOSE_FLAG_WAIT)
	};

	// readers for members owned by other classes (DOL offsets in sConsCar / the comments); null = fallback
	// Car / gimmick models are read directly where the DOL reads them through cTaskModel::pModel (0x8009a430): the radar icon's rotation =
	// pModel(0) +0x14 (vRotate.y), a flag's position = pModel(0) vTrans, "the flag is shown" = model 0 or 1 visible (+0x44).
	struct sHooks
	{
		bool (*CarState)(kingdom::cTaskModel* car, sConsCar& out){};          // fallback: all zero (the parts show an idle HUD)
		bool (*GimmickRespawn)(kingdom::cTaskModel* gimmick){};               // gimmick +0x1dc byte (0x800b1d6c); fallback false
		int (*SysPlayerNum)(){};                                               // sys +0x0c (0x800c5c68); fallback 1
		int (*SysViewMode)(int player){};                                      // sys +0x14 + player * 0x14 (0x800c5b88); fallback 2 (sys default)
		int (*StageHighScore)(int stage){};                                    // save record 0x804844b4 + stage * 0x14 (0x800c62a8); fallback 0
	};
	static sHooks Hooks;

	explicit cTaskConsole(kingdom::cAppKingdom* pApp);         // DOL 0x80124468 + 0x8006093c
	virtual ~cTaskConsole();                                   // DOL 0x801275e4 (+ 0x80065d04)

	void SetAppState(const sAppState& s) { m_App = s; s_pGameState = s.pState; }
	static const int* s_pGameState;   // the game app's +0x2e4 for PlaySE (KingSys +0xa40 +0x2e4 in the DOL); cleared by the app's Free

	// ---- called by the app (DOL callers in cAppG02RallyXGame ctor 0x801236bc / Execute 0x80125b1c / cTaskMap 0x80077e48, 0x800780f4) ----
	void Regist(kingdom::cTaskModel* pCar, int nColour, bool bPlayer);   // DOL 0x80065db0: every car once, player first (colour 6 Pac, 5 red)
	void RegistFlag(kingdom::cTaskModel* pGimmick, int nType);           // DOL 0x800662c8: a flag gimmick (type 11 .. 33) for the radar
	bool IsSignalGo() const;                                   // DOL 0x80066554: the start lamp counts (signal mode > 1; true without one)
	bool IsResultEnd() const;                                  // DOL 0x80066584: cTaskConsResult mode == 3
	bool IsLoseShown() const;                                  // DOL 0x800665fc: cTaskConsLose mode == 2
	bool IsLoseEnd() const;                                    // DOL 0x80066614: cTaskConsLose mode == 7
	bool IsWinShown() const;                                   // DOL 0x8006659c: cTaskConsWinner mode == 2
	bool IsWinEnd() const;                                     // DOL 0x800665b4: cTaskConsWinner mode == 4
	bool IsDrawShown() const;                                  // DOL 0x800665cc: cTaskConsDraw mode == 2
	bool IsDrawEnd() const;                                    // DOL 0x800665e4: cTaskConsDraw mode == 4
	void HitMark(int nPlayer);                                 // DOL 0x800666e8 (the multiplayer car hit: cRallyUnitHost::UnitMultiHit)
	bool IsMulti() const { return m_bMulti; }                  // +0x1bc
	bool ShowBang();                                           // DOL 0x8006662c: cTaskConsBang anita 0 anim 0x118, shown
	bool HideBang();                                           // DOL 0x8006669c
	void HideBang2();                                          // DOL 0x800667c4 (the same, no result)

	// ---- for the parts ----
	kingdom::cAppKingdom* pApp() const { return m_pApp; }
	bool IsFrozen() const;                                     // app paused (+0x1b4) || stage end (+0x284) || HOME held
	bool IsPaused() const;                                     // app +0x1b4 only (cTaskConsRadar's test)
	int nState() const { return m_App.pState ? *m_App.pState : 0; }
	int nStep() const { return m_App.pStep ? *m_App.pStep : 0; }
	int nScore() const { return m_App.pScore ? *m_App.pScore : 0; }
	float fPlayTime() const { return m_App.pPlayTime ? *m_App.pPlayTime : 0.0f; }
	float fCount() const { return m_App.pCount ? *m_App.pCount : 0.0f; }
	int nRest() const { return m_App.pRest ? *m_App.pRest : 0; }
	float fClearFlagWait() const;                              // app +0x280 -> +0x38 (0 without a parameter block)
	float fLoseFlagWait() const;                               // app +0x280 -> +0x44
	float fWinnerFlagWait() const;                             // app +0x280 -> +0x3c (CSV_GAME WINNER_FLAG_WAIT)
	float fDrawFlagWait() const;                               // app +0x280 -> +0x40
	const cParamGame* pParam() const { return m_App.pParam; }  // app +0x280 (the font colours of the players)
	static void PlaySE(const char* cue);                       // cRallyXSndHnd + 0x80129d78: play (volume 0.2 in game state 6) and detach
	int nPlayer() const { return m_nPlayer; }                  // +0x1c4
	int nFlagTotal() const { return m_nFlagTotal; }            // +0x1c0 (flags registered, at most 20)
	u32 Rand() { return (u32)m_Rand(); }                       // +0x1b8 cKingdomRand genrand (0x8014146c, standard MT19937)
	cTaskConsWarning* pWarning() const { return m_pWarning; }

	static bool CarState(kingdom::cTaskModel* car, sConsCar& out);   // Hooks.CarState or zeros
	static bool GimmickRespawn(kingdom::cTaskModel* g);
	static int SysPlayerNum();
	static int SysViewMode(int player);
	static int StageHighScore(int stage);

private:
	kingdom::cAppKingdom* m_pApp{};          // +0x1b4
	std::mt19937 m_Rand{0x20061206u};        // +0x1b8 (cKingdomRand, 0x80060eb8)
	bool m_bMulti{};                         // +0x1bc
	int m_nFlagTotal{};                      // +0x1c0
	int m_nPlayer{1};                        // +0x1c4
	cTaskConsRadar* m_pRadar{};              // +0x1c8
	cTaskConsSignal* m_pSignal{};            // +0x1cc
	cTaskConsInfo* m_pInfo{};                // +0x1d0
	cTaskConsTime* m_pTime{};                // +0x1d4
	cTaskConsStage* m_pStage{};              // +0x1d8
	cTaskConsMessage* m_pMessage{};          // +0x1dc
	cTaskConsResult* m_pResult{};            // +0x1e0
	cTaskConsLose* m_pLose{};                // +0x1ec
	cTaskConsBang* m_pBang{};                // +0x1f0
	cTaskConsWarning* m_pWarning{};          // +0x1f4
	cTaskConsFuel* m_pFuel[4]{};             // +0x1f8 (one per player; single player: [0])
	cTaskConsMud* m_pMud[4]{};               // +0x208
	cTaskConsWinner* m_pWinner{};            // +0x1e4 (multiplayer)
	cTaskConsDraw* m_pDraw{};                // +0x1e8 (multiplayer)
	cTaskConsInfoMulti* m_pInfoMulti{};      // +0x218 (multiplayer)
	cTaskConsPBMultiBG* m_pBG{};             // +0x21c (multiplayer, 3 players)
	cTaskConsPBMultiNO* m_pNO[4]{};          // +0x220 (multiplayer)
	sAppState m_App;
};
}
