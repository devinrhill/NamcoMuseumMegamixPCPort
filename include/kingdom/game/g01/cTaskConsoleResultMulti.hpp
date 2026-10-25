#pragma once

#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g01/cObject.hpp"

// Galaga Remix (g01) stage result, multiplayer: kingdom::game::g01::cTaskConsoleResultMulti (vtable 0x80439b2c, a cTaskGameConsoleBase + cResultAnita
// with +0x224 = 1; ctor 0x800dd06c, Init 0x800dd444 (vtable +0xa0), Start 0x800dfc48, phase machine 0x800de638, Kill 0x800ddb3c (+0x98), IsKillEnd 0x800de52c
// (+0x9c), SumKills 0x800dd3b4 (+0x94)); console manager +0x23c, built by 0x800ebab0 when more than one player plays (priority 0x80000004).
// The co-op team result: title, the TEAM's kills over the stage norma (cG01GalagaSys::pScoreNorma(-1)[players - 1]), the life gauge, a banner, then one
// cTaskPlayerAnita panel per player (hit rate digits + bar, the crown for the player(s) with the most kills).
//   anita 0 frame, 1 title, 2 KILL header, 3 KILL label, 4..6 team kills (layers 2 1 0), 7..9 norma (6 5 4), 0xd "/" (layer 3), 10 LIFE header, 0xb / 0xc
//   LIFE base / icons (ANT_LIFE), 0xe banner. Phases 0 frame + jingle, 1 title / KILL header, 2 kills (roulette 75 frames), 3 life, 4 banner + panels,
//   5 skip to the end state, 6 wait ~2 s, 7 everything goes out.
namespace kingdom::game::g01
{
class cTaskConsolePlayer;
class cTaskConsoleLife;

// kingdom::game::g01::cTaskConsoleResultMulti::cTaskPlayerAnita (0x240 B, vtable 0x80439c30; ctor 0x800db828): one player's panel (anitas 0 bar frame,
// 1..4 hit rate digits, 5 ".", 6 "%", 7 crown, 0xf = only the layout position). Execute 0x800dcd38; Rate 0x800dc0a8, IsRateEnd 0x800dc8f8, Gauge 0x800dc9ec,
// Instant 0x800dcbb0.
class cTaskResultPlayerAnita : public cObject, public kingdom::cTaskAnita
{
public:
	cTaskResultPlayerAnita(cTaskConsolePlayer* player, int nPlayers);   // DOL 0x800db828 (state / step: cObjectBase +0 / +4, 4 = idle)
	bool Execute(cJetTask* pParam) override;    // DOL 0x800dcd38
	void Draw(int nStatus, cJetTask* pParam) override;   // cTaskGameConsoleBase::Draw 0x800eb548
	cTaskConsolePlayer* pPlayer() const { return m_pPlayer; }                     // 0x800dd43c (+0x40)
	bool IsWinner() const { return m_bWinner; }                                    // 0x800dfca8 (+0x3c)
	void Idle() { m_nState = 4; m_nStep = 0; }                                    // 0x800dcb38 (+0x30)
	bool IsIdle() const { return m_nState == 4; }                                 // 0x800dcb4c (+0x34)
	void Appear(bool winner) { m_nState = 0; m_nStep = 0; m_bWinner = winner; }   // 0x800dcb60 (vtable +0x7c)
	bool IsShown() const { return m_nState == 1; }                                // 0x800dcb74 (+0x80)
	void GoOut() { m_nState = 2; m_nStep = 0; }                                   // 0x800dcb88 (+0x84)
	bool IsOut() const { return m_nState == 3; }                                  // 0x800dcb9c (+0x88)
	void Instant(bool winner);                  // DOL 0x800dcbb0 (+0x94): straight to the shown state

private:
	void Rate(int kind);                        // DOL 0x800dc0a8
	bool IsRateEnd();                           // DOL 0x800dc8f8
	bool Gauge();                               // DOL 0x800dc9ec
	float Ratio() const;                        // the hit ratio the bar and digits follow (+0x224 player)
	cTaskConsolePlayer* m_pPlayer{};            // +0x224
	bool m_bWinner{};                           // +0x228
	float m_fTimer{};                           // +0x22c
	float m_fBar{};                             // +0x230
};

class cTaskConsoleResultMulti : public cObject, public kingdom::cTaskAnita
{
public:
	cTaskConsoleResultMulti(cTaskConsolePlayer** players, int nPlayers, cTaskConsoleLife* life);   // DOL 0x800dd06c + Init 0x800dd444
	bool Execute(cJetTask* pParam) override;   // DOL 0x800eb400 (state 4 part) + the anita update
	void Draw(int nStatus, cJetTask* pParam) override;   // cTaskGameConsoleBase::Draw 0x800eb548
	void Start();                              // DOL 0x800dfc48 (state 4, phase 0, SumKills)
	bool IsEnd() const { return m_nState == 5; }   // DOL 0x800eb6e0
	unsigned nWinMask() const { return m_nWinMask; }   // +0x244: the player(s) with the most kills (bit per player)

private:
	bool PhaseExec();                          // DOL 0x800de638
	void SetPhase(int n) { m_nPhase = n; m_nStep = 0; }   // m_nState / m_nStep: cObjectBase +0 / +4 (state 4 result, 5 result done; the step)
	   // DOL 0x800eb63c
	void Kill(int kind);                       // DOL 0x800ddb3c
	bool IsKillEnd();                          // DOL 0x800de52c
	void SumKills();                           // DOL 0x800dd3b4
	int m_nPhase{};                            // +0x21c
	cTaskResultPlayerAnita* m_pPanel[4]{};     // +0x230
	int m_nPlayers{};                          // +0x240
	unsigned m_nWinMask{};                     // +0x244
	int m_nKills{};                            // +0x248 the team's kills
	int m_nNorma{};                            // +0x24c the stage norma for this player count
	int m_nWaitFrame{};                        // +0x250 (an OSGetTick stamp in the DOL)
	cTaskConsoleLife* m_pLife{};               // +0x258
	float m_fTimer{};                          // +0x254
	bool m_bOut{};                             // +0x228 the out phase started
};
}
