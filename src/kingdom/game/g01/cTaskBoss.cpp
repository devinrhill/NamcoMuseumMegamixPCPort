#include "kingdom/game/g01/cTaskBoss.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskBossEnemy.hpp"
#include "kingdom/game/g01/cTaskBossEnemyMan.hpp"
#include "kingdom/game/g01/cTaskBossLife.hpp"
#include "kingdom/game/g01/cTaskBossMissile.hpp"
#include "kingdom/game/g01/cTaskCamera.hpp"
#include "kingdom/game/g01/cTaskCollisionMan.hpp"
#include "kingdom/game/g01/cTaskConsolePlayer.hpp"
#include "kingdom/game/g01/cTaskNodeEffect.hpp"
#include "kingdom/game/g01/cTaskPacman.hpp"
#include "kingdom/game/g01/cTaskScene.hpp"
#include "kingdom/game/g01/cTaskShell.hpp"
#include "snd/Mixer.hpp"
#include "wii/Archive.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g01
{

cTaskBoss* cTaskBoss::s_pInstance = nullptr;

namespace
{
constexpr float BLEND = 30.0f;              // FLOAT_8047b404
constexpr float HASSYA_FRAME = 130.0f;      // FLOAT_8047b408
constexpr float HASSYA_VOL = 0.2f;          // FLOAT_8047b40c (0x80026394 on the handle: UNVERIFIED meaning, not applied)
constexpr float RELOAD = 120.0f;            // FLOAT_8047b410
constexpr float VOMIT_TIME = 200.0f;        // FLOAT_8047b414
constexpr float STRUGGLE_SE = 115.0f;       // FLOAT_8047b418
constexpr float MISSILE_RADIUS = 0.6f;      // FLOAT_8047b41c
constexpr uint FX_HIT = 0x34, FX_BITE = 0x35, FX_INHALE = 0x36, FX_VOMIT = 0x37, FX_CHARGE = 0x38, FX_THREAT = 0x3a, FX_STRUGGLE = 0x3b,
               FX_BOM = 0x3d, FX_WING = 0x3f, FX_VOMIT2 = 0x40;

// 4x4 column major
void Mul(const float a[16], const float b[16], float out[16])
{
	float r[16];
	for (int c = 0; c < 4; c++)
		for (int i = 0; i < 4; i++)
			r[c * 4 + i] = a[i] * b[c * 4] + a[4 + i] * b[c * 4 + 1] + a[8 + i] * b[c * 4 + 2] + a[12 + i] * b[c * 4 + 3];
	std::memcpy(out, r, sizeof r);
}
// 3x4 row major (ModelDraw::NodeMatrix) -> 4x4 column major
void FromNode(const float n[12], float out[16])
{
	const float m[16] = {n[0], n[4], n[8], 0, n[1], n[5], n[9], 0, n[2], n[6], n[10], 0, n[3], n[7], n[11], 1};
	std::memcpy(out, m, sizeof m);
}
// nw4r::math::IntersectionRay3AABB (as cTaskConsolePlayer's)
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
// nw4r::math::IntersectionSegment3Sphere
bool SegmentSphere(const float a[3], const float b[3], const float c[3], float r)
{
	const float d[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
	const float f[3] = {a[0] - c[0], a[1] - c[1], a[2] - c[2]};
	const float dd = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
	float t = 0.0f;
	if (dd > 1e-12f) t = std::max(0.0f, std::min(1.0f, -(f[0] * d[0] + f[1] * d[1] + f[2] * d[2]) / dd));
	const float p[3] = {f[0] + d[0] * t, f[1] + d[1] * t, f[2] + d[2] * t};
	return p[0] * p[0] + p[1] * p[1] + p[2] * p[2] <= r * r;
}
// the DOL's "max < frame" (NW4R keeps a one-time clip's frame running, PlayPolicy_Onetime); the native frame stops at max, so "max <= frame"
// (one frame earlier)
bool ClipEnd(cKingdomModel* m, uint slot)
{
	return m->GetFrameMaxAnmChr(slot) <= m->GetFrameAnmChr(slot);
}
float Dist2(const float a[3], const float b[3])
{
	const float d[3] = {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
	return d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
}
}

// ---- sBossParam / cBossLeaveEnemy -----------------------------------------------------------------------------------------------------

// DOL 0x800ca8e4: the rows by name, column 1
void sBossParam::Load(const char* name)
{
	kingdom::cKingdomCSV csv;
	if (!csv.RegistData(name)) { std::fprintf(stderr, "g01::cTaskBoss: %s not found\n", name); return; }
	auto f32 = [&csv](const char* key, float& v) { const int r = csv.SearchRow(0, key, 0, 0); if (r >= 0) csv.F32(r, 1, v); };
	f32("MISSILE_SPEED", fMissileSpeed);
	f32("MISSILE_STRUGGLE_SPEED", fMissileStruggleSpeed);
	f32("SP_MISSILE_SPEED", fSpMissileSpeed);
	f32("SP_MISSILE_STRUGGLE_SPEED", fSpMissileStruggleSpeed);
	f32("STRUGGLE_RATE", fStruggleRate);
	{
		const int r = csv.SearchRow(0, "LASER_DAMAGE", 0, 0);
		if (r >= 0) csv.S32(r, 1, nLaserDamage);
	}
	f32("o_Vomit01", fVomit);
	f32("o_Breath01", fBreath);
	f32("o_Bite_ToReady01", fBiteToReady);
	f32("o_Bite_ToPac01", fBiteToPac);
	f32("o_Bite_ToWait01", fBiteToWait);
	f32("o_Bite_ToBack01", fBiteToBack);
	f32("o_Beam_Attack01", fBeamAttack);
	f32("a_Danger01", fDanger);
	f32("e_Bite01", fBite);
	f32("e_Threat01", fThreat);
	f32("s_Beam01", fBeamSe);
}

// DOL 0x800cad0c: count = rows - 1 (the last row is a comment), {type, orbit} per row, the index list 0..n-1
void cBossLeaveEnemy::Load(const char* name)
{
	kingdom::cKingdomCSV csv;
	entry.clear();
	order.clear();
	nNext = 0;
	if (!csv.RegistData(name)) return;
	const int n = csv.nRow() - 1;
	for (int r = 0; r < n; r++)
	{
		sEntry e{};
		csv.S32(r, 0, e.nType);
		csv.S32(r, 1, e.nOrbit);
		entry.push_back(e);
		order.push_back((u32)r);
	}
}

void cBossLeaveEnemy::Shuffle()
{
	nNext = 0;
	if (order.empty()) return;
	for (int i = 0; i < 300; i++)
	{
		const u32 a = (u32)std::rand() % (u32)order.size();   // PtkSim::Rand15 (0x80211048)
		const u32 b = (u32)std::rand() % (u32)order.size();
		std::swap(order[a], order[b]);
	}
}

const cBossLeaveEnemy::sEntry* cBossLeaveEnemy::Next()
{
	if (nNext >= order.size()) return nullptr;
	return &entry[order[nNext++]];
}

// ---- cTaskBoss ------------------------------------------------------------------------------------------------------------------------

// DOL 0x801ba078 (cTaskEnemyBase + cTaskBoss vtables, the two particle handles, four sound handles) + Init 0x800cae80: kind 0xd, stage /
// world, no attack, state 2 (waiting for the app's GAME)
cTaskBoss::cTaskBoss()
{
	SetName("g01::cTaskBoss");
	m_nKind = 0xd;
	m_nStage = KingdomGlobal::GalagaSys->nStage();
	m_nWorld = KingdomGlobal::GalagaSys->nWorld();
	m_nFirePattern = 0;   // +0x290
	m_nState = 2;
	m_nStep = 0;
	s_pInstance = this;
}

cTaskBoss::~cTaskBoss()
{
	if (s_pInstance == this) s_pInstance = nullptr;
}

cKingdomModel* cTaskBoss::Boss() const
{
	cKingdomModel* orbit = const_cast<cTaskBoss*>(this)->pModel(0);
	return orbit != nullptr ? orbit->pExpandMdl(0) : nullptr;
}

cKingdomModel* cTaskBoss::Hit() const
{
	cKingdomModel* orbit = const_cast<cTaskBoss*>(this)->pModel(0);
	return orbit != nullptr ? orbit->pExpandMdl(1) : nullptr;
}

// world (column major) of a node of the model expanded at "orbit": orbit world * orbit node * child local * child node
bool cTaskBoss::ChildWorld(int nExp, const char* node, float out[16]) const
{
	cKingdomModel* orbit = const_cast<cTaskBoss*>(this)->pModel(0);
	cKingdomModel* child = orbit != nullptr ? orbit->pExpandMdl((uint)nExp) : nullptr;
	float a[12], b[12];
	if (child == nullptr || !orbit->NodeMatrix("orbit", a) || !child->NodeMatrix(node, b)) return false;
	float w[16], l[16], an[16], bn[16];
	orbit->CalcWorld(w);
	child->CalcWorld(l);
	FromNode(a, an);
	FromNode(b, bn);
	Mul(w, an, out);
	Mul(out, l, out);
	Mul(out, bn, out);
	return true;
}

// DOL 0x800cebbc
void cTaskBoss::Setup(cTaskScene* pScene, cTaskPacman* pPacman)
{
	m_pScene = pScene;
	m_pPacman = pPacman;
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	// 0x800e3800: EnemyParam row 11 (the last boss)
	m_nType = 11;
	const sEnemyParam& prm = cTaskEnemyMan::s_pInstance->Param(11);
	m_nSize = prm.nSize;
	m_n18 = prm.nLife;
	m_nMaxLife = prm.nLife > 0 ? prm.nLife : 1;
	m_nScore = prm.nScore;
	m_fBulletSpeed = prm.fBulletSpeed;
	m_fMissileSpeed = prm.fMissileSpeed;
	char name[0x100];
	std::snprintf(name, sizeof name, "BossParam_%dp.csv", sys->nPlayer());   // DAT_80477600
	m_Param.Load(name);
	std::snprintf(name, sizeof name, "BossEnemyList_%dp.csv", sys->nPlayer());   // DAT_80477604
	m_Leave.Load(name);
	m_Leave.Shuffle();
	SetOrbitSpec(1);
	m_b08 = true;
	m_b14 = true;
	// +0xbc = 0x800cb278: cDemoBossBoss for stage 10 (ctor 0x801bd094: Demo_sa.csv e_bomb01 / s_voice3_01 / s_voice4_01, column 1)
	// registered with the demo manager, then the clips (+0x114)
	if (m_nStage == 10)
	{
		kingdom::cKingdomCSV d;
		if (d.RegistData("Demo_sa.csv"))
		{
			const char* key[3] = {"e_bomb01", "s_voice3_01", "s_voice4_01"};   // PTR_s_e_bomb01_8037d8f0 ..
			for (int i = 0; i < 3; i++) d.F32(d.SearchRow(0, key[i], 0, 0), 1, m_fGoalTime[i]);
		}
		if (cDemoMan* dm = sys->pDemoMan()) dm->Regist(this);
	}
	StartClips();
	if (cTaskCollisionMan* col = sys->pTaskCollisionMan()) col->Regist(this);   // 0x800d41c8
	// the first missiles: the mask of the first missile node
	ReloadMissiles(NextMissileMask(-1.0f));   // FLOAT_8047b420
	// the effects at the boss's nodes (0x800e06dc x5; 0x800e0a38 ef_L_wing / ef_R_wing slot 0x3f looping; 0x800e0820 bom%d)
	auto bossNode = [this](const char* node) {
		std::string n(node);
		return [this, n](float o[3]) {
			float m[16];
			if (!ChildWorld(0, n.c_str(), m)) return false;
			o[0] = m[12]; o[1] = m[13]; o[2] = m[14];
			return true;
		};
	};
	// cTaskEffectExpand follows the node's whole matrix (0x800e11fc)
	auto bossMtx = [this](const char* node) {
		std::string n(node);
		return [this, n](float m[16]) { return ChildWorld(0, n.c_str(), m); };
	};
	cTaskNodeEffect** fx[5] = {&m_pFxMouth, &m_pFxBody, &m_pFxVomit2, &m_pFxInhale, &m_pFxVomit};
	for (cTaskNodeEffect** p : fx)
	{
		*p = new cTaskNodeEffect(bossNode("ef_body"));
		(*p)->SetMtx(bossMtx("ef_body"));
		ConnectChild(*p);
	}
	m_pFxWing[0] = new cTaskNodeEffect(bossNode("ef_L_wing"));
	m_pFxWing[1] = new cTaskNodeEffect(bossNode("ef_R_wing"));
	m_pFxWing[0]->SetMtx(bossMtx("ef_L_wing"));
	m_pFxWing[1]->SetMtx(bossMtx("ef_R_wing"));
	for (cTaskNodeEffect* e : m_pFxWing)
	{
		ConnectChild(e);
		e->Start(FX_WING, false, false, true);
	}
	for (int i = 0; i < 4; i++)
	{
		char bom[16];
		std::snprintf(bom, sizeof bom, "bom%d", i + 1);   // PTR_s_bom_d_804775ec
		m_pFxBom[i] = new cTaskNodeEffect(bossNode(bom));
		m_pFxBom[i]->SetMtx(bossMtx(bom));
		ConnectChild(m_pFxBom[i]);
	}
	// +0x564: the danger sign (ANT_DANGER, priority 0x80000000), hidden
	m_pDanger = new kingdom::cTaskAnita();
	m_pDanger->nDrawPriority(0x80000000u);
	m_pDanger->pAnita(0)->RegistData("ANT_DANGER");
	m_pDanger->pAnita(0)->SetState(1u, false);
	ConnectChild(m_pDanger);
}

void cTaskBoss::SetOrbitSpec(int n)
{
	const int players = KingdomGlobal::GalagaSys->nPlayer();
	m_pOrbit = cTaskEnemyMan::s_pInstance->pOrbitDataMan()->pOrbit((u32)((players - 1) * 10 + n));
	StartOrbit();
}

// DOL 0x800cb308: the orbit skeleton with the boss at "orbit" (light set 1, fog 0), the four shells at shell1..4 (cTaskShell 0x800c9e04)
void cTaskBoss::LoadModel()
{
	DeleteModel(0);
	cKingdomModel* orbit = CreateModel(0);
	orbit->RegistModel("CHAR_ENEMY_LASTBOSS_ORBIT_MDL", 0x36f);
	orbit->SetLightSetIdx(1);
	orbit->SetFogIdx(0);
	orbit->RegistExpand(0, "orbit", "CHAR_ENEMY_LASTBOSS_MDL", 0x36f);
	cKingdomModel* b = orbit->pExpandMdl(0);
	if (b != nullptr)
	{
		b->SetLightSetIdx(1);
		b->SetFogIdx(0);
	}
	for (int i = 0; i < 4; i++)
	{
		char node[16];
		std::snprintf(node, sizeof node, "shell%d", i + 1);   // PTR_s_shell_d_804775e8
		m_pShell[i] = new cTaskShell(i, this, b, node);
		ConnectChild(m_pShell[i]);
	}
}

// DOL 0x800cb4f0
void cTaskBoss::SetSegmentClip(int nCode)
{
	char name[0x80];
	std::snprintf(name, sizeof name, "m01m_s%x_lb_ob_cha.brres", nCode);
	if (cKingdomModel* orbit = pModel(0)) orbit->RegistAnm(0u, name);
}

// DOL 0x800cc7f4: model, the hit model at "orbit" (cTaskHitModel 0x801c398c; invisible 0x8001a330), the segment's clip, the idle clips
void cTaskBoss::StartClips()
{
	LoadModel();
	cKingdomModel* orbit = pModel(0);
	orbit->RegistExpand(1, "orbit", "CHAR_ENEMY_LASTBOSS_HIT_MDL", 0x36f);
	m_HitNodes.clear();
	if (cKingdomModel* h = Hit())
	{
		h->Invisible();
		if (const wii::Mdl0* mdl = h->pMdl0()) m_HitNodes = mdl->nodes;
	}
	SetSegmentClip(m_nStage + m_nSeg);
	if (cKingdomModel* b = Boss()) b->RegistAnm(0u, "CHAR_ENEMY_LASTBOSS_ANM_CHA_01");
	if (cKingdomModel* h = Hit()) h->RegistAnm(0u, "CHAR_ENEMY_LASTBOSS_HIT_ANM_CHA_01");
	SyncCourse();
}

// DOL 0x800cc908: the models again (+0xc0 = 0x800cb308: new orbit / boss models and four new shells; natively the old shells, whose
// models hung on the deleted boss model, are released), the new shells' models invisible (0x8001a330), the orbit clip m01m_lb_ob_a50_cha
// (rate 1.0, FLOAT_8047b3f4) and the boss's m01m_g09_a50_{cha, cla, tsa, tpa, via} where they exist (cha / tpa / via rate 1.0); the longest of
// cha / tpa / via is the demo's cAnmCheck. The hit model is not expanded again.
void cTaskBoss::GoalClips()
{
	for (cTaskShell*& sh : m_pShell)
		if (sh != nullptr) { sh->Release(); sh = nullptr; }   // native (see above)
	LoadModel();
	for (cTaskShell* sh : m_pShell)
		if (sh != nullptr)
			if (cKingdomModel* m = sh->pModel(0)) m->Invisible();
	m_HitNodes.clear();
	cKingdomModel* orbit = pModel(0);
	char name[0x80];
	std::snprintf(name, sizeof name, "m01m_lb_ob_a%x0_cha.brres", 5);   // s_m01m_lb_ob_a_x0_cha_brres_80371b0a
	orbit->RegistAnm(0u, name);
	orbit->SetUpdateRateAnmChr(0u, 1.0f);
	static const char* const AN[5] = {"m01m_g09_a%x0_cha.brres", "m01m_g09_a%x0_cla.brres", "m01m_g09_a%x0_tsa.brres", "m01m_g09_a%x0_tpa.brres",
	                                  "m01m_g09_a%x0_via.brres"};   // PTR_s_m01m_g09_a_x0_cha_brres_803719d0 ..
	cKingdomModel* b = Boss();
	m_fGoalMax = 0.0f;
	m_nGoalKind = 0;
	for (int i = 0; b != nullptr && i < 5; i++)
	{
		std::snprintf(name, sizeof name, AN[i], 5);
		std::size_t size = 0;
		if (wii::ArchiveStore::Find(name, size) == nullptr) continue;
		b->RegistAnm(0u, name);
		float max = 0.0f;   // FLOAT_8047b3f8
		if (i == 0) { b->SetUpdateRateAnmChr(0u, 1.0f); max = b->GetFrameMaxAnmChr(0u); }
		else if (i == 3) { b->SetUpdateRateAnmTexPat(0u, 1.0f); max = b->GetFrameMaxAnmTexPat(0u); }
		else if (i == 4) { b->SetUpdateRateAnmVis(0u, 1.0f); max = b->GetFrameMaxAnmVis(0u); }
		if (m_fGoalMax < max) { m_fGoalMax = max; m_nGoalKind = i; }
	}
}

// cDemoBossBoss::Start 0x801bd298: the goal clips (+0x110 with the cAnmCheck), the effect object at the boss's ef_body (0x800e0820,
// DAT_80478508) connected to the app's task root (0x8008fe30, KingSys +0xa40; natively a child of the boss)
void cTaskBoss::GoalInit()
{
	GoalClips();
	m_pGoalFx = new cTaskNodeEffect([this](float o[3]) {
		float m[16];
		if (!ChildWorld(0, "ef_body", m)) return false;
		o[0] = m[12]; o[1] = m[13]; o[2] = m[14];
		return true;
	});
	m_pGoalFx->SetMtx([this](float m[16]) { return ChildWorld(0, "ef_body", m); });
	ConnectChild(m_pGoalFx);
}

// cDemoBossBoss::Execute 0x801bd33c: false once the longest clip ended (cAnmCheck 0x801bcd20: max < frame; natively max <= frame, the
// one-time clip stops at max); before that, at the Demo_sa.csv frames (-1 = done, FLOAT_8047ce78): e_bomb01 effect 0x30 (one-time,
// dependent, clipped; 0x800e0fe4) + m01se_33big_bom (DAT_8047ce68), s_voice3_01 m01se_31boss_03 (DAT_8047ce60), s_voice4_01 m01se_32boss_04
// (DAT_8047ce64)
bool cTaskBoss::GoalCheck()
{
	cKingdomModel* b = Boss();
	if (b == nullptr) return false;
	float f, max = m_fGoalMax;
	if (m_nGoalKind == 3) f = b->GetFrameAnmTexPat(0u);   // 0x801bccbc
	else if (m_nGoalKind == 4) f = b->GetFrameAnmVis(0u);
	else f = b->GetFrameAnmChr(0u);
	if (max <= f) return false;
	auto due = [f](float& t) { if (t != -1.0f && t <= f) { t = -1.0f; return true; } return false; };
	if (due(m_fGoalTime[0]))
	{
		if (m_pGoalFx != nullptr) m_pGoalFx->Start(0x30u, true, true, true);
		snd::Play("m01se_33big_bom");
	}
	if (due(m_fGoalTime[1])) snd::Play("m01se_31boss_03");
	if (due(m_fGoalTime[2])) snd::Play("m01se_32boss_04");
	return true;
}

// DOL 0x800cb554
void cTaskBoss::SyncCourse()
{
	cKingdomModel* orbit = pModel(0);
	if (orbit == nullptr || m_pScene == nullptr) return;
	orbit->SetFrameAnmChr(0u, m_pScene->fFrame());
	orbit->SetUpdateRateAnmChr(0u, m_pScene->fRate());
}

// a clip pair of an attack (the boss and its hit model in the same slot), rate 1, optionally weight 0 (blended in)
void cTaskBoss::SetAnm(int nSlot, const char* boss, const char* hit, bool bWeight0)
{
	if (cKingdomModel* b = Boss())
	{
		b->RegistAnm((uint)nSlot, boss);
		b->SetUpdateRateAnmChr((uint)nSlot, 1.0f);
		b->SetWeightAnmChr((uint)nSlot, bWeight0 ? 0.0f : 1.0f);
	}
	if (cKingdomModel* h = Hit())
	{
		h->RegistAnm((uint)nSlot, hit);
		h->SetUpdateRateAnmChr((uint)nSlot, 1.0f);
		h->SetWeightAnmChr((uint)nSlot, bWeight0 ? 0.0f : 1.0f);
	}
}

// DOL 0x800cc664
void cTaskBoss::Blend(float fFrames, int nFrom, int nTo)
{
	m_bBlend = true;
	m_nBlendFrom = nFrom;
	m_nBlendTo = nTo;
	m_fBlendFrames = fFrames;
	m_fBlendTime = 0.0f;
}

// DOL 0x800cc684
void cTaskBoss::UpdateBlend()
{
	if (!m_bBlend) return;
	m_fBlendTime += 1.0f;
	if (m_fBlendFrames < m_fBlendTime)
	{
		m_fBlendTime = m_fBlendFrames;
		m_bBlend = false;
	}
	const float w = m_fBlendFrames > 0.0f ? m_fBlendTime / m_fBlendFrames : 1.0f;
	for (cKingdomModel* m : {Boss(), Hit()})
	{
		if (m == nullptr) continue;
		m->SetWeightAnmChr((uint)m_nBlendFrom, 1.0f - w);
		m->SetWeightAnmChr((uint)m_nBlendTo, w);
	}
}

// DOL 0x800cb5b4: +0x3d0 = the orbit node's rotation * (0, 0, 1); +0x440.. = the bom%d matrices; +0x3f0 = ef_body
void cTaskBoss::UpdateNodes()
{
	float n[12];
	cKingdomModel* orbit = pModel(0);
	if (orbit != nullptr && orbit->NodeMatrix("orbit", n))
	{
		m_vForward[0] = n[2];
		m_vForward[1] = n[6];
		m_vForward[2] = n[10];
	}
	for (int i = 0; i < 4; i++)
	{
		char bom[16];
		std::snprintf(bom, sizeof bom, "bom%d", i + 1);
		m_bBom[i] = ChildWorld(0, bom, m_mBom[i]);
	}
	float m[16];
	if (ChildWorld(0, "ef_body", m))
	{
		m_vBody[0] = m[12];
		m_vBody[1] = m[13];
		m_vBody[2] = m[14];
	}
	// cTaskHitModel::Execute (0x801c4764 -> cNode / cMesh::Create): every node's volume as a world box (natively cached in m_HitNodes' volume)
}

bool cTaskBoss::BomMatrix(int i, float out[16]) const
{
	if ((unsigned)i >= 4 || !m_bBom[i]) return false;
	std::memcpy(out, m_mBom[i], sizeof m_mBom[i]);
	return true;
}

// DOL 0x800cc44c: +0x20 = the translate of the orbit skeleton's node 1 ("orbit"), the sound actor follows; +0x554 = +0x3f0
void cTaskBoss::UpdateBossModel()
{
	float n[12];
	cKingdomModel* orbit = pModel(0);
	if (orbit != nullptr && orbit->NodeMatrix("orbit", n))
	{
		m_v20 = Vector4(n[3], n[7], n[11], 1.0f);
		for (int i = 0; i < 3; i++) m_vWorldPos[i] = (&m_v20.x)[i];
	}
	m_SndActor.SetPosition(m_v20);   // 0x800cc5dc: the sound actor (+0x344) at +0x20
}

// DOL 0x800cc618
void cTaskBoss::UpdateHitEffect()
{
	if (m_HitFx.IsValid()) m_HitFx.vSetTrans(Vector4(m_vBody[0], m_vBody[1], m_vBody[2], 1.0f));
}

// DOL 0x800ccb30: the orbit as a timetable against the course frame; a hold node starts attack nParam + 1 when none runs
bool cTaskBoss::UpdateBossOrbit(float fStep)
{
	(void)fStep;
	m_fOrbitTime = m_pScene != nullptr ? m_pScene->fFrame() : 0.0f;   // 0x800f7dc0
	bool bAgain;
	do
	{
		bAgain = false;
		const sEmyNode* cur = m_pCurNode;
		const sEmyNode* next = m_pNextNode;
		if (cur == nullptr || next == nullptr) return false;   // native guard (the DOL reads through the null node)
		if ((cur->nCmd & 0xffff) == 3 && m_nFirePattern == 0) m_nFirePattern = (s32)cur->nParam + 1;
		if (next->fFrame <= m_fOrbitTime)
		{
			m_nOrbitIdx++;
			m_pCurNode = m_pNextNode;
			m_pNextNode = pOrbitNode(m_nOrbitIdx);
			if (m_pNextNode == nullptr) return false;
			if (next->fFrame <= m_fOrbitTime) bAgain = true;
		}
	} while (bAgain);
	return true;
}

// DOL 0x800ccc40
void cTaskBoss::UpdateBossFire()
{
	bool bDone = false;
	switch (m_nFirePattern)
	{
	case 4: bDone = AttackBeam(); break;
	case 2: bDone = AttackBite(); break;
	case 1: bDone = AttackMissile(); break;
	case 3: bDone = AttackVomit(); break;
	default: break;
	}
	if (!bDone) return;
	m_fAtkTime = 0.0f;
	m_nAtkStep = 0;
	m_nFirePattern = 0;
	m_bHitInhale = false;
	m_Param.bBite = m_Param.bThreat = m_Param.bDanger = m_Param.bBeam = m_Param.bBeamSe = false;
}

// (its step jump table at 0x80438ab8, right after cTaskBomb::cBombParam's vtable 0x80438aac, which the vtable audit reads as slots: case
// targets 0x800ccf0c / 0x800ccfd4 / 0x800cd0d4 / 0x800cd178 / 0x800cd2e0 / 0x800cd3d4 / 0x800cd40c / 0x800cd488 / 0x800cd4e0)
// DOL 0x800cceb8: inhale (CHA_03 -> CHA_04, mouth open o_Breath01 frames), then the beam (CHA_08 / CLA_08) that hurts Pac-Man at
// o_Beam_Attack01; a missile in the mouth cuts the inhale short (CHA_06)
bool cTaskBoss::AttackBeam()
{
	cKingdomModel* b = Boss();
	if (b == nullptr) return true;
	switch (m_nAtkStep)
	{
	case 0:
		SetAnm(1, "CHAR_ENEMY_LASTBOSS_ANM_CHA_03", "CHAR_ENEMY_LASTBOSS_HIT_ANM_CHA_03", true);
		Blend(BLEND, 0, 1);
		m_nAtkStep = 1;
		break;
	case 1:
		if (ClipEnd(b, 1u))
		{
			SetAnm(1, "CHAR_ENEMY_LASTBOSS_ANM_CHA_04", "CHAR_ENEMY_LASTBOSS_HIT_ANM_CHA_04", false);
			if (m_pFxInhale != nullptr) m_pFxInhale->Start(FX_INHALE, false, true, false);
			m_nSeSuikomi = m_SndActor.Play("m01se_34suikomi");   // on the actor (+0x344), as charge / bite / missiles / struggle
			m_bMouth = true;
			m_nAtkStep = 2;
		}
		break;
	case 2:
		if (m_fAtkTime <= m_Param.fBreath)
		{
			if (!m_bHitInhale)
				m_fAtkTime += 1.0f;
			else
			{
				m_fAtkTime = 0.0f;
				m_nAtkStep = 6;
				if (m_pFxInhale != nullptr) m_pFxInhale->Stop();
				snd::Stop(m_nSeSuikomi);
			}
		}
		else
		{
			m_fAtkTime = 0.0f;
			m_nAtkStep = 3;
			if (m_pFxInhale != nullptr) m_pFxInhale->Stop();
			snd::Stop(m_nSeSuikomi);
			m_nSeCharge = m_SndActor.Play("m01se_21charge");
			m_bMouth = false;
		}
		break;
	case 3:
		SetAnm(2, "CHAR_ENEMY_LASTBOSS_ANM_CHA_08", "CHAR_ENEMY_LASTBOSS_HIT_ANM_CHA_08", true);
		b->RegistAnm(0u, "CHAR_ENEMY_LASTBOSS_ANM_CLA_08");
		b->SetUpdateRateMatClr(0u, 1.0f);
		Blend(BLEND, 1, 2);
		if (m_pFxMouth != nullptr)
		{
			m_pFxMouth->SetPos([this](float o[3]) {
				float m[16];
				if (!ChildWorld(0, "ef_mouth", m)) return false;
				o[0] = m[12]; o[1] = m[13]; o[2] = m[14];
				return true;
			});
			m_pFxMouth->SetMtx([this](float m[16]) { return ChildWorld(0, "ef_mouth", m); });   // 0x800e0ecc
			m_pFxMouth->Start(FX_CHARGE, false, true, false);
		}
		{
			float p[3] = {m_vBody[0], m_vBody[1], m_vBody[2]};
			if (m_pFxMouth != nullptr) m_pFxMouth->Pos(p);
			m_Beam.Generate(FX_CHARGE, true, true, false, 0.0f, Vector4(p[0], p[1], p[2], 1.0f));   // +0x570 (0x80021a9c, slot 0x38)
		}
		m_bHassya = false;
		m_nAtkStep = 4;
		break;
	case 4:
		if (HASSYA_FRAME < b->GetFrameAnmChr(2u) && !m_bHassya)
		{
			snd::Stop(m_nSeCharge);
			m_nSeHassya = snd::Play("m01se_22hassya");
			snd::SetVolume(m_nSeHassya, 0.2f, 0);   // 0x800cd324: 0x80026394(0.2 (FLOAT_8047b40c), +0x660, 0)
			m_bHassya = true;
			(void)HASSYA_VOL;
		}
		if (b->GetFrameMaxAnmChr(2u) - BLEND < b->GetFrameAnmChr(2u))
		{
			b->ReleaseAnmChr(1u);
			b->ReleaseAnmClr(0u);
			if (cKingdomModel* h = Hit()) h->ReleaseAnmChr(1u);
			Blend(BLEND, 2, 0);
			m_nAtkStep = 5;
		}
		if (m_Param.fBeamAttack <= b->GetFrameAnmChr(2u) && !m_Param.bBeam)
		{
			m_Param.bBeam = true;
			m_bBeam = true;   // this frame the beam hurts Pac-Man (HitCheck kind 3)
		}
		break;
	case 5:
		if (!m_bBlend)
		{
			if (m_pFxMouth != nullptr) m_pFxMouth->Stop();
			if (m_Beam.IsValid()) m_Beam.Detach(true);
			b->ReleaseAnmChr(2u);
			if (cKingdomModel* h = Hit()) h->ReleaseAnmChr(2u);
			return true;
		}
		break;
	case 6:
		SetAnm(1, "CHAR_ENEMY_LASTBOSS_ANM_CHA_06", "CHAR_ENEMY_LASTBOSS_HIT_ANM_CHA_06", false);
		m_nAtkStep = 7;
		break;
	case 7:
		if (b->GetFrameMaxAnmChr(1u) - BLEND < b->GetFrameAnmChr(1u))
		{
			Blend(BLEND, 1, 0);
			m_nAtkStep = 8;
		}
		break;
	case 8:
		if (!m_bBlend)
		{
			b->ReleaseAnmChr(1u);
			if (cKingdomModel* h = Hit()) h->ReleaseAnmChr(1u);
			m_bMouth = false;
			return true;
		}
		break;
	default: break;
	}
	AimBeam();
	return false;
}

// DOL 0x800ccd38: the beam effect at the mouth effect's position, turned toward Pac-Man (0x801c1dbc)
void cTaskBoss::AimBeam()
{
	if (!m_Beam.IsValid() || m_pFxMouth == nullptr || !m_pFxMouth->IsValid()) return;
	float p[3];
	if (!m_pFxMouth->Pos(p)) return;
	const float* pac = m_pPacman != nullptr ? m_pPacman->vPos() : p;
	const float d[3] = {pac[0] - p[0], pac[1] - p[1], pac[2] - p[2]};
	const float yaw = std::atan2(d[0], d[2]);
	const float pitch = std::atan2(d[1], std::sqrt(d[0] * d[0] + d[2] * d[2]));
	// 0x8008a5b0 / 0x8008a49c / 0x8008a4c0: move and turn the handle (UNVERIFIED angle convention)
	m_Beam.vSetTrans(Vector4(p[0], p[1], p[2], 1.0f));
	m_Beam.vSetRotate(Vector4(-pitch, yaw, 0.0f, 0.0f));
}

// DOL 0x800cd540: the bite (CHA_07): the danger sign at a_Danger01, the threat effect at e_Threat01, the bite (effect, m01se_29kamituki,
// it hurts Pac-Man this frame) at e_Bite01; the model lunges at Pac-Man (0x800cd894)
bool cTaskBoss::AttackBite()
{
	cKingdomModel* b = Boss();
	if (b == nullptr) return true;
	if (m_nAtkStep == 0)
	{
		SetAnm(1, "CHAR_ENEMY_LASTBOSS_ANM_CHA_07", "CHAR_ENEMY_LASTBOSS_HIT_ANM_CHA_07", true);
		Blend(BLEND, 0, 1);
		m_nAtkStep = 1;
	}
	else if (m_nAtkStep == 1)
	{
		if (b->GetFrameMaxAnmChr(1u) - BLEND < b->GetFrameAnmChr(1u))
		{
			Blend(BLEND, 1, 0);
			m_nAtkStep = 2;
		}
	}
	else if (m_nAtkStep == 2 && !m_bBlend)
	{
		b->ReleaseAnmChr(1u);
		if (cKingdomModel* h = Hit()) h->ReleaseAnmChr(1u);
		return true;
	}
	const float f = b->GetFrameAnmChr(1u);
	auto mouth = [this](float o[3]) {
		float m[16];
		if (!ChildWorld(0, "ef_mouth", m)) return false;
		o[0] = m[12]; o[1] = m[13]; o[2] = m[14];
		return true;
	};
	auto mouthMtx = [this](float m[16]) { return ChildWorld(0, "ef_mouth", m); };
	if (m_Param.fDanger <= f && !m_Param.bDanger)
	{
		ShowDanger();
		m_Param.bDanger = true;
	}
	if (m_Param.fThreat <= f && !m_Param.bThreat)
	{
		if (m_pFxMouth != nullptr) { m_pFxMouth->SetPos(mouth); m_pFxMouth->SetMtx(mouthMtx); m_pFxMouth->Start(FX_THREAT, true, true, false); }
		m_Param.bThreat = true;
	}
	if (m_Param.fBite <= f && !m_Param.bBite)
	{
		if (m_pFxMouth != nullptr) { m_pFxMouth->SetPos(mouth); m_pFxMouth->SetMtx(mouthMtx); m_pFxMouth->Start(FX_BITE, true, true, false); }
		m_SndActor.Play("m01se_29kamituki");
		m_Param.bBite = true;
		m_bBite = true;
	}
	BiteMove();
	return false;
}

// DOL 0x800cd894: the lunge, as shares of the clip: wait (ToReady), to Pac-Man (ToPac), stay (ToWait), back (ToBack); +0x3e0 = the offset
void cTaskBoss::BiteMove()
{
	cKingdomModel* b = Boss();
	if (b == nullptr) return;
	const float len = b->GetFrameMaxAnmChr(1u);
	const float a = m_Param.fBiteToReady * len;
	const float c = m_Param.fBiteToPac * len + a;
	const float d = m_Param.fBiteToWait * len + c;
	const float e = m_Param.fBiteToBack * len + d;
	m_fAtkTime += 1.0f;
	const float f = b->GetFrameAnmChr(1u);
	// Pac-Man's +0x290 (node boss_bite, cTaskPacman 0x800f4388)
	const float* pac = m_pPacman != nullptr ? m_pPacman->vBite() : &m_v20.x;
	const float* me = &m_v20.x;
	if (f < a) return;
	if (f < c)
	{
		const float t = (m_fAtkTime - a) / (c - a);
		for (int i = 0; i < 3; i++) m_vBite[i] = (me[i] + (pac[i] - me[i]) * t) - me[i];
	}
	else if (f < d)
	{
		for (int i = 0; i < 3; i++) m_vBite[i] = pac[i] - me[i];
	}
	else if (f < e)
	{
		const float t = (m_fAtkTime - d) / (len - d);
		for (int i = 0; i < 3; i++) m_vBite[i] = (pac[i] + (me[i] - pac[i]) * t) - me[i];
	}
	else if (f < m_fAtkTime)
	{
		m_vBite[0] = m_vBite[1] = m_vBite[2] = 0.0f;
		m_vBite[3] = 1.0f;
	}
}

// DOL 0x800cb48c
void cTaskBoss::ShowDanger()
{
	if (m_pDanger == nullptr) return;
	m_pDanger->pAnita(0)->SetState(1u, true);
	m_pDanger->pAnita(0)->SetAnimNo(0, true, 0.0f);
}

// DOL 0x800cdcdc: CHA_03 (the shells open, +0x78) -> CHA_04 (they can be shot, +0x74); all four broken within o_Vomit01 frames: no vomit;
// else they close (+0x7c) and it vomits (effects 0x37 / 0x40 at ef_body, m01se_35hakidasi, 200 frames; one enemy of the shuffled
// BossEnemyList per frame: cTaskBossEnemyMan 0x800e7ca8); CHA_05 back
bool cTaskBoss::AttackVomit()
{
	cKingdomModel* b = Boss();
	if (b == nullptr) return true;
	switch (m_nAtkStep)
	{
	case 0:
		SetAnm(1, "CHAR_ENEMY_LASTBOSS_ANM_CHA_03", "CHAR_ENEMY_LASTBOSS_HIT_ANM_CHA_03", true);
		for (cTaskShell* sh : m_pShell) if (sh != nullptr) sh->Open();
		Blend(BLEND, 0, 1);
		m_nAtkStep = 1;
		break;
	case 1:
		if (ClipEnd(b, 1u))
		{
			SetAnm(1, "CHAR_ENEMY_LASTBOSS_ANM_CHA_04", "CHAR_ENEMY_LASTBOSS_HIT_ANM_CHA_04", false);
			for (cTaskShell* sh : m_pShell) if (sh != nullptr) sh->Activate();
			m_nAtkStep = 2;
		}
		break;
	case 2:
	{
		// the shells' vtable +0x8c: all four closed (broken)
		bool bShellsDone = true;
		for (cTaskShell* sh : m_pShell) if (sh != nullptr && !sh->IsClosed()) { bShellsDone = false; break; }
		if (m_fAtkTime <= m_Param.fVomit)
		{
			if (!bShellsDone) m_fAtkTime += 1.0f;
			else { m_fAtkTime = 0.0f; m_nAtkStep = 4; }
		}
		else
		{
			m_fAtkTime = 0.0f;
			m_nAtkStep = 3;
			m_Leave.Shuffle();
			for (cTaskShell* sh : m_pShell) if (sh != nullptr) sh->Close();
		}
		break;
	}
	case 3:
	{
		if (m_fAtkTime == 0.0f)
		{
			if (m_pFxVomit != nullptr) m_pFxVomit->Start(FX_VOMIT, false, true, false);
			if (m_pFxVomit2 != nullptr) m_pFxVomit2->Start(FX_VOMIT2, false, true, false);
			m_nSeHakidasi = snd::Play("m01se_35hakidasi");
		}
		if (m_fAtkTime <= VOMIT_TIME)
			m_fAtkTime += 1.0f;
		else
		{
			m_fAtkTime = 0.0f;
			if (m_pFxVomit != nullptr) m_pFxVomit->Stop();
			if (m_pFxVomit2 != nullptr) m_pFxVomit2->Stop();
			snd::Stop(m_nSeHakidasi);
			m_nAtkStep = 4;
		}
		if (const cBossLeaveEnemy::sEntry* e = m_Leave.Next())
		{
			// 0x800e7ca8(enemy man, ef_body in the camera frame (0x800d3714), type, orbit, this)
			float local[3];
			KingdomGlobal::GalagaSys->pTaskCamera()->WorldToLocal(local, m_vBody);
			if (cTaskBossEnemyMan* man = dynamic_cast<cTaskBossEnemyMan*>(cTaskEnemyMan::s_pInstance))
				man->SpawnBossEnemy(local, e->nType, e->nOrbit, this);
		}
		break;
	}
	case 4:
		SetAnm(2, "CHAR_ENEMY_LASTBOSS_ANM_CHA_05", "CHAR_ENEMY_LASTBOSS_HIT_ANM_CHA_05", true);
		Blend(BLEND, 1, 2);
		m_nAtkStep = 5;
		break;
	case 5:
		if (b->GetFrameMaxAnmChr(2u) - BLEND < b->GetFrameAnmChr(2u))
		{
			b->ReleaseAnmChr(1u);
			if (cKingdomModel* h = Hit()) h->ReleaseAnmChr(1u);
			Blend(BLEND, 2, 0);
			m_nAtkStep = 6;
		}
		break;
	case 6:
		if (!m_bBlend)
		{
			b->ReleaseAnmChr(2u);
			if (cKingdomModel* h = Hit()) h->ReleaseAnmChr(2u);
			return true;
		}
		break;
	default: break;
	}
	return false;
}

// the missile mask (+0x3c) of the first missile node (hold, parameter 0) after a course frame; -1 when there is none
u32 cTaskBoss::NextMissileMask(float fAfter) const
{
	u32 mask = 0xffffffffu;
	if (m_pOrbit == nullptr) return mask;
	for (const sEmyNode& n : m_pOrbit->nodes)
	{
		if ((n.nCmd & 0xffff) != 3 || n.nParam != 0) continue;
		if (mask == 0xffffffffu) mask = n.n3c;
		if (fAfter < n.fFrame) return n.n3c;
	}
	return mask;
}

// DOL 0x800cdaf0: the hanging missiles fly (bom flash 0x3d, m01se_11missile), then the empty bom nodes get new ones
bool cTaskBoss::AttackMissile()
{
	if (m_nAtkStep == 0)
	{
		for (int i = 0; i < 4; i++)
		{
			if (m_pMissile[i] == nullptr) continue;
			m_pMissile[i]->Launch();   // vtable +0xcc
			m_pMissile[i] = nullptr;
			if (m_pFxBom[i] != nullptr) m_pFxBom[i]->Start(FX_BOM, true, true, true);
			m_SndActor.Play("m01se_11missile");
		}
		m_nAtkStep++;
		return false;
	}
	if (m_nAtkStep == 1)
	{
		if (m_fReload < RELOAD)
		{
			for (int i = 0; i < 4; i++)
			{
				ReloadMissiles(NextMissileMask(m_fOrbitTime));
				m_fReload = 0.0f;
			}
		}
		else
			m_fReload += 1.0f;
		return true;
	}
	return false;
}

// DOL 0x800cb058: a bit of the mask set = an SP missile (pool +0x200, MISSILE_SPEED), clear = a normal one (pool +0x1fc, SP_MISSILE_SPEED);
// the STRUGGLE speeds in state 4
void cTaskBoss::ReloadMissiles(u32 nMask)
{
	cTaskBossEnemyMan* man = dynamic_cast<cTaskBossEnemyMan*>(cTaskEnemyMan::s_pInstance);
	if (man == nullptr) return;
	const bool bStruggle = m_nState == 4;
	const float fSp = bStruggle ? m_Param.fMissileStruggleSpeed : m_Param.fMissileSpeed;
	const float fNormal = bStruggle ? m_Param.fSpMissileStruggleSpeed : m_Param.fSpMissileSpeed;
	for (int i = 0; i < 4; i++)
	{
		if (m_pMissile[i] != nullptr) continue;
		const bool bSp = (nMask & (1u << i)) != 0;
		cTaskBossMissile* mi = man->GetBossMissile(bSp);
		if (mi == nullptr) continue;
		mi->Setup(bSp ? fSp : fNormal, i, this);
		mi->Attach();   // vtable +0xdc
		m_pMissile[i] = mi;
	}
}

// DOL 0x800ce340: the struggle starts: CHA_09 / CLA_09 (not collidable; the shells break (+0x80); effect 0x3b at ef_body,
// m01se_08bak4, m01se_27boss_02 at frame 115), then CHA_10 / CLA_10 becomes the idle clip
bool cTaskBoss::StruggleStart()
{
	cKingdomModel* b = Boss();
	if (b == nullptr) return true;
	if (m_nAtkStep == 0)
	{
		m_b14 = false;
		SetAnm(1, "CHAR_ENEMY_LASTBOSS_ANM_CHA_09", "CHAR_ENEMY_LASTBOSS_HIT_ANM_CHA_09", true);
		b->RegistAnm(0u, "CHAR_ENEMY_LASTBOSS_ANM_CLA_09");
		for (cTaskShell* sh : m_pShell) if (sh != nullptr) sh->Break();
		if (m_pFxBody != nullptr) m_pFxBody->Start(FX_STRUGGLE, true, true, false);
		m_SndActor.Play("m01se_08bak4");
		Blend(BLEND, 0, 1);
		m_nAtkStep = 1;
	}
	else if (m_nAtkStep == 1)
	{
		if (m_bStruggleSe && STRUGGLE_SE < b->GetFrameAnmChr(1u))
		{
			m_SndActor.Play("m01se_27boss_02");
			m_bStruggleSe = false;
		}
		if (ClipEnd(b, 1u))
		{
			m_fAtkTime = 0.0f;
			m_nAtkStep++;
		}
	}
	else if (m_nAtkStep == 2)
	{
		SetAnm(0, "CHAR_ENEMY_LASTBOSS_ANM_CHA_10", "CHAR_ENEMY_LASTBOSS_HIT_ANM_CHA_10", true);
		b->RegistAnm(0u, "CHAR_ENEMY_LASTBOSS_ANM_CLA_10");
		Blend(BLEND, 1, 0);
		m_nAtkStep = 3;
	}
	else if (m_nAtkStep == 3 && !m_bBlend)
	{
		b->ReleaseAnmChr(1u);
		if (cKingdomModel* h = Hit()) h->ReleaseAnmChr(1u);
		m_b14 = true;
		return true;
	}
	return false;
}

// DOL 0x800cf2e8
void cTaskBoss::FightStep()
{
	UpdateBossOrbit(1.0f);
	UpdateBossFire();
	if (m_pScene != nullptr)
	{
		if (!m_pScene->IsSegmentChanged())
		{
			if (m_pScene->IsLooped()) SetOrbitSpec(2);
		}
		else
		{
			m_nSeg++;
			SetSegmentClip(m_nStage + m_nSeg);
			SyncCourse();
			SetOrbitSpec(2);
		}
	}
	SyncCourse();   // +0x1fa / +0x1f9 (frame / rate set): natively pinned every frame, as cTaskPacman
	if ((float)m_n18 / (float)m_nMaxLife < m_Param.fStruggleRate && m_nFirePattern == 0)
	{
		m_nState = 4;
		m_nStep = 0;
	}
	if (cKingdomModel* orbit = pModel(0)) orbit->UpdateFrame();   // +0xfc (0x800cc65c)
}

// DOL 0x800cf4bc
void cTaskBoss::StruggleStep()
{
	if (m_nStep == 0)
	{
		if (StruggleStart())
		{
			m_fAtkTime = 0.0f;
			m_nAtkStep = 0;
			SetOrbitSpec(6);
			m_nStep++;
		}
	}
	else if (m_nStep == 1)
	{
		UpdateBossOrbit(1.0f);
		UpdateBossFire();
		if (m_pScene != nullptr && m_pScene->IsLooped()) SetOrbitSpec(6);
		if (m_pLifeGauge != nullptr && m_pLifeGauge->IsEmpty())   // 0x800d0a84 on the app's +0x2d4
		{
			m_nState = 5;
			m_nStep = 0;
			// 0x801c47f0 on the hit model task (UNVERIFIED: stops its boxes; natively the boss is not collidable any more below)
		}
	}
	SyncCourse();
	if (cKingdomModel* orbit = pModel(0)) orbit->UpdateFrame();
}

// Term 0x800cafe8: the boss parameters (+0x358) and the sound actor (+0x380) are freed (natively by the destructor / not ported).
// DOL 0x800cf654
bool cTaskBoss::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08 || KingdomGlobal::GalagaSys->bUnk9()) return true;   // 0x800e4704: nothing once Pac-Man missed (0x800afe2c)
	m_bBeam = false;
	m_bBite = false;
	if (std::getenv("MEGAMIX_G01DBG") && m_nState * 100 + m_nFirePattern * 10 + m_nAtkStep != m_nDbg)
	{
		m_nDbg = m_nState * 100 + m_nFirePattern * 10 + m_nAtkStep;
		std::fprintf(stderr, "g01 boss state %d.%d attack %d.%d life %d/%d frame %.1f pos (%.1f %.1f %.1f) body (%.1f %.1f %.1f)\n", m_nState, m_nStep, m_nFirePattern, m_nAtkStep, m_n18,
		             m_nMaxLife, m_fOrbitTime, m_v20.x, m_v20.y, m_v20.z, m_vBody[0], m_vBody[1], m_vBody[2]);
	}
	switch (m_nState)
	{
	case 0:   // the opening demo (0x801bce00 on +0x348): Init (nothing), then while IsWaitEnd (0x801bd53c: always 0); clips while not done
		if (m_nDemoStep == 0) m_nDemoStep = 1;
		else if (!m_bDemoDone) m_bDemoDone = true;
		if (!m_bDemoDone) UpdateFrame();   // +0xfc = cTaskModel::UpdateFrame (0x8009a478)
		break;
	case 1:   // the goal demo (0x801bce94 on +0x348): Start (+0x1c), then while Execute (+0xc); clips while not done
		if (m_nDemoStep == 0)
		{
			GoalInit();
			m_nDemoStep = 1;
		}
		else if (!m_bGoalDone && !GoalCheck())
		{
			m_bGoalDone = true;
		}
		if (!m_bGoalDone) UpdateFrame();
		break;
	case 3: FightStep(); break;
	case 4: StruggleStep(); break;
	case 5:
		SyncCourse();
		if (cKingdomModel* orbit = pModel(0)) orbit->UpdateFrame();
		for (cTaskNodeEffect* e : {m_pFxMouth, m_pFxBody, m_pFxVomit2, m_pFxInhale, m_pFxVomit, m_pFxWing[0], m_pFxWing[1]})
			if (e != nullptr) e->Stop();
		for (cTaskNodeEffect* e : m_pFxBom) if (e != nullptr) e->Stop();
		m_b14 = false;
		m_nState = 6;
		m_nStep = 0;
		break;
	default:   // 2 (waiting) and 6 (defeated): nothing (the clips stand)
		break;
	}
	cTaskModel::Execute(pParam);
	UpdateBlend();       // +0xd4
	UpdateNodes();       // +0x108
	UpdateBossModel();   // +0x90
	UpdateHitEffect();   // +0x10c
	return true;
}

// DOL 0x800cf8b4: the orbit skeleton at T(+0x3e0, the bite) * T(vTrans) * R(vRotate)
void cTaskBoss::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08) return;
	if (cKingdomModel* orbit = pModel(0)) orbit->vTrans() = Vector4(m_vBite[0], m_vBite[1], m_vBite[2], 1.0f);
	cTaskModel::Draw(nStatus, pParam);
}

