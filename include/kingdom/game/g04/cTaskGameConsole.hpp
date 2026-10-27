#pragma once

#include <functional>

#include "common.hpp"
#include "WiiSnd.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g04/cScoreMessageListener.hpp"
#include "kingdom/game/menu/cTaskConsMenuTutorialBase.hpp"

namespace kingdom { class cTaskCursor; }
namespace kingdom::game { class cTaskConsRetry; }

// Gator Panic Remix (g04) the 2D console: kingdom::game::g04::cTaskGameConsole (0x20c bytes, vtable 0x8044a424; builder 0x800937bc at app
// +0x294, priority 0x10002; build 0x801b0808, mode 0x801b1678, Execute 0x801b1434) and the anita tasks it owns (the multiplayer menu /
// score consoles further down; NOT PORTED: the Mii icons, out of scope):
//   +0x1f0 a menu pointer per player (kingdom::cTaskMenuCursor, COMMON_ANITA_POINTER anim 2i / 2i+1 with A B Z, enabled in modes 2 / 3 / 6)
//   +0x200 the tutorial (cTaskConsMenuTutorial, COMMON_ANITA_TUTORIAL + TUTORIAL01_ANITA pages, MENU_TEXT_TUTORIAL)
//   +0x1d0 the menu (cTaskConsMenu, SELECT_ANITA: item 0 back, 1 how to play, 2 start, 3 players)
//   +0x1d4 the result (cTaskConsResult, RESULT_ANITA, CSV_RESULT)   +0x1d8 the score (cTaskConsScore, GAME_ANITA)
//   +0x1dc READY / GO (cTaskConsReadly, GAME_ANITA 99 / 100 / 0x65)   +0x1ec the retry (kingdom::game::cTaskConsRetry, COMMON_ANITA_RETRY)
// The timer (cTaskConsTimer) is the app's own child (+0x2a0).
// Modes: 0 nothing, 1 the score (start), 2 the tutorial, 3 the menu + high score, 4 the score (game), 5 the result, 6 the result gone,
// 7 the menu closes, 8 the score's end.
namespace kingdom::game::g04
{
class cTaskScoreNum;
class cTaskConsComboScore;

// kingdom::game::g04::cTaskScoreNum (0x1e0, vtable 0x8044a6d8; ctor 0x801b6788): a 3-digit number of GAME_ANITA (digit d = anims 3d, 3d+1,
// 3d+2 of table 0x8037d660: +0 shown, +4 appear, +8 change)
class cTaskScoreNum : public kingdom::cTaskAnita
{
public:
	cTaskScoreNum();                               // DOL 0x801b6788
	bool Execute(cJetTask* pParam) override;       // DOL 0x801b6c04
	void Reset();                                  // DOL 0x801b688c
	void SetMode(int mode);                        // DOL 0x801b691c
	void SetNumber(uint n);                        // DOL 0x801b713c
	void ShowNumber(uint n);                       // DOL 0x801b72e4
	void SetPos(uint digit, const Vector4& v);     // DOL 0x801b7458
	void SetBlink(bool b, int which);              // DOL 0x801b74a0
	uint nNumber() const { return m_nNum; }

private:
	void Step();                                   // DOL 0x801b6c9c
	void StepBlink();                              // DOL 0x801b6ebc
	bool m_bBlink{};                               // +0x1c8
	int m_nMode{};                                 // +0x1cc
	uint m_nNum{};                                 // +0x1d0
	int m_nDigit[3]{1, 1, 1};                      // +0x1d4
};

// kingdom::game::g04::cTaskConsComboScore (0x1d4, vtable 0x80449790; ctor 0x8019dc8c): "n COMBO", GAME_ANITA anim 0x84 / 0x83 / 0x85, the two
// digits from 0x8037c4c0
class cTaskConsComboScore : public kingdom::cTaskAnita
{
public:
	cTaskConsComboScore();                         // DOL 0x8019dc8c
	bool Execute(cJetTask* pParam) override;       // DOL 0x8019df84
	void SetMode(int mode);                        // DOL 0x8019e060
	void First(uint n);                            // DOL 0x8019e378
	void Set(uint n);                              // DOL 0x8019e444
	void ScoreNumMode3();                          // cTaskScoreNum::SetMode(3) run on this object (cTaskConsScore mode 7)

private:
	int m_nMode{};                                 // +0x1c8
	uint m_nTimer{};                               // +0x1cc
	uint m_nNum{};                                 // +0x1d0
};

// kingdom::game::g04::cTaskConsScore (0x1fc, vtable 0x80449c90; ctor 0x801a6160): the single player's score board; the Pac-Man's
// cScoreMessageListener (+0x1c8). Score = bonus (+0x1d0) + combo (+0x1e4) + hits (+0x1dc) - damage (+0x1d4), at least 0.
class cTaskConsScore : public kingdom::cTaskAnita, public sScoreListener
{
public:
	cTaskConsScore();                              // DOL 0x801a6160
	bool Execute(cJetTask* pParam) override;       // DOL 0x801a669c
	void Reset();                                  // DOL 0x801a6548
	void SetMode(int mode);                        // DOL 0x801a671c
	void SetHigh(uint n) { m_nHigh = n; }          // DOL 0x801a6d1c
	uint Score() const;                            // DOL 0x801a6d50
	uint nHits() const { return (uint)m_nHits; }

