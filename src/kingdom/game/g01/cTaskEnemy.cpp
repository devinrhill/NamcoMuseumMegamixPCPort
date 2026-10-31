#include "kingdom/MslRand.hpp"
#include "kingdom/game/g01/cTaskEnemy.hpp"
#include "kingdom/game/g01/cTaskEnemyGate.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "KingdomGlobal.hpp"
#include "platform/devtools.hpp"
#include "platform/devtools_g01edit.hpp"
#include "cJetCSV.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g01/cTaskBomb.hpp"
#include "kingdom/game/g01/cTaskBullet.hpp"
#include "kingdom/game/g01/cTaskFighter.hpp"
#include "kingdom/game/g01/cTaskCamera.hpp"
#include "kingdom/game/g01/cTaskEffect.hpp"
#include "kingdom/game/g01/cTaskMissile.hpp"
#include "kingdom/game/g01/cTaskCollisionMan.hpp"
#include "kingdom/game/g01/cTaskConsolePlayer.hpp"
#include "wii/Mdl0.hpp"
#include "kingdom/game/g01/cTaskScene.hpp"
#include "snd/Mixer.hpp"
#include "wii/Archive.hpp"

namespace kingdom::game::g01
{

cTaskEnemyMan* cTaskEnemyMan::s_pInstance = nullptr;

namespace
{
u32 Be32(const u8* p) { return (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3]; }
float BeF(const u8* p) { u32 v = Be32(p); float f; std::memcpy(&f, &v, 4); return f; }

enum { eINTERP_LINEAR = 0, eINTERP_HERMITE = 1, eINTERP_JUMP = 2, eINTERP_HOLD = 3 };
enum { eFACE_NODE = 0, eFACE_TARGET = 1, eFACE_FROM_CAMERA = 2, eFACE_CAMERA = 3 };

// DOL 0x80372c10 [audited]: model per enemy type; 0x80372c50: its clips {cha, cla, tpa, tsa} (only these are non-zero)
const char* const MODEL[15] = {"CHAR_ENEMY_ZAKO_MDL", "CHAR_ENEMY_GOEI_MDL", "CHAR_ENEMY_BOSS_MDL", "CHAR_ENEMY_GYALBOSS_MDL",
                               "CHAR_ENEMY_ROOT_MDL", "CHAR_ENEMY_QUEEN_MDL", "CHAR_ENEMY_MIDORI_MDL", "CHAR_ENEMY_MOMIZI_MDL",
                               "CHAR_ENEMY_BOMB_MDL", "CHAR_ENEMY_QUEEN_MDL", "CHAR_FIGHTER_MDL", "CHAR_ENEMY_LASTBOSS_MDL",
                               "CHAR_ENEMY_GATE_MDL", "CHAR_ENEMY_SPIKE_MDL", "CHAR_ENEMY_SPIKEMASTER_MDL"};
const char* const CLIP[15][4] = {
	{"CHAR_ENEMY_ZAKO_ANM_CHA_01", nullptr, nullptr, nullptr}, {"CHAR_ENEMY_GOEI_ANM_CHA_01", nullptr, nullptr, nullptr},
	{"CHAR_ENEMY_BOSS_ANM_CHA_01", nullptr, nullptr, nullptr}, {"CHAR_ENEMY_GYALBOSS_ANM_CHA_01", nullptr, nullptr, nullptr},
	{"CHAR_ENEMY_ROOT_ANM_CHA_01", nullptr, nullptr, nullptr}, {"CHAR_ENEMY_QUEEN_ANM_CHA_01", nullptr, nullptr, nullptr},
	{"CHAR_ENEMY_MIDORI_ANM_CHA_01", nullptr, nullptr, nullptr}, {"CHAR_ENEMY_MOMIZI_ANM_CHA_01", nullptr, nullptr, nullptr},
	{"CHAR_ENEMY_BOMB_ANM_CHA_01", nullptr, nullptr, "CHAR_ENEMY_BOMB_ANM_TSA_01"}, {"CHAR_ENEMY_QUEEN_ANM_CHA_01", nullptr, nullptr, nullptr},
	{nullptr, nullptr, nullptr, nullptr}, {"CHAR_ENEMY_LASTBOSS_ANM_CHA_01", nullptr, nullptr, nullptr},
	{"CHAR_ENEMY_GATE_ANM_CHA_01", "CHAR_ENEMY_GATE_ANM_CLA_01", "CHAR_ENEMY_GATE_ANM_TPA_01", nullptr},
	{"CHAR_ENEMY_SPIKE_ANM_CHA_03", "CHAR_ENEMY_SPIKE_ANM_CLA_03", nullptr, nullptr},
	{"CHAR_ENEMY_SPIKEMASTER_ANM_CHA_03", "CHAR_ENEMY_SPIKEMASTER_ANM_CLA_03", nullptr, nullptr}};

// DOL 0x80373138 [audited] (0x24 bytes per stage): pool sizes {enemy, bullet dummy, bullet hit, missile, ?, ?, bomb, fighter, ?}; the enemy column
u32 EnemyPoolSize(int nStage)
{
	static const u32 n[11] = {0, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x01};   // stage 0..10 (stage 10 = the boss app)
	return (u32)nStage < 11 ? n[nStage] : 0x50;
}

// DOL 0x801c1b24 [audited]
void OrbitLerp(float t, float out[4], const float a[4], const float b[4])
{
	for (int i = 0; i < 3; i++) out[i] = a[i] + (b[i] - a[i]) * t;
	out[3] = 1.0f;
}

// DOL 0x801c1be8 [audited]: cubic Hermite
void OrbitHermite(float t, float out[4], const float p0[4], const float m0[4], const float p1[4], const float m1[4])
{
	const float t2 = t * t, t3 = t2 * t;
	const float h00 = 1.0f + (2.0f * t3 - 3.0f * t2), h10 = t - (2.0f * t2 - t3), h01 = 3.0f * t2 - 2.0f * t3, h11 = t3 - t2;
	for (int i = 0; i < 3; i++) out[i] = h11 * m1[i] + h10 * m0[i] + h00 * p0[i] + h01 * p1[i];
	out[3] = 1.0f;
}

// DOL 0x801c1dbc [audited: atan2 in radians = the DOL's 256-step atan2 * 2pi/256]: pitch / yaw of from -> to (the DOL's atan2 returns 256 steps per turn; plain atan2 here)
void OrbitLookAngle(float out[4], const float from[3], const float to[3])
{
	const float d[3] = {from[0] - to[0], from[1] - to[1], from[2] - to[2]};
	const float yaw = std::atan2(d[0], d[2]);
	const float len = std::sqrt(d[0] * d[0] + d[2] * d[2]);
	const float pitch = std::atan2(d[1], len);
	out[0] = -pitch;
	out[1] = yaw;
	out[2] = 0.0f;
	out[3] = 1.0f;
}

// rotation about one axis (FUN_80298d00 with 'X' / 'Y' / 'Z'), 4x4 row major, column vectors
void RotAxis(float m[16], char axis, float a)
{
	const float c = std::cos(a), s = std::sin(a);
	for (int i = 0; i < 16; i++) m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
	if (axis == 'X') { m[5] = c; m[6] = -s; m[9] = s; m[10] = c; }
	else if (axis == 'Y') { m[0] = c; m[2] = s; m[8] = -s; m[10] = c; }
	else { m[0] = c; m[1] = -s; m[4] = s; m[5] = c; }
}
void Concat(const float a[16], const float b[16], float out[16])
{
	float r[16];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
		{
			float v = 0.0f;
			for (int k = 0; k < 4; k++) v += a[i * 4 + k] * b[k * 4 + j];
			r[i * 4 + j] = v;
		}
	std::memcpy(out, r, sizeof r);
}
}

// ---- cOrbitDataMan ---------------------------------------------------------------------------------------------------------------

// DOL 0x801c12dc [audited: port also trims trailing blanks of the .emy name]
void cOrbitDataMan::Load(int nStage)
{
	m_Orbit.clear();
	char name[0x80];
	std::snprintf(name, sizeof name, "EnemyOrbit_s%x.csv", nStage);
	kingdom::cKingdomCSV csv;
	csv.RegistData(name);
	for (int row = 0; row < csv.nRow(); row++)
	{
		string emy;
		if (!csv.Str(row, 1, emy, 0x80) || emy.empty()) break;
		while (!emy.empty() && (emy.back() == '\r' || emy.back() == ' ')) emy.pop_back();
		cOrbit orbit;
		std::size_t size = 0;
		const u8* d = wii::ArchiveStore::Find(emy, size);
		if (d == nullptr) std::fprintf(stderr, "g01::cOrbitDataMan: %s not found\n", emy.c_str());
		for (std::size_t o = 0; d != nullptr && o + 0x40 <= size; o += 0x40)
		{
			sEmyNode n{};
			for (int i = 0; i < 4; i++)
			{
				n.vPos[i] = BeF(d + o + 4 * i);
				n.vRot[i] = BeF(d + o + 0x10 + 4 * i);
				n.vTangent[i] = BeF(d + o + 0x20 + 4 * i);
			}
			n.fFrame = BeF(d + o + 0x30);
			n.nCmd = Be32(d + o + 0x34);
			n.nParam = Be32(d + o + 0x38);
			n.n3c = Be32(d + o + 0x3c);
			orbit.nodes.push_back(n);
		}
		m_Orbit.push_back(std::move(orbit));
	}
	devtools::G01EditOrbits(nStage, m_Orbit);   // devtools: the Galaga editor's applied orbits replace the table (not 1:1)
}

cOrbit* cOrbitDataMan::pOrbit(u32 n)
{
	return n < m_Orbit.size() ? &m_Orbit[n] : nullptr;
}

// ---- cTaskEnemy ------------------------------------------------------------------------------------------------------------------

// DOL 0x800e3640 [audited]: kind 0xb, priority 0x10005
cTaskEnemy::cTaskEnemy()
{
	SetName("g01::cTaskEnemy");
	m_nKind = 0xb;
	nDrawPriority(0x10005u);
	m_b08 = false;
}

// DOL 0x800e1ca8 [audited]: model and clips of the type (light set 1, fog 0, clip rates 1.0; types 12..14 (gate / spike) set a model flag, slot 6)
void cTaskEnemy::LoadModel(int nType)
{
	if (nType < 0 || nType >= 15) return;
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(MODEL[nType], 0x36f);
	m->SetLightSetIdx(1);
	m->SetFogIdx(0);
	if (CLIP[nType][0] != nullptr) { m->RegistAnm(0u, CLIP[nType][0]); m->SetUpdateRateAnmChr(0u, 1.0f); }
	if (CLIP[nType][1] != nullptr) { m->RegistAnm(0u, CLIP[nType][1]); m->SetUpdateRateMatClr(0u, 1.0f); }
	if (CLIP[nType][2] != nullptr) { m->RegistAnm(0u, CLIP[nType][2]); m->SetUpdateRateAnmTexPat(0u, 1.0f); }
	if (CLIP[nType][3] != nullptr) { m->RegistAnm(0u, CLIP[nType][3]); m->SetUpdateRateAnmTexSrt(0u, 1.0f); }
	if (nType >= 12 && nType <= 14) m->IsDrawShadowDepth(true);   // vtable +0x20 (1) at 0x800e1e90: only the gate and the spikes cast a depth shadow
	// CalcAABB (0x801c1780): min / max over the model's position arrays (natively: every vertex of every shape, unskinned)
	if (const wii::Mdl0* mdl = m->pMdl0())
	{
		float mn[3] = {1e30f, 1e30f, 1e30f}, mx[3] = {-1e30f, -1e30f, -1e30f};
		for (const wii::Mdl0Mesh& mesh : mdl->meshes)
			for (const wii::Mdl0Vertex& v : mesh.verts)
				for (int i = 0; i < 3; i++) { mn[i] = std::min(mn[i], v.pos[i]); mx[i] = std::max(mx[i], v.pos[i]); }
		if (mn[0] <= mx[0]) SetLocalAABB(mn, mx);
	}
}

// DOL 0x800e37a0 [audited]
void cTaskEnemy::StartOrbit()
{
	m_fOrbitTime = 0.0f;
	m_nOrbitIdx = 0;
	m_pCurNode = (m_pOrbit != nullptr && m_pOrbit->nNumNode() != 0) ? &m_pOrbit->nodes[0] : nullptr;
	m_nOrbitIdx++;
	m_pNextNode = pOrbitNode(m_nOrbitIdx);
}

// DOL 0x800e3860 [audited]
const sEmyNode* cTaskEnemy::pOrbitNode(u32 n) const
{
	return (m_pOrbit != nullptr && n < m_pOrbit->nNumNode()) ? &m_pOrbit->nodes[n] : nullptr;
}

// DOL 0x800e1f88 [audited: the EnemyParam row, AABB and the sound actor reset are the port's own]. The EnemyParam row (0x800e3800: +0x258 kind, life, +0x84, size +0x260, +0x264), the AABB are not ported; the sound actor's velocity reset (0x80026660) has no native effect (no Doppler).
void cTaskEnemy::SetupOrbit(const sScnRecord* pRec, int nType, int nOrbit, u32 nGroup)
{
	m_nState = 0;
	m_nStep = 0;
	m_nFirePattern = -1;
	m_nType = nType;
	m_nGroup = nGroup;
	const sEnemyParam& prm = cTaskEnemyMan::s_pInstance->Param(nType);   // 0x800e3800
	m_nSize = prm.nSize;
	m_nLife = prm.nLife;
	m_nScore = prm.nScore;
	m_fBulletSpeed = prm.fBulletSpeed;
	m_fMissileSpeed = prm.fMissileSpeed;
	m_nFireStep = 0;
	for (int i = 0; i < 3; i++) m_vBasePos[i] = pRec->vParent[i];
	m_vBasePos[3] = 1.0f;
	m_pOrbit = cTaskEnemyMan::s_pInstance->pOrbitDataMan()->pOrbit((u32)nOrbit);
	for (int i = 0; i < 3; i++) m_vOrbitPos[i] = m_vOrbitRot[i] = 0.0f;
	m_vOrbitPos[3] = 1.0f;
	StartOrbit();
	DeleteModel(0);
	LoadModel(nType);
	m_b08 = true;
	m_b14 = true;
	if (m_pCurNode != nullptr)
	{
		float local[3];
		for (int i = 0; i < 3; i++) local[i] = m_vBasePos[i] + m_pCurNode->vPos[i];
		KingdomGlobal::GalagaSys->pTaskCamera()->LocalToWorld(m_vWorldPos, local);
		for (int i = 0; i < 3; i++) m_vRot[i] = m_pCurNode->vRot[i];
	}
	UpdateModel();
}

// DOL 0x800e2da8 [audited: Start also clears m_bKilled (the DOL does it in SetupOrbit's reset)] (the appear sound DAT_8047b518 = m01se_12hirai on the enemy's sound actor)
void cTaskEnemy::Start()
{
	m_nState = 1;
	m_nStep = 0;
	m_bKilled = false;
	m_SndActor.Play("m01se_12hirai");
}

// DOL 0x800e22c4 [audited] (hand-reversed in the old tree, checked against the DOL structure): false at the end of the orbit
bool cTaskEnemy::UpdateOrbit(float fStep)
{
	cTaskCamera* cam = KingdomGlobal::GalagaSys->pTaskCamera();
	bool bAgain;
	do
	{
		const sEmyNode* cur = m_pCurNode;
		const sEmyNode* next = m_pNextNode;
		if (cur == nullptr || next == nullptr) return false;
		bool bPassed = false;
		bAgain = false;
		const u32 nInterp = cur->nCmd & 0xffff;
		const u32 nFacing = cur->nCmd >> 28;
		if (nInterp == eINTERP_JUMP)
		{
			m_nOrbitIdx = (cur->nCmd >> 16) & 0xfff;
			m_pCurNode = cur = pOrbitNode(m_nOrbitIdx);
			m_nOrbitIdx++;
			m_pNextNode = next = pOrbitNode(m_nOrbitIdx);
			if (cur == nullptr || next == nullptr) return false;
			std::memcpy(m_vOrbitPos, cur->vPos, sizeof m_vOrbitPos);
			if (nFacing == eFACE_NODE) std::memcpy(m_vOrbitRot, cur->vRot, sizeof m_vOrbitRot);
			m_fOrbitTime = cur->fFrame;
		}
		else if (nInterp == eINTERP_LINEAR || nInterp == eINTERP_HERMITE)
		{
			float t = next->fFrame - cur->fFrame;
			if (t != 0.0f) t = (m_fOrbitTime - cur->fFrame) / t;
			if (nInterp == eINTERP_LINEAR)
				OrbitLerp(t, m_vOrbitPos, cur->vPos, next->vPos);
			else
				OrbitHermite(t, m_vOrbitPos, cur->vPos, cur->vTangent, next->vPos, next->vTangent);
			if (nFacing == eFACE_NODE) OrbitLerp(t, m_vOrbitRot, cur->vRot, next->vRot);
			bPassed = next->fFrame <= m_fOrbitTime;
		}
		else if (nInterp == eINTERP_HOLD)
		{
			std::memcpy(m_vOrbitPos, cur->vPos, sizeof m_vOrbitPos);
			if (nFacing == eFACE_NODE) std::memcpy(m_vOrbitRot, cur->vRot, sizeof m_vOrbitRot);
			m_nFirePattern = (s32)cur->nParam;
			bPassed = next->fFrame <= m_fOrbitTime;
		}
		// world position: the camera frame of (base + orbit position)
		float local[3];
		for (int i = 0; i < 3; i++) local[i] = m_vBasePos[i] + m_vOrbitPos[i];
		cam->LocalToWorld(m_vWorldPos, local);
		switch (nFacing)
		{
		case eFACE_NODE:   // the model faces along the view (aim -> eye); the node rotation is applied on top (m_vOrbitRot)
			OrbitLookAngle(m_vRot, cam->vAim(), cam->vPos());
			break;
		case eFACE_TARGET:
			OrbitLookAngle(m_vRot, cTaskEnemyMan::s_pInstance->m_vTarget, m_vWorldPos);
			break;
		case eFACE_FROM_CAMERA:
			OrbitLookAngle(m_vRot, cam->vPos(), m_vWorldPos);
			break;
		case eFACE_CAMERA:
			OrbitLookAngle(m_vRot, m_vWorldPos, cam->vPos());
			break;
		}
		if (bPassed)
		{
			m_nOrbitIdx++;
			m_pCurNode = m_pNextNode;
			m_pNextNode = pOrbitNode(m_nOrbitIdx);
			if (m_pNextNode == nullptr) return false;
			if (next->fFrame <= m_fOrbitTime) bAgain = true;
		}
	} while (bAgain);
	m_fOrbitTime += fStep;
	return true;
}

// DOL 0x800e2200 [audited] (hand-reversed in the old tree): 2 = a random 0 / 1; 3 = a missile
void cTaskEnemy::UpdateFire()
{
	if (m_nFirePattern == 2) m_nFirePattern = (kingdom::MslRand() & 1) ? 0 : 1;
	switch (m_nFirePattern)
	{
	case 0: FireBullet(); break;
	case 1: FireSignBullet(); break;
	case 3: FireMissile(); break;
	default: break;
	}
}

// DOL 0x800e3888 [audited]
void cTaskEnemy::FireBullet()
{
	if (cTaskBullet* b = cTaskEnemyMan::s_pInstance->GetBullet(false))
	{
		b->Setup(m_fBulletSpeed, m_vWorldPos);   // 0x800d2174, then slot 49
		m_SndActor.Play("m01se_09shot");         // DAT_8047b544 on the actor (+0x344)
	}
	m_nFirePattern = -1;
}

// DOL 0x800e39c0 [audited]: step 0 the attack sign (PTK_ENEMY_ATTACK_SIGN01, slot 0x13) on the enemy; step 1 it follows him and, once it is gone,
// a BulletHit flies (sounds DAT_8047b548 m01se_10keikoku / DAT_8047b544 m01se_09shot)
void cTaskEnemy::FireSignBullet()
{
	const Vector4 pos(m_vWorldPos[0], m_vWorldPos[1], m_vWorldPos[2], 1.0f);
	if (m_nFireStep == 0)
	{
		if (m_pSign == nullptr) m_pSign = new cKingdomPtkHnd();
		m_pSign->Generate((uint)cTaskEffect::eENEMY_ATTACK_SIGN01, true, true, true, 0.0f, pos);
		snd::Play("m01se_10keikoku");   // DAT_8047b548
		m_nFireStep++;
	}
	else if (m_nFireStep == 1)
	{
		m_pSign->vSetTrans(pos);
		if (!m_pSign->IsValid())
		{
			if (cTaskBullet* b = cTaskEnemyMan::s_pInstance->GetBullet(true))
			{
				b->Setup(m_fBulletSpeed, m_vWorldPos);
				m_SndActor.Play("m01se_09shot");   // DAT_8047b544 on the actor (the warning above is unpositioned)
			}
			m_nFireStep = 0;
			m_nFirePattern = -1;
		}
	}
}

// DOL 0x800e3bd4 [audited]: as 0x800e39c0, the sign first; then a missile (0x800f03e0) with the missile speed along the launch direction
void cTaskEnemy::FireMissile()
{
	const Vector4 pos(m_vWorldPos[0], m_vWorldPos[1], m_vWorldPos[2], 1.0f);
	if (m_nFireStep == 0)
	{
		if (m_pSign == nullptr) m_pSign = new cKingdomPtkHnd();
		m_pSign->Generate((uint)cTaskEffect::eENEMY_ATTACK_SIGN01, true, true, true, 0.0f, pos);
		snd::Play("m01se_10keikoku");
		m_nFireStep++;
	}
	else if (m_nFireStep == 1)
	{
		m_pSign->vSetTrans(pos);
		if (!m_pSign->IsValid())
		{
			if (cTaskMissile* mi = cTaskEnemyMan::s_pInstance->GetMissile())
			{
				mi->Setup(m_fMissileSpeed, m_vWorldPos, m_vLaunch);
				mi->Launch();   // vtable +0xcc
				m_SndActor.Play("m01se_11missile");   // DAT_8047b54c on the actor
			}
			m_nFireStep = 0;
			m_nFirePattern = -1;
		}
	}
}

// DOL 0x800e271c [audited] (the not-shot-down part; shot down, the caller runs the death step +0xa8 first): the attack sign (+0x2a0) is killed if it
// still lives (FUN_8008a508(+0x2a0, 1)); gone; the scene learns that its group is cleared
void cTaskEnemy::Leave()
{
	if (!m_bKilled && m_pSign != nullptr && m_pSign->IsValid()) m_pSign->Detach(true);
	m_b08 = false;
	if (cTaskScene* scene = cTaskEnemyMan::s_pInstance->pScene()) scene->ClearGroup(m_nGroup);   // 0x800f8480
}

// DOL 0x800e40cc [audited: the orbit-node rotation product is 0x800e2ac8 (marked there)] / 0x800e2ac8: T(pos) * Rz Ry Rx (facing) * Rz Ry Rx (orbit node rotation)
void cTaskEnemy::UpdateModel()
{
	cKingdomModel* m = pModel(0);
	if (m == nullptr) return;
	float mtx[16], r[16];
	for (int i = 0; i < 16; i++) mtx[i] = (i % 5 == 0) ? 1.0f : 0.0f;
	mtx[3] = m_vWorldPos[0];
	mtx[7] = m_vWorldPos[1];
	mtx[11] = m_vWorldPos[2];
	const float* rots[2] = {m_vRot, m_vOrbitRot};
	for (const float* rv : rots)
	{
		if (rv[2] != 0.0f) { RotAxis(r, 'Z', rv[2]); Concat(mtx, r, mtx); }
		if (rv[1] != 0.0f) { RotAxis(r, 'Y', rv[1]); Concat(mtx, r, mtx); }
		if (rv[0] != 0.0f) { RotAxis(r, 'X', rv[0]); Concat(mtx, r, mtx); }
	}
	// 0x800e3ed8 (draw path, r5 = 0): +0x270 = normalize(rotation part * DAT_80372f10 (0, 1, -1.625))
	{
		const float v[3] = {0.0f, 1.0f, -1.625f};
		for (int i = 0; i < 3; i++) m_vLaunch[i] = mtx[i * 4 + 0] * v[0] + mtx[i * 4 + 1] * v[1] + mtx[i * 4 + 2] * v[2];
		const float l = std::sqrt(m_vLaunch[0] * m_vLaunch[0] + m_vLaunch[1] * m_vLaunch[1] + m_vLaunch[2] * m_vLaunch[2]);
		if (l > 0.0f)
			for (float& x : m_vLaunch) x /= l;
	}
	Matrix4x4 M;
	std::memcpy(M.m, mtx, sizeof mtx);
	m->SetNativeMtx(M);
	m->SetNativeScale(1.0f, 1.0f, 1.0f);
	m_v20 = Vector4(m_vWorldPos[0], m_vWorldPos[1], m_vWorldPos[2], 1.0f);   // cObject +0x20 / +0x30 (0x800e40cc copies them to the model)
	m_v30 = Vector4(m_vRot[0], m_vRot[1], m_vRot[2], 0.0f);
	m_SndActor.SetPosition(m_v20);   // 0x800e41ec: the sound actor (+0x344) at +0x20
	UpdateWorldAABB();   // 0x801c1950 at the end of the exec
}

// DOL 0x800e431c [audited]: the beam spark of a hit: Zako / Goei (type < 2) PTK_BEAM06 (slot 0x43), the others the player's PTK_BEAM0x (0xc + player)
void cTaskEnemy::HitSpark()
{
	const uint slot = m_nType < 2 ? 0x43u : (uint)(0xc + m_nHitPlayer);
	(new cKingdomPtkHnd())->Generate(slot, true, false, true, 0.0f, Vector4(m_vWorldPos[0], m_vWorldPos[1], m_vWorldPos[2], 1.0f));
}

// DOL 0x800e46b0 [audited]
void cTaskEnemy::FlashStep()
{
	if (0.0f < m_fFlash) m_fFlash -= 1.0f;   // FLOAT_8047b550 / 8047b554
	else if (cKingdomModel* m = pModel(0)) m->SetFogIdx(0);
}

// DOL 0x800e4494 [audited: the DOL also routes the cue to the player's remote speaker (801c15ac, not ported)]: the explosion sound by enemy size (2 bak2, 3 bak3, 4 bak4, else bak1; DAT_8047b534..540)
void cTaskEnemy::DeathSound()
{
	static const char* const CUE[5] = {"m01se_05bak1", "m01se_05bak1", "m01se_06bak2", "m01se_07bak3", "m01se_08bak4"};
	snd::Play(CUE[(unsigned)m_nSize < 5 ? m_nSize : 1]);
}

// DOL 0x800e4220 [audited]: the explosion by enemy size (EnemyParam column 1): 1 -> PTK_BOMB_SMALL0x, 2 -> PTK_BOMB_MIDDLE0x (x = player + 1),
// else PTK_BOMB_BIG01; one-shot, clipped, at the model position. The DOL fills a local cKingdomPtkHnd (0x44 bytes) and lets it go.
void cTaskEnemy::HitEffect(int nPlayer)
{
	const int slot = m_nSize == 1 ? nPlayer : m_nSize == 2 ? nPlayer + 4 : 8;
	cKingdomPtkHnd* h = new cKingdomPtkHnd();   // the native handle outlives the call (cf. cAppG03PacnRollGame::SetEffect)
	const Vector4 pos(m_vWorldPos[0], m_vWorldPos[1], m_vWorldPos[2], 1.0f);
	h->Generate((uint)slot, true, false, true, 0.0f, pos);
}

// DOL 0x800e27b4 [audited]: a player's shot (kind 1) or a bomb (kind 0xc, which hands its player: task vtable +0x54 = 0x800c9d54). The spark (+0x70),
// the hit flash (fog 1 for FLOAT_8047b524 = 15 frames), type 9 (Queen 2) does not lose life; killed: score, kill count, and an enemy of type
// 8 or 5 drops a bomb (pool +0x1e4) at its offset to the camera aim point (pTaskCamera +0x220), then the death (+0xc4); else the hit sound (+0x78)
void cTaskEnemy::HitCallback(cObject* other)
{
	if (!m_b14) return;
	cTaskConsolePlayer* player = other->nKind() == 0xc ? static_cast<cTaskBomb*>(other)->pOwner() : dynamic_cast<cTaskConsolePlayer*>(other);
	if (player == nullptr) return;
	m_nHitPlayer = player->nPlayer();   // +0x288 (the player's +0x228)
	HitSpark();                         // +0x70 = 0x800e431c
	if (cKingdomModel* m = pModel(0)) m->SetFogIdx(1);   // FUN_8001e400
	m_fFlash = 15.0f;                   // FLOAT_8047b524
	if (m_nType != 9) m_nLife--;
	if (m_nLife < 1)
	{
		m_bKilled = true;             // +0x348: the death path (0x800e20f0) instead of a plain leave
		player->AddScore(m_nScore);   // 0x800d73cc
		player->AddKill();            // +0x234
		if (std::getenv("MEGAMIX_G01DBG")) std::fprintf(stderr, "g01 kill %p type %d group %u\n", (void*)this, m_nType, m_nGroup);
		if (m_nType == 8 || m_nType == 5)
			if (cTaskBomb* bomb = cTaskEnemyMan::s_pInstance != nullptr ? cTaskEnemyMan::s_pInstance->GetBomb() : nullptr)
			{
				const cTaskCamera* cam = KingdomGlobal::GalagaSys->pTaskCamera();
				float off[3] = {m_v20.x, m_v20.y, m_v20.z};
				if (cam != nullptr) for (int i = 0; i < 3; i++) off[i] -= cam->vAim()[i];   // PSVECSubtract(+0x20, camera +0x220)
				bomb->Start(player, off);   // 0x800c9a78
				if (std::getenv("MEGAMIX_G01DBG")) std::fprintf(stderr, "g01 bomb from type %d (frame %d)\n", m_nType, devtools::FrameNo()); // devtools: frame number
			}
		Dead();                       // +0xc4
	}
	else
	{
		snd::Play("m01se_04hit2");    // +0x78 = 0x800e4634 (DAT_8047b530)
	}
}

// DOL 0x800e1eb4 [audited]: the death clips of the enemy type (0x80372d3c, two CRC32 names per type: bone clip, colour clip; 0 = none), both in
// slot 0 at rate 1 (0x8001cad4 / 0x8001cd4c)
void cTaskEnemy::DeathClip(int nType)
{
	static const char* const CLIP[15][2] = {
		{"CHAR_ENEMY_ZAKO_ANM_CHA_02", "CHAR_ENEMY_ZAKO_ANM_CLA_02"},               // 0
		{"CHAR_ENEMY_GOEI_ANM_CHA_02", "CHAR_ENEMY_GOEI_ANM_CLA_02"},               // 1
		{"CHAR_ENEMY_BOSS_ANM_CHA_02", "CHAR_ENEMY_BOSS_ANM_CLA_02"},               // 2
		{"CHAR_ENEMY_GYALBOSS_ANM_CHA_02", "CHAR_ENEMY_GYALBOSS_ANM_CLA_02"},       // 3
		{"CHAR_ENEMY_ROOT_ANM_CHA_02", "CHAR_ENEMY_ROOT_ANM_CLA_02"},               // 4
		{"CHAR_ENEMY_QUEEN_ANM_CHA_02", "CHAR_ENEMY_QUEEN_ANM_CLA_02"},             // 5
		{"CHAR_ENEMY_MIDORI_ANM_CHA_02", "CHAR_ENEMY_MIDORI_ANM_CLA_02"},           // 6
		{"CHAR_ENEMY_MOMIZI_ANM_CHA_02", "CHAR_ENEMY_MOMIZI_ANM_CLA_02"},           // 7
		{"CHAR_ENEMY_BOMB_ANM_CHA_02", "CHAR_ENEMY_BOMB_ANM_CLA_02"},               // 8
		{"CHAR_ENEMY_QUEEN_ANM_CHA_02", "CHAR_ENEMY_QUEEN_ANM_CLA_02"},             // 9
		{"CHAR_FIGHTER_ANM_CHA_02", "CHAR_FIGHTER_ANM_CLA_02"},                     // 10
		{nullptr, nullptr},                                                         // 11 (the last boss)
		{"CHAR_ENEMY_GATE_ANM_CHA_02", "CHAR_ENEMY_GATE_ANM_CLA_02"},               // 12
		{"CHAR_ENEMY_SPIKE_ANM_CHA_02", "CHAR_ENEMY_SPIKE_ANM_CLA_02"},             // 13
		{"CHAR_ENEMY_SPIKEMASTER_ANM_CHA_02", "CHAR_ENEMY_SPIKEMASTER_ANM_CLA_02"}, // 14
	};
	cKingdomModel* m = pModel(0);
	if (m == nullptr || (unsigned)nType >= 15) return;
	if (CLIP[nType][0] != nullptr)
	{
		m->RegistAnm(0u, CLIP[nType][0]);   // FUN_8001a4d8(model, 0, crc, 0, 1)
		m->SetUpdateRateAnmChr(0u, 1.0f);   // FLOAT_8047b51c
	}
	if (CLIP[nType][1] != nullptr)
	{
		m->RegistAnm(0u, CLIP[nType][1]);
		m->SetUpdateRateMatClr(0u, 1.0f);
	}
}

// DOL 0x800e20f0 [audited]: step 0 the death clips (+0xb0 with the type), the explosion (+0x6c) and its sound (+0x74), the enemy's sound stopped
// (+0x34c, no 3D actors natively) and the attack sign detached (+0x2a0); step 1 waits for the colour clip to end (0x8001c804 max < 0x8001c42c
// frame; natively "<=" since the port's one-time clips stop at max), then inactive
bool cTaskEnemy::DieStep()
{
	if (m_nStep == 0)
	{
		DeathClip(m_nType);
		HitEffect(m_nHitPlayer);
		DeathSound();
		// the enemy's sound handle +0x34c is stopped (0x80026424 / 0x80026318: its sounds are not kept natively)
		if (m_pSign != nullptr && m_pSign->IsValid()) m_pSign->Detach(true);
		m_nStep++;
	}
	else if (m_nStep == 1)
	{
		cKingdomModel* m = pModel(0);
		if (m == nullptr || m->GetFrameMaxMatClr(0u) <= m->GetFrameMatClr(0u))
		{
			m_b08 = false;
			return true;
		}
	}
	return false;
}

// DOL 0x800e2de0 [audited]
void cTaskEnemy::Dead()
{
	m_nState = 2;
	m_nStep = 0;
	m_b14 = false;
}

// DOL 0x800e4704 -> 0x800e29c8 (nothing while paused)
bool cTaskEnemy::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08 || KingdomGlobal::GalagaSys->bUnk9()) return true;   // 0x800e4704: nothing once Pac-Man missed (0x800afe2c)
	if (m_nState == 1)
	{
		if (!UpdateOrbit(1.0f))
		{
			Dead();   // vtable +0xc4
		}
		if (m_nFirePattern != -1) UpdateFire();   // +0x80
	}
	else if (m_nState == 2)
	{
		// +0x84 = 0x800e271c: a killed enemy runs its death step (+0xa8) and goes when the death clip ran out; else it is gone at once
		if (!m_bKilled || DieStep()) Leave();
		if (!m_b08) return true;
	}
	FlashStep();   // +0x88
	cTaskModel::Execute(pParam);
	UpdateFrame();
	UpdateModel();
	return true;
}

