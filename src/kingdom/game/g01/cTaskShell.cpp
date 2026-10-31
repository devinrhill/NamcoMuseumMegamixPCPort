#include "kingdom/game/g01/cTaskShell.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskBoss.hpp"
#include "snd/Mixer.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g01
{

// cTaskShell small slots: +0x6c 0x800cfedc (state == 1, open), +0x70 0x800cfec8 (state == 2) - inlined.

namespace
{
bool RayAABB(const float org[3], const float dir[3], const float mn[3], const float mx[3], float& t)
{
	float t0 = 0.0f, t1 = 1e30f;
	for (int i = 0; i < 3; i++)
	{
		if (std::fabs(dir[i]) < 1e-12f)
		{
			if (org[i] < mn[i] || org[i] > mx[i]) return false;
			continue;
		}
		float a = (mn[i] - org[i]) / dir[i], b = (mx[i] - org[i]) / dir[i];
		if (a > b) std::swap(a, b);
		t0 = std::max(t0, a);
		t1 = std::min(t1, b);
		if (t0 > t1) return false;
	}
	t = t0;
	return true;
}
}

// DOL 0x800c9e04 [audited]: kind 0xb, index, state 0, the shoot marker child, the model (+0x64 = 0x800ca35c: CHAR_ENEMY_SHELL_MDL, light set 1,
// fog 0, expanded onto the boss model at the node, CalcAABB), active, ShellParam_%dp.csv HP, the sound actor (natively none: the hit cues go through FUN_801c15ac, unpositioned), +0x5c
cTaskShell::cTaskShell(int nIndex, cTaskBoss* pBoss, cKingdomModel* pBossModel, const char* node) : m_nIndex(nIndex), m_pBoss(pBoss)
{
	SetName("g01::cTaskShell");
	m_nKind = 0xb;
	m_nState = 0;
	m_pMarker = new cTaskShootMarker(this);
	ConnectChild(m_pMarker);
	cKingdomModel* m = CreateModel(0);
	m->RegistModel("CHAR_ENEMY_SHELL_MDL", 0x36f);
	m->SetLightSetIdx(1);
	m->SetFogIdx(0);
	if (pBossModel != nullptr) pBossModel->RegistExpand((uint)nIndex, node, m);   // 0x8001f798 (drawn and advanced by the boss model)
	if (const wii::Mdl0* mdl = m->pMdl0())
	{
		float mn[3] = {1e30f, 1e30f, 1e30f}, mx[3] = {-1e30f, -1e30f, -1e30f};
		for (const auto& mesh : mdl->meshes)
			for (const auto& v : mesh.verts)
				for (int i = 0; i < 3; i++) { mn[i] = std::min(mn[i], v.pos[i]); mx[i] = std::max(mx[i], v.pos[i]); }
		if (mn[0] <= mx[0]) SetLocalAABB(mn, mx);
	}
	m_b08 = true;
	char name[0x80];
	std::snprintf(name, sizeof name, "ShellParam_%dp.csv", KingdomGlobal::GalagaSys->nPlayer());   // DAT_8047764c
	kingdom::cKingdomCSV csv;
	if (csv.RegistData(name))
	{
		const int r = csv.SearchRow(0, "HP", 0, 0);   // PTR_DAT_80477650
		if (r >= 0) csv.S32(r, 1, m_nHp);
	}
	m_n18 = m_nHp;
	UpdatePos();
}

// DOL 0x800ca208 [audited: the position is the boss model's node matrix (natively BossNodeWorld) = the expanded shell's world matrix]
void cTaskShell::UpdatePos()
{
	char node[16];
	std::snprintf(node, sizeof node, "shell%d", m_nIndex + 1);
	m_bMtx = m_pBoss != nullptr && m_pBoss->BossNodeWorld(node, m_mWorld);
	if (m_bMtx) m_v20 = Vector4(m_mWorld[12], m_mWorld[13], m_mWorld[14], 1.0f);
}

// DOL 0x800ca464 [audited] (active, not paused): +0x5c, the clips (0x8009a478: the boss model advances them too, as in the DOL), the model, the
// world box by +0x270
bool cTaskShell::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08) return true;
	UpdatePos();
	UpdateFrame();
	cTaskModel::Execute(pParam);
	if (m_bMtx) UpdateWorldAABB(m_mWorld);
	return true;
}

// DOL 0x800ca4f4 [audited]: drawn only while active and visible (+9); natively the boss model draws the expanded shell
void cTaskShell::Draw(int nStatus, cJetTask* pParam)
{
	(void)nStatus;
	(void)pParam;
	if (cKingdomModel* m = pModel(0))
	{
		if (m_b08 && m_b09) m->Visible();
		else m->Invisible();
	}
}

// DOL 0x800ca518 [audited]
bool cTaskShell::RayHit(const float org[3], const float dir[3], float& t) const
{
	if (!m_b14) return false;
	return RayAABB(org, dir, m_vMin0, m_vMax0, t);
}

// DOL 0x800ca548 [audited]: a player's shot: HP - 1 (m01se_04hit2); broken at 0 (+0x7c, m01se_06bak2)
void cTaskShell::HitCallback(cObject* other)
{
	if (other->nKind() != 1) return;
	m_n18--;
	snd::Play("m01se_04hit2");
	if (m_n18 < 1)
	{
		Close();
		snd::Play("m01se_06bak2");
	}
}

// DOL 0x800ca6ac [audited]
void cTaskShell::Open()
{
	if (cKingdomModel* m = pModel(0))
	{
		m->RegistAnm(0u, "CHAR_ENEMY_SHELL_ANM_CHA_01");
		m->RegistAnm(0u, "CHAR_ENEMY_SHELL_ANM_CLA_01");
		m->SetUpdateRateAnmChr(0u, 1.0f);
		m->SetUpdateRateMatClr(0u, 1.0f);
	}
	m_nState = 1;
	m_nAnm |= 3u;
}

// DOL 0x800ca664 [audited]: collidable, the marker shows, HP again
void cTaskShell::Activate()
{
	m_b14 = true;
	if (m_pMarker != nullptr) m_pMarker->Show();
	m_n18 = m_nHp;
}

// DOL 0x800ca690 [audited]
void cTaskShell::Hide()
{
	m_b14 = false;
	if (m_pMarker != nullptr) m_pMarker->Hide();
}

// DOL 0x800ca7e8 [audited]
void cTaskShell::Close()
{
	cKingdomModel* m = pModel(0);
	if ((m_nAnm & 1u) != 0)
	{
		if (m != nullptr) m->ReleaseAnmChr(0u);
		m_nAnm &= ~1u;
	}
	if ((m_nAnm & 2u) != 0)
	{
		if (m != nullptr) m->ReleaseAnmClr(0u);
		m_nAnm &= ~2u;
	}
	m_nState = 0;
	Hide();
}

// DOL 0x800ca760 [audited]
void cTaskShell::Break()
{
	cKingdomModel* m = pModel(0);
	if (m != nullptr) m->RegistAnm(0u, "CHAR_ENEMY_SHELL_ANM_CLA_02");
	const u32 old = m_nAnm;
	m_nAnm |= 2u;
	if ((old & 1u) != 0)
	{
		if (m != nullptr) m->ReleaseAnmChr(0u);
		m_nAnm &= ~1u;
	}
	m_nState = 2;
}

}
