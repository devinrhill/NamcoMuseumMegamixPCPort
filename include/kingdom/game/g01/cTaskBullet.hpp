#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g01/cObject.hpp"

// Galaga Remix (g01) enemy shots: kingdom::game::g01::cTaskBullet (vtable 0x80439198) with cTaskBulletDummy (0x804392e0, kind 6,
// CHAR_BULLET01_MDL; pool cBulletDummyWork) and cTaskBulletHit (0x80439438, kind 7, CHAR_BULLET02_MDL + CHAR_BULLET02_ANM_CHA_01; pool
// cBulletHitWork). Pooled by the enemy manager (+0x1ec / +0x1f0). A shot flies from the enemy toward Pac-Man (+0x20 at the moment of the shot)
// at the EnemyParam bullet speed and moves along with Pac-Man every frame; it hits Pac-Man (kind 3) when the two 0.35 spheres touch.
// Pac-Man takes damage from kind 7 only (cTaskPacman::HitCallback). A kind 6 shot (cTaskBulletDummy) moves with Pac-Man's aim point +0x2f0
// (3 units along his forward axis, 0x800d2ae8) and is tested against that point (0x800d2bfc): it flies past him and does no harm.
namespace kingdom::game::g01
{
class cTaskBullet : public cObjectAABB, public kingdom::cTaskModel
{
public:
	explicit cTaskBullet(bool bHit);           // the pools' Init (cBulletDummyWork / cBulletHitWork, 0x800d2080 + model)
	bool IsFree() const { return !m_b08; }
	void Setup(float fSpeed, const float vPos[3]);   // DOL 0x800d2174 (r1 speed, bullet, start)
	bool Execute(cJetTask* pParam) override;   // DOL 0x800d26b4
	void Draw(int nStatus, cJetTask* pParam) override;
	bool HitCheck(cObject* other) override;    // DOL 0x800d25cc
	void HitCallback(cObject* other) override; // DOL 0x800d2648

private:
	void Step();                               // DOL 0x800d23b0 (vtable +0xa8)
	bool IsOut() const;                        // DOL 0x800d24b0 (vtable +0xb0)
	void Die();                                // DOL 0x800d258c (vtable +0xac; state 2 runs it, +0xd4 = 0x800d2860 sets state 2)
	const float* Target() const;               // Pac-Man +0x20 (kind 7) / +0x2f0 (kind 6)
	void UpdateModel();
	float m_vDir[3]{};                         // +0x330
	float m_vPrev[3]{};                        // +0x340 the position before this frame's step (kind 6, 0x800d2ae8)
	float m_vLastTarget[3]{};                  // +0x350 Pac-Man's position last frame
	float m_fSpeed{};                          // +0x364
	float m_fTime{};                           // +0x360
};
}