// The small enemy slots, inlined: +0xb4 0x800e2d88 (state / step 0), +0xb8 0x800e2d98 (state == 0), +0xc0 0x800e2dcc (state == 1), +0xc8
// 0x800e2df8 (state == 2); cTaskEnemyBase's Execute 0x800e4790 (the model task) and Draw 0x800e47e4 (only while shown), Term 0x800e369c (the
// sound actor +0x344 is released: natively the member's destructor, its cues play on where it was).
// DOL 0x800e47b8 [audited]: only while active (+8) and shown (+9; the fighter waits hidden)
void cTaskEnemy::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08 || !m_b09) return;
	cTaskModel::Draw(nStatus, pParam);
}

// ---- cTaskEnemyMan ---------------------------------------------------------------------------------------------------------------

// DOL 0x801bb9fc [audited: pools are vectors]
cTaskEnemyMan::cTaskEnemyMan()
{
	SetName("g01::cTaskEnemyMan");
}

// DOL 0x800e7fa0 [audited: pools are vectors, no free] (ReleaseStageData)
cTaskEnemyMan::~cTaskEnemyMan()
{
	if (s_pInstance == this) s_pInstance = nullptr;
}

// The pools (tWorkObject<T> / c*Work, 0x10 bytes: +0 the round-robin start, +4 count, +8 array; vtable +0xc create, +0x10 GetFreeWork (the
// first inactive entry from +0, which then points past it), +0x14 ResetWork (every entry inactive), +0x18 nUseWork (active count)):
//   enemies 0x800e69f8 / 0x800e6dd8 / 0x800e6fd4 / 0x800e80dc; dummy shots cBulletDummyWork 0x800e64ec / 0x800e3960 / 0x800e6f38 / 0x800e821c;
//   hit shots cBulletHitWork 0x800e6674 / 0x800e3b74 / 0x800e6f6c / 0x800e81dc; missiles cMissileWork 0x800e6858 / 0x800e3d8c / 0x800e6fa0 /
//   0x800e819c; bombs 0x800e6b9c / 0x800e2968 / 0x800e7008 / 0x800e805c; fighters 0x800e6c60 / 0x800e6e38 / 0x800e703c / 0x800e801c; the boss
//   stage's boss enemies 0x800e7adc / 0x800e7d24 / 0x800e7e50 / 0x800e809c, cBossMissileWork 0x800e7de8 / 0x800e815c, cBossSpMissileWork
//   0x800e7e1c / 0x800e811c. cTaskEnemyMan::ResetWork (vtable +0x40) 0x800e6e98 / cTaskBossEnemyMan 0x800e7d84 reset every pool: no caller
//   found (the apps rebuild the stage on retry). Natively the pools are vectors; bombs / fighters / enemies keep the round-robin start; the shot
//   and missile pools take the first free entry (the same entry as long as the pool is not full). cEnemyParam's dtor 0x800e5c10.
// DOL 0x800e6244 [audited]: the enemy parameters (cEnemyParam, "EnemyParam_s%xp%d.csv") and the pools (enemies, shots, missiles, bombs, fighters).
void cTaskEnemyMan::Init(int nStage, cTaskScene* pScene, cTaskPacman* pPacman)
{
	m_pScene = pScene;
	m_pPacman = pPacman;
	s_pInstance = this;
	m_OrbitDataMan.Load(nStage);
	{
		char name[0x80];
		std::snprintf(name, sizeof name, "EnemyParam_s%xp%x.csv", nStage, KingdomGlobal::GalagaSys->nPlayer());
		kingdom::cKingdomCSV csv;
		csv.RegistData(name);
		for (int r = 0; r < 15; r++)
		{
			sEnemyParam& p = m_Param[r];
			csv.S32(r, 1, p.nSize);
			csv.S32(r, 2, p.nLife);
			csv.F32(r, 3, p.fBulletSpeed);
			csv.F32(r, 4, p.fMissileSpeed);
			csv.S32(r, 5, p.nScore);
		}
	}
	const u32 n = EnemyPoolSize(nStage);
	cTaskCollisionMan* col = KingdomGlobal::GalagaSys->pTaskCollisionMan();
	// bullet pools: 0x28 dummy and 0x14 hit shots (table 0x80373138 columns 1 / 2; stage 10's row 0x803732a0 too)
	for (int k = 0; k < 2; k++)
	{
		const u32 nb = k == 0 ? 0x28u : 0x14u;
		for (u32 i = 0; i < nb; i++)
		{
			cTaskBullet* b = new cTaskBullet(k == 1);
			m_Bullet[k].push_back(b);
			ConnectChild(b);
			if (col != nullptr) col->Regist(b);
		}
	}
	const u32 nm = (nStage == 10) ? 1u : 0x14u;   // missile pool (table 0x80373138 column 3)
	for (u32 i = 0; i < nm; i++)
	{
		cTaskMissile* mi = new cTaskMissile();
		m_Missile.push_back(mi);
		ConnectChild(mi);
		if (col != nullptr) col->Regist(mi);
	}
	// the fighter pool (0x800e6c60: table 0x80373154 + stage * 0x24 = 2), inactive, registered for collision
	for (u32 i = 0; i < 2u; i++)
	{
		cTaskFighter* f = new cTaskFighter();
		m_Fighter.push_back(f);
		ConnectChild(f);
		if (col != nullptr) col->Regist(f);
	}
	// the bomb pool (0x800e6b9c: table 0x80373150 + stage * 0x24 = 5 on every stage), inactive, registered for collision
	for (u32 i = 0; i < 5u; i++)
	{
		cTaskBomb* b = new cTaskBomb();
		m_Bomb.push_back(b);
		ConnectChild(b);
		if (col != nullptr) col->Regist(b);
	}
	for (u32 i = 0; i < n; i++)
	{
		cTaskEnemy* e = new cTaskEnemy();
		m_Enemy.push_back(e);
		ConnectChild(e);
		if (col != nullptr) col->Regist(e);   // UNVERIFIED where the DOL registers the pool members (the pool ctor, 0x8043a464)
	}
}

