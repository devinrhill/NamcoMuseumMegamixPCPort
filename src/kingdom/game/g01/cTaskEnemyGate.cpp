#include "kingdom/game/g01/cTaskEnemyGate.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskScene.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g01
{

namespace
{
constexpr float DEG2RAD = 0.017453292f;   // FLOAT_8047b558
constexpr uint GATE_BOMB01 = 0x3e;        // PTK_GATE_BOMB01

// 4x4 row major, column vectors (as cTaskEnemy.cpp)
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
// Rz Ry Rx of an angle vector (skipping zero angles like the DOL)
void RotZYX(const float r[3], float m[16])
{
	float t[16];
	for (int i = 0; i < 16; i++) m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
	if (r[2] != 0.0f) { RotAxis(t, 'Z', r[2]); Concat(m, t, m); }
	if (r[1] != 0.0f) { RotAxis(t, 'Y', r[1]); Concat(m, t, m); }
	if (r[0] != 0.0f) { RotAxis(t, 'X', r[0]); Concat(m, t, m); }
}
}

std::vector<cTaskEnemyGate*> cTaskEnemyGate::s_Live;

cTaskEnemyGate::cTaskEnemyGate()
{
	SetName("g01::cTaskEnemyGate");
	s_Live.push_back(this);   // devtools: autoaim
}

cTaskEnemyGate::~cTaskEnemyGate()
{
	s_Live.erase(std::remove(s_Live.begin(), s_Live.end(), this), s_Live.end());
}

// DOL 0x800e4814: type 12, the CSV row, the orbit (timetable) of column 1, not collidable yet, active; the model (+0xac = 0x800e514c:
// fog 3) and its AABB; the place = node translate + Rz Ry Rx(node rotate) * -(model node "root" translate); the shoot marker child
// (cTaskShootMarker 0x800f84d4: an ANT_SCORE anita 10 above the gate)
void cTaskEnemyGate::SetupGate(const wii::Mdl0Node& node, int nStage, int nIndex)
{
	m_nState = 0;
	m_nStep = 0;
	m_bKilled = false;   // 0x800e1c74
	m_nType = 12;        // +0x25c
	m_bArrived = false;
	const sEnemyParam& prm = cTaskEnemyMan::s_pInstance->Param(12);   // 0x800e3800
	m_nSize = prm.nSize;
	m_nLife = prm.nLife;
	m_nScore = prm.nScore;
	m_fBulletSpeed = prm.fBulletSpeed;
	m_fMissileSpeed = prm.fMissileSpeed;
	char name[0x80];
	std::snprintf(name, sizeof name, "GateGalaga_s%xp%d.csv", nStage, KingdomGlobal::GalagaSys->nPlayer());   // DAT_8047779c
	kingdom::cKingdomCSV csv;
	if (csv.RegistData(name))
	{
		int group = 0;
		csv.S32(nIndex - 1, 1, m_nOrbitNo);
		csv.S32(nIndex - 1, 3, group);
		csv.F32(nIndex - 1, 2, m_fStart);
		csv.F32(nIndex - 1, 4, m_fHit);
		m_nGroup = (u32)group;   // +0x340
	}
	m_pOrbit = cTaskEnemyMan::s_pInstance->pOrbitDataMan()->pOrbit((u32)m_nOrbitNo);   // FormationSpec(+0x1d8, orbit)
	m_nOrbitIdx = 0;
	m_pCurNode = pOrbitNode(m_nOrbitIdx++);
	m_pNextNode = pOrbitNode(m_nOrbitIdx);
	m_fOrbitTime = 0.0f;
	m_b14 = false;
	m_bHittable = false;
	m_b08 = true;
	DeleteModel(0);
	LoadModel(12);                              // vtable +0xac = 0x800e514c
	if (cKingdomModel* m = pModel(0)) m->SetFogIdx(3);
	const float rot[3] = {node.rotate[0] * DEG2RAD, node.rotate[1] * DEG2RAD, node.rotate[2] * DEG2RAD};
	float root[3] = {0, 0, 0};
	if (cKingdomModel* m = pModel(0))
		if (const wii::Mdl0* mdl = m->pMdl0())
			for (const wii::Mdl0Node& n : mdl->nodes)
				if (n.name == "root") for (int i = 0; i < 3; i++) root[i] = -n.translate[i];   // 0x8001dd54(model, "root"), negated
	float r[16];
	RotZYX(rot, r);
	for (int i = 0; i < 3; i++)
		m_vPlace[i] = node.translate[i] + r[i * 4 + 0] * root[0] + r[i * 4 + 1] * root[1] + r[i * 4 + 2] * root[2];
	m_vPlace[3] = 1.0f;
	for (int i = 0; i < 3; i++) m_vRot[i] = rot[i];
	m_v30 = Vector4(rot[0], rot[1], rot[2], 0.0f);
	m_v40 = Vector4(node.scale[0], node.scale[1], node.scale[2], 1.0f);
	UpdateGateModel();   // vtable +0x90
	const Vector4 ofs(0.0f, 10.0f, 0.0f, 1.0f);   // DAT_80372f68
	m_pMarker = new cTaskShootMarker(this, &ofs);
	ConnectChild(m_pMarker);
}

// DOL 0x800e5048: the enemy's state 2 (0x800e2de0), not arrived, the marker goes
void cTaskEnemyGate::Dead()
{
	cTaskEnemy::Dead();
	m_bArrived = false;
	if (m_pMarker != nullptr) m_pMarker->Hide();
}

// DOL 0x800e5234: the orbit as a timetable: a jump node (2) moves on, a hold node (3) sets the fire pattern; false at the end (the orbit
// restarts from node 0)
bool cTaskEnemyGate::UpdateTimetable(float fStep)
{
	bool bAgain;
	do
	{
		bAgain = false;
		if (m_pCurNode == nullptr || m_pNextNode == nullptr) return false;
		const u32 interp = m_pCurNode->nCmd & 0xffff;
		bool passed;
		if (interp == 2)
		{
			m_nOrbitIdx = (m_pCurNode->nCmd >> 16) & 0xfff;
			const sEmyNode* cur = pOrbitNode(m_nOrbitIdx);
			m_pCurNode = cur;
			m_nOrbitIdx++;
			m_pNextNode = pOrbitNode(m_nOrbitIdx);
			if (cur == nullptr || m_pNextNode == nullptr) return false;
			m_fOrbitTime = cur->fFrame;
			passed = false;
		}
		else
		{
			if (interp == 3) m_nFirePattern = (s32)m_pCurNode->nParam;
			passed = m_pNextNode->fFrame <= m_fOrbitTime;
		}
		if (passed)
		{
			const sEmyNode* prevNext = m_pNextNode;
			m_nOrbitIdx++;
			m_pCurNode = m_pNextNode;
			m_pNextNode = pOrbitNode(m_nOrbitIdx);
			if (m_pNextNode == nullptr)
			{
				m_nOrbitIdx = 0;
				m_pCurNode = pOrbitNode(m_nOrbitIdx++);
				m_pNextNode = pOrbitNode(m_nOrbitIdx);
				return false;
			}
			if (prevNext->fFrame <= m_fOrbitTime) bAgain = true;
		}
	} while (bAgain);
	m_fOrbitTime += fStep;
	return true;
}

// DOL 0x800e53b8: step 0 the death clips (+0xb0 = 0x800e518c: CHAR_ENEMY_GATE_ANM_CHA_02 in blend 1, CHAR_ENEMY_SPIKE_ANM_CLA_02, rates 1.0,
// fog 3), the explosion (+0x6c = 0x800e508c: PTK_GATE_BOMB01 at +0x20) and its sound (+0x74); step 1 gone when the colour clip ended
bool cTaskEnemyGate::DieStep()
{
	cKingdomModel* m = pModel(0);
	if (m_nStep == 0)
	{
		if (m != nullptr)
		{
			m->RegistAnm(1u, "CHAR_ENEMY_GATE_ANM_CHA_02");
			m->SetUpdateRateAnmChr(0u, 1.0f);
			m->RegistAnm(0u, "CHAR_ENEMY_SPIKE_ANM_CLA_02");
			m->SetUpdateRateMatClr(0u, 1.0f);
			m->SetFogIdx(3);
		}
		(new cKingdomPtkHnd())->Generate(GATE_BOMB01, true, false, true, 0.0f, Vector4(m_v20.x, m_v20.y, m_v20.z, 1.0f));
		DeathSound();
		m_fFlash = 0.0f;   // +0x370
		m_nStep++;
	}
	else if (m_nStep == 1)
	{
		// 0x8001c804 (colour clip max) < 0x8001c42c (its frame): the clip ran out
		if (m == nullptr || m->GetFrameMaxMatClr(0u) <= m->GetFrameMatClr(0u))
		{
			m_b08 = false;
			return true;
		}
	}
	return false;
}

// DOL 0x800e54f8: M = T(place) * Rz Ry Rx(+0x30) * (+0x380); +0x20 = the origin of M; the model at +0x20 while it waits / flies, at the place
// once it arrived (its clip moves it then)
void cTaskEnemyGate::UpdateGateModel()
{
	float m[16], r[16];
	for (int i = 0; i < 16; i++) m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
	m[3] = m_vPlace[0];
	m[7] = m_vPlace[1];
	m[11] = m_vPlace[2];
	const float rot[3] = {m_v30.x, m_v30.y, m_v30.z};
	RotZYX(rot, r);
	Concat(m, r, m);
	Concat(m, m_mBase, m);
	m_v20 = Vector4(m[3], m[7], m[11], 1.0f);
	for (int i = 0; i < 3; i++) m_vWorldPos[i] = (&m_v20.x)[i];
	if (cKingdomModel* mdl = pModel(0))
	{
		if (!m_bArrived) mdl->vTrans() = m_v20;
		else mdl->vTrans() = Vector4(m_vPlace[0], m_vPlace[1], m_vPlace[2], 1.0f);
		mdl->vRotate() = m_v30;
		mdl->vScale() = m_v40;
	}
	m_SndActor.SetPosition(m_v20);   // 0x800e5758: the sound actor (+0x344) at +0x20
	UpdateWorldAABB();
}

// DOL 0x800e5778 (vtable +0xa0)
bool cTaskEnemyGate::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08 || KingdomGlobal::GalagaSys->bUnk9()) return true;   // 0x800e4704: nothing once Pac-Man missed (0x800afe2c)
	cTaskModel::Execute(pParam);
	UpdateWorldAABB();   // 0x801c1950
	cTaskScene* scene = cTaskEnemyMan::s_pInstance->pScene();
	if (scene == nullptr || !scene->IsGame()) return true;   // 0x800f6a94
	const float frame = scene->fFrame();                      // 0x800f7dc0
	if (std::getenv("MEGAMIX_G01DBG") && (m_nState * 16 + m_nStep + (m_bArrived ? 8 : 0) + (m_bHittable ? 100 : 0)) != m_nDbg)
	{
		m_nDbg = m_nState * 16 + m_nStep + (m_bArrived ? 8 : 0) + (m_bHittable ? 100 : 0);
		std::fprintf(stderr, "g01 gate group %u state %d step %d arrived %d hittable %d life %d frame %.1f pos (%.1f %.1f %.1f)\n", m_nGroup, m_nState, m_nStep, m_bArrived, m_bHittable, m_nLife, frame, m_v20.x, m_v20.y, m_v20.z);
	}
	if (m_nState == 0)
	{
		if (m_fStart < frame) Start();   // vtable +0xbc
	}
	else if (m_nState == 1)
	{
		if (!m_bArrived)
		{
			if (!UpdateTimetable(1.0f))
			{
				m_bArrived = true;
				m_nStep = 0;
			}
			if (m_nFirePattern != -1) UpdateFire();   // +0x80
		}
		else
		{
			cKingdomModel* m = pModel(0);
			if (m_nStep == 0)
			{
				if (m != nullptr) m->RegistAnm(0u, "CHAR_ENEMY_GATE_ANM_CHA_03");   // 0x3f82f0c6
				m_nStep++;
			}
			else if (m_nStep == 1 && m != nullptr && m->GetFrameMaxAnmChr(0u) < m->GetFrameAnmChr(0u))
			{
				Dead();   // +0xc4
			}
			// +0x380 = the model matrix of node "base" (0x800e0254)
			float n[12];
			if (m != nullptr && m->NodeMatrix("base", n))
			{
				const float b[16] = {n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8], n[9], n[10], n[11], 0, 0, 0, 1};
				std::memcpy(m_mBase, b, sizeof b);
			}
		}
	}
	else if (m_nState == 2)
	{
		if (DieStep()) Leave();   // +0xc8 / +0xa8, then +0x84 (the formation group is cleared)
	}
	// +0x88 = 0x800e54a4: the hit flash counts down, then fog 3 again
	if (0.0f < m_fFlash) m_fFlash -= 1.0f;
	else if (cKingdomModel* m = pModel(0)) m->SetFogIdx(3);
	if (!m_bHittable && m_fHit < frame)
	{
		m_b14 = true;          // collidable from the "hit" frame on
		m_bHittable = true;
		if (m_pMarker != nullptr) m_pMarker->Show();
	}
	if (m_nState != 0) UpdateFrame();   // the clips run once it started (0x8009a478)
	UpdateGateModel();                  // +0x90
	return true;
}

}
