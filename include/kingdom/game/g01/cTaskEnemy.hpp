#pragma once

#include <vector>

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g01/cObject.hpp"

namespace kingdom { class cKingdomPtkHnd; }

// Galaga Remix (g01) enemies. Ported from main.dol; the hand-reversed parts of the old tree (reversing/dol/g01_ghidra: cOrbit.cpp,
// cOrbitDataMan.cpp, cTaskEnemy.cpp UpdateOrbit / UpdateFire, cTaskEnemyBase.cpp StartOrbit) and ~/Code/proj/unpac/kingdom/galaga
// (emy.py, orbit.py) were the guides.
namespace kingdom::game::g01
{
class cTaskBomb;
class cTaskFighter;
class cTaskScene;
class cTaskPacman;
class cTaskBullet;
class cTaskMissile;
struct sScnRecord;

// One key of an enemy orbit (.emy file, headerless big endian, 0x40 bytes per key)
struct sEmyNode
{
	float vPos[4];       // +0x00 local position
	float vRot[4];       // +0x10 rotation (radians)
	float vTangent[4];   // +0x20 Hermite tangent
	float fFrame;        // +0x30 key time
	u32 nCmd;            // +0x34 bits 0-15 interpolation (0 linear, 1 Hermite, 2 jump, 3 hold), 16-27 jump node, 28-31 facing
	u32 nParam;          // +0x38 fire pattern of a hold node
	u32 n3c;
};
static_assert(sizeof(sEmyNode) == 0x40);

// DOL cOrbit (vtable 0x8044b0ac): the keys of one .emy file
struct cOrbit
{
	std::vector<sEmyNode> nodes;
	u32 nNumNode() const { return (u32)nodes.size(); }
};

// DOL cOrbitDataMan (0xc bytes, vtable 0x8044b0a0)
class cOrbitDataMan
{
public:
	void Load(int nStage);              // DOL 0x801c12dc: "EnemyOrbit_s%x.csv", column 1 = the .emy file of each row
	cOrbit* pOrbit(u32 n);               // DOL 0x801c14b4
	u32 nNumOrbit() const { return (u32)m_Orbit.size(); }

private:
	std::vector<cOrbit> m_Orbit;
};

// DOL kingdom::game::g01::cTaskEnemyBase (vtable 0x8043a1f8) + cTaskEnemy (0x350 bytes, vtable 0x8043a0d4), pooled by the enemy manager
// (tWorkObject<cTaskEnemy>).
class cTaskEnemy : public cObjectAABB, public kingdom::cTaskModel
{
public:
	cTaskEnemy();                       // DOL 0x800e3640 (cTaskEnemyBase::Init) + 0x800e1c64
	bool IsFree() const { return !m_b08; }
	bool bKilled() const { return m_bKilled; }   // +0x348 shot down (not just gone)
	void SetupOrbit(const sScnRecord* pRec, int nType, int nOrbit, u32 nGroup);   // DOL 0x800e1f88
	void Start();                       // DOL 0x800e2da8 (vtable +0xbc): state 1 + the appear cue m01se_12hirai (DAT_8047b518)
	bool Execute(cJetTask* pParam) override;   // DOL 0x800e4704 -> vtable +0xa0 = 0x800e29c8
	void Draw(int nStatus, cJetTask* pParam) override;   // DOL 0x800e47b8 -> +0xa4 = 0x800e2ac8; a free pool entry is not drawn
	void HitCallback(cObject* other) override;   // DOL 0x800e27b4
	virtual void Dead();                // DOL vtable +0xc4 = 0x800e2de0: state 2, not collidable (gate 0x800e5048, spike 0x800e8e38)

protected:
	void LoadModel(int nType);          // DOL 0x800e1ca8 (vtable +0xac)
	void StartOrbit();                  // DOL 0x800e37a0
	const sEmyNode* pOrbitNode(u32 n) const;   // DOL 0x800e3860
	bool UpdateOrbit(float fStep);      // DOL 0x800e22c4
	void Leave();                       // DOL 0x800e271c (vtable +0x84, state 2): gone; the scene learns that its group is cleared
	void DeathClip(int nType);          // DOL 0x800e1eb4 (vtable +0xb0): the type's _CHA_02 / _CLA_02 clips (table 0x80372d3c)
	bool DieStep();                     // DOL 0x800e20f0 (vtable +0xa8): death clip + explosion + sound; true when the clip ran out
	void UpdateModel();                 // DOL 0x800e40cc (+0x90) and the draw matrix of 0x800e2ac8 (+0xa4)
	void HitEffect(int nPlayer);        // DOL 0x800e4220 (vtable +0x6c): the explosion of a killed enemy
	void HitSpark();                    // DOL 0x800e431c (vtable +0x70)
	void FlashStep();                   // DOL 0x800e46b0 (vtable +0x88): the hit flash counts down, then fog 0 again
	void DeathSound();                  // DOL 0x800e4494 (vtable +0x74)
	void UpdateFire();                  // DOL 0x800e2200 (vtable +0x80)
	void FireBullet();                  // DOL 0x800e3888 (+0x94): pattern 0, a BulletDummy (kind 6) at once
	void FireSignBullet();              // DOL 0x800e39c0 (+0x98): pattern 1, the attack sign effect, then a BulletHit (kind 7)
	void FireMissile();                 // DOL 0x800e3bd4 (+0x9c): pattern 3, the attack sign, then a missile