// devtools: --g01-autoaim (not 1:1): the nearest active enemy / missile in front of the camera
bool cTaskEnemyMan::NearestTarget(const float eye[3], const float view[3], float out[3]) const
{
	float best = 1e30f;
	auto consider = [&](const cObject* o) {
		if (!o->bActive() || !o->bCollide()) return;
		const float* p = o->vPos();
		const float d[3] = {p[0] - eye[0], p[1] - eye[1], p[2] - eye[2]};
		if (d[0] * view[0] + d[1] * view[1] + d[2] * view[2] <= 0.0f) return;
		const float l = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
		if (l < best) { best = l; for (int i = 0; i < 3; i++) out[i] = p[i]; }
	};
	for (const cTaskEnemy* e : m_Enemy) consider(e);
	for (const cTaskMissile* m : m_Missile) consider(m);
	for (const cTaskEnemyGate* g : cTaskEnemyGate::s_Live) consider(g);   // devtools: the enemy gates too
	MoreTargets([](const cObject* o, void* ctx) { (*static_cast<decltype(consider)*>(ctx))(o); }, &consider);
	return best < 1e30f;
}

cTaskMissile* cTaskEnemyMan::GetMissile()
{
	for (cTaskMissile* m : m_Missile)
		if (m->IsFree()) return m;
	return nullptr;
}

