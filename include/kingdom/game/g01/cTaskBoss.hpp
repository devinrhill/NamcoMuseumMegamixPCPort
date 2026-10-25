#pragma once

#include <vector>

#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g01/cDemoMan.hpp"
#include "kingdom/game/g01/cTaskEnemy.hpp"

namespace kingdom { class cTaskAnita; }

// Galaga Remix (g01) stage 10, the last boss: kingdom::game::g01::cTaskBoss (0x670 bytes, vtable 0x80438ae0, derived from cTaskEnemyBase;
// new 0x801ba078, setup 0x800cebbc from cAppG01GalagaBoss::Init 0x801b88cc). Kind 0xd, EnemyParam row 11 (life, score).
//
// The boss is CHAR_ENEMY_LASTBOSS_MDL expanded at node "orbit" of CHAR_ENEMY_LASTBOSS_ORBIT_MDL, whose clip m01m_s%x_lb_ob_cha (x = stage +
// course segment) runs in sync with the course: the boss flies with the course. The invisible CHAR_ENEMY_LASTBOSS_HIT_MDL (expanded at the
// same node, clips CHAR_ENEMY_LASTBOSS_HIT_ANM_* in step with the boss's) holds the hit boxes the players' shots are tested against
// (cTaskHitModel, natively HitModelRay). Its attacks come from an orbit file used as a timetable against the course frame (EnemyOrbit_sa.csv
// row (players - 1) * 10 + 1 for the first segment, + 2 for the looping second, + 6 once it struggles); a hold node with parameter p starts
// attack p + 1:
//   1 missiles (0x800cdaf0): the four missiles hanging at bom1..4 fly off, new ones are hung (the node's mask: which are SP missiles),
//   2 bite (0x800cd540): it lunges at Pac-Man (CHAR_ENEMY_LASTBOSS_ANM_CHA_07, BossParam o_Bite_*), the danger sign, a bite that hurts unless
//     Pac-Man jumps,
//   3 vomit (0x800cdcdc): the shells (cTaskShell) open; unless the players break all four in time it spits out the enemies of
//     BossEnemyList_%dp.csv (cTaskBossEnemy),
//   4 beam (0x800cceb8): it inhales (o_Breath01 frames, mouth open), then fires a beam that hurts Pac-Man (o_Beam_Attack01).
// It only takes damage from SP missiles the players shot at (cTaskBossSpMissile: kind 0xf) that it swallows while inhaling (the inhale is
// then cut short, CHA_06). Below BossParam STRUGGLE_RATE of its life (and between attacks) it struggles (state 4: CHA_09 / CHA_10, the
// shells break, its missiles get the STRUGGLE speeds), and then the players' lasers hurt it directly (LASER_DAMAGE); once the life gauge
// (cTaskBossLife) has drained it is defeated (state 6, the app's IsBossDefeated 0x800cfb54).
// The demo interface (+0x348, stage 10: cDemoBossBoss 0x801bd094): no opening part; the goal part (states 1) reloads the model, plays the
// a50 clips (0x800cc908) and the explosion at ef_body + three cues at their Demo_sa.csv frames. Not ported: the 3D sound actor.
namespace kingdom::game::g01
{
class cTaskBossLife;
class cTaskBossMissile;
class cTaskNodeEffect;
class cTaskShell;

// DOL 0x800ca884 (init) / 0x800ca8e4 (load "BossParam_%dp.csv"), 0x4c bytes
struct sBossParam
{
	float fMissileSpeed{};          // +0x00 MISSILE_SPEED
	float fMissileStruggleSpeed{};  // +0x04 MISSILE_STRUGGLE_SPEED
	float fSpMissileSpeed{};        // +0x08 SP_MISSILE_SPEED
	float fSpMissileStruggleSpeed{};// +0x0c SP_MISSILE_STRUGGLE_SPEED
	float fStruggleRate{};          // +0x10 STRUGGLE_RATE
	int nLaserDamage{};             // +0x14 LASER_DAMAGE
	float fVomit{};                 // +0x18 o_Vomit01
	float fBreath{};                // +0x1c o_Breath01
	float fBiteToReady{};           // +0x20 o_Bite_ToReady01 (share of the bite clip)
	float fBiteToPac{};             // +0x24 o_Bite_ToPac01
	float fBiteToWait{};            // +0x28 o_Bite_ToWait01
	float fBiteToBack{};            // +0x2c o_Bite_ToBack01
	float fBeamAttack{};            // +0x30 o_Beam_Attack01
	float fBite{};                  // +0x34 e_Bite01
	float fThreat{};                // +0x38 e_Threat01
	float fDanger{};                // +0x3c a_Danger01
	float fBeamSe{};                // +0x40 s_Beam01 (not read by the ported code)
	bool bDanger{};                 // +0x44 the per-attack "done" flags (cleared when an attack ends, 0x800ccc40)
	bool bBite{};                   // +0x45
	bool bThreat{};                 // +0x46
	bool bBeamSe{};                 // +0x47
	bool bBeam{};                   // +0x48
	void Load(const char* name);    // DOL 0x800ca8e4
};

// DOL cBossLeaveEnemy (0x14 bytes, vtable at +0x10; load 0x800cad0c): "BossEnemyList_%dp.csv" rows {enemy type, orbit}, drawn in a shuffled
// order (300 random swaps of the index list)
struct cBossLeaveEnemy
{
	struct sEntry { int nType; int nOrbit; int n08; };
	u32 nNext{};                    // +0x00
	std::vector<u32> order;         // +0x08 (count +0x04)
	std::vector<sEntry> entry;      // +0x0c
	void Load(const char* name);    // DOL 0x800cad0c
	void Shuffle();                 // the 300 swaps (0x800cebbc / 0x800cdcdc)
	const sEntry* Next();           // 0x800cdcdc step 3
};

class cTaskBoss : public cTaskEnemy, public cDemoInterface
{
public:
	// cDemoBossBoss (stage 10): Init / Term / IsWaitEnd of cDemoBossBase do nothing (0x800c9620 / 0x800c961c / 0x801bd53c returns 0)
	void DemoStart() override { m_nState = 0; m_nStep = 0; m_nDemoStep = 0; m_bDemoDone = false; }   // DOL 0x800cfaf8 (vtable +0x118)
	bool IsDemoDone() const override { return m_bDemoDone; }
	void DemoFinish() override { m_bDemoDone = true; m_bGoalDone = true; }
	void DemoGoalStart() override { m_nState = 1; m_nStep = 0; m_nDemoStep = 0; m_bGoalDone = false; }   // DOL 0x800cfb10 (vtable +0x11c)
	bool IsGoalDone() const override { return m_bGoalDone; }
	cTaskBoss();                                                  // DOL 0x801ba078 + Init 0x800cae80
	~cTaskBoss() override;
	void Setup(cTaskScene* pScene, cTaskPacman* pPacman);         // DOL 0x800cebbc
	void SetLife(cTaskBossLife* p) { m_pLifeGauge = p; }          // the gauge the struggle waits for (app +0x2d4, 0x800d0a84)
	bool Execute(cJetTask* pParam) override;                      // DOL 0x800cf654 (vtable +0xa0)
	void Draw(int nStatus, cJetTask* pParam) override;            // DOL 0x800cf8b4 (vtable +0xa4)
	bool HitCheck(cObject* other) override;                       // DOL 0x800ce73c
	void HitCallback(cObject* other) override;                    // DOL 0x800ce928