// (cNode::Create 0x801c23ec / cMesh::Create 0x801c2790 build the node tree of the hit model from the CHR result; cNode::HitCheck 0x801c2670
// asks its child, then its sibling: any hit in the tree. Natively the posed node matrices of the native model stand in for that tree.)
// cTaskHitModel 0x801c473c -> cMesh::HitCheck 0x801c2a30: the ray against every node's volume (as a world AABB of the node's posed matrix)
bool cTaskBoss::HitModelRay(const float org[3], const float dir[3], float& t)
{
	for (const wii::Mdl0Node& n : m_HitNodes)
	{
		if (n.volMin[0] == n.volMax[0] && n.volMin[1] == n.volMax[1] && n.volMin[2] == n.volMax[2]) continue;
		float m[16];
		if (!ChildWorld(1, n.name.c_str(), m)) continue;
		float mn[3] = {1e30f, 1e30f, 1e30f}, mx[3] = {-1e30f, -1e30f, -1e30f};
		for (int k = 0; k < 8; k++)
		{
			const float p[3] = {(k & 1) ? n.volMax[0] : n.volMin[0], (k & 2) ? n.volMax[1] : n.volMin[1], (k & 4) ? n.volMax[2] : n.volMin[2]};
			for (int i = 0; i < 3; i++)
			{
				const float w = m[i] * p[0] + m[4 + i] * p[1] + m[8 + i] * p[2] + m[12 + i];
				mn[i] = std::min(mn[i], w);
				mx[i] = std::max(mx[i], w);
			}
		}
		if (RayAABB(org, dir, mn, mx, t)) return true;
	}
	return false;
}

