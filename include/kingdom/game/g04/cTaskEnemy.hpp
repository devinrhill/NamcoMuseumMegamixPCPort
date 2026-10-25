#pragma once

#include <random>
#include <vector>

#include "common.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g04/cAnimeBase.hpp"

// Gator Panic Remix (g04) a gator: kingdom::game::g04::cTaskEnemy (0x630 bytes, vtable 0x8044a148, a cTaskModel + cAnimeBase at +0x1cc).
// Four of them, one per hole (place 0..3 = A..D), built by cTaskEnemyManager (0x801ae84c). Init (vtable +0x3c, 0x801a91e0):
//   model 0 = BIN_MODEL_WANI (place 0: BIN_MODEL_WANI_2) with ANIME_G_W1_TPA / ANIME_G_W1_CLA / ANIME_EYE_W2_TSA, model 1 = the red gator
//   BIN_MODEL_WANI_RED (place 0: _RED_2, hidden) with ANIME_EYE_W6_TSA; light set / fog = place + 2 (CHA_A_SCA ..); CSV_ENEMY_COMMON
//   (cParamEnemyCommon, 0x48 bytes); a cKingdomRand seeded 0x20061206; place 0 also gets two cTaskEnemyShadow (BIN_MODEL_WANI_SH, cTaskEnemyShadow.hpp).
// The manager feeds each gator a command list (CSV_ENEMYnn, see cTaskEnemyManager): a command sets the timing of one appearance (wait, move
// out, wait before / after the bite, return, total), the kind (W1..W5 texture set), red / red HP and the colour (multiplayer).
// States (+0x1d0, set by 0x801aafd4, run by Execute 0x801a992c):
//   0 hidden      1 initial          2 waits (+0x228), then moves out   3 waits for the sync flag   4 waits for the total (+0x218), then 5
//   5 next command (0x801ad250)       6 TIME_SYNC reached (+0x25d)      7 moving out (0x801a9d60)    8 going back (0x801aa254)
//   9 out, waits "before" (+0x234), then bites (0xb) or not (10)      10 waits "after" (+0x238), then goes back (8)
//   0xb bites (0x801aa548: hurts the player(s), camera shake)         0xc hit, backs off (0x801aa718)
//   0xd command list done   0xe SET_RAND: waits for the others of its group   0xf skipped (not enough time left)   0x10 result
//   0x11 the angry event: comes out (0x801aac00)   0x12 angry, stays out   0x13 game over (place C shown at CSV_GAME's event position)
//   0x17 the event ends: goes back (0x801ae380)
// NOT PORTED: the multiplayer colour rules beyond the tables.
namespace kingdom::game::g04
{
class cTaskEnemyManager;

struct sEnemyPattern   // cParamEnemyPattern row block "PATTERN,n" (0x20 bytes, 0x8019be50)
{
	s32 nId;
	float fWait, fMove, fAttack, fReturn, fAttackBeforeWait, fAttackAfterWait, fAllFrame;
};
struct sEnemyCommand   // 0x1c bytes (0x8019c3a8): 0 SET_PATTERN, 1 TIME_SYNC, 2 SET_RAND
{
	s32 nType;
	const sEnemyPattern* pPattern;
	s32 nKind;    // the gator type (W1..W5 texture set)
	u32 nMask;    // the places of a SET_RAND group (bit 0 = A)
	s32 nRed;     // red gator
	s32 nRedHP;
	s32 nColor;   // multiplayer: the player colour
};

class cTaskEnemy : public kingdom::cTaskModel
{
public:
	cTaskEnemy(cTaskEnemyManager* pMan, uint nPlace);   // the manager's 0x801ae84c new 0x630 + ctor
	~cTaskEnemy() override;
	void Init();                                        // DOL 0x801a91e0
	void Term() override;                               // DOL 0x801a9810
	bool Execute(cJetTask* pParam) override;            // DOL 0x801a992c
	void Draw(int nStatus, cJetTask* pParam) override;  // DOL 0x801a9d5c (empty: the scene root draws the models)

	void SetState(int state);                           // DOL 0x801aafd4
	void Reset();                                       // DOL 0x801a96fc
	void SetHomePos(const Vector4& v);                  // DOL 0x801ad238 (+0x1f0)
	void ClearCommands() { m_cmd.clear(); }             // DOL 0x801ad66c
	void AddCommand(const sEnemyCommand* c) { m_cmd.push_back(c); }   // DOL 0x801ad378
	void Rewind() { m_nCmd = 0; }                       // DOL 0x801ad67c
	void NextCommand();                                 // DOL 0x801ad250
	void SetupCommand(const sEnemyCommand* c);          // DOL 0x801ad688
	void SetLook(int kind, int color);                  // DOL 0x801ad840
	void SetResultLook(int kind);                       // DOL 0x801adf08
	void MouthPos(float out[3]);                        // DOL 0x801adcb0 (chest + "?" + mouth node, turned by the model's Y)
	void HitBlink(int frames, uint player);             // DOL 0x801ae024
	void HitEffect(uint player);                        // DOL 0x801ae1f0
	bool IsHittableOut(float range) const;              // DOL 0x801ade58 (moving out, past 0.8 / 0.9 of the move)
	bool IsBeforeBiteEnd(float range) const;            // DOL 0x801aded8
	bool IsBackingOff() const { return m_nState == 0xc; }   // DOL 0x801ae36c
	bool IsAfterBite(float range) const;                // DOL 0x801ae714