	void StartFight() { m_nState = 3; m_nStep = 0; }              // DOL 0x800cfb2c (the app's GAME)
	bool IsDead() const { return m_nState == 5; }                 // DOL 0x800cfb40
	bool IsDefeated() const { return m_nState == 6; }             // DOL 0x800cfb54
	bool IsInhaling() const { return m_bMouth; }                  // +0x36f (the SP missiles fly into the mouth, 0x800f224c)
	int nLife() const { return m_n18; }                           // +0x18 (the gauge reads it)
	int nMaxLife() const { return m_nMaxLife; }                   // EnemyParam row 11 life (+0x254 -> +4)
	const float* vBody() const { return m_vBody; }                // +0x3f0 the world position of node ef_body
	// the world matrix (column major) of bom1..4 (+0x440 + i * 0x40, 0x800cb5b4): where the missiles hang
	bool BomMatrix(int i, float out[16]) const;
	bool BossNodeWorld(const char* node, float out[16]) const { return ChildWorld(0, node, out); }   // a node of the boss model (the shells)
	// the shot's ray against the shells and the hit model (DOL 0x800cf250); t = distance along dir
	bool RayHit(const float org[3], const float dir[3], float& t);
	static cTaskBoss* s_pInstance;

private:
	bool UpdateBossOrbit(float fStep);  // DOL 0x800ccb30 (vtable +0x7c): the timetable against the course frame
	void UpdateBossFire();              // DOL 0x800ccc40 (vtable +0x80)
	void FightStep();                   // DOL 0x800cf2e8 (vtable +0xc8)
	void StruggleStep();                // DOL 0x800cf4bc (vtable +0xcc)
	void SetOrbitSpec(int n);           // FormationSpec((players - 1) * 10 + n) + StartOrbit
	void LoadModel();                   // DOL 0x800cb308 (vtable +0xc0)
	void SetSegmentClip(int nCode);     // DOL 0x800cb4f0 (vtable +0xc4)
	void StartClips();                  // DOL 0x800cc7f4 (vtable +0x114)
	void GoalClips();                   // DOL 0x800cc908 (vtable +0x110): model again, shells hidden, m01m_lb_ob_a50 + m01m_g09_a50_*
	void GoalInit();                    // cDemoBossBoss::Start 0x801bd298
	bool GoalCheck();                   // cDemoBossBoss::Execute 0x801bd33c: true while the longest a50 clip runs
	void SyncCourse();                  // DOL 0x800cb554 (vtable +0xf8): frame (+0x100 0x800cc7b4) and rate (+0x104 0x800cc774)
	void Blend(float fFrames, int nFrom, int nTo);   // DOL 0x800cc664 (vtable +0xd0)
	void UpdateBlend();                 // DOL 0x800cc684 (vtable +0xd4)
	void UpdateNodes();                 // DOL 0x800cb5b4 (vtable +0x108): bom matrices, ef_body, forward
	void UpdateBossModel();             // DOL 0x800cc44c (vtable +0x90): +0x20 = the orbit node's position
	void UpdateHitEffect();             // DOL 0x800cc618 (vtable +0x10c)
	bool AttackBeam();                  // DOL 0x800cceb8 (vtable +0xd8)
	bool AttackBite();                  // DOL 0x800cd540 (vtable +0xdc)
	bool AttackVomit();                 // DOL 0x800cdcdc (vtable +0xe0)
	bool AttackMissile();               // DOL 0x800cdaf0 (vtable +0xe4)
	void BiteMove();                    // DOL 0x800cd894 (vtable +0xe8)
	void AimBeam();                     // DOL 0x800ccd38 (vtable +0xec)
	void ShowDanger();                  // DOL 0x800cb48c (vtable +0xf0)
	bool StruggleStart();               // DOL 0x800ce340 (vtable +0xf4)
	void ReloadMissiles(u32 nMask);     // DOL 0x800cb058
	u32 NextMissileMask(float fAfter) const;   // the missile mask (+0x3c) of the next missile node after a course frame (0x800cebbc / 0x800cdaf0)
	void SetAnm(int nSlot, const char* boss, const char* hit, bool bWeight0);   // the clip pairs (boss + hit model) of the attacks
	bool HitModelRay(const float org[3], const float dir[3], float& t);    // cTaskHitModel 0x801c473c -> cMesh::HitCheck 0x801c2a30
	bool ChildWorld(int nExp, const char* node, float out[16]) const;      // world (column major) of a node of an expanded model
	cKingdomModel* Boss() const;
	cKingdomModel* Hit() const;

