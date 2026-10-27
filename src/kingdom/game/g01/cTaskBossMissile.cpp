#include "kingdom/game/g01/cTaskBossMissile.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "KingdomGlobal.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskBoss.hpp"
#include "kingdom/game/g01/cTaskCamera.hpp"
#include "kingdom/game/g01/cTaskConsolePlayer.hpp"
#include "kingdom/game/g01/cTaskNodeEffect.hpp"
#include "kingdom/game/g01/cTaskPacman.hpp"
#include "snd/Mixer.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g01
{

sBulletParam cTaskBossMissile::s_Param;

namespace
{
constexpr float HIT_RADIUS = 0.35f;     // FLOAT_8047b670
constexpr float TURN_START = 0.03f;     // FLOAT_8047b664
constexpr float TURN_ADD = 0.0004f;     // FLOAT_8047b66c
constexpr float FLASH = 15.0f;          // FLOAT_8047b674
constexpr float OUT_RANGE = 150.0f;     // FLOAT_8047b678 (0x800d22ec: distance from the camera)
constexpr float SUCK_MAX = 2.5f;        // FLOAT_8047b688
constexpr float SUCK_ADD = 0.1f;        // FLOAT_8047b68c
constexpr uint FX_HIP = 0x31, FX_SPARK = 0x32, FX_BOMB = 0x33, FX_LEVEL = 0x3c;

void Normalize(float v[3])
{
	const float l = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (l > 0.0f)
		for (int i = 0; i < 3; i++) v[i] /= l;
}

void Effect(uint slot, const Vector4& pos)
{
	(new cKingdomPtkHnd())->Generate(slot, true, false, true, 0.0f, pos);
}

// the world position of a node of a model drawn at its vTrans / vRotate (the missile's ef_hip / ef_b1 / ef_b2)
bool ModelNode(cKingdomModel* m, const char* node, float out[3])
{
	float w[16], n[12];
	if (m == nullptr || !m->NodeMatrix(node, n)) return false;
	m->CalcWorld(w);
	const float p[3] = {n[3], n[7], n[11]};
	for (int i = 0; i < 3; i++) out[i] = w[i] * p[0] + w[4 + i] * p[1] + w[8 + i] * p[2] + w[12 + i];
	return true;
}
}

// DOL 0x800efef4 ("Bullet_s%x.csv", rows by name, column 1)
void sBulletParam::Load(int nStage)
{
	char name[0x80];
	std::snprintf(name, sizeof name, "Bullet_s%x.csv", nStage);
	kingdom::cKingdomCSV csv;
	if (!csv.RegistData(name)) return;
	auto s32 = [&csv](const char* key, int& v) { const int r = csv.SearchRow(0, key, 0, 0); if (r >= 0) csv.S32(r, 1, v); };
	auto f32 = [&csv](const char* key, float& v) { const int r = csv.SearchRow(0, key, 0, 0); if (r >= 0) csv.F32(r, 1, v); };
	s32("MISSILE_HP", nMissileHp);
	s32("BOSS_MISSILE_HP", nBossMissileHp);
	f32("BOSS_SP_MISSILE_SPEED_MAX", fSpSpeedMax);
	f32("BOSS_SP_MISSILE_STRUGGLE_SPEED_MAX", fSpStruggleSpeedMax);
	s32("BOSS_SP_MISSILE_LEVEL_1", nSpLevel1);
	s32("BOSS_SP_MISSILE_LEVEL_2", nSpLevel2);
	s32("BOSS_SP_MISSILE_DAMAGE", nSpDamage);
	s32("BOSS_SP_MISSILE_DAMAGE_ADD", nSpDamageAdd);
	s32("BOSS_SP_MISSILE_DAMAGE_MAX", nSpDamageMax);
	f32("BOSS_SP_MISSILE_STOP_BASE", fSpStopBase);
	f32("BOSS_SP_MISSILE_STOP_UNDER", fSpStopUnder);
	f32("BOSS_SP_MISSILE_STOP_MAX", fSpStopMax);
}

// the pool ctor (0x800e7480: CHAR_LB_MISSILE_MDL 0xf8bcc491; 0x800e7760: CHAR_LB_SP_MISSILE_MDL 0xe3ee06e7) + 0x800f0d50 (kind 0xe) /
// 0x800f1f70 (kind 0xf, the damage level effects ef_b1 / ef_b2); the jet effect at ef_hip (+0x380, the cTaskMissile part)
cTaskBossMissile::cTaskBossMissile(bool bSp) : m_bSp(bSp)
{
	SetName(bSp ? "g01::cTaskBossSpMissile" : "g01::cTaskBossMissile");
	nDrawPriority(0x10004u);
	m_nKind = bSp ? 0xf : 0xe;
	m_b08 = false;
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(bSp ? "CHAR_LB_SP_MISSILE_MDL" : "CHAR_LB_MISSILE_MDL", 0x36f);
	m->SetLightSetIdx(1);
	m->SetFogIdx(0);
	if (const wii::Mdl0* mdl = m->pMdl0())   // CalcAABB (vtable +0x14 in the setup)
	{
		float mn[3] = {1e30f, 1e30f, 1e30f}, mx[3] = {-1e30f, -1e30f, -1e30f};
		for (const auto& mesh : mdl->meshes)
			for (const auto& v : mesh.verts)
				for (int i = 0; i < 3; i++) { mn[i] = std::min(mn[i], v.pos[i]); mx[i] = std::max(mx[i], v.pos[i]); }
		if (mn[0] <= mx[0]) SetLocalAABB(mn, mx);
	}
	m_pMarker = new cTaskShootMarker(this);   // 0x800f01b0
	ConnectChild(m_pMarker);
	m_pFxHip = new cTaskNodeEffect([this](float o[3]) { return ModelNode(pModel(0), "ef_hip", o); });
	m_pFxHip->SetMtx([this](float m[16]) { return cTaskNodeEffect::ModelNodeMtx(pModel(0), "ef_hip", m); });   // the node's whole matrix (0x800e11fc)
	ConnectChild(m_pFxHip);
	if (bSp)
	{
		m_pFxB[0] = new cTaskNodeEffect([this](float o[3]) { return ModelNode(pModel(0), "ef_b1", o); });
		m_pFxB[1] = new cTaskNodeEffect([this](float o[3]) { return ModelNode(pModel(0), "ef_b2", o); });
		m_pFxB[0]->SetMtx([this](float m[16]) { return cTaskNodeEffect::ModelNodeMtx(pModel(0), "ef_b1", m); });
		m_pFxB[1]->SetMtx([this](float m[16]) { return cTaskNodeEffect::ModelNodeMtx(pModel(0), "ef_b2", m); });
		ConnectChild(m_pFxB[0]);
		ConnectChild(m_pFxB[1]);
	}
}

cTaskBossMissile::~cTaskBossMissile() = default;

// DOL 0x800f0de4 / 0x800f2074 (after the cTaskBullet setup 0x800d2174 at the origin): bom slot, BOSS_MISSILE_HP, the boss, not collidable yet,
// turn 0.03, light set 1, active. SP: the speed saved (+0x418), BOSS_SP_MISSILE_DAMAGE, level 0, no stop.
void cTaskBossMissile::Setup(float fSpeed, int nSlot, cTaskBoss* pBoss)
{
	m_nState = 0;
	m_nStep = 0;
	m_v20 = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	m_fSpeed = fSpeed;
	m_nSlot = nSlot;
	m_n18 = s_Param.nBossMissileHp;
	m_pBoss = pBoss;
	m_fFlash = 0.0f;
	m_bBroken = false;
	m_b14 = false;
	m_fTurn = TURN_START;
	m_fTime = 0.0f;
	if (cKingdomModel* m = pModel(0))
	{
		m->SetLightSetIdx(1);
		m->SetFogIdx(0);
		m->ReleaseAnmChr(0u);
		m->ReleaseAnmChr(1u);
		m->ReleaseAnmClr(0u);
	}
	m_b08 = true;
	if (m_bSp)
	{
		m_fSpeedSave = fSpeed;
		m_nDamage = s_Param.nSpDamage;
		m_nLevel = 0;
		m_fStop = 0.0f;
	}
	UpdateModel();
}

// DOL 0x800f039c
void cTaskBossMissile::Die()
{
	m_nState = 2;
	m_nStep = 0;
	if (m_pMarker != nullptr) m_pMarker->Hide();
	m_b14 = false;
}

// DOL 0x800d258c + 0x8008a508(ef_hip, 0) (0x800f16f4)
void cTaskBossMissile::Free()
{
	m_nState = 0;
	m_nStep = 0;
	m_b08 = false;
	m_b14 = false;
	if (m_pFxHip != nullptr) m_pFxHip->Stop();
}

// DOL 0x800f0edc
bool cTaskBossMissile::IsOut() const
{
	float eye[3], aim[3], fovy;
	kingdom::NativeScene::GetCamera(eye, aim, fovy);
	const float d[3] = {eye[0] - m_v20.x, eye[1] - m_v20.y, eye[2] - m_v20.z};
	if (OUT_RANGE * OUT_RANGE < d[0] * d[0] + d[1] * d[1] + d[2] * d[2]) return true;
	if (cTaskPacman::s_pInstance != nullptr && cTaskPacman::s_pInstance->nState() == 8) return true;   // 0x800f3e94
	return m_pBoss != nullptr && m_pBoss->IsDead();                                                      // 0x800cfb40
}

// DOL 0x800f1490 (normal) / 0x800f224c (SP)
void cTaskBossMissile::Fly()
{
	cTaskCamera* cam = KingdomGlobal::GalagaSys->pTaskCamera();
	cKingdomModel* m = pModel(0);
	if (m_nStep == 0)
	{
		if (m != nullptr)
		{
			m->ReleaseAnmClr(0u);   // 0x8001af88(model, 0)
			if (m_bSp) m->RegistAnm(0u, "CHAR_LB_SP_MISSILE_ANM_CHA_01");
		}
		if (m_pFxHip != nullptr) m_pFxHip->Start(FX_HIP, false, m_bSp, true);
		// the launch direction: the bom matrix without its translation times (0, 0, 1) (DAT_80374ce0 / 80374d00)
		float bom[16];
		if (m_pBoss != nullptr && m_pBoss->BomMatrix(m_nSlot, bom))
			for (int i = 0; i < 3; i++) m_vDir[i] = bom[8 + i];
		Normalize(m_vDir);
		if (cam != nullptr) cam->WorldToLocal(m_vLocal, &m_v20.x);
		m_b14 = true;
		m_nStep++;
	}
	else if (m_nStep == 1)
	{
		if (!m_bSp)
			Home(1.0f);
		else
		{
			if (0.0f < m_fStop || m_bBroken)
			{
				m_fSpeed = 0.0f;
				if (0.0f < m_fStop) m_fStop -= 1.0f;
				if (m_bBroken && m != nullptr && m->GetFrameMaxAnmChr(1u) <= m->GetFrameAnmChr(1u))
				{
					m_bBroken = false;
					m->ReleaseAnmChr(1u);
					if (m_Spark.IsValid()) m_Spark.Detach(true);
				}
			}
			if (m_fStop <= 0.0f && !m_bBroken) m_fSpeed = m_fSpeedSave;
			Home(1.0f);
			if (m_pBoss != nullptr && m_pBoss->IsInhaling())   // 0x8004... (boss +0x36f): into the mouth (vtable +0xe4 = 0x800f2bc4)
			{
				m_fSpeed = 1.0f;
				m_nState = 4;
				m_nStep = 0;
			}
		}
	}
	if (cam != nullptr) cam->LocalToWorld(&m_v20.x, m_vLocal);
	if (IsOut()) Die();
}

// DOL 0x800f10f8: turn toward Pac-Man (in the camera frame) by at most +0x388, move, face along the direction
void cTaskBossMissile::Home(float fStep)
{
	cTaskCamera* cam = KingdomGlobal::GalagaSys->pTaskCamera();
	if (cam == nullptr) return;
	Normalize(m_vDir);
	cam->WorldDirToLocal(m_vDir, m_vDir);
	float target[3] = {0, 0, 0};
	if (cTaskPacman::s_pInstance != nullptr) cam->WorldToLocal(target, cTaskPacman::s_pInstance->vPos());
	float to[3] = {target[0] - m_vLocal[0], target[1] - m_vLocal[1], target[2] - m_vLocal[2]};
	Normalize(to);
	float c = m_vDir[0] * to[0] + m_vDir[1] * to[1] + m_vDir[2] * to[2];
	c = std::max(-1.0f, std::min(1.0f, c));
	const float ang = std::max(-m_fTurn, std::min(m_fTurn, std::acos(c)));
	float axis[3] = {m_vDir[1] * to[2] - m_vDir[2] * to[1], m_vDir[2] * to[0] - m_vDir[0] * to[2], m_vDir[0] * to[1] - m_vDir[1] * to[0]};
	const float al = std::sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
	if (al > 1e-6f)
	{
		for (float& v : axis) v /= al;
		// FUN_80298f20 = PSMTXRotAxisRad (Rodrigues)
		const float s = std::sin(ang), k = 1.0f - std::cos(ang);
		const float d = axis[0] * m_vDir[0] + axis[1] * m_vDir[1] + axis[2] * m_vDir[2];
		const float cr[3] = {axis[1] * m_vDir[2] - axis[2] * m_vDir[1], axis[2] * m_vDir[0] - axis[0] * m_vDir[2], axis[0] * m_vDir[1] - axis[1] * m_vDir[0]};
		float r[3];
		for (int i = 0; i < 3; i++) r[i] = m_vDir[i] * std::cos(ang) + cr[i] * s + axis[i] * d * k;
		for (int i = 0; i < 3; i++) m_vDir[i] = r[i];
	}
	Normalize(m_vDir);
	for (int i = 0; i < 3; i++) m_vLocal[i] += m_vDir[i] * m_fSpeed;
	cam->LocalDirToWorld(m_vDir, m_vDir);
	// +0x30 = the angles of the direction (0x801c1ccc, as cTaskMissile)
	const float len = std::sqrt(m_vDir[0] * m_vDir[0] + m_vDir[2] * m_vDir[2]);
	m_v30 = Vector4(-std::atan2(m_vDir[1], len), std::atan2(m_vDir[0], m_vDir[2]), 0.0f, 0.0f);
	m_fTime += fStep;
	m_fTurn += TURN_ADD;
}

// DOL 0x800f2620 (SP state 4): straight at the boss's ef_body, speeding up to 2.5
void cTaskBossMissile::SuckIn()
{
	if (m_pBoss == nullptr) return;
	const float* b = m_pBoss->vBody();
	float d[3] = {b[0] - m_v20.x, b[1] - m_v20.y, b[2] - m_v20.z};
	Normalize(d);
	m_v20 = Vector4(m_v20.x + d[0] * m_fSpeed, m_v20.y + d[1] * m_fSpeed, m_v20.z + d[2] * m_fSpeed, 1.0f);
	if (m_fSpeed < SUCK_MAX) m_fSpeed += SUCK_ADD;
}

// DOL 0x800f172c (normal: shot down -> CHA_02 / CLA_02 until the colour clip ends; the clip-end tests read "max <= frame": the native
// frame of a one-time clip stops at max, NW4R's keeps running) / 0x800f26fc (SP: the effects stop)
void cTaskBossMissile::DieStep()
{
	cKingdomModel* m = pModel(0);
	if (m_bSp)
	{
		m_b14 = false;
		for (cTaskNodeEffect* e : m_pFxB) if (e != nullptr) e->Stop();
		if (m_Spark.IsValid()) m_Spark.Detach(true);
		Free();
		return;
	}
	if (!m_bBroken)
	{
		m_b14 = false;
		Free();
		return;
	}
	if (m_nStep == 0)
	{
		if (m != nullptr)
		{
			m->RegistAnm(0u, "CHAR_LB_MISSILE_ANM_CHA_02");
			m->RegistAnm(0u, "CHAR_LB_MISSILE_ANM_CLA_02");
		}
		m_b14 = false;
		m_nStep++;
	}
	else if (m_nStep == 1)
	{
		if (m == nullptr || m->GetFrameMaxMatClr(0u) <= m->GetFrameMatClr(0u)) Free();
	}
}

// DOL 0x800f1f20 / 0x800f25cc (+ the SP's spark follows it)
void cTaskBossMissile::UpdateModel()
{
	if (cKingdomModel* m = pModel(0))
	{
		m->vTrans() = Vector4(m_v20.x, m_v20.y, m_v20.z, 1.0f);
		m->vRotate() = Vector4(m_v30.x, m_v30.y, m_v30.z, 0.0f);
	}
	if (m_bSp && m_Spark.IsValid()) m_Spark.vSetTrans(Vector4(m_v20.x, m_v20.y, m_v20.z, 1.0f));
	m_SndActor.SetPosition(m_v20);   // 0x800f1f40 / 0x800f25ec: the sound actor (+0x384) at +0x20
}

// DOL 0x800f1b84 (active, not paused, Pac-Man has not missed)
bool cTaskBossMissile::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08 || KingdomGlobal::GalagaSys->bUnk9()) return true;   // 0x800f1b84: nothing once Pac-Man missed (0x800afe2c)
	for (int i = 0; i < 3; i++) m_vPrev[i] = (&m_v20.x)[i];
	cKingdomModel* m = pModel(0);
	switch (m_nState)
	{
	case 1: Fly(); break;
	case 2: DieStep(); break;
	case 3:   // 0x800f1418 / 0x800f21d4: the hanging clips once
		if (m_nStep == 0 && m != nullptr)
		{
			m->RegistAnm(0u, "CHAR_LB_MISSILE_SET_ANM_CHA_01");
			m->RegistAnm(0u, m_bSp ? "CHAR_LB_SP_MISSILE_ANM_CLA_01" : "CHAR_LB_MISSILE_ANM_CLA_01");
			m_nStep++;
		}
		break;
	case 4:
		if (m_bSp) SuckIn();
		break;
	default: break;
	}
	if (!m_b08) return true;
	// 0x800f0b74: the hit flash (fog 1) runs out
	if (m != nullptr)
	{
		if (m_fFlash <= 0.0f) m->SetFogIdx(0);
		else m_fFlash -= 1.0f;
	}
	float bom[16];
	if (m_nState == 3 && m_pBoss != nullptr && m_pBoss->BomMatrix(m_nSlot, bom))
	{
		// hanging: at the boss's bom node (+0x20 = the matrix times (0, 0, 0, 1))
		m_v20 = Vector4(bom[12], bom[13], bom[14], 1.0f);
		if (m != nullptr)
		{
			Matrix4x4 rm;
			for (int c = 0; c < 4; c++)
				for (int r = 0; r < 4; r++) rm.m[r * 4 + c] = bom[c * 4 + r];
			m->SetNativeMtx(rm);
			m->SetNativeScale(std::sqrt(bom[0] * bom[0] + bom[1] * bom[1] + bom[2] * bom[2]), std::sqrt(bom[4] * bom[4] + bom[5] * bom[5] + bom[6] * bom[6]),
			                  std::sqrt(bom[8] * bom[8] + bom[9] * bom[9] + bom[10] * bom[10]));
		}
	}
	else
	{
		if (m != nullptr)
		{
			m->ClearNativeMtx();
			m->SetNativeScale(1.0f, 1.0f, 1.0f);
		}
		UpdateModel();
	}
	UpdateWorldAABB();
	if (m_nState != 0 && m != nullptr) m->UpdateFrame();   // 0x8001ded4
	cTaskModel::Execute(pParam);
	return true;
}