	int nState() const { return m_nState; }
	uint nPlace() const { return m_nPlace; }
	int nModel() const { return m_nModel; }
	int nColor() const { return m_nColor; }
	bool IsRed() const { return m_nRed != 0; }
	int& nRedHP() { return m_nRedHP; }
	bool& bSynced() { return m_bSynced; }
	int& nSync() { return m_nSync; }
	float fMove() const { return m_fMove; }
	float fReturn() const { return m_fReturn; }
	float fBefore() const { return m_fBefore; }
	float fAfter() const { return m_fAfter; }
	kingdom::cKingdomModel* pCurModel() { return pModel((uint)m_nModel); }
	kingdom::cKingdomModel* pRedSelModel() { return pModel((uint)m_nRed); }
	// gator A's shadow of the current model (+0x270 + 4 * +0x1dc); the DOL tests place 0 before every call
	class cTaskEnemyShadow* Shadow() { return m_nPlace == 0 ? m_pShadow[m_nModel & 1] : nullptr; }

	struct sParamCommon   // cParamEnemyCommon (CSV_ENEMY_COMMON)
	{
		float fMoveDistance{};   // +0x20 row 0
		float fAngle[4]{};       // +0x24 rows 1..4 (A..D)
		float fReturnFrame{};    // +0x34 row 6
		float fAdvanceFrame{};   // +0x38 row 5
		float fClrAdvance{};     // +0x3c row 7
		float fClrReturn{};      // +0x40 row 8
		float fDamageSpeed{};    // +0x44 row 9
	};

private:
	void Move(cJetTask* pApp);          // DOL 0x801a9d60 (state 7)
	void Back();                        // DOL 0x801aa254 (state 8)
	void Bite();                        // DOL 0x801aa548 (state 0xb)
	void BackOff();                     // DOL 0x801aa718 (state 0xc)
	void AngryOut();                    // DOL 0x801aac00 (state 0x11)
	void EventBack();                   // DOL 0x801ae380 (state 0x17)
	void FootFx(float t, const float pos[3]);   // DOL 0x801ae4a4
	void FootFxAt(int k, const float pos[3], bool hip = false);
	void ColorClip();                   // the colour clip of the look (single: 0x80449ffc by kind; red: ANIME_OA_R_A1_CLA)
	void BodyClr(bool rate1);           // 0x8037cff8 [colour * 5 + kind] / red ANIME_OA_R_CLA in slots 0 and 1
	void SwapChr(const char* next);     // slot 1's clip / rate / frame into slot 0, `next` into slot 1, weights 1 / 0
	void FrontPos();                    // +0x200 = +0x1f0 + MOVE_DISTANCE along the model's Y direction

	cTaskEnemyManager* m_pMan{};        // +0x278
	cAnimeBase m_Anime;                 // +0x1cc
	sParamCommon m_Param;               // +0x24c
	std::mt19937 m_Rand;                // +0x27c (cKingdomRand, seed 0x20061206; UNVERIFIED: std::mt19937 = the DOL's MT19937 genrand 0x8014146c)
	int m_nState{1};                    // +0x1d0
	int m_nOldState{};                  // +0x1d4
	int m_nRedHP{};                     // +0x1d8
	int m_nModel{};                     // +0x1dc
	Vector4 m_vHome;                    // +0x1f0
	Vector4 m_vFront;                   // +0x200
	int m_nColor{};                     // +0x210
	int m_nKind{};                      // +0x214
	float m_fAllFrame{};                // +0x218
	float m_fFrame{};                   // +0x21c
	float m_fAttackProb{};              // +0x220
	float m_fTimer{};                   // +0x224
	float m_fWait{};                    // +0x228
	float m_fMove{};                    // +0x22c
	float m_fReturn{-0.05f};            // +0x230
	float m_fBefore{};                  // +0x234
	float m_fAfter{};                   // +0x238
	int m_nAttack{};                    // +0x23c
	int m_nRed{};                       // +0x240
	int m_nSync{};                      // +0x244
	int m_nClip{};                      // +0x248 (index into ANIME_W1_A01..A05 of the clip in CHR slot 1)
	uint m_nPlace{};                    // +0x250
	class cTaskEnemyShadow* m_pShadow[2]{};   // +0x270 (place 0 only)
	bool m_b254{};                      // +0x254
	int m_nEyeTimer{};                  // +0x258
	bool m_bEye{};                      // +0x25c
	bool m_bSynced{};                   // +0x25d
	std::vector<const sEnemyCommand*> m_cmd;   // +0x260
	std::size_t m_nCmd{};               // +0x26c
	kingdom::cKingdomPtkHnd m_HitFx[5]; // +0x280
	kingdom::cKingdomPtkHnd m_FootFx[6];   // +0x410
	bool m_bFoot[6]{};                  // +0x5f0
	Vector4 m_vHide;                    // +0x600 (BG node hide_X)
	Vector4 m_vShow;                    // +0x610 (BG node show_X)
	bool m_b620{};                      // +0x620
	float m_f624{};                     // +0x624
	friend class cTaskEnemyManager;
};
}
