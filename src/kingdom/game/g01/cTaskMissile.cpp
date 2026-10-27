#include "kingdom/game/g01/cTaskMissile.hpp"
#include "kingdom/game/g01/cTaskNodeEffect.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "KingdomGlobal.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskConsolePlayer.hpp"
#include "kingdom/game/g01/cTaskEnemy.hpp"
#include "snd/Mixer.hpp"

namespace kingdom::game::g01
{

// cTaskMissile's Term 0x800f02d4: the sound actor (+0x384) and the shared bullet parameters (DAT_80479054, freed with the last missile,
// DAT_80479050 counts them) are released (natively owned by the stage).

namespace
{
constexpr float HIT_RADIUS = 0.35f;     // FLOAT_8047b670
constexpr float TURN_START = 0.03f;     // FLOAT_8047b664
constexpr float TURN_ADD = 0.0004f;     // FLOAT_8047b66c
constexpr float FLASH = 15.0f;          // FLOAT_8047b674
constexpr float OUT_DIST2 = 22500.0f;   // the cTaskBullet out test (0x800d24b0)

void Normalize(float v[3])
{
	const float l = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (l > 0.0f)
		for (int i = 0; i < 3; i++) v[i] /= l;
}

void Effect(int slot, const Vector4& pos)
{
	(new cKingdomPtkHnd())->Generate((uint)slot, true, false, true, 0.0f, pos);
}
}

// DOL 0x800f01b0 (kind 8, the shoot marker child, the jet effect child (0x800e06dc, unbound); MISSILE_HP of Bullet_s%x.csv, loaded once per stage
// in the DOL into DAT_80479054)
cTaskMissile::cTaskMissile()
{
	SetName("g01::cTaskMissile");
	nDrawPriority(0x10004u);
	m_nKind = 8;
	m_b08 = false;
	m_pMarker = new cTaskShootMarker(this);
	ConnectChild(m_pMarker);
	m_pJet = new cTaskNodeEffect(nullptr);
	ConnectChild(m_pJet);
	cKingdomModel* m = CreateModel(0);
	m->RegistModel("CHAR_MISSILE01_MDL", 0x36f);
	m->SetLightSetIdx(1);
	m->SetFogIdx(0);
	char name[0x80];
	std::snprintf(name, sizeof name, "Bullet_s%x.csv", KingdomGlobal::GalagaSys->nStage());
	kingdom::cKingdomCSV csv;
	csv.RegistData(name);
	csv.S32(csv.SearchRow(0, "MISSILE_HP", 0, 0), 1, m_nMissileHp);
	if (const wii::Mdl0* mdl = m->pMdl0())
	{
		float mn[3] = {1e30f, 1e30f, 1e30f}, mx[3] = {-1e30f, -1e30f, -1e30f};
		for (const auto& mesh : mdl->meshes)
			for (const auto& v : mesh.verts)
				for (int i = 0; i < 3; i++) { mn[i] = std::min(mn[i], v.pos[i]); mx[i] = std::max(mx[i], v.pos[i]); }
		if (mn[0] <= mx[0]) SetLocalAABB(mn, mx);
	}
}

// DOL 0x800f03e0
void cTaskMissile::Setup(float fSpeed, const float vPos[3], const float vDir[3])
{
	m_nState = 1;
	m_nStep = 0;
	m_v20 = Vector4(vPos[0], vPos[1], vPos[2], 1.0f);
	m_fSpeed = fSpeed;
	m_nHp = m_nMissileHp;
	m_fFlash = 0.0f;
	for (int i = 0; i < 3; i++) m_vDir[i] = vDir[i];
	Normalize(m_vDir);
	m_fTurn = TURN_START;
	m_b08 = true;
	m_b14 = true;
	UpdateModel();
	// the jet bound to node ef_jet (0x800e0ecc, PTR_s_ef_jet_80477920) and started: slot 0x2c, looping, not dependent, clipped (0x800e0fe4)
	m_pJet->SetPos([this](float o[3]) { float mm[16]; if (!cTaskNodeEffect::ModelNodeMtx(pModel(0), "ef_jet", mm)) return false;
	                                   o[0] = mm[12]; o[1] = mm[13]; o[2] = mm[14]; return true; });
	m_pJet->SetMtx([this](float mm[16]) { return cTaskNodeEffect::ModelNodeMtx(pModel(0), "ef_jet", mm); });
	m_pJet->Start(0x2cu, false, false, true);
}

// DOL 0x800f0358
void cTaskMissile::Launch()
{
	m_nState = 1;   // 0x800d2838
	m_nStep = 0;
	if (m_pMarker != nullptr) m_pMarker->Show();
}

// DOL 0x800f04dc: turn toward Pac-Man by min(angle, +0x388) about dir x to-Pac-Man, face along dir, move, the turn limit grows
void cTaskMissile::Step()
{
	Normalize(m_vDir);
	float to[3] = {0.0f, 0.0f, 0.0f};
	if (cTaskEnemyMan::s_pInstance != nullptr)
		for (int i = 0; i < 3; i++) to[i] = cTaskEnemyMan::s_pInstance->m_vTarget[i] - (&m_v20.x)[i];
	Normalize(to);
	float c = m_vDir[0] * to[0] + m_vDir[1] * to[1] + m_vDir[2] * to[2];
	c = std::max(-1.0f, std::min(1.0f, c));
	const float ang = std::max(-m_fTurn, std::min(m_fTurn, std::acos(c)));
	float axis[3] = {m_vDir[1] * to[2] - m_vDir[2] * to[1], m_vDir[2] * to[0] - m_vDir[0] * to[2], m_vDir[0] * to[1] - m_vDir[1] * to[0]};
	const float al = std::sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
	if (al > 1e-6f)
	{
		for (float& v : axis) v /= al;
		// Rodrigues (FUN_80298f20 = PSMTXRotAxisRad)
		const float s = std::sin(ang), k = 1.0f - std::cos(ang);
		const float d = axis[0] * m_vDir[0] + axis[1] * m_vDir[1] + axis[2] * m_vDir[2];
		const float cr[3] = {axis[1] * m_vDir[2] - axis[2] * m_vDir[1], axis[2] * m_vDir[0] - axis[0] * m_vDir[2], axis[0] * m_vDir[1] - axis[1] * m_vDir[0]};
		for (int i = 0; i < 3; i++) m_vDir[i] = m_vDir[i] * std::cos(ang) + cr[i] * s + axis[i] * d * k;
	}
	Normalize(m_vDir);
	// +0x30 = the angles of dir (0x801c1ccc; UNVERIFIED sign convention, the model's +z points along dir)
	const float len = std::sqrt(m_vDir[0] * m_vDir[0] + m_vDir[2] * m_vDir[2]);
	m_v30 = Vector4(-std::atan2(m_vDir[1], len), std::atan2(m_vDir[0], m_vDir[2]), 0.0f, 0.0f);
	for (int i = 0; i < 3; i++) (&m_v20.x)[i] += m_vDir[i] * m_fSpeed;
	m_fTurn += TURN_ADD;
}

// DOL 0x800f0b3c (+ the marker's state 2 of vtable +0xd4 = 0x800f039c, which the hit callback and the out test reach first)
void cTaskMissile::Die()
{
	if (m_pMarker != nullptr) m_pMarker->Hide();
	m_b08 = false;
	m_b14 = false;
	if (m_pJet != nullptr) m_pJet->Stop(false);   // FUN_8008a508(+0x380, 0)
}

void cTaskMissile::UpdateModel()
{
	cKingdomModel* m = pModel(0);
	if (m == nullptr) return;
	m->vTrans() = Vector4(m_v20.x, m_v20.y, m_v20.z, 1.0f);
	m->vRotate() = Vector4(m_v30.x, m_v30.y, m_v30.z, 0.0f);
	if (0.0f < m_fFlash) m_fFlash -= 1.0f;   // 0x800f0b74 (the fog flash itself is not drawn natively)
	UpdateWorldAABB();
}

// DOL 0x800f0bdc (active, not paused): state 1 = step +0xa8 (0x800f04dc), +0xb4 (0x800f0ba4: previous position + 3D sound actor), out
// test +0xb0 -> +0xd4; state 2 = +0xac (0x800f0b3c: inactive + jet effect detached; natively Die() runs at the hit, one frame early)
bool cTaskMissile::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08 || KingdomGlobal::GalagaSys->bUnk9()) return true;   // 0x800f0bdc: nothing once Pac-Man missed (0x800afe2c)
	if (m_nState == 1)
	{
		Step();
		m_SndActor.SetPosition(m_v20);   // 0x800f0bc4 (in +0xb4): the sound actor (+0x384) at +0x20
		float eye[3], aim[3], fovy;
		kingdom::NativeScene::GetCamera(eye, aim, fovy);
		const float d[3] = {eye[0] - m_v20.x, eye[1] - m_v20.y, eye[2] - m_v20.z};
		if (d[0] * d[0] + d[1] * d[1] + d[2] * d[2] > OUT_DIST2) Die();
	}
	UpdateModel();
	cTaskModel::Execute(pParam);
	UpdateFrame();
	return true;
}