	cTaskScene* m_pScene{};             // +0x350
	cTaskPacman* m_pPacman{};           // +0x354
	sBossParam m_Param;                 // +0x358
	cTaskBossLife* m_pLifeGauge{};
	int m_nMaxLife{1};
	int m_nStage{};                     // +0x364
	int m_nWorld{};                     // +0x368
	bool m_bHitInhale{};                // +0x36c a missile reached the mouth while it inhaled
	bool m_bBite{};                     // +0x36d (this frame) the bite hurts Pac-Man
	bool m_bBeam{};                     // +0x36e (this frame) the beam hurts Pac-Man
	bool m_bMouth{};                    // +0x36f inhaling
	bool m_bStruggleSe{true};           // +0x370
	int m_nAtkStep{};                   // +0x374
	float m_fAtkTime{};                 // +0x378
	float m_fReload{};                  // +0x37c
	cBossLeaveEnemy m_Leave;            // +0x380
	int m_nSeg{};                       // +0x384
	float m_vForward[4]{0, 0, 1, 1};    // +0x3d0
	float m_vBite[4]{0, 0, 0, 1};       // +0x3e0 the bite's offset of the whole model
	float m_vBody[4]{0, 0, 0, 1};       // +0x3f0 ef_body (world)
	int m_nShellHit{-1};                // +0x410
	cTaskShell* m_pShell[4]{};          // +0x414
	cTaskBossMissile* m_pMissile[4]{};  // +0x424
	float m_mBom[4][16]{};              // +0x440 (column major natively)
	bool m_bBom[4]{};
	int m_nBlendFrom{}, m_nBlendTo{};   // +0x540 / +0x544
	float m_fBlendFrames{}, m_fBlendTime{};   // +0x548 / +0x54c
	bool m_bBlend{};                    // +0x550
	float m_fRadius{0.5f};              // +0x560 (sphere at +0x554 = ef_body)
	kingdom::cTaskAnita* m_pDanger{};   // +0x564 ANT_DANGER
	kingdom::cKingdomPtkHnd m_Beam;     // +0x570 the beam
	kingdom::cKingdomPtkHnd m_HitFx;    // +0x5c0 the spark where a shot met it
	cTaskNodeEffect* m_pFxBody{};       // +0x610 (struggle 0x3b)
	cTaskNodeEffect* m_pFxInhale{};     // +0x614 (0x36)
	cTaskNodeEffect* m_pFxVomit2{};     // +0x618 (0x40)
	cTaskNodeEffect* m_pFxVomit{};      // +0x61c (0x37)
	cTaskNodeEffect* m_pFxMouth{};      // +0x620 (0x38 charge / 0x3a threat / 0x35 bite)
	cTaskNodeEffect* m_pFxWing[2]{};    // +0x624 / +0x628 ef_L_wing / ef_R_wing (0x3f, looping)
	cTaskNodeEffect* m_pFxBom[4]{};     // +0x62c bom%d (0x3d, the launch flash)
	int m_nSeHakidasi{-1};              // +0x63c
	int m_nSeSuikomi{-1};               // +0x648
	int m_nSeCharge{-1};                // +0x654
	int m_nSeHassya{-1};                // +0x660
	bool m_bHassya{};                   // native: m01se_22hassya once (the DOL restarts the handle every frame past 130, UNVERIFIED audibly)
	std::vector<wii::Mdl0Node> m_HitNodes;   // the hit model's nodes (cNode / cMesh tree of 0x801c398c)
	int m_nDbg{-1};
	// the demo object (cDemoBossBoss, 0x34 bytes): +8 / +9 done flags, +0x14 cAnmCheck {model, kind, max}, +0x24 e_bomb01 / +0x28
	// s_voice3_01 / +0x2c s_voice4_01 (Demo_sa.csv, -1 = done), +0x30 the explosion effect at ef_body (0x800e0820)
	int m_nDemoStep{};
	bool m_bDemoDone{true}, m_bGoalDone{true};
	int m_nGoalKind{};
	float m_fGoalMax{};
	float m_fGoalTime[3]{-1, -1, -1};
	cTaskNodeEffect* m_pGoalFx{};
};
}
