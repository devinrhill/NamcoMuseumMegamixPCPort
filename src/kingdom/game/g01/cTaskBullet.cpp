#include "kingdom/game/g01/cTaskBullet.hpp"

#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "KingdomGlobal.hpp"
#include <cmath>

#include "kingdom/NativeScene.hpp"
#include "kingdom/game/g01/cTaskEnemy.hpp"
#include "kingdom/game/g01/cTaskPacman.hpp"

namespace kingdom::game::g01
{

namespace
{
constexpr float HIT_RADIUS = 0.35f;     // FLOAT_8047b47c (both spheres, FUN_801c1aa4)
constexpr float OUT_DIST2 = 22500.0f;   // FLOAT_8047b478: 150 units from the camera

const float* PacmanPos()   // DAT_80479038 + 0x20 (natively the enemy manager's copy of Pac-Man's position)
{
	return cTaskEnemyMan::s_pInstance != nullptr ? cTaskEnemyMan::s_pInstance->m_vTarget : nullptr;
}
}

// DOL 0x800d2080 (kind 6; cTaskBulletHit 0x800d2da8 makes it 7) and the pools' model setup (cBulletDummyWork: CHAR_BULLET01_MDL 0x46dfb5c6;
// cBulletHitWork: CHAR_BULLET02_MDL 0x017fcf16 + clip 0x07583216). Priority 0x10004.
cTaskBullet::cTaskBullet(bool bHit)
{
	SetName(bHit ? "g01::cTaskBulletHit" : "g01::cTaskBulletDummy");
	nDrawPriority(0x10004u);
	m_nKind = bHit ? 7 : 6;
	m_b08 = false;
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(bHit ? "CHAR_BULLET02_MDL" : "CHAR_BULLET01_MDL", 0x36f);
	if (bHit) m->RegistAnm(0u, "CHAR_BULLET02_ANM_CHA_01");
	m->SetLightSetIdx(0);
	m->SetFogIdx(0);
}

// The small slots of the shot vtables (0x80439198 / 0x804392e0 / 0x80439438), inlined here: +0xa4 0x800d2118 (DAT_80479038 = 0: the
// global Pac-Man pointer is cleared; cTaskBulletDummy / Hit +0xa4 0x800d29dc / 0x800d2db4 do nothing), +0xc4 0x800d2818 (state / step 0),
// +0xc8 0x800d2828 (state == 0), +0xcc 0x800d2838 (state 1, the enemy's fire 0x800e3888 after Setup), +0xd0 0x800d284c (state == 1),
// +0xd4 0x800d2860 (state 2), +0xd8 0x800d2874 (state == 2), +0xe0 0x800d288c (state == 3), +0xe8 0x800d28a4 (state == 4); +0xb8 = the
// kind (0x800d29d0: 6, 0x800d2da8: 7). cTaskBulletDummy +0xec 0x800d29e0 (a Setup toward the aim point, not collidable) has no caller found.
// The cTaskKingdom accessors 0x80088bdc / 0x80088be4 (+0x5c / +0x54) are the engine's.
// DOL 0x800d2174: start at the enemy, aim at Pac-Man's position, active and collidable, state 1
void cTaskBullet::Setup(float fSpeed, const float vPos[3])
{
	m_nState = 1;
	m_nStep = 0;
	m_fTime = 0.0f;
	m_v20 = Vector4(vPos[0], vPos[1], vPos[2], 1.0f);
	const float* pac = PacmanPos();
	float len = 0.0f;
	for (int i = 0; i < 3; i++)
	{
		m_vDir[i] = (pac != nullptr ? pac[i] : 0.0f) - vPos[i];
		len += m_vDir[i] * m_vDir[i];
		m_vLastTarget[i] = pac != nullptr ? pac[i] : 0.0f;
	}
	len = std::sqrt(len);
	if (len > 0.0f)
		for (float& v : m_vDir) v /= len;
	m_fSpeed = fSpeed;
	m_b08 = true;
	m_b14 = true;
	UpdateModel();
}

const float* cTaskBullet::Target() const
{
	if (m_nKind == 6 && cTaskPacman::s_pInstance != nullptr) return cTaskPacman::s_pInstance->vAimPoint();   // DAT_80479038 + 0x2f0
	return PacmanPos();
}

// DOL 0x800d23b0 (cTaskBulletDummy 0x800d2ae8: the previous position to +0x340, Pac-Man's aim point +0x2f0 instead of his position): pos +=
// dir * speed + how far the target moved since the last frame (the first frame of a dummy shot: aim point - position, set by 0x800d2174)
void cTaskBullet::Step()
{
	for (int i = 0; i < 3; i++) m_vPrev[i] = (&m_v20.x)[i];
	const float* pac = Target();
	for (int i = 0; i < 3; i++)
	{
		const float carry = pac != nullptr ? pac[i] - m_vLastTarget[i] : 0.0f;
		(&m_v20.x)[i] += m_vDir[i] * m_fSpeed + carry;
		if (pac != nullptr) m_vLastTarget[i] = pac[i];
	}
	m_fTime += 1.0f;
}

// DOL 0x800d24b0: out when farther than 150 from the camera, or while Pac-Man runs to the goal (state 8, 0x800f3e94)
bool cTaskBullet::IsOut() const
{
	float eye[3], aim[3], fovy;
	kingdom::NativeScene::GetCamera(eye, aim, fovy);
	const float d[3] = {eye[0] - m_v20.x, eye[1] - m_v20.y, eye[2] - m_v20.z};
	if (d[0] * d[0] + d[1] * d[1] + d[2] * d[2] > OUT_DIST2) return true;
	return cTaskPacman::s_pInstance != nullptr && cTaskPacman::s_pInstance->IsGoalRun();
}

// DOL 0x800d258c
void cTaskBullet::Die()
{
	m_b08 = false;
	m_b14 = false;
}

void cTaskBullet::UpdateModel()
{
	cKingdomModel* m = pModel(0);
	if (m == nullptr) return;
	m->vTrans() = Vector4(m_v20.x, m_v20.y, m_v20.z, 1.0f);
}

// DOL 0x800d26b4 (nothing while paused)
bool cTaskBullet::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08 || KingdomGlobal::GalagaSys->bUnk9()) return true;   // 0x800d26b4: nothing once Pac-Man missed (0x800afe2c)
	if (m_nState == 1)
	{
		Step();                      // +0xa8 (1.0)
		if (IsOut())                 // +0xb0
		{
			m_nState = 2;            // +0xd4 = 0x800d2860
			m_nStep = 0;
		}
	}
	else if (m_nState == 2)
	{
		Die();                       // +0xac
	}
	UpdateModel();                   // +0xb4 = 0x800d2658 (model translate / rotate = +0x20 / +0x30)
	cTaskModel::Execute(pParam);
	UpdateFrame();
	return true;
}

// DOL 0x800d27f4: only while active (+8) and shown (+9)
void cTaskBullet::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08 || !m_b09) return;
	cTaskModel::Draw(nStatus, pParam);
}

// DOL 0x800d25cc (cTaskBulletDummy 0x800d2bfc tests Pac-Man's aim point +0x2f0 instead)
bool cTaskBullet::HitCheck(cObject* other)
{
	if (other->nKind() != 3 || other->bJumping()) return false;   // 0x800f4bc4
	const float* p = m_nKind == 6 ? Target() : other->vPos();     // cTaskBulletDummy 0x800d2bfc: Pac-Man's +0x2f0
	if (p == nullptr) return false;
	const float d[3] = {p[0] - m_v20.x, p[1] - m_v20.y, p[2] - m_v20.z};
	const float r = HIT_RADIUS + HIT_RADIUS;
	return d[0] * d[0] + d[1] * d[1] + d[2] * d[2] <= r * r;
}

// DOL 0x800d2648
void cTaskBullet::HitCallback(cObject* other)
{
	(void)other;
	Die();
}

}
