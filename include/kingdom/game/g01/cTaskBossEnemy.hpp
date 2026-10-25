#pragma once

#include "kingdom/game/g01/cTaskEnemy.hpp"

// Galaga Remix (g01) the enemies the last boss vomits: kingdom::game::g01::cTaskBossEnemy (vtable 0x80439fa0, a cTaskEnemy; pool
// tWorkObject<cTaskBossEnemy> of cTaskBossEnemyMan +0x1f8, 50 entries). Spawned by 0x800e7ca8 at the boss's ef_body (camera local) with the
// type / orbit of a BossEnemyList_%dp.csv row. They fly their orbit like stage enemies; while the boss inhales (+0x36f) they are sucked
// toward its ef_body (state 3, speed 1 growing by 0.1 up to 2.5) and die when the boss's body sphere catches them (cTaskBoss::HitCheck kind
// 0xb: the segment from the previous position); all die when the boss dies. Vtable overrides: Init 0x800e2e0c, +0xa0 Execute 0x800e3060,
// +0xcc 0x800e2e74 (boss dead?), +0xd0 0x800e3264 (sucked), +0xd4 0x800e3358 (state 3), HitCallback 0x800e2e7c.
namespace kingdom::game::g01
{
class cTaskBoss;

class cTaskBossEnemy : public cTaskEnemy
{
public:
	cTaskBossEnemy();                          // DOL 0x800e2e0c (Init)
	void Setup(const float vLocal[3], int nType, int nOrbit, cTaskBoss* pBoss);   // DOL 0x800e2e2c
	bool Execute(cJetTask* pParam) override;   // DOL 0x800e4704 -> 0x800e3060
	void HitCallback(cObject* other) override; // DOL 0x800e2e7c
	const float* vPrevPos() const { return m_vPrev; }   // +0x360 the position at the start of the frame

private:
	void Suck() { m_nState = 3; }              // DOL 0x800e3358 (vtable +0xd4)
	void SuckStep();                           // DOL 0x800e3264 (vtable +0xd0)
	cTaskBoss* m_pBoss{};                      // +0x358
	float m_vPrev[4]{0, 0, 0, 1};              // +0x360
	float m_fSpeed{};                          // +0x370
};
}