// DOL 0x800cf250: the shells first (+0x410 = the one hit), then the hit model
bool cTaskBoss::RayHit(const float org[3], const float dir[3], float& t)
{
	m_nShellHit = -1;
	for (int i = 0; i < 4; i++)
		if (m_pShell[i] != nullptr && m_pShell[i]->RayHit(org, dir, t))
		{
			m_nShellHit = i;
			return true;
		}
	return HitModelRay(org, dir, t);
}

// DOL 0x800ce73c: SP missiles (and the boss enemies, kind 0xb) reach the mouth while it inhales; Pac-Man is hurt by the bite
// (unless he jumps) and the beam
bool cTaskBoss::HitCheck(cObject* other)
{
	const int kind = other->nKind();
	if (kind == 0xf)
	{
		if (!m_bMouth) return false;
		const cTaskBossMissile* mi = dynamic_cast<const cTaskBossMissile*>(other);
		if (mi == nullptr) return false;
		if (SegmentSphere(mi->vPrevPos(), mi->vPos(), m_vBody, m_fRadius)) return true;
		const float r = MISSILE_RADIUS + m_fRadius;   // 0x80302600 (sphere vs sphere)
		return Dist2(mi->vPos(), m_vBody) <= r * r;
	}
	if (kind == 0xb)
	{
		// the vomited enemies while it inhales: the segment from their previous position (+0x360) vs the ef_body sphere (the DOL tests any
		// kind 0xb, the shells too, reading their +0x360: natively only cTaskBossEnemy, UNVERIFIED for the shells)
		const cTaskBossEnemy* e = dynamic_cast<const cTaskBossEnemy*>(other);
		return m_bMouth && e != nullptr && SegmentSphere(e->vPrevPos(), e->vPos(), m_vBody, m_fRadius);
	}
	if (kind == 3)
	{
		if (m_bBite) return !other->bJumping();
		return m_bBeam;
	}
	return false;
}