	int m_nType{};                      // +0x25c
	int m_nSize{};                      // +0x258 EnemyParam column 1 (explosion size 1 / 2 / 3)
	int m_nLife{};                      // +0x25c.. EnemyParam column 2 (hit points)
	int m_nScore{};                     // +0x84 EnemyParam column 5
	float m_fBulletSpeed{};             // +0x260 EnemyParam column 3
	float m_fMissileSpeed{};            // +0x264 EnemyParam column 4
	int m_nFireStep{};                  // +0x28c
	int m_nHitPlayer{};                 // +0x288
	float m_fFlash{};                   // +0x284 frames of the hit flash left (fog 1, 0x800e46b0)
	bool m_bKilled{};                   // +0x348
	kingdom::cKingdomPtkHnd* m_pSign{};   // +0x2a0 the attack sign effect
	float m_vLaunch[3]{0, 0, -1};       // +0x270 launch direction (0x800e3ed8): the model rotation applied to (0, 1, -1.625)
	u32 m_nGroup{};                     // +0x340 formation group (scn p2 bits 0-8)
	const cOrbit* m_pOrbit{};           // +0x334
	u32 m_nOrbitIdx{};                  // +0x2e8
	const sEmyNode* m_pCurNode{};
	const sEmyNode* m_pNextNode{};
	float m_vBasePos[4]{};              // +0x2f0 the spawn record's parent translation
	float m_vOrbitPos[4]{0, 0, 0, 1};   // +0x310
	float m_vOrbitRot[4]{};             // +0x320
	float m_fOrbitTime{};               // +0x280
	s32 m_nFirePattern{-1};             // +0x290
	float m_vWorldPos[4]{0, 0, 0, 1};   // cObject +0x20 (m_vPos)
	float m_vRot[4]{};                  // cObject +0x30 (facing)
	bool m_bModelType[15]{};
};

// DOL cEnemyParam (vtable 0x8044b068, Load 0x801c10ec): "EnemyParam_s%xp%x.csv" (stage, players), 15 rows of 0x14 bytes
struct sEnemyParam
{
	s32 nSize;            // +0x00 column 1
	s32 nLife;            // +0x04 column 2
	s32 nScore;           // +0x08 column 5
	float fBulletSpeed;   // +0x0c column 3
	float fMissileSpeed;  // +0x10 column 4
};

// DOL kingdom::game::g01::cTaskEnemyMan (vtable 0x8043a548). g_pTaskEnemyMan = DAT_80479048.
class cTaskEnemyMan : public kingdom::cTaskKingdom
{
public:
	cTaskEnemyMan();                    // DOL 0x801bb9fc
	~cTaskEnemyMan() override;          // DOL 0x800e7fa0
	virtual void Init(int nStage, cTaskScene* pScene, cTaskPacman* pPacman);   // DOL 0x800e6244 (vtable slot 13; cTaskBossEnemyMan 0x800e7368)
	void Spawn(const sScnRecord* pRec, int nType, int nOrbit, u32 nParam2);   // DOL 0x800e6d24
	cOrbitDataMan* pOrbitDataMan() { return &m_OrbitDataMan; }
	const sEnemyParam& Param(int nType) const { return m_Param[(unsigned)nType < 15 ? nType : 0]; }   // DOL 0x801c1218
	cTaskScene* pScene() { return m_pScene; }
	cTaskMissile* GetMissile();         // pool +0x1f4 (cMissileWork)
	cTaskBomb* GetBomb();               // pool +0x1e4: vtable +0x10 = 0x800e2968 (round robin from the last one taken)
	bool NearestTarget(const float eye[3], const float view[3], float out[3]) const;   // devtools: --g01-autoaim (not 1:1)
	virtual void MoreTargets(void (*fn)(const cObject*, void*), void* ctx) const { (void)fn; (void)ctx; }   // devtools: the boss stage's targets
	cTaskBullet* GetBullet(bool bHit);   // pool +0x1ec (cBulletDummyWork) / +0x1f0 (cBulletHitWork): a free entry or nullptr (tWorkObject slot 2)
	float m_vTarget[4]{0, 0, 0, 1};     // +0x1c0 Pac-Man's position (cTaskPacman 0x800f53cc copies it every frame): facing target

	static cTaskEnemyMan* s_pInstance;  // g_pTaskEnemyMan (DAT_80479048)

private:
	cTaskScene* m_pScene{};             // +0x1d0
	cTaskPacman* m_pPacman{};           // +0x1d4
	cOrbitDataMan m_OrbitDataMan;       // +0x1d8
	sEnemyParam m_Param[15]{};          // +0x1dc (cEnemyParam)
	std::vector<cTaskEnemy*> m_Enemy;   // +0x1e0 tWorkObject<cTaskEnemy>
	std::vector<cTaskBullet*> m_Bullet[2];   // +0x1ec dummy, +0x1f0 hit
	std::vector<cTaskMissile*> m_Missile;    // +0x1f4
	std::vector<cTaskFighter*> m_Fighter;    // +0x1e8 tWorkObject<cTaskFighter> (0x800e6c60)
	u32 m_nFighterNext{};
	u32 m_nEnemyNext{};                      // tWorkObject<cTaskEnemy> +0 (GetFreeWork round robin)
	std::vector<cTaskBomb*> m_Bomb;          // +0x1e4 tWorkObject<cTaskBomb> (0x800e6b9c)
	u32 m_nBombNext{};                       // the pool's +0: the round-robin start of GetFreeWork (0x800e2968)
};
}
