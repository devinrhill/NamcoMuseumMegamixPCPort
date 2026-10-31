#include "kingdom/game/g01/cTaskFighter.hpp"
#include "kingdom/game/g01/IntersectionSegment3Sphere.hpp"
#include "kingdom/game/g01/cTaskScene.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "KingdomGlobal.hpp"
#include "platform/devtools.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskCollisionMan.hpp"
#include "kingdom/game/g01/cTaskNodeEffect.hpp"
#include "kingdom/game/g01/cTaskPacman.hpp"

namespace kingdom::game::g01
{

namespace
{
constexpr float HIT_RADIUS = 0.35f;      // FLOAT_8047b598
constexpr float COOKIE_RADIUS = 0.15f;   // FLOAT_8047b59c
constexpr float ROT_LIMIT_ADD = 0.02f;   // FLOAT_8047b5a4
constexpr float OUT_DIST2 = 22500.0f;    // FLOAT_8047b5a8: 150 units from the camera

// nw4r::math::IntersectionSegment3Sphere (0x80302150): the segment must cross the sphere's surface
bool SegmentSphere(const float a[3], const float b[3], const float c[3], float r)
{
	return IntersectionSegment3Sphere(a, b, c, r);
}
void Normalize(float v[3])
{
	const float l = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (l > 0.0f) for (int i = 0; i < 3; i++) v[i] /= l;
}

// Rz Ry Rx (angles in radians) applied to v
void RotateZYX(const float r[3], const float in[3], float out[3])
{
	float t[3], v[3];
	t[0] = in[0]; t[1] = std::cos(r[0]) * in[1] - std::sin(r[0]) * in[2]; t[2] = std::sin(r[0]) * in[1] + std::cos(r[0]) * in[2];
	v[0] = std::cos(r[1]) * t[0] + std::sin(r[1]) * t[2]; v[1] = t[1]; v[2] = -std::sin(r[1]) * t[0] + std::cos(r[1]) * t[2];
	out[0] = std::cos(r[2]) * v[0] - std::sin(r[2]) * v[1]; out[1] = std::sin(r[2]) * v[0] + std::cos(r[2]) * v[1]; out[2] = v[2];
}
}

// ---- cTaskCookie --------------------------------------------------------------------------------------------------------------------

// DOL 0x800e9f60 [audited] (the 0x370-byte cookie: CHAR_COOKIE_MDL 0x5fa8d58c, fighter +0x328) + 0x800e9c04: kind 4, no target, fall speed 0,
// attached, inactive, last Pac-Man position 0 (w 1). The fighter's ctor registers it for collision.
cTaskCookie::cTaskCookie(cTaskFighter* pFighter) : m_pFighter(pFighter)
{
	SetName("g01::cTaskCookie");
	m_nKind = 4;
	m_b08 = false;
	cKingdomModel* m = CreateModel(0);
	m->RegistModel("CHAR_COOKIE_MDL", 0x36f);
}

// the cookie part of 0x800ea304: target Pac-Man (enemy manager +0x1d4), COOKIE_FALL_SPEED, attached, his position, collidable, active, hidden
void cTaskCookie::Attach(float fFallSpeed)
{
	m_fFallSpeed = fFallSpeed;
	m_bAttached = true;
	if (const cTaskPacman* pac = cTaskPacman::s_pInstance)
		for (int i = 0; i < 3; i++) m_vLastPac[i] = pac->vPos()[i];
	m_b14 = true;
	m_b08 = true;
	m_b09 = false;
}

// DOL 0x800e9d4c [audited] (active, not paused): attached it sits at the fighter's ef_cooky; let go it falls toward Pac-Man and moves along with him;
// the segment of the step (+0x350); gone while Pac-Man runs to the goal (0x800f3e94)
bool cTaskCookie::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08) return true;
	const cTaskPacman* pac = cTaskPacman::s_pInstance;
	const float prev[3] = {m_v20.x, m_v20.y, m_v20.z};
	float* p = &m_v20.x;
	if (!m_bAttached)
	{
		if (pac != nullptr)
		{
			float d[3];
			for (int i = 0; i < 3; i++) d[i] = pac->vPos()[i] - p[i];
			Normalize(d);
			for (int i = 0; i < 3; i++) p[i] += d[i] * m_fFallSpeed + (pac->vPos()[i] - m_vLastPac[i]);
		}
	}
	else
	{
		for (int i = 0; i < 3; i++) p[i] = m_pFighter->vCookiePoint()[i];
	}
	for (int i = 0; i < 3; i++) { m_vSeg[i] = prev[i]; m_vSeg[3 + i] = p[i]; }
	if (pac != nullptr) for (int i = 0; i < 3; i++) m_vLastPac[i] = pac->vPos()[i];
	if (cKingdomModel* m = pModel(0)) m->vTrans() = Vector4(p[0], p[1], p[2], 1.0f);
	if (pac != nullptr && pac->IsGoalRun()) m_b08 = false;
	return cTaskModel::Execute(pParam);
}