// DOL 0x800f1e40
void cTaskBossMissile::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08) return;
	cTaskModel::Draw(nStatus, pParam);
}

// DOL 0x800f07a0
bool cTaskBossMissile::HitCheck(cObject* other)
{
	if (other->nKind() != 3) return false;
	const float* p = other->vPos();
	const float d[3] = {p[0] - m_v20.x, p[1] - m_v20.y, p[2] - m_v20.z};
	const float r = HIT_RADIUS + HIT_RADIUS;
	return d[0] * d[0] + d[1] * d[1] + d[2] * d[2] <= r * r;
}

// DOL 0x800f1820 (normal) / 0x800f2764 (SP). Sounds DAT_8047b630 m01se_04hit2, 634 m01se_05bak1, 638 m01se_07bak3, all on the missile's actor (+0x384).
void cTaskBossMissile::HitCallback(cObject* other)
{
	const Vector4 pos(m_v20.x, m_v20.y, m_v20.z, 1.0f);
	const int kind = other->nKind();
	if (kind == 1)
	{
		const cTaskConsolePlayer* player = dynamic_cast<const cTaskConsolePlayer*>(other);
		const int pl = player != nullptr ? player->nPlayer() : 0;
		if (!m_bSp)
		{
			m_n18--;
			if (cKingdomModel* m = pModel(0)) m->SetFogIdx(1);
			m_fFlash = FLASH;
			Effect((uint)(pl + 0xc), pos);
			if (m_n18 < 1)
			{
				m_bBroken = true;
				Die();
				Effect((uint)(pl + 4), pos);
				m_SndActor.Play("m01se_05bak1");
			}
			else
				m_SndActor.Play("m01se_04hit2");
			return;
		}
		// SP: more damage carried, a stop of (STOP_BASE - damage) frames (at least STOP_UNDER), at most STOP_MAX in all
		m_nDamage += s_Param.nSpDamageAdd;
		m_fStop += std::max(s_Param.fSpStopBase - (float)m_nDamage, s_Param.fSpStopUnder);   // vtable +0x104 = 0x800f202c
		if (s_Param.fSpStopMax < m_fStop) m_fStop = s_Param.fSpStopMax;
		if (!m_bBroken)
		{
			if (cKingdomModel* m = pModel(0)) m->RegistAnm(1u, "CHAR_LB_SP_MISSILE_ANM_CHA_02");
			m_bBroken = true;
		}
		Effect((uint)(pl + 0xc), pos);
		if (!m_Spark.IsValid()) m_Spark.Generate(FX_SPARK, false, true, true, 0.0f, pos);
		if (m_nLevel == 0)
		{
			if (s_Param.nSpLevel1 < m_nDamage)
			{
				if (m_pFxB[0] != nullptr) m_pFxB[0]->Start(FX_LEVEL, false, true, true);
				m_nLevel++;
			}
		}
		else if (m_nLevel == 1)
		{
			if (s_Param.nSpLevel2 < m_nDamage)
			{
				if (m_pFxB[1] != nullptr) m_pFxB[1]->Start(FX_LEVEL, false, true, true);
				m_nLevel++;
			}
		}
		else if (s_Param.nSpDamageMax < m_nDamage)
			m_nDamage = s_Param.nSpDamageMax;
	}
	else if (kind == 3)
	{
		Die();
		Effect(FX_BOMB, pos);
		m_SndActor.Play(m_bSp ? "m01se_07bak3" : "m01se_05bak1");
	}
	else if (kind == 0xd)
	{
		Die();
		if (m_bSp)
		{
			Effect(FX_BOMB, pos);
			m_SndActor.Play("m01se_07bak3");
		}
	}
}

}