	void SetBonus(cTaskPacmanBase*, int n) override { m_nBonus = n; }   // DOL 0x801a6d40
	void SetHits(cTaskPacmanBase*, int n) override;                     // DOL 0x801a6bc4
	void SetDamage(cTaskPacmanBase*, int n) override;                   // DOL 0x801a6c38
	void Combo(cTaskPacmanBase*, int combo, uint chain) override;       // DOL 0x801a6cdc
	bool CanPay(cTaskPacmanBase*, int n) override;                      // DOL 0x801a6cb8

private:
	int m_nMode{};                                 // +0x1cc
	int m_nBonus{};                                // +0x1d0
	int m_nDamage{};                               // +0x1d4
	int m_nShown{};                                // +0x1d8
	int m_nHits{};                                 // +0x1dc
	uint m_nHigh{};                                // +0x1e0
	int m_nCombo{};                                // +0x1e4
	cTaskScoreNum* m_pNum[4]{};                    // +0x1e8 total, +0x1ec hits, +0x1f0 damage, +0x1f4 high score
	cTaskConsComboScore* m_pCombo{};               // +0x1f8
};

// kingdom::game::g04::cTaskConsMultiScore (0x2d8, vtable 0x80449aa0, ctor inlined in the console's 0x801b0808 + init 0x801a2e7c): the multiplayer score boards, one per
// player (GAME_ANITA anims 0x75 + 3p .., score digits = patterns 6 + p) with a crown (anita 4 + p, anims 0x66 / 0x67 / 0x68) over the leader(s). The Pac-Mans' score
// listener in multiplayer (+0x1c8). Per player: damage +0x1d0, hits +0x1e0, combo +0x1f0 (the best combo bonus), crown state +0x200, number +0x210;
// score = hits + combo - damage. Names: Draw 0x801a36a0 / 0x801a3cd8 (font writer +0x220). NOT PORTED: Mii icons.
class cTaskConsMultiScore : public kingdom::cTaskAnita, public sScoreListener
{
public:
	cTaskConsMultiScore();                         // DOL 0x801a2e7c
	bool Execute(cJetTask* pParam) override;       // DOL 0x801a35cc
	void Draw(int nStatus, cJetTask* pParam) override;   // DOL 0x801a36a0
	void Reset();                                  // DOL 0x801a34d0
	void SetMode(int mode);                        // DOL 0x801a393c
	int Score(uint p) const { return m_nHits[p] + m_nCombo[p] - m_nDamage[p]; }
	uint Hits() const;                             // DOL 0x801a3fc0 (the sum of +0x1e0, the hit counts the gators' level follows)
	void Leaders(uint& mask, int& count) const;    // DOL 0x801a3ef4: the players with the best score (all of them while tied)
	void Finish();                                 // DOL 0x801a4078: the winner's number blinks (a single winner only), mode 4

