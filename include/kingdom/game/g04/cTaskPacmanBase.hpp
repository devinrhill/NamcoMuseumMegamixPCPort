#pragma once

#include "common.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g04/cAnimeBase.hpp"

// Gator Panic Remix (g04) a player's Pac-Man: kingdom::game::g04::cTaskPacmanBase (0x730 bytes, vtable 0x8044a5d0, cTaskModel + cAnimeBase at
// +0x1cc; ctor 0x801b2298, Init 0x801b23b8, Execute 0x801b294c). Four are built by cTaskPacmanManager (0x801b6310); the player count shows
// 1..4 (0x801b6550). Pac-Man floats at CSV_PACMAN's INIT_POS bobbing (WAIT_SHAKE). The Nunchuk stick picks the hole (0x801b50e0: up A, left B,
// right D, down / centre C; dead zone INPUT_EX_PAD_RADIUS) and a swing of the remote (0x801b5258, the SDK swing demo: a high-passed
// acceleration of at least INPUT_PLUS_RANGE whose direction has |y| >= 0.4, while the pitch does not rise by 10 degrees) makes him lunge at
// it (state 3, FAIL_TIME frames along a curve to 6 units in front of the hole). The collision (cTaskCollision) tests him against each gator
// while lunging (3) or combo-jumping (5): a hit (0x801b48d0) counts, hurts the gator and, with a second swing at another hole during the
// lunge (COMBO_INPUT_RANGE), chains a combo jump (state 5) to it. A miss splashes into the water (m04se_11mizu).
// States (+0x1f0, 0x801b4328): 0 hidden, 1 idle (no input), 2 input, 3 lunge, 4 back home (RETURN_TIME), 5 combo jump, 6 / 7 the result
// (single), 8 bob, 9 / 10 the multiplayer result walk, 0xb back home after the result.
// NOT PORTED: the Mii listener (+0x1e4, multiplayer only; Mii is out of scope). The 3D sound actor (+0x1dc) is placed every frame but no cue plays through it.
namespace kingdom::game::g04
{
class cTaskBG;
class cTaskEnemy;
class cTaskEnemyManager;
class cTaskPacmanManager;
class cTaskCursor;

struct sScoreListener;   // cScoreMessageListener (the console's score: +0x70c)

struct cParamPacman   // CSV_PACMAN (cParamPacman 0x1b0 bytes, parse 0x8019ca4c)
{
	Vector4 vInitPos[4];       // +0x20 rows 9..12
	float fFailTime{};         // +0x60 row 0
	float fReturnTime{};       // +0x64 row 1
	float fBezierDiv{};        // +0x7c row 6
	float fComboInput{};       // +0x80 row 4
	float fJumpHeight{};       // +0x84 row 5
	float fExPadRadius{};      // +0x88 row 45
	float fMinusRange{};       // +0x8c row 3
	float fPlusRange{};        // +0x90 row 2
	float fComboMoveFrame{};   // +0x94 row 7
	Vector4 vFailOffset[4];    // +0xa0 rows 13..16 (player 1's A..D)
	float fWaitShake{};        // +0x120 row 8
	float fWinDistance{};      // +0x124 row 46
	float fLoseDistance{};     // +0x128 row 47
	Vector4 vWinOffset[4];     // +0x130 rows 48..51
	Vector4 vLoseOffset[4];    // +0x170 rows 52..55
	void Load();
};

class cTaskPacmanBase : public kingdom::cTaskModel
{
public:
	cTaskPacmanBase(uint nPlayer, cTaskBG* pBG, cTaskPacmanManager* pMan);   // DOL 0x801b2298
	~cTaskPacmanBase() override;
	void Init();                                      // DOL 0x801b23b8
	void Restart();                                   // DOL 0x801b26f8
	bool Execute(cJetTask* pParam) override;          // DOL 0x801b294c
	void SetState(int state);                         // DOL 0x801b4328
	bool HitEnemy(cTaskEnemy* e);                     // DOL 0x801b48d0 (the collision callback)
	void Damage();                                    // DOL 0x801b5e24 (a gator bit: the penalty)
	void SetWin(int win);                             // DOL 0x801b6120
	void ForgetPlace(uint place) { if (m_nLastPlace == (int)place) m_nLastPlace = -1; }   // DOL 0x801b61b0
	void SetEnemyManager(cTaskEnemyManager* p) { m_pEnemyMan = p; }   // DOL 0x80091d28 (+0x1d4)
	void SetScoreListener(sScoreListener* p) { m_pScore = p; }       // DOL 0x80091d40 (+0x70c)
	int nState() const { return m_nState; }
	int nHits() const { return m_nHits; }
	uint nPlayer() const { return m_nPlayer; }
	const cParamPacman& Param() const { return m_Param; }

private:
	void Input(cJetTask* pApp);                       // DOL 0x801b2ee4 (states 1 / 2)
	void Lunge();                                     // DOL 0x801b3458 (state 3)
	void ComboJump();                                 // DOL 0x801b3b34 (state 5)
	void Return();                                    // DOL 0x801b3fc8 (state 4)
	void MultiResult();                               // DOL 0x801b40f8 (state 9)
	int Target(int pad) const;                        // DOL 0x801b50e0
	bool Swing(float minSpeed, float decay);          // DOL 0x801b5258
	void PushAcc(int pad);                            // the acc / pitch history step every input state does
	void ResetAcc(int pad);
	bool HitTest(const float pac[4], kingdom::cKingdomModel* gator);   // DOL 0x801b5778 (+ 0x801b5644)
	void LineFx();                                    // DOL 0x801b5cd8
	void ClearTrail();                                // inlined in 0x801b2ee4 / 0x801b294c: multiplayer, detach every trail particle
	static void Bezier(float t, float div, float height, float out[3], const float from[3], const float to[3]);   // DOL 0x801b5ed0
	int Pad() const;
	float Bob() const;