// DOL 0x800e2968 [audited] (tWorkObject<cTaskBomb> vtable +0x10): the first inactive one from the slot after the last one taken
cTaskBomb* cTaskEnemyMan::GetBomb()
{
	const u32 n = (u32)m_Bomb.size();
	for (u32 k = 0; k < n; k++)
	{
		const u32 i = (m_nBombNext + k) % n;
		if (m_Bomb[i]->IsFree()) { m_nBombNext = i + 1; return m_Bomb[i]; }
	}
	return nullptr;
}

cTaskBullet* cTaskEnemyMan::GetBullet(bool bHit)
{
	for (cTaskBullet* b : m_Bullet[bHit ? 1 : 0])
		if (b->IsFree()) return b;
	return nullptr;
}

// DOL 0x800e6d24 [audited]: a free enemy (GetFreeWork 0x8043a458 +0x10: round robin from the last one taken) flies the orbit and starts (+0xbc);
// p2 bits 9-10: a free fighter (pool +0x1e8, 0x800e6e38) rides along (0x800ea304)
void cTaskEnemyMan::Spawn(const sScnRecord* pRec, int nType, int nOrbit, u32 nParam2)
{
	const u32 n = (u32)m_Enemy.size();
	for (u32 k = 0; k < n; k++)
	{
		const u32 i = (m_nEnemyNext + k) % n;
		cTaskEnemy* e = m_Enemy[i];
		if (!e->IsFree()) continue;
		m_nEnemyNext = i + 1;
		e->SetupOrbit(pRec, nType, nOrbit, nParam2 & 0x1ff);
		e->Start();
		if ((nParam2 >> 9 & 3) != 0)
		{
			const u32 nf = (u32)m_Fighter.size();
			for (u32 j = 0; j < nf; j++)
			{
				const u32 fi = (m_nFighterNext + j) % nf;
				if (!m_Fighter[fi]->IsFree()) continue;
				m_nFighterNext = fi + 1;
				m_Fighter[fi]->Setup(pRec, nOrbit, e);
				break;
			}
		}
		return;
	}
}

}