	void SetBonus(cTaskPacmanBase* p, int n) override;                   // listener +0x0c: blr (0x801a4274); +0x10 = Refresh 0x801a37a4, which the Pac-Man never calls
	void SetHits(cTaskPacmanBase* p, int n) override;                    // DOL 0x801a38dc
	void SetDamage(cTaskPacmanBase* p, int n) override;                  // DOL 0x801a390c
	void Combo(cTaskPacmanBase* p, int combo, uint chain) override;      // DOL 0x801a3fa0
	bool CanPay(cTaskPacmanBase* p, int n) override;                     // DOL 0x801a3cb0

private:
	void Refresh(uint p);                          // DOL 0x801a37a4
	void DrawNames();                              // DOL 0x801a3cd8
	int m_nMode{};                                 // +0x1cc
	int m_nDamage[4]{};                            // +0x1d0
	int m_nHits[4]{};                              // +0x1e0
	int m_nCombo[4]{};                             // +0x1f0
	int m_nState[4]{};                             // +0x200 crown: 0 appearing, 1 shown, 2 leaving
	cTaskScoreNum* m_pNum[4]{};                    // +0x210
};

// kingdom::game::g04::cTaskConsReadly (0x1cc, vtable 0x80449b60; ctor 0x801a42b8): READY (99, then 100) and GO (0x65)
class cTaskConsReadly : public kingdom::cTaskAnita
{
public:
	cTaskConsReadly();                             // DOL 0x801a42b8
	bool Execute(cJetTask* pParam) override;       // DOL 0x801a46e4
	void Reset();                                  // DOL 0x801a4440
	void SetMode(int mode);                        // DOL 0x801a4544
	void Ready();                                  // DOL 0x801a461c

private:
	int m_nMode{};                                 // +0x1c8
};

// kingdom::game::g04::cTaskConsTimer (0x1f4, vtable 0x80449d50; builder 0x800940b8 at app +0x2a0, ctor 0x801a6e90): the play time m:ss,
// GAME_ANITA digits (0x8037cc98 + colour * 0x84 + digit * 0xc: +0 out, +4 shown, +8 in; the colon +0x78), placed on the layers of pattern
// 0x13; red (colour 1) from 10 s (m04se_13speed_up), m04se_14time_up at 0. Counts down in mode 2 while +0x1c8 (run) is set.
class cTaskConsTimer : public kingdom::cTaskAnita
{
public:
	cTaskConsTimer();                              // DOL 0x801a6e90
	bool Execute(cJetTask* pParam) override;       // DOL 0x801a7234
	void Reset();                                  // DOL 0x801a70ac
	void SetMode(int mode);                        // DOL 0x801a7918
	void SetTime(uint frames);                     // DOL 0x801a7bec
	int nFrames() const { return m_nFrames; }      // +0x1d0
	bool& bRun() { return m_bRun; }                // +0x1c8

private:
	int Anim(int digit, int which) const;
	bool m_bRun{true};                             // +0x1c8
	int m_nMode{};                                 // +0x1cc
	int m_nFrames{0xb4};                           // +0x1d0
	int m_nStep{};                                 // +0x1d4
	uint m_nDigit[3]{};                            // +0x1d8
	int m_nChange[3]{1, 0, 0};                     // +0x1e4 (+0x1e8 tens, +0x1ec minutes change this second)
	int m_nColor{};                                // +0x1f0
};

// kingdom::game::g04::cTaskConsMenu (0x1e8, vtable 0x80449810; ctor 0x8019e5a0): the single player menu, SELECT_ANITA x 4.
class cTaskConsMenu : public kingdom::cTaskAnita
{
public:
	cTaskConsMenu();                               // DOL 0x8019e5a0
	bool Execute(cJetTask* pParam) override;       // DOL 0x8019e924
	void Reset();                                  // DOL 0x8019e880 (interface +0xc)
	void SetMode(int mode);                        // DOL 0x8019ea3c (interface +0x24)
	void Close();                                  // DOL 0x8019f41c
	bool IsClosed();                               // DOL 0x8019f50c (interface +0x20)
	bool IsSel(int i) const { return m_bDecided && m_nSel == i; }   // DOL 0x8019f37c / 0x8019f3cc / 0x8019f3a4 / 0x8019f3f4
	bool bDecided() const { return m_bDecided; }
	std::function<void()> onDecide;                // the console's cMenuMessageListener (+0x1dc) +0xc

private:
	void Input();                                  // DOL 0x8019ec48
	bool Pointer(int pad);                         // DOL 0x8019ee04
	bool Keys(int pad);                            // DOL 0x8019f134
	int m_nMode{};                                 // +0x1d0
	int m_nSel{2};                                 // +0x1d4
	int m_nOldSel{2};                              // +0x1cc
	bool m_bDecided{};                             // +0x1d8
};

// kingdom::game::g04::cTaskConsMenuTutorial (0x2f0, vtable 0x80394358; ctor 0x80094870): the base with COMMON_ANITA_TUTORIAL and
// data 0x8036d548 (2 pages of TUTORIAL01_ANITA, text MENU_TEXT_TUTORIAL)
class cTaskConsMenuTutorial : public kingdom::game::menu::cTaskConsMenuTutorialBase
{
public:
	explicit cTaskConsMenuTutorial(kingdom::cTaskCursor* pCursor);
	bool IsDeleteEnd() override;                   // DOL 0x800948cc (listener slot +0x24: state == 6)
	void SetListener(void* p) { m_pListener = p; } // DOL 0x80094a48 (listener slot +0x28; nothing in the DOL calls it)

private:
	void* m_pListener{};                           // +0x2e8 (ctor clears it)
};

// kingdom::game::g04::cTaskConsMenuMultiTutorial (0x2f0, vtable 0x80394298; ctor 0x80094688): the same class with data 0x8036d508
// (3 pages of TUTORIAL01_ANITA: anims 0 / 2 / 1, text rows 4 / 5 / 6); built by the console when the player count != 1
class cTaskConsMenuMultiTutorial : public kingdom::game::menu::cTaskConsMenuTutorialBase
{
public:
	explicit cTaskConsMenuMultiTutorial(kingdom::cTaskCursor* pCursor);
	bool IsDeleteEnd() override;                   // DOL 0x800946e4 (listener slot +0x24: state == 6)
	void SetListener(void* p) { m_pListener = p; } // DOL 0x80094860 (listener slot +0x28; nothing in the DOL calls it)

private:
	void* m_pListener{};                           // +0x2e8 (ctor clears it)
};

// kingdom::game::g04::cTaskConsResult (0x2b0, vtable 0x80449be0; ctor 0x801a488c): RESULT_ANITA x 16 + the rank (CSV_RESULT thresholds)
class cTaskConsResult : public kingdom::cTaskAnita
{
public:
	cTaskConsResult();                             // DOL 0x801a488c
	bool Execute(cJetTask* pParam) override;       // DOL 0x801a5194
	void Draw(int nStatus, cJetTask* pParam) override;   // DOL 0x801a5674
	void Reset();                                  // DOL 0x801a4d7c
	void SetMode(int mode);                        // DOL 0x801a5778
	void SetScore(uint score);                     // DOL 0x801a609c
	void SetWinners(uint mask, int count);         // DOL 0x801a6000: sys +0x160 = mask; one winner -> its index (+0x1cc), else 4 (a tie: the red gator)
	int Rank(uint score) const;                    // DOL 0x801a60fc
	int nRank() const { return m_nRank; }
	std::function<void()> onDone;                  // the console's listener (+0x1d0) +0xc = 0x801b1a7c

private:
	void DrawNames();                              // DOL 0x801a5c68
	void DrawName(int pat, int player, bool tie);  // DOL 0x801a5e4c
	int m_nMode{};                                 // +0x1c8
	int m_nRank{};                                 // +0x1cc
	int m_nWinCount{};                             // +0x1d8
	uint m_nWinMask{};                             // +0x1dc
	s32 m_nBorder[5]{};                            // cParamResult +0x20
	float m_fTimer{};                              // +0x294
	WiiSnd::Handle m_Voice;                        // +0x298
	WiiSnd::Handle m_Hiscore;                      // +0x2a4
};

// kingdom::game::g04::cTaskConsMultiMenuTime (0x1d0, vtable 0x80449a20; ctor 0x801a219c): the play time picker, SELECT02_ANITA x 4
// (0..2 the digits seconds / tens / minutes, 3 the colon). Times 60 / 180 / 300 s (DAT_8037c8b0), the index lives in sys +0x000.
class cTaskConsMultiMenuTime : public kingdom::cTaskAnita
{
public:
	cTaskConsMultiMenuTime();                      // DOL 0x801a219c
	bool Execute(cJetTask* pParam) override;       // DOL 0x801a271c
	void Reset();                                  // DOL 0x801a24a4
	void SetMode(int mode);                        // DOL 0x801a27c0
	void Left();                                   // DOL 0x801a2bd0
	void Right();                                  // DOL 0x801a2d14
	int Time() const;                              // DOL 0x801a2e5c (seconds)

private:
	int m_nMode{};                                 // +0x1c8
	int m_nIndex{1};                               // +0x1cc
};

// kingdom::game::g04::cTaskConsMultiMenu (0x2a8, vtable 0x80449958; ctor 0x801a00d0): the multiplayer menu. SELECT02_ANITA x 15:
// 0..3 player panels, 4..8 the items (focus 0 back, 1 how to play, 2 play time, 3 start, 4 players), 9 / 10 the time arrows, 11..14 player marks.
// Draw 0x801a070c prints the player names (0x801a1ddc, font writer +0x1f0) in modes 2 / 3.
class cTaskConsMultiMenu : public kingdom::cTaskAnita
{
public:
	cTaskConsMultiMenu();                          // DOL 0x801a00d0
	bool Execute(cJetTask* pParam) override;       // DOL 0x801a0658
	void Draw(int nStatus, cJetTask* pParam) override;   // DOL 0x801a070c
	void Reset();                                  // DOL 0x801a0480 (interface +0xc)
	void SetMode(int mode);                        // DOL 0x801a0810 (interface +0x24)
	bool IsClosed() const { return true; }         // DOL 0x801a214c (interface +0x20)
	// interface +0x10 / +0x14 / +0x18 / +0x1c (0x801a1d3c / 1d64 / 1d8c / 1db4): the single menu's IsSel(0 / 3 / 1 / 2)
	bool IsSel(int i) const;
	int MultiTime() const { return m_pTime->Time(); }   // DOL 0x801a1fec
	std::function<void()> onDecide;                // the console's listener (+0x1ec) +0xc

private:
	bool Pointer(int pad);                         // DOL 0x801a0fe4
	bool Keys(int pad);                            // DOL 0x801a197c
	void Input();                                  // DOL 0x801a0ce4
	void DrawNames();                              // DOL 0x801a1ddc
	void Arrow(bool right);
	void Focus(int focus, int arrowLeft, int arrowRight);
	int m_nMode{};                                 // +0x1d4
	int m_nFocus{};                                // +0x1d8
	bool m_bDecided{};                             // +0x1e4
	cTaskConsMultiMenuTime* m_pTime{};             // +0x2a4
};

class cTaskGameConsole : public kingdom::cTaskKingdom
{
public:
	cTaskGameConsole();                            // DOL 0x801b07f0 + 0x801b0808
	bool Execute(cJetTask* pParam) override;       // DOL 0x801b1434
	void SetMode(int mode);                        // DOL 0x801b1678
	void Restart();                                // DOL 0x801b13b4
	uint Hits() const;                             // DOL 0x801b1a1c (the score the gators' level follows: the hit count)
	bool bMenuDecided() const { return m_bMenuDecided; }   // DOL 0x801b1a68
	void ResultDone() { SetMode(6); m_bResultDone = true; }   // DOL 0x801b1a7c
	int MultiTime() const;                         // DOL 0x801b1ab4 (alone: 180)
	// the menu interface (single cTaskConsMenu or multiplayer cTaskConsMultiMenu)
	bool MenuIsSel(int i) const;
	void MenuSetMode(int mode);
	int ResultRank() const;                        // DOL 0x801b1afc
	bool MenuClosed();                             // DOL 0x801b1c0c
	void StartGame();                              // DOL 0x801b1c20
	bool bResultDone() const { return m_bResultDone; }
	cTaskConsMenu* pMenu() { return m_pMenu; }
	cTaskConsScore* pScore() { return m_pScore; }
	sScoreListener* pListener();                   // the Pac-Mans' listener: the single board, or the multiplayer one (+0x1e8)
	cTaskConsReadly* pReadly() { return m_pReadly; }
	cTaskConsResult* pResult() { return m_pResult; }
	kingdom::game::cTaskConsRetry* pRetry() { return m_pRetry; }

private:
	int m_nMode{};                                 // +0x1bc
	cTaskConsMenu* m_pMenu{};                      // +0x1d0 (single player)
	cTaskConsMultiMenu* m_pMultiMenu{};            // +0x1d0 (multiplayer)
	cTaskConsResult* m_pResult{};                  // +0x1d4
	cTaskConsScore* m_pScore{};                    // +0x1d8
	cTaskConsMultiScore* m_pMultiScore{};          // +0x1e8 (more than one player)
	cTaskConsReadly* m_pReadly{};                  // +0x1dc
	kingdom::game::cTaskConsRetry* m_pRetry{};     // +0x1ec
	kingdom::cTaskCursor* m_pCursor[4]{};          // +0x1f0
	kingdom::game::menu::cTaskConsMenuTutorialBase* m_pTutorial{};   // +0x200: cTaskConsMenuTutorial (1 player) / cTaskConsMenuMultiTutorial
	bool m_bMenuDecided{};                         // +0x208
	bool m_bResultDone{};                          // +0x209
};
}