	cTaskBG* m_pBG{};                       // +0x1d0
	cTaskEnemyManager* m_pEnemyMan{};       // +0x1d4
	cTaskPacmanManager* m_pMan{};           // +0x1d8
	cAnimeBase m_Anime;                     // +0x1cc
	int m_nOldState{};                      // +0x1ec
	int m_nState{};                         // +0x1f0
	int m_nChain{};                         // +0x1f4
	uint m_nPlayer{};                       // +0x1f8
	int m_nTarget{};                        // +0x1fc
	Vector4 m_vDest;                        // +0x200
	Vector4 m_vFrom;                        // +0x210
	Vector4 m_vHome;                        // +0x220
	Vector4 m_v230;                         // +0x230
	float m_fTimer{};                       // +0x240
	float m_f244{};                         // +0x244
	float m_vAcc[6][4]{};                   // +0x250 (pad +0x30 history)
	float m_fPitch[6]{};                    // +0x2b0
	int m_nAcc{};                           // +0x2c8
	cParamPacman m_Param;                   // +0x2cc (a plain member natively: Term DOL 0x801b28f4 only deletes it)
	int m_nComboReq{};                      // +0x2d0
	int m_nComboTarget{};                   // +0x2d4
	int m_nLastPlace{-1};                   // +0x2d8
	int m_nHits{};                          // +0x2dc
	int m_nRedHits{};                       // +0x2e0
	int m_nDamage{};                        // +0x2e4
	int m_nAimGator{};                      // +0x2e8
	kingdom::cKingdomPtkHnd m_MissFx;       // +0x2f0
	kingdom::cKingdomPtkHnd m_LineFx;       // +0x340
	Vector4 m_vTrail[8];                    // +0x3a0 multiplayer trail positions (newest first, shifted every frame)
	kingdom::cKingdomPtkHnd m_TrailFx[8];   // +0x420 (0x50 each) the trail particles, slot player + 0x17 (PTK_EFFECT24..27_LIGHT01)
	bool m_bTrail[8]{};                     // +0x6a0 trail handle in use
	kingdom::cKingdomPtkHnd m_RingFx;       // +0x6b0 multiplayer light ring, slot player + 0x12 (PTK_EFFECT19..22_LIGHT01)
	cTaskCursor* m_pCursor{};               // +0x700
	float m_fSwingMax{1.0f};                // +0x704
	int m_nCombo{};                         // +0x708
	sScoreListener* m_pScore{};             // +0x70c
	float m_fBob{};                         // +0x710
	int m_nComboFlag{};                     // +0x714
	int m_nWin{};                           // +0x718
	bool m_bMouth{};                        // +0x71c
};

// Gator Panic Remix (g04) kingdom::game::g04::cTaskPacmanManager (0x1cc bytes, vtable 0x8044a678; builder 0x80093624 at app +0x298)
class cTaskPacmanManager : public kingdom::cTaskKingdom
{
public:
	explicit cTaskPacmanManager(cTaskBG* pBG);        // DOL 0x801b6310
	bool Execute(cJetTask* pParam) override;          // DOL 0x801b63fc
	void Restart();                                   // DOL 0x801b63b0
	cTaskPacmanBase* pPacman(uint i) { return m_pPac[i]; }   // DOL 0x801b6474
	void Damage(uint player);                         // DOL 0x801b6484
	void SetAllState(int state);                      // DOL 0x801b64e0
	void SetPlayers(uint n);                          // DOL 0x801b6550
	void ForgetPlace(uint place);                     // DOL 0x801b65f8
	void Result(uint winner);                         // DOL 0x801b6668
	uint nPlayer() const { return m_nPlayer; }

private:
	cTaskPacmanBase* m_pPac[4]{};                     // +0x1b4
	uint m_nPlayer{};                                 // +0x1c4
	cTaskBG* m_pBG{};                                 // +0x1c8
};
}
