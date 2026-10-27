#pragma once

#include "common.hpp"
#include <vector>
#include "kingdom/cTaskAnita.hpp"

// Grobda Remix (g11) 2D console tasks: kingdom::game::g11::cTaskGrobdaConsoleBase (0x1d0 bytes, vtable 0x804419b8, parent cTaskAnita; ctor
// 0x8012d738) and its children, all built by cTaskGrobdaConsoleManager (builder 0x8012f7c8, priorities 0x80000004.. 0x80000009).
// Anita file m11c_game01.anp (UNVERIFIED: the port resolves the name through the archive store). Ported here: Base, ReadyGo, Time, Finish, Score, Gage, HighScore, Combo,
// Icon and Result (multiplayer), Manager. NOT PORTED: the Mii faces (cTaskFaceIcon children of Icon / Result: RFL).
namespace kingdom::game::g11
{
class cTaskGrobdaConsoleBase : public kingdom::cTaskAnita
{
public:
	cTaskGrobdaConsoleBase();                       // DOL 0x8012d738
	bool Execute(cJetTask* pParam) override;        // DOL 0x8012d79c (pause gate; the anita update is cTaskAnita's)

	virtual void SetVisible(uint n, bool b);        // DOL 0x8012d800 (vtable +0x50)
	virtual bool IsVisible(uint n);                 // DOL 0x8012d854 (+0x54)
	virtual bool IsAnimEnd(uint n);                 // DOL 0x8012d880 (+0x58; true when the anita has no data)
	int Mode() const { return m_nMode; }
	bool PlaceLayer(uint anita, uint layer, int pat, bool scale);   // FUN_80014408 (+ the layer's scale, as the Score ctor 0x80131fd4 does)

protected:
	int m_nMode{};                                  // +0x1c8
};

// kingdom::game::g11::cTaskGrobdaConsoleReadyGo (0x1d4 bytes; ctor 0x80130e54, Start 0x8013111c, Execute 0x80130f20): anita anim 0x32 READY
// in, 0x33 hold (60 frames), 0x34 GO (hold 30), 0x35, 0x36 out.
class cTaskGrobdaConsoleReadyGo : public cTaskGrobdaConsoleBase
{
public:
	cTaskGrobdaConsoleReadyGo();                    // DOL 0x80130e54
	bool Execute(cJetTask* pParam) override;        // DOL 0x80130f20
	void Start();                                   // DOL 0x8013111c (vtable +0x0c)
	bool IsEnd() const { return m_nMode == 5; }

private:
	int m_nWait{};                                  // +0x1cc
};

// kingdom::game::g11::cTaskGrobdaConsoleTime (vtable 0x80441fa0; ctor 0x801326b8, Execute 0x801327e0, Start 0x80132c10, End 0x80132c7c): the
// stage timer m:ss:cc, 8 layers of m11c_game01.anp (layers 0 / 1 = frame parts, anim 0x28 + mode; 2..7 = digits, anim 4 * digit + mode, table
// 0x80132b6c). Digit layer positions come from pattern 2 (single player) or 0x16 (multiplayer) via FUN_80014408 (= GetLayerPos).
class cTaskGrobdaConsoleTime : public cTaskGrobdaConsoleBase
{
public:
	cTaskGrobdaConsoleTime();                       // DOL 0x801326b8
	bool Execute(cJetTask* pParam) override;        // DOL 0x801327e0
	void Start();                                   // DOL 0x80132c10 (vtable +0x0c)
	void End();                                     // DOL 0x80132c7c (vtable +0x10)
	void SetTimeMs(int ms) { m_nTimeMs = ms; }      // +0x1cc

private:
	static int AnimNo(uint layer, int mode, int digit);   // DOL 0x80132b6c
	void ShowTime(int mode);                        // DOL 0x8013295c
	int m_nTimeMs{};                                // +0x1cc
};

// kingdom::game::g11::cTaskGrobdaConsoleFinish (0x1d0, vtable 0x80441ae0; ctor 0x8012df78, Execute 0x8012e044, Start 0x8012e1a4, End 0x8012e20c):
// the "FINISH" banner (anim 0x3a in, 0x3b hold for 360 frames (120 multiplayer), 0x3c out).
class cTaskGrobdaConsoleFinish : public cTaskGrobdaConsoleBase
{
public:
	cTaskGrobdaConsoleFinish();                     // DOL 0x8012df78
	bool Execute(cJetTask* pParam) override;        // DOL 0x8012e044
	void Start();                                   // DOL 0x8012e1a4
	void End();                                     // DOL 0x8012e20c
private:
	int m_nWait{};                                  // +0x1cc
};

// kingdom::game::g11::cTaskGrobdaConsoleScore (0x208, vtable 0x80441f08; ctor 0x80131fd4, Execute 0x80132188, Start 0x801322dc, End 0x80132390,
// SetScore 0x8013241c, Flash 0x80132524): 6 digit layers (5 in multiplayer, pattern 14 + player)
class cTaskGrobdaConsoleScore : public cTaskGrobdaConsoleBase
{
public:
	explicit cTaskGrobdaConsoleScore(int player);   // DOL 0x80131fd4
	bool Execute(cJetTask* pParam) override;        // DOL 0x80132188
	void Start();                                   // DOL 0x801322dc
	void End();                                     // DOL 0x80132390
	void SetScore(int score);                       // DOL 0x8013241c
	void Flash();                                   // DOL 0x80132524 (new record: anim 4 * digit + 3)
	int nScore() const { return m_nScore; }         // +0x204
private:
	uint Count() const { return m_bMulti ? 5u : 6u; }
	bool m_bMulti{};                                // +0x1fc
	int m_nPlayer{};                                // +0x200
	int m_nScore{};                                 // +0x204
	int m_nDigit[6]{};                              // +0x1cc (stride 8)
	int m_nState[6]{};                              // +0x1d0
};

// kingdom::game::g11::cTaskGrobdaConsoleHighScore (0x200, vtable 0x80441c10; ctor 0x8012ec48, Execute 0x8012ed50, Start 0x8012ee8c, End 0x8012ef50,
// SetNew 0x8012efd4): the saved best score, 6 digit layers of pattern 1
class cTaskGrobdaConsoleHighScore : public cTaskGrobdaConsoleBase
{
public:
	cTaskGrobdaConsoleHighScore();                  // DOL 0x8012ec48
	bool Execute(cJetTask* pParam) override;        // DOL 0x8012ed50
	void Start();                                   // DOL 0x8012ee8c
	void End();                                     // DOL 0x8012ef50
	void SetNew(int score);                         // DOL 0x8012efd4
	int nHigh() const { return m_nHigh; }           // +0x1fc
private:
	int m_nHigh{};                                  // +0x1fc
	int m_nDigit[6]{};
	int m_nState[6]{};
};

// kingdom::game::g11::cTaskGrobdaConsoleCombo (0x1d4, vtable 0x80441a48; ctor 0x8012d8f8, Execute 0x8012d9f4, Add 0x8012dce4, Set 0x8012dcf0):
// "n COMBO" (layer 0 anim 0x37 / 0x38 / 0x39, tens + ones digits of anim 3 * digit + 0x66 / 0x67 / 0x68), held 180 frames
class cTaskGrobdaConsoleCombo : public cTaskGrobdaConsoleBase
{
public:
	cTaskGrobdaConsoleCombo();                      // DOL 0x8012d8f8
	bool Execute(cJetTask* pParam) override;        // DOL 0x8012d9f4
	void Add() { Set(m_nCombo + 1); }               // DOL 0x8012dce4
	void Set(int n);                                // DOL 0x8012dcf0
private:
	int m_nCombo{};                                 // +0x1cc
	int m_nWait{};                                  // +0x1d0
};

// kingdom::game::g11::cTaskGrobdaConsoleGage (0x1d4, vtable 0x80441b78; ctor 0x8012e358, Execute 0x8012e484, Start 0x8012e728, End 0x8012e818,
// SetGauge 0x8012e8d0, anim table 0x8012e900): the energy gauge, two layers: 0 = the frame, 1 = the bar (its frame = length * gauge)
class cTaskGrobdaConsoleGage : public cTaskGrobdaConsoleBase
{
public:
	explicit cTaskGrobdaConsoleGage(int player);    // DOL 0x8012e358
	bool Execute(cJetTask* pParam) override;        // DOL 0x8012e484
	void Start();                                   // DOL 0x8012e728
	void End();                                     // DOL 0x8012e818
	void SetGauge(float f);                         // DOL 0x8012e8d0 (clamped 0..1)
private:
	int AnimNo(uint layer, int mode) const;         // DOL 0x8012e900
	void ClearHold();                               // the "&= 0xfffdffff; +0x14 = 1.0" pair of every state
	int m_nPlayer{};                                // +0x1cc
	float m_fGauge{1.0f};                           // +0x1d0
};

// kingdom::game::g11::cTaskGrobdaConsoleIcon (0x1d4 bytes, vtable 0x80441ce4 secondary; ctor 0x8012f1b4, Execute 0x8012f498): a player's panel
// of multiplayer, anita anims 0x4f / 0x50 / 0x51 + 3 * player (in / idle / out). Its cTaskFaceIcon child (+0x1d0: the player's Mii at layer 0
// pattern player + 10, scale 0.5) is NOT PORTED (RFL Mii faces).
class cTaskGrobdaConsoleIcon : public cTaskGrobdaConsoleBase
{
public:
	explicit cTaskGrobdaConsoleIcon(int player);    // DOL 0x8012f1b4
	bool Execute(cJetTask* pParam) override;        // DOL 0x8012f498
	void Start();                                   // DOL 0x8012f5d0 (vtable +0x48)
	void End();                                     // DOL 0x8012f644 (vtable +0x4c)
private:
	int m_nPlayer{};                                // +0x1cc
};

// kingdom::game::g11::cTaskGrobdaConsoleResult (0x1e4 bytes; ctor tail 0x80131288, Execute 0x8013147c, Start 0x80131604, Hold 0x80131d7c,
// End 0x80131c7c): the multiplayer result: anita 0 the frame (0x5b in, 0x5c idle, 0x5d out), anita 1 WIN (0x3d / 0x3e / 0x3f) or DRAW when
// every player has the top score (0x40 / 0x41 / 0x42), anita 2.. one per winner (winner * 3 + 0x43 / 0x44 / 0x45, at layer i, pattern
// winners + 0x16). The winners' Mii faces (cTaskFaceIcon +0x1dc, pattern winners + 0x19) are NOT PORTED.
class cTaskGrobdaConsoleResult : public cTaskGrobdaConsoleBase
{
public:
	cTaskGrobdaConsoleResult();                     // DOL 0x80131288
	bool Execute(cJetTask* pParam) override;        // DOL 0x8013147c
	void Start(const int* scores);                  // DOL 0x80131604 (the players' scores; count = the sys player count)
	void End();                                     // DOL 0x80131c7c (vtable +0x4c)
private:
	void Hold();                                    // DOL 0x80131d7c
	bool m_bDraw{};                                 // +0x1cc
	std::vector<int> m_vWinner;                     // +0x1d0 (std::vector: +0x1d4 size, +0x1d8 capacity)
	int m_nWait{};                                  // +0x1e0
};

// kingdom::game::g11::cTaskGrobdaConsoleManager (0x218.., vtable 0x80441d40; builder 0x8012f7c8, Execute 0x80130778, Start 0x80130a5c, End 0x80130b84,
// Finish 0x80130ca8): owns the children. Modes: 1 start (frame anim 0x2c), 2 idle (0x2d), 3 finish (60 frames, then the high score check),
// 4 multiplayer result, 5 ending (anim 0x2e / 0x31), 6 hidden. Multiplayer: Result (+0x1dc) and Icons (+0x1e0..) instead of HighScore and Combo.
class cTaskGrobdaConsoleManager : public cTaskGrobdaConsoleBase
{
public:
	explicit cTaskGrobdaConsoleManager(int nPlayer);   // DOL 0x8012f7c8
	bool Execute(cJetTask* pParam) override;           // DOL 0x80130778
	void Start();                                      // DOL 0x80130a5c (vtable +0x0c)
	void End();                                        // DOL 0x80130b84 (vtable +0x10)
	void Finish();                                     // DOL 0x80130ca8
	void SetTime(int ms) { m_pTime->SetTimeMs(ms < 0 ? 0 : ms); }   // DOL 0x80130cdc -> 0x801327c8
	bool IsReadyDone() const;                          // DOL 0x80130d08: READY / GO shown and FINISH not started
	bool IsEnd() const { return m_nMode > 4; }         // DOL 0x80130d38
	void StartReadyGo();                               // DOL 0x80130c94
	cTaskGrobdaConsoleScore* pScore(int i) { return m_pScore[i]; }
	cTaskGrobdaConsoleGage* pGage(int i) { return m_pGage[i]; }
	cTaskGrobdaConsoleCombo* pCombo() { return m_pCombo; }
	void SetCombo(int n) { if (m_pCombo != nullptr) m_pCombo->Set(n); }   // DOL 0x80130cf4 -> 0x8012dcf0
	void SetGauge(int player, float g) { if (m_pGage[player] != nullptr) m_pGage[player]->SetGauge(g); }   // DOL 0x80130ce4
private:
	bool m_bHighChecked{};                             // +0x1cc
	int m_nFinishWait{};                               // +0x1d0
	cTaskGrobdaConsoleReadyGo* m_pReadyGo{};           // +0x1d4
	cTaskGrobdaConsoleFinish* m_pFinish{};             // +0x1d8
	cTaskGrobdaConsoleResult* m_pResult{};             // +0x1dc (multiplayer)
	cTaskGrobdaConsoleIcon* m_pIcon[4]{};              // +0x1e0 (multiplayer)
	cTaskGrobdaConsoleGage* m_pGage[4]{};              // +0x1f0
	cTaskGrobdaConsoleScore* m_pScore[4]{};            // +0x200
	cTaskGrobdaConsoleHighScore* m_pHigh{};            // +0x210
	cTaskGrobdaConsoleCombo* m_pCombo{};               // +0x214
	cTaskGrobdaConsoleTime* m_pTime{};                 // +0x218
};
}