// DOL 0x800ce928
void cTaskBoss::HitCallback(cObject* other)
{
	const int kind = other->nKind();
	if (kind == 1)
	{
		cTaskConsolePlayer* player = dynamic_cast<cTaskConsolePlayer*>(other);
		const int pl = player != nullptr ? player->nPlayer() : 0;
		if (m_nState == 4)
		{
			const float* hp = player != nullptr ? player->vHitPos() : m_vBody;
			(new kingdom::cKingdomPtkHnd())->Generate((uint)pl, true, false, true, 0.0f, Vector4(hp[0], hp[1], hp[2], 1.0f));
			snd::Play("m01se_04hit2");
			if (0 < m_n18)
			{
				m_n18 -= m_Param.nLaserDamage;
				if (m_n18 < 1 && player != nullptr)
				{
					player->AddKill();
					player->AddScore(m_nScore);   // 0x800d73cc
				}
			}
		}
		else
		{
			if (m_nShellHit < 0) snd::Play("m01se_03hit1");
			else if (m_pShell[m_nShellHit] != nullptr) m_pShell[m_nShellHit]->HitCallback(other);
			if (!m_HitFx.IsValid()) m_HitFx.Generate(FX_HIT, true, false, true, 0.0f, Vector4(m_vBody[0], m_vBody[1], m_vBody[2], 1.0f));
		}
	}
	else if (kind == 0xf)
	{
		const cTaskBossMissile* mi = dynamic_cast<const cTaskBossMissile*>(other);
		m_n18 -= mi != nullptr ? mi->nDamage() : 0;
		if (m_n18 < 1) m_n18 = 1;
		if (m_bMouth) m_bHitInhale = true;
	}
}

}
