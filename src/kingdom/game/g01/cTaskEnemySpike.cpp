#include "kingdom/game/g01/cTaskEnemySpike.hpp"

#include <cmath>
#include <cstdio>

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskScene.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g01
{

// cTaskEnemySpike Init 0x800e825c (+0x358..+0x364 = 0, not collidable) = the member initialisers; Term 0x800e8278 frees its parameter
// object (+0x364).

namespace
{
constexpr float DEG2RAD = 0.017453292f;   // FLOAT_8047b574
constexpr float PAC_RADIUS = 0.35f;       // FLOAT_8047b580
constexpr float SPIKE_RADIUS = 1.0f;      // FLOAT_8047787c
constexpr uint SPIKE_HIT01 = 0x12;        // PTK_SPIKE_HIT01
}

cTaskEnemySpike::cTaskEnemySpike()
{
	SetName("g01::cTaskEnemySpike");
}

// DOL 0x800e8310: active, not collidable, type 13, its EnemyParam row (0x800e3800), the model and place (vtable +0xcc = 0x800e8514: model
// of type 13 with fog 3; position = node translate + Rz Ry Rx(node rotate) * -(model node "root" translate), rotation, scale), the
// SpikeGalaga_s%x.csv row nIndex (0-based)
void cTaskEnemySpike::SetupSpike(const wii::Mdl0Node& node, int nStage, int nIndex, int nModel, const char* csvFormat)
{
	m_bKilled = false;   // 0x800e1c74
	m_nState = 0;
	m_nStep = 0;
	m_b08 = true;
	m_b14 = false;
	m_bHitOn = false;
	m_nType = 13;
	m_nGrow = 0;
	DeleteModel(0);
	LoadModel(nModel);
	cKingdomModel* m = pModel(0);
	if (m != nullptr) m->SetFogIdx(3);
	const float rot[3] = {node.rotate[0] * DEG2RAD, node.rotate[1] * DEG2RAD, node.rotate[2] * DEG2RAD};
	float root[3] = {0, 0, 0};
	if (m != nullptr)
		if (const wii::Mdl0* mdl = m->pMdl0())
			for (const wii::Mdl0Node& n : mdl->nodes)
				if (n.name == "root") for (int i = 0; i < 3; i++) root[i] = -n.translate[i];   // 0x8001dd54(model, "root"), negated
	const float cx = std::cos(rot[0]), sx = std::sin(rot[0]), cy = std::cos(rot[1]), sy = std::sin(rot[1]), cz = std::cos(rot[2]), sz = std::sin(rot[2]);
	const float r[3][3] = {{cz * cy, cz * sy * sx - sz * cx, cz * sy * cx + sz * sx}, {sz * cy, sz * sy * sx + cz * cx, sz * sy * cx - cz * sx}, {-sy, cy * sx, cy * cx}};
	for (int i = 0; i < 3; i++) m_vWorldPos[i] = node.translate[i] + r[i][0] * root[0] + r[i][1] * root[1] + r[i][2] * root[2];
	m_v20 = Vector4(m_vWorldPos[0], m_vWorldPos[1], m_vWorldPos[2], 1.0f);
	m_v30 = Vector4(rot[0], rot[1], rot[2], 0.0f);
	m_v40 = Vector4(node.scale[0], node.scale[1], node.scale[2], 1.0f);
	const sEnemyParam& prm = cTaskEnemyMan::s_pInstance->Param(13);
	m_nSize = prm.nSize;
	m_nLife = prm.nLife;
	m_nScore = prm.nScore;
	char name[0x80];
	std::snprintf(name, sizeof name, csvFormat, nStage);   // DAT_80477880 (the master: DAT_80477884 "SpikeMasterGalaga_s%x.csv")
	kingdom::cKingdomCSV csv;
	if (csv.RegistData(name))
	{
		csv.F32(nIndex, 1, m_fAction);
		csv.F32(nIndex, 2, m_fHitFrom);
		csv.F32(nIndex, 3, m_fHitUntil);
	}
	if (m != nullptr)
	{
		m->vTrans() = m_v20;
		m->vRotate() = m_v30;
		m->vScale() = m_v40;
	}
	UpdateWorldAABB();
	const Vector4 ofs(0.0f, 10.0f, 0.0f, 1.0f);   // DAT_80373910
	m_pMarker = new cTaskShootMarker(this, &ofs);
	ConnectChild(m_pMarker);
}

// DOL 0x800e8e38: the enemy's state 2 (0x800e2de0), the marker goes
void cTaskEnemySpike::Dead()
{
	cTaskEnemy::Dead();
	if (m_pMarker != nullptr) m_pMarker->Hide();
}

// DOL 0x800e8934: step 0 the grow clips CHAR_ENEMY_SPIKE_ANM_CHA_04 / CLA_04 (rate 1.0); step 1 at the bone clip's end the loop CHA_01
void cTaskEnemySpike::GrowStep()
{
	cKingdomModel* m = pModel(0);
	if (m == nullptr) return;
	if (m_nGrow == 0)
	{
		m->RegistAnm(0u, "CHAR_ENEMY_SPIKE_ANM_CHA_04");
		m->SetUpdateRateAnmChr(0u, 1.0f);
		m->RegistAnm(0u, "CHAR_ENEMY_SPIKE_ANM_CLA_04");
		m->SetUpdateRateMatClr(0u, 1.0f);
		m_nGrow = 1;
	}
	else if (m_nGrow == 1 && m->GetFrameMaxAnmChr(0u) <= m->GetFrameAnmChr(0u))
	{
		m->RegistAnm(0u, "CHAR_ENEMY_SPIKE_ANM_CHA_01");
		m->SetUpdateRateAnmChr(0u, 1.0f);
		m->SetFrameAnmChr(0u, 0.0f);   // 0x8001af88
		m_nGrow = 2;
	}
}

// DOL 0x800e8cdc: Pac-Man (kind 3) touches it while not jumping (0x800f4bc4): spheres 0.35 / 1.0 (0x801c1aa4)
bool cTaskEnemySpike::HitCheck(cObject* other)
{
	if (other->nKind() != 3 || other->bJumping()) return false;
	const float* a = other->vPos();
	const float d[3] = {a[0] - m_v20.x, a[1] - m_v20.y, a[2] - m_v20.z};
	const float r = PAC_RADIUS + SPIKE_RADIUS;
	return d[0] * d[0] + d[1] * d[1] + d[2] * d[2] <= r * r;
}

// DOL 0x800e8d4c: Pac-Man ran into it -> PTK_SPIKE_HIT01 at the spike (Pac-Man's own callback takes the damage); a shot -> as any enemy
void cTaskEnemySpike::HitCallback(cObject* other)
{
	if (other->nKind() == 3)
	{
		(new cKingdomPtkHnd())->Generate(SPIKE_HIT01, true, false, true, 0.0f, Vector4(m_v20.x, m_v20.y, m_v20.z, 1.0f));
		return;
	}
	cTaskEnemy::HitCallback(other);
}

// DOL 0x800e8ab0
bool cTaskEnemySpike::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08 || KingdomGlobal::GalagaSys->bUnk9()) return true;   // 0x800e4704: nothing once Pac-Man missed (0x800afe2c)
	UpdateFrame();   // 0x8009a478
	cTaskModel::Execute(pParam);
	UpdateWorldAABB();
	cTaskScene* scene = cTaskEnemyMan::s_pInstance->pScene();
	if (scene == nullptr || !scene->IsGame()) return true;
	const float frame = scene->fFrame();
	if (m_nState == 0)
	{
		if (m_fAction < frame) { m_nState = 1; m_nStep = 0; }   // +0xbc = 0x800e8e2c (no appear sound)
	}
	else if (m_nState == 1)
		GrowStep();                                               // +0x7c
	else if (m_nState == 2)
	{
		if (DieStep()) Leave();                                   // +0xc8 / +0xa8, then +0x84
		return true;
	}
	if (!m_b14 && m_nState != 2 && !m_bHitOn && m_fHitFrom < frame)
	{
		m_b14 = true;      // collidable, the marker appears
		m_bHitOn = true;
		if (m_pMarker != nullptr) m_pMarker->Show();
	}
	if (m_b14 && m_nState != 2 && m_bHitOn && m_fHitUntil < frame)
	{
		m_b14 = false;     // the marker goes
		if (m_pMarker != nullptr) m_pMarker->Hide();
	}
	// +0x88 = 0x800e8a5c: the hit flash, then fog 3 again
	if (0.0f < m_fFlash) m_fFlash -= 1.0f;
	else if (cKingdomModel* m = pModel(0)) m->SetFogIdx(3);
	return true;
}

}