void cTaskCookie::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08 || !m_b09) return;
	cTaskModel::Draw(nStatus, pParam);
}

// DOL 0x800e9c48 [audited]: Pac-Man (kind 3): the step's segment against his 0.35 sphere, else the 0.15 sphere against it (FUN_80302600)
bool cTaskCookie::HitCheck(cObject* other)
{
	if (other->nKind() != 3) return false;
	const float* c = other->vPos();
	if (SegmentSphere(m_vSeg, m_vSeg + 3, c, HIT_RADIUS)) return true;
	const float d[3] = {m_v20.x - c[0], m_v20.y - c[1], m_v20.z - c[2]};
	const float r = HIT_RADIUS + COOKIE_RADIUS;
	return d[0] * d[0] + d[1] * d[1] + d[2] * d[2] <= r * r;
}

// DOL 0x800e9d0c [audited]: eaten
void cTaskCookie::HitCallback(cObject* other)
{
	if (other->nKind() == 3) m_b08 = false;
}

// ---- cTaskFighter -------------------------------------------------------------------------------------------------------------------

// DOL 0x800e9f60: kind 5, inactive, no leader, wait 0; the cookie (a child, registered for collision); not let go, riding, not collidable;
// FighterParam.csv WAIT_MAX_FRAME / SPEED / ROT_LIMIT_INIT / COOKIE_FALL_SPEED (column 1); two node effects (0x800e06dc), children
cTaskFighter::cTaskFighter()
{
	SetName("g01::cTaskFighter");
	m_nKind = 5;
	m_b08 = false;
	m_pCookie = new cTaskCookie(this);
	ConnectChild(m_pCookie);
	if (cTaskCollisionMan* col = KingdomGlobal::GalagaSys->pTaskCollisionMan()) col->Regist(m_pCookie);
	m_bDelivered = false;
	m_bRiding = true;
	m_b14 = false;
	kingdom::cKingdomCSV csv;
	if (csv.RegistData("FighterParam.csv"))   // s_FighterParam_csv_80373c80
	{
		csv.F32(csv.SearchRow(0, "WAIT_MAX_FRAME", 0, 0), 1, m_Param.fWaitMax);
		csv.F32(csv.SearchRow(0, "SPEED", 0, 0), 1, m_Param.fSpeed);
		csv.F32(csv.SearchRow(0, "ROT_LIMIT_INIT", 0, 0), 1, m_Param.fRotLimitInit);
		csv.F32(csv.SearchRow(0, "COOKIE_FALL_SPEED", 0, 0), 1, m_Param.fCookieFallSpeed);
	}
	const char* node[2] = {"ef_l_boost", "ef_r_boost"};   // s_ef_l_boost_80373c91 / s_ef_r_boost_80373c9c (bound by 0x800ea304)
	for (int i = 0; i < 2; i++)
	{
		std::string n = node[i];
		m_pBoost[i] = new cTaskNodeEffect([this, n](float o[3]) { return NodeWorld(n.c_str(), o); });
		m_pBoost[i]->SetMtx([this, n](float m[16]) { return cTaskNodeEffect::ModelNodeMtx(pModel(0), n.c_str(), m); });   // 0x800e11fc
		ConnectChild(m_pBoost[i]);
	}
}

// the world position of a node of the fighter model (the model's world matrix * the node's model-space matrix)
bool cTaskFighter::NodeWorld(const char* node, float out[3])
{
	cKingdomModel* m = pModel(0);
	float w[16], a[12];
	if (m == nullptr || !m->NodeMatrix(node, a)) return false;
	m->CalcWorld(w);
	const float p[3] = {a[3], a[7], a[11]};
	for (int i = 0; i < 3; i++) out[i] = w[i] * p[0] + w[4 + i] * p[1] + w[8 + i] * p[2] + w[12 + i];
	return true;
}

