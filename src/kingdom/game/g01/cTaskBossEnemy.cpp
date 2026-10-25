#include "kingdom/game/g01/cTaskBossEnemy.hpp"

#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "KingdomGlobal.hpp"
#include <cmath>

#include "kingdom/game/g01/cTaskBoss.hpp"
#include "kingdom/game/g01/cTaskScene.hpp"

namespace kingdom::game::g01
{

namespace
{
constexpr float SPEED_START = 1.0f;   // FLOAT_8047b51c
constexpr float SPEED_MAX = 2.5f;     // FLOAT_8047b528
constexpr float SPEED_ADD = 0.1f;     // FLOAT_8047b52c
}

// DOL 0x800e2e0c: +0x360 = (0, 0, 0, 1), speed 0
cTaskBossEnemy::cTaskBossEnemy()
{
	SetName("g01::cTaskBossEnemy");
}

// DOL 0x800e2e2c: the enemy's SetupOrbit (0x800e1f88) with the camera-local spawn point as the record's parent translation, the boss, speed 1.
// The DOL passes the boss pointer as the formation group too (r7); natively group 0 (already "cleared" in every scene, so Leave changes
// nothing) - UNVERIFIED what the DOL's ClearGroup makes of the pointer.
void cTaskBossEnemy::Setup(const float vLocal[3], int nType, int nOrbit, cTaskBoss* pBoss)
{
	sScnRecord rec{};
	for (int i = 0; i < 3; i++) rec.vParent[i] = vLocal[i];
	SetupOrbit(&rec, nType, nOrbit, 0u);
	m_pBoss = pBoss;
	m_fSpeed = SPEED_START;
	for (int i = 0; i < 3; i++) m_vPrev[i] = m_vWorldPos[i];
}

// DOL 0x800e3264: toward the boss's ef_body (+0x3f0) by the speed, which grows by 0.1 per frame up to 2.5
void cTaskBossEnemy::SuckStep()
{
	if (m_pBoss == nullptr) return;
	const float* body = m_pBoss->vBody();
	float d[3] = {body[0] - m_vWorldPos[0], body[1] - m_vWorldPos[1], body[2] - m_vWorldPos[2]};
	const float l = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
	if (l > 0.0f)
		for (int i = 0; i < 3; i++) m_vWorldPos[i] += d[i] / l * m_fSpeed;
	if (m_fSpeed < SPEED_MAX) m_fSpeed += SPEED_ADD;
}

// DOL 0x800e4704 (active, not paused) -> 0x800e3060: +0x360 = the position; state 1 orbit (end -> +0xc4), fire, the boss inhales -> state 3;
// state 2 = +0x84 (0x800e271c: a killed enemy's death path 0x800e20f0, then gone); state 3 sucked in; the boss dead (+0xcc) -> +0xc4. The
// hit flash (+0x88 = 0x800e46b0, fog 1) is not drawn natively (as for the stage enemies).
bool cTaskBossEnemy::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08 || KingdomGlobal::GalagaSys->bUnk9()) return true;   // 0x800e4704: nothing once Pac-Man missed (0x800afe2c)
	for (int i = 0; i < 3; i++) m_vPrev[i] = m_vWorldPos[i];
	if (m_nState == 1)
	{
		if (!UpdateOrbit(1.0f)) Dead();
		if (m_nFirePattern != -1) UpdateFire();
		if (m_pBoss != nullptr && m_pBoss->IsInhaling()) Suck();   // 0x800ccc38 (boss +0x36f)
		UpdateModel();
	}
	else if (m_nState == 2)
	{
		// 0x800e271c: killed -> the death step (+0xa8 = 0x800e20f0, death clip), gone when it ran out; else gone at once
		if (!m_bKilled || DieStep()) Leave();
		UpdateModel();
	}
	else if (m_nState == 3)
	{
		SuckStep();
		UpdateModel();
	}
	if (m_pBoss != nullptr && m_pBoss->IsDead()) Dead();   // +0xcc = 0x800e2e74 (0x800cfb40)
	if (!m_b08) return true;
	if (m_nState != 0) UpdateFrame();   // +0xb8 = 0x800e2d98, 0x8009a478
	cTaskModel::Execute(pParam);
	return true;
}

// DOL 0x800e2e7c: as the stage enemy's (0x800e27b4) for the players' shots; caught by the boss (kind 0xd) -> +0xc4 (gone, no score)
void cTaskBossEnemy::HitCallback(cObject* other)
{
	if (!m_b14) return;
	if (other->nKind() == 0xd)
	{
		Dead();
		return;
	}
	cTaskEnemy::HitCallback(other);
}

}
