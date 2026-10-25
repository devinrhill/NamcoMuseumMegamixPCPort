#pragma once

#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g01/cObject.hpp"

// Galaga Remix (g01) stage result, single player: kingdom::game::g01::cTaskConsoleResultSingle (vtable 0x80439d24, a cTaskGameConsoleBase
// + cResultAnita (+0x224 = 0 single, 1 in cTaskConsoleResultMulti 0x800dd06c); console manager +0x23c, built by 0x800ebab0 when alone).
// 28 anitas (ANT_RESULT 0xd2031f29, 0x17 / 0x18 ANT_LIFE 0xb7610d93), priority 0x80000004. The app's CLEAR state starts it (console manager
// vtable +0x80 = 0x800eb914 -> 0x800dfcb0: console state 4, phase 0); the console base Execute 0x800eb400 calls the phase machine 0x800da4a0
// every frame in state 4 and moves to state 5 (result done, 0x800eb6e0) when it returns 1.
//   anita 0 frame, 1 title, 2 / 3 KILL header, 4..6 kills (layers 2 1 0), 7..9 enemies of the stage (6 5 4), 0x19 "/" (layer 3), 0x11 HIT RATE
//   header, 0x12..0x15 rate digits (layers 11 9 8 7), 0x1a rate "." (10), 0x1b "%" (12), 0x16 life header, 0x17 / 0x18 life icons (ANT_LIFE
//   layer 0 pattern 1), 10 SCORE header, 0xb..0x10 score digits (layers 18..13).
namespace kingdom::game::g01
{
class cTaskConsolePlayer;
class cTaskConsoleLife;

class cTaskConsoleResultSingle : public cObject, public kingdom::cTaskAnita
{
public:
	cTaskConsoleResultSingle(cTaskConsolePlayer* player, cTaskConsoleLife* life);   // DOL 0x800d7c40 + Init 0x800d7f78
	bool Execute(cJetTask* pParam) override;   // DOL 0x800eb400 (state 4 part) + the anita update
	void Draw(int nStatus, cJetTask* pParam) override;   // cTaskGameConsoleBase::Draw 0x800eb548: only while active (+8) and shown (+9)
	void Start();                              // DOL 0x800dfcb0 (0x800eb6c4: console state 4; phase 0)
	bool IsEnd() const { return m_nState == 5; }   // DOL 0x800eb6e0

private:
	bool PhaseExec();                          // DOL 0x800da4a0
	void SetPhase(int n) { m_nPhase = n; m_nStep = 0; }   // DOL 0x800eb63c
	void Headers(int kind);                    // DOL 0x800d8908
	void Score(int kind);                      // DOL 0x800d8e74
	void Rate(int kind);                       // DOL 0x800d914c
	void Kill(int kind);                       // DOL 0x800d999c
	bool IsKillEnd();                          // DOL 0x800da394
	int m_nState{};                            // +0x0 console state (4 result, 5 result done)
	int m_nPhase{};                            // +0x21c
	int m_nStep{};                             // +0x220
	int m_nType{};                             // +0x224 cResultAnita: 0 single
	cTaskConsolePlayer* m_pPlayer{};           // +0x230
	float m_fTimer{};                          // +0x238
	int m_nScoreShown{};                       // +0x23c
	int m_nScoreTotal{};                       // +0x240
	int m_nMaxKill{};                          // +0x244 ScoreNorma +0x00 (nMaxKill1: the enemies of the stage)
	cTaskConsoleLife* m_pLife{};               // +0x248
	int m_nWaitFrame{};                        // +0x24c (an OSGetTick stamp in the DOL)
	int m_nDbg{}, m_nDbgFrame{};
};
}
