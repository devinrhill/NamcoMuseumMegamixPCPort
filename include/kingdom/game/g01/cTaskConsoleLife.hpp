#pragma once

#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g01/cObject.hpp"

// Galaga Remix (g01) life gauge: kingdom::game::g01::cTaskConsoleLife (vtable 0x804396b8, a cTaskGameConsoleBase under the console manager;
// cTaskGameConsoleMan +0x228, the "FUN_801ba060" object the app and Pac-Man use). One ANT_LIFE (0xb7610d93) anita, priority 0x80000007.
// Life +0x224 = stage work +0xc - 1 (0x800d4d38; the work starts at 7, so 0..6 = 1..7 Pac-Man icons). Anim = table 0x80371eb0[life] + kind
// (0x800d4b8c: wait +0, heal -1, damage +1, damage end +2, out +3); appear 0x31, dead 0x33.
namespace kingdom::game::g01
{
class cTaskConsoleLife : public cObject, public kingdom::cTaskAnita
{
public:
	cTaskConsoleLife();                        // DOL 0x800d4b5c + 0x800d4c34 + 0x800d4d38
	bool Execute(cJetTask* pParam) override;   // the console base Execute -> 0x800d4ea8 (states 1 heal / 2 damage) and the appear 0x800d4d9c
	void Draw(int nStatus, cJetTask* pParam) override;   // cTaskGameConsoleBase::Draw 0x800eb548: only while active (+8) and shown (+9)
	void Appear() { m_bAppear = true; m_nAppearStep = 0; }   // console vtable slot 1 (READY)
	bool Damage();                             // DOL 0x800eb6b0 + state 2 (+0x21c = 2) when idle
	bool Heal();                               // DOL 0x800eb6b0 + state 1 (+0x21c = 1) when idle (Pac-Man ate a cookie)
	bool IsMiss() const { return m_bMiss; }    // +0x22c
	int nLife() const { return m_nLife; }
	void StartResult();                        // DOL 0x800d54a4: vtable +0x8c (0x800d54e4, the life bonus) + 0x800eb6c4 (state 4)
	int nBonus() const { return m_nBonus; }    // +0x228 (the result adds it to the score)

private:
	int Anm(int kind) const;                   // DOL 0x800d4b8c
	void AppearExec();                         // DOL 0x800d4d9c
	void StateExec();                          // DOL 0x800d4ea8
	int m_nState2{};                           // +0x21c 0 idle, 1 heal, 2 damage
	int m_nStep2{};                            // +0x220
	int m_nLife{6};                            // +0x224
	int m_nBonus{};                            // +0x228 (0 from 0x800d4b5c; (life + 1) * 1000 at the result)
	bool m_bMiss{};                            // +0x22c
	bool m_bResult{};                          // console state 4 (result) not yet handled
	bool m_bAppear{};
	int m_nAppearStep{};
};
}
