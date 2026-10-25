#pragma once

#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g01/cObject.hpp"

// Galaga Remix (g01) the last boss's life gauge: kingdom::game::g01::cTaskBossLife (0x240 bytes, vtable 0x80438db0; new 0x801ba1f4, setup
// 0x800cff48; the app's +0x2d4). Three ANT_BOSS anitas: 0 the frame (anims: appear 0, normal 1, low (ratio <= 0.2) 2, out 5), 1 the bar
// (appear 6, then anim 4 used as a 1000-frame scale: frame = 1000 * (1 - shown / max)), 2 the shake (anim 3 on a change: its layer 0
// position offsets 0 and 1). The shown life runs down to the boss's life by 2 per frame; the boss's struggle waits for it to reach 0
// (0x800d0a84). Priority 0x80000008.
namespace kingdom::game::g01
{
class cTaskBoss;

class cTaskBossLife : public cObject, public kingdom::cTaskAnita
{
public:
	explicit cTaskBossLife(cTaskBoss* pBoss);  // DOL 0x801ba1f4 + 0x800cff48
	bool Execute(cJetTask* pParam) override;   // DOL 0x800d08bc
	void Appear() { m_nState = 0; m_nStep = 0; m_b08 = true; }   // DOL 0x800d0a58 (vtable +0x80, the app's READY)
	void Out() { m_nState = 2; m_nStep = 0; }                    // DOL 0x800d0a70 (vtable +0x84, the boss is defeated)
	bool IsEmpty() const { return m_fShown <= 0.0f; }            // DOL 0x800d0a84

private:
	static int FrameAnm(int n);                // DOL 0x800d02ac (vtable +0x6c)
	static int BarAnm(int n);                  // DOL 0x800d0308 (vtable +0x70)
	bool IsShaking();                          // DOL 0x800d0228 (vtable +0x74)
	void AppearStep();                         // DOL 0x800d0438 (state 0)
	void GaugeStep();                          // DOL 0x800d05f8 (state 1)
	void OutStep();                            // DOL 0x800d07c4 (state 2)
	void Shake();                              // DOL 0x800d0334 (vtable +0x68)
	cTaskBoss* m_pBoss{};                      // +0x218
	float m_vBase[4]{0, 0, 0, 2};              // +0x220 (w = 2.0, FLOAT_8047b438)
	float m_fMax{1};                           // +0x230
	float m_fShown{};                          // +0x234
	float m_fTarget{};                         // +0x238
	float m_fRatio{1};                         // +0x23c
};
}