// DOL 0x800ea304 [audited]: the leader's orbit as an enemy of type 10 (group 0), its start position, wait 0, SPEED / ROT_LIMIT_INIT, riding, not let
// go, collidable, the AABB of the model (+0x14), the boost effects bound to ef_l_boost / ef_r_boost and started (slot 0x14, looping,
// dependent, clipped), hidden, active; the cookie: Pac-Man, COOKIE_FALL_SPEED, attached, collidable, active, hidden
void cTaskFighter::Setup(const sScnRecord* pRec, int nOrbit, cTaskEnemy* pLeader)
{
	SetupOrbit(pRec, 10, nOrbit, 0);   // 0x800e1f88
	// the DOL copies the record's first vector (its parent position) to +0x380 (previous position) and +0x20 (the position) after SetupOrbit
	for (int i = 0; i < 3; i++) m_vWorldPos[i] = m_vPrev[i] = pRec->vParent[i];
	m_v20 = Vector4(pRec->vParent[0], pRec->vParent[1], pRec->vParent[2], 1.0f);
	m_pLeader = pLeader;
	m_fWait = 0.0f;
	m_fSpeed = m_Param.fSpeed;
	m_fRotLimit = m_Param.fRotLimitInit;
	m_bDelivered = false;
	m_bRiding = true;
	m_b14 = true;
	UpdateModel();
	for (cTaskNodeEffect* e : m_pBoost) e->Start(0x14u, false, true, true);
	m_b09 = false;
	m_b08 = true;
	m_pCookie->Attach(m_Param.fCookieFallSpeed);
	if (std::getenv("MEGAMIX_G01DBG")) std::fprintf(stderr, "g01 fighter spawned (frame %d)\n", devtools::FrameNo()); // devtools: frame number
}

// DOL 0x800ea614 [audited]: turn the flight direction toward `to` by at most the rotation limit (acos of the dot product, about their cross product,
// PSMTXRotAxisRad), move SPEED along it, face it (0x801c1ccc), the limit grows by 0.02, the orbit time by fStep
void cTaskFighter::Turn(float fStep, const float to[3])
{
	float dot = m_vDir[0] * to[0] + m_vDir[1] * to[1] + m_vDir[2] * to[2];
	dot = std::clamp(dot, -1.0f, 1.0f);
	const float ang = std::clamp(std::acos(dot), -m_fRotLimit, m_fRotLimit);
	float axis[3] = {m_vDir[1] * to[2] - m_vDir[2] * to[1], m_vDir[2] * to[0] - m_vDir[0] * to[2], m_vDir[0] * to[1] - m_vDir[1] * to[0]};
	const float al = std::sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
	if (al > 1e-6f)
	{
		for (float& v : axis) v /= al;
		const float s = std::sin(ang), c = std::cos(ang), k = 1.0f - c;
		const float d = axis[0] * m_vDir[0] + axis[1] * m_vDir[1] + axis[2] * m_vDir[2];
		const float cr[3] = {axis[1] * m_vDir[2] - axis[2] * m_vDir[1], axis[2] * m_vDir[0] - axis[0] * m_vDir[2], axis[0] * m_vDir[1] - axis[1] * m_vDir[0]};
		for (int i = 0; i < 3; i++) m_vDir[i] = m_vDir[i] * c + cr[i] * s + axis[i] * d * k;
	}
	Normalize(m_vDir);
	for (int i = 0; i < 3; i++) m_vWorldPos[i] += m_fSpeed * m_vDir[i];
	const float len = std::sqrt(m_vDir[0] * m_vDir[0] + m_vDir[2] * m_vDir[2]);
	m_vRot[0] = -std::atan2(m_vDir[1], len);
	m_vRot[1] = std::atan2(m_vDir[0], m_vDir[2]);
	m_vRot[2] = 0.0f;
	m_fOrbitTime += fStep;
	m_fRotLimit += ROT_LIMIT_ADD;
}

// DOL 0x800ea9a8 [audited] (state 2): not shot down -> the boost effects stop and it is gone; shot down -> the death step (0x800ea8b8: death clip,
// explosion, sound, the boosts stop; gone when the clip ran out). Gone: the scene learns its group (0) is cleared (0x800f8480).
// Natively the death path is the enemy's (0x800e20f0, unreachable: the fighter's hit callback never sets +0x348).
void cTaskFighter::LeaveStep()
{
	if (!m_bKilled)
	{
		for (cTaskNodeEffect* e : m_pBoost) e->Stop();
		Leave();
	}
	else if (DieStep())
	{
		for (cTaskNodeEffect* e : m_pBoost) e->Stop();
		Leave();
	}
}