void cTaskMissile::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08) return;
	cTaskModel::Draw(nStatus, pParam);
}

// DOL 0x800f07a0 (no jump test here, unlike the shots)
bool cTaskMissile::HitCheck(cObject* other)
{
	if (other->nKind() != 3) return false;
	const float* p = other->vPos();
	const float d[3] = {p[0] - m_v20.x, p[1] - m_v20.y, p[2] - m_v20.z};
	const float r = HIT_RADIUS + HIT_RADIUS;
	return d[0] * d[0] + d[1] * d[1] + d[2] * d[2] <= r * r;
}

// DOL 0x800f07f8: shot by a player: HP - 1, flash, the player's beam spark (slot 0xc + player); at 0 HP the player's small explosion (slot
// player) and gone. Hitting Pac-Man: gone with PTK_BOMB_SMALL01 (slot 0). Sounds DAT_8047b630 m01se_04hit2 / 634 m01se_05bak1, all on the missile's actor (+0x384).
void cTaskMissile::HitCallback(cObject* other)
{
	const Vector4 pos(m_v20.x, m_v20.y, m_v20.z, 1.0f);
	if (other->nKind() == 1)
	{
		const cTaskConsolePlayer* player = dynamic_cast<const cTaskConsolePlayer*>(other);
		const int pl = player != nullptr ? player->nPlayer() : 0;
		m_nHp--;
		m_fFlash = FLASH;
		Effect(0xc + pl, pos);
		if (m_nHp < 1)
		{
			Die();
			Effect(pl, pos);
			m_SndActor.Play("m01se_05bak1");   // DAT_8047b634
		}
		else
		{
			m_SndActor.Play("m01se_04hit2");   // DAT_8047b630
		}
	}
	else if (other->nKind() == 3)
	{
		Die();
		Effect(0, pos);
		m_SndActor.Play("m01se_05bak1");
	}
}

}
