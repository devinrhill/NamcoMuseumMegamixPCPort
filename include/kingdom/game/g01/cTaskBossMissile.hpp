#pragma once

#include "common.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cKingdomSndActor.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g01/cObject.hpp"
#include "kingdom/game/g01/cTaskShootMarker.hpp"

// Galaga Remix (g01) the last boss's missiles (a cTaskBullet / cTaskMissile line; pools of the boss enemy manager, cTaskBossEnemyMan +0x1fc /
// +0x200, 10 each = table 0x80373138 stage 10):
//   cTaskBossMissile (vtable 0x8043b484, kind 0xe, CHAR_LB_MISSILE_MDL; init 0x800f0d50, setup 0x800f0de4): hangs at the boss's node bom%d
//     (state 3, CHAR_LB_MISSILE_SET_ANM_CHA_01 + _CLA_01), is launched along the node's z (state 1, 0x800f1490) and homes in on Pac-Man in the
//     camera frame (0x800f10f8, the turn limit growing like cTaskMissile's). BOSS_MISSILE_HP shots destroy it (CHA_02 / CLA_02, then gone).
//   cTaskBossSpMissile (vtable 0x8043b308, kind 0xf, CHAR_LB_SP_MISSILE_MDL; init 0x800f1f70, setup 0x800f2074): the same flight, but a shot
//     does not destroy it: each hit raises the damage it carries (+0x410, Bullet_s%x.csv BOSS_SP_MISSILE_DAMAGE / _ADD / _MAX; level effects
//     at ef_b1 / ef_b2) and stops it for a while (BOSS_SP_MISSILE_STOP_*). While the boss inhales it flies into the boss's ef_body (state 4,
//     0x800f2620) and hurts it by the carried damage (cTaskBoss::HitCallback).
// The missile orbit model (CHAR_LB_MISSILE_ORBIT_MDL +0x398, made by the init 0x800f0d50) gets no clip and its wobble (vtable +0xa8 =
// 0x800f0f3c) is only called by cTaskMissile's Execute, which this class overrides: neither is ported (no effect). Not ported: the 3D sound actor.
namespace kingdom::game::g01
{
class cTaskBoss;
class cTaskNodeEffect;

// DOL cBulletParam (DAT_80479054, "Bullet_s%x.csv", load 0x800efef4)
struct sBulletParam
{
	int nMissileHp{1};              // +0x00 MISSILE_HP
	int nBossMissileHp{1};          // +0x04 BOSS_MISSILE_HP
	float fSpSpeedMax{1};           // +0x08 BOSS_SP_MISSILE_SPEED_MAX
	float fSpStruggleSpeedMax{1};   // +0x0c BOSS_SP_MISSILE_STRUGGLE_SPEED_MAX
	int nSpLevel1{200};             // +0x10 BOSS_SP_MISSILE_LEVEL_1
	int nSpLevel2{300};             // +0x14 BOSS_SP_MISSILE_LEVEL_2
	int nSpDamage{100};             // +0x18 BOSS_SP_MISSILE_DAMAGE
	int nSpDamageAdd{20};           // +0x1c BOSS_SP_MISSILE_DAMAGE_ADD
	int nSpDamageMax{400};          // +0x20 BOSS_SP_MISSILE_DAMAGE_MAX
	float fSpStopBase{200};         // +0x24 BOSS_SP_MISSILE_STOP_BASE
	float fSpStopUnder{30};         // +0x28 BOSS_SP_MISSILE_STOP_UNDER
	float fSpStopMax{300};          // +0x2c BOSS_SP_MISSILE_STOP_MAX
	void Load(int nStage);          // DOL 0x800efef4
};

class cTaskBossMissile : public cObjectAABB, public kingdom::cTaskModel
{
public:
	explicit cTaskBossMissile(bool bSp);               // pool ctor 0x800e7480 / 0x800e7760 + init 0x800f0d50 / 0x800f1f70
	~cTaskBossMissile() override;
	bool IsFree() const { return !m_b08; }
	void Setup(float fSpeed, int nSlot, cTaskBoss* pBoss);   // DOL 0x800f0de4 / 0x800f2074
	void Attach() { m_nState = 3; m_nStep = 0; }       // DOL 0x800f1f58 (vtable +0xdc)
	void Launch() { m_nState = 1; m_nStep = 0; if (m_pMarker != nullptr) m_pMarker->Show(); }   // DOL 0x800f0358 (vtable +0xcc)
	void Die();                                        // DOL 0x800f039c (vtable +0xd4): state 2, not collidable
	bool Execute(cJetTask* pParam) override;           // DOL 0x800f1b84
	void Draw(int nStatus, cJetTask* pParam) override; // DOL 0x800f1e40
	bool HitCheck(cObject* other) override;            // DOL 0x800f07a0 (cTaskMissile's)
	void HitCallback(cObject* other) override;         // DOL 0x800f1820 / 0x800f2764
	int nDamage() const { return m_nDamage; }          // +0x410 (SP)
	const float* vPrevPos() const { return m_vPrev; }  // +0x370

	static sBulletParam s_Param;                       // DAT_80479054

private:
	kingdom::cKingdomSndActor m_SndActor;   // +0x384: the cGalagaSndActor (0x801c1680, the DOL class derives from cTaskMissile, whose ctor 0x800f01b0 makes it: UNVERIFIED)
	void Fly();                        // DOL 0x800f1490 / 0x800f224c (vtable +0xf8, state 1)
	void Home(float fStep);            // DOL 0x800f10f8 (vtable +0xfc)
	void DieStep();                    // DOL 0x800f172c / 0x800f26fc (vtable +0x100, state 2)
	void SuckIn();                     // DOL 0x800f2620 (vtable +0xf0, SP state 4)
	bool IsOut() const;                // DOL 0x800f0edc (vtable +0xb0)
	void Free();                       // DOL 0x800d258c (vtable +0xac): state 0, inactive
	void UpdateModel();                // DOL 0x800f1f20 / 0x800f25cc (vtable +0xb4)
	bool m_bSp{};
	cTaskBoss* m_pBoss{};              // +0x394
	cTaskShootMarker* m_pMarker{};     // +0x368 (made by the cTaskMissile init 0x800f01b0)
	int m_nSlot{};                     // +0x390 bom slot
	float m_fSpeed{};                  // +0x364
	float m_fSpeedSave{};              // +0x418 (SP)
	float m_fTurn{};                   // +0x388
	float m_fFlash{};                  // +0x36c
	float m_fTime{};                   // +0x360
	bool m_bBroken{};                  // +0x38c normal: shot down; SP: the struggle clip plays
	float m_vDir[3]{0, 0, 1};          // +0x3a0
	float m_vLocal[3]{};               // +0x3b0 position in the camera frame
	float m_vPrev[3]{};                // +0x370
	int m_nDamage{};                   // +0x410 (SP)
	int m_nLevel{};                    // +0x414 (SP)
	float m_fStop{};                   // +0x41c (SP)
	kingdom::cKingdomPtkHnd m_Spark;   // +0x420 (SP, slot 0x32 while it is being shot)
	cTaskNodeEffect* m_pFxHip{};       // +0x380 ef_hip (0x31, the jet)
	cTaskNodeEffect* m_pFxB[2]{};      // +0x470 / +0x474 ef_b1 / ef_b2 (0x3c, the damage levels)
};
}
