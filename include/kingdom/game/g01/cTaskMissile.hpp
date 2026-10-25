#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g01/cObject.hpp"
#include "kingdom/game/g01/cTaskShootMarker.hpp"

namespace kingdom { class cKingdomPtkHnd; }

// Galaga Remix (g01) homing missile: kingdom::game::g01::cTaskMissile (vtable 0x8043b5f8, kind 8, a cTaskBullet; pool cMissileWork with
// CHAR_MISSILE01_MDL 0x3d9cd73a, enemy manager +0x1f4). Fired by enemies with fire pattern 3 (0x800e3bd4: the attack sign, then 0x800f03e0
// with the EnemyParam missile speed and the enemy's launch direction +0x270). It turns toward Pac-Man by at most +0x388 rad per frame
// (0.03, growing by 0.0004 each frame), can be shot down (HP = Bullet_s%x.csv MISSILE_HP) and hurts Pac-Man on contact.
// Its shoot marker child (cTaskShootMarker) shows from the launch until it dies. The jet effect (cTaskEffectExpand +0x380) burns at node "ef_jet" (slot 0x2c) from the setup until it dies.
namespace kingdom::game::g01
{
class cTaskNodeEffect;
class cTaskMissile : public cObjectAABB, public kingdom::cTaskModel
{
public:
	cTaskMissile();                            // DOL 0x800f01b0 (Init) + the pool's model
	bool IsFree() const { return !m_b08; }
	void Setup(float fSpeed, const float vPos[3], const float vDir[3]);   // DOL 0x800f03e0
	void Launch();                             // DOL 0x800f0358 (vtable +0xcc): state 1 (0x800d2838), the marker shows
	bool Execute(cJetTask* pParam) override;   // the cTaskBullet Execute 0x800d26b4 with the missile slots
	void Draw(int nStatus, cJetTask* pParam) override;
	bool HitCheck(cObject* other) override;    // DOL 0x800f07a0
	void HitCallback(cObject* other) override; // DOL 0x800f07f8

private:
	void Step();                               // DOL 0x800f04dc (vtable +0xa8)
	void Die();                                // DOL 0x800f0b3c
	void UpdateModel();
	float m_vDir[3]{};                         // +0x330
	float m_fSpeed{};                          // +0x364
	float m_fTurn{};                           // +0x388
	float m_fFlash{};                          // +0x36c
	int m_nHp{};                               // +0x18
	cTaskShootMarker* m_pMarker{};             // +0x368
	cTaskNodeEffect* m_pJet{};                 // +0x380 the jet (cTaskEffectExpand, unbound until the launch)
	int m_nMissileHp{1};                       // DAT_80479054 (Bullet_s%x.csv MISSILE_HP)
};
}