// DOL 0x800eaa3c [audited: the cookie point (ef_cooky) is the model's node through its world matrix, the DOL builds T(pos) Rz Ry Rx(rot) itself; the delivered Turn step argument is whatever f1 held (1.0 assumed)] (through 0x800e4704: active, not paused, Pac-Man has not missed)
bool cTaskFighter::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08 || KingdomGlobal::GalagaSys->bUnk9()) return true;
	for (int i = 0; i < 3; i++) m_vPrev[i] = m_vWorldPos[i];   // +0x380
	if (m_nState == 0)
	{
		// hidden until WAIT_MAX_FRAME frames have passed, then shown and started (+0xbc = 0x800e2da8: state 1, the appear sound)
		if (m_fWait <= m_Param.fWaitMax) m_fWait += 1.0f;
		else
		{
			m_b09 = true;
			Start();
		}
	}
	else if (m_nState == 1)
	{
		if (m_bRiding)
		{
			// the leader is gone: shot down -> free, left -> leave too
			if (m_pLeader != nullptr && m_pLeader->IsFree())
			{
				if (!m_pLeader->bKilled()) Dead();
				else m_bRiding = false;
			}
			if (!UpdateOrbit(1.0f)) Dead();   // +0x7c
		}
		else if (!m_bDelivered)
		{
			if (m_nStep == 0)
			{
				// the flight direction = R(+0x30) * (0, 0, 1) (DAT_80373bf8)
				const float z[3] = {0.0f, 0.0f, 1.0f};
				RotateZYX(m_vRot, z, m_vDir);
				m_nStep++;
			}
			const cTaskPacman* pac = cTaskPacman::s_pInstance;
			if (pac != nullptr)
			{
				float to[3];
				for (int i = 0; i < 3; i++) to[i] = pac->vBite()[i] - m_vWorldPos[i];
				Normalize(to);
				Turn(1.0f, to);
			}
		}
		else
		{
			const float up[3] = {0.0f, 1.0f, 0.0f};
			Turn(1.0f, up);
			float eye[3], aim[3], fovy;
			kingdom::NativeScene::GetCamera(eye, aim, fovy);
			const float d[3] = {eye[0] - m_vWorldPos[0], eye[1] - m_vWorldPos[1], eye[2] - m_vWorldPos[2]};
			if (OUT_DIST2 < d[0] * d[0] + d[1] * d[1] + d[2] * d[2]) Dead();
		}
	}
	else if (m_nState == 2)
	{
		LeaveStep();   // +0x84 (the flash step, the clips and the model update still follow in the DOL)
	}
	FlashStep();   // +0x88
	if (m_nState != 0) UpdateFrame();   // 0x8009a478
	cTaskModel::Execute(pParam);
	UpdateModel();   // +0x90
	NodeWorld("ef_cooky", m_vCookie);   // s_ef_cooky_80373ca7 -> +0x390
	// +0xd0 = 0x800eb06c: this frame's segment +0x3a8 = (+0x380, +0x20)
	for (int i = 0; i < 3; i++) { m_vSeg[i] = m_vPrev[i]; m_vSeg[3 + i] = m_vWorldPos[i]; }
	return true;
}

// DOL 0x800ea4c4 [audited]: Pac-Man (kind 3): the step's segment against a 0.35 sphere at his +0x290
bool cTaskFighter::HitCheck(cObject* other)
{
	if (other->nKind() != 3 || cTaskPacman::s_pInstance == nullptr) return false;
	return SegmentSphere(m_vSeg, m_vSeg + 3, cTaskPacman::s_pInstance->vBite(), HIT_RADIUS);
}

// DOL 0x800ea550 [audited]: Pac-Man -> the cookie is let go (+0x334 = 0, shown), CHAR_FIGHTER_ANM_CHA_01 (0xcbd91d7b) in slot 0, climb away (+0x369),
// the rotation limit from ROT_LIMIT_INIT again, not collidable; a player's shot (kind 1) -> leave, not collidable
void cTaskFighter::HitCallback(cObject* other)
{
	const int k = other->nKind();
	if (k == 3)
	{
		m_pCookie->Release();
		if (cKingdomModel* m = pModel(0)) m->RegistAnm(0u, "CHAR_FIGHTER_ANM_CHA_01");   // FUN_8001a4d8(m, 0, crc, 0, 1)
		m_bDelivered = true;
		m_fRotLimit = m_Param.fRotLimitInit;
		m_b14 = false;
		if (std::getenv("MEGAMIX_G01DBG")) std::fprintf(stderr, "g01 fighter delivers its cookie (frame %d) at (%.2f %.2f %.2f), Pac-Man (%.2f %.2f %.2f) bite (%.2f %.2f %.2f)\n", devtools::FrameNo(), m_vWorldPos[0], m_vWorldPos[1], m_vWorldPos[2], other->vPos()[0], other->vPos()[1], other->vPos()[2], cTaskPacman::s_pInstance->vBite()[0], cTaskPacman::s_pInstance->vBite()[1], cTaskPacman::s_pInstance->vBite()[2]); // devtools: frame number
	}
	else if (k == 1)
	{
		Dead();
		m_b14 = false;
	}
}

}
