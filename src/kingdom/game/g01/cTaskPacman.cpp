#include "kingdom/game/g01/cTaskPacman.hpp"
#include "kingdom/game/g01/cTaskNodeEffect.hpp"
#include "wii/Mdl0.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "KingdomGlobal.hpp"
#include "platform/devtools.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g01/cTaskCollisionMan.hpp"
#include "kingdom/game/g01/cTaskConsoleLife.hpp"
#include "kingdom/game/g01/cTaskEffect.hpp"
#include "kingdom/game/g01/cTaskEnemy.hpp"
#include "kingdom/game/g01/cTaskScene.hpp"
#include "snd/Mixer.hpp"
#include "wii/Anim.hpp"
#include "wii/Archive.hpp"

// The demo objects (0x800f3eec: cDemoPacmanW1S1 / W1S3 / W2S3 / W3S3, cTaskBossPacman's cDemoPacmanBoss) run on Pac-Man natively. Not
// ported: the remote rumble (0x80021a14), the shadow of 0x800f4388.
namespace kingdom::game::g01
{

// DOL 0x800f40f0 [audited: param/demo setup (+0x3a4/+0x3a8, PacmanParam read) is the port's own]
cTaskPacman::cTaskPacman(int nStage, cTaskScene* pScene)
{
	SetName("g01::cTaskPacman");
	nDrawPriority(0x10000u);
	m_nStage = nStage;
	m_nWorld = KingdomGlobal::GalagaSys->nWorld();
	m_pScene = pScene;
	LoadModel();
	m_b08 = true;
	// kind 3, registered with the collision pass (0x800d41c8); PacmanParam.csv (cPacmanParam) MAX_INVINCIBLE
	m_nKind = 3;
	m_b14 = true;
	if (cTaskCollisionMan* col = KingdomGlobal::GalagaSys->pTaskCollisionMan()) col->Regist(this);
	kingdom::cKingdomCSV csv;
	csv.RegistData("PacmanParam.csv");
	int inv = 30;
	if (csv.S32(csv.SearchRow(0, "MAX_INVINCIBLE", 0, 0), 1, inv)) m_fMaxInvincible = (float)inv;
	csv.F32(csv.SearchRow(0, "GRAVITY", 0, 0), 1, m_fGravity);
	csv.F32(csv.SearchRow(0, "JUMP_START_SPEED", 0, 0), 1, m_fJumpStart);
	csv.F32(csv.SearchRow(0, "JUMP_MAX_RIGIDITY", 0, 0), 1, m_fRigidity);
	s_pInstance = this;
	m_nState = 2;   // 0x800f3e04 (natively explicit: state 0 is the demo)
	if (nStage != 10)   // 0x800f40f0: the goal skeleton as model 1, the run starts at node 1
	{
		char name[0x80];
		std::snprintf(name, sizeof name, "m01m_s%x_goal_s.brres", nStage);   // s_m01m_s_x_goal_s_brres_8037502e
		m_nGoalNode = 1;
		CreateModel(1)->RegistModel(name, 0x36f);
	}
	// 0x800f3eec: cDemoPacmanW1S1 (stage 1, the opening demo), cDemoPacmanW1S3 / W2S3 / W3S3 (stages 3 / 6 / 9, the goal demos); stage 10:
	// cTaskBossPacman's +0x78 = 0x800d0bf8 (cDemoPacmanBoss)
	if (nStage == 1 || nStage == 3 || nStage == 6 || nStage == 9 || nStage == 10)
		if (cDemoMan* dm = KingdomGlobal::GalagaSys->pDemoMan()) dm->Regist(this);
	if (nStage == 3)
	{
		// cDemoPacmanW1S3 0x801bef18: Demo_s3.csv frames
		kingdom::cKingdomCSV d;
		if (d.RegistData("Demo_s3.csv"))
		{
			const char* key[6] = {"e_jump01", "e_shine01", "e_line01", "s_shine01", "s_jump01", "s_set01"};
			for (int i = 0; i < 6; i++) d.F32(d.SearchRow(0, key[i], 0, 0), 1, m_fGoalTime[i]);
		}
	}
	else if (nStage == 6)
	{
		// cDemoPacmanW2S3 0x801bf53c: Demo_s6.csv (s_Demo_s6_csv_8037dfb4)
		kingdom::cKingdomCSV d;
		if (d.RegistData("Demo_s6.csv"))
		{
			const char* key[5] = {"e_abduct01", "e_tractor01", "s_fly01", "s_tractor01", "s_appear01"};
			for (int i = 0; i < 5; i++) d.F32(d.SearchRow(0, key[i], 0, 0), 1, m_fGoalTime2[i]);
		}
	}
	else if (nStage == 9)
	{
		// cDemoPacmanW3S3 0x801bfb44: Demo_s9.csv (s_Demo_s9_csv_8037dfd9)
		kingdom::cKingdomCSV d;
		if (d.RegistData("Demo_s9.csv"))
		{
			const char* key[14] = {"e_question01", "e_question02", "e_exclamation01", "e_exclamation02", "s_pac_bound_01", "s_pac_bound_02",
			                       "s_pac_bound_03", "s_pac_bound_04", "s_pac_bound_05", "s_pac_bound_06", "s_pac_bound_07", "s_pac_bound_08",
			                       "s_pac_bound_09", "s_pac_bound_10"};
			for (int i = 0; i < 14; i++) d.F32(d.SearchRow(0, key[i], 0, 0), 1, m_fGoalTime2[i]);
		}
	}
}

// DOL 0x800f520c [audited] (n = 1, the opening demo) / 0x800f5860 (n = 2 / 3 / 4 for stages 3 / 6 / 9, the goal demos): the model again (+0xac), the
// orbit clip m01m_pac_ob_a%x0_cha at 1.0, Pac-Man's a%x0 clips (cha / tpa / via at 1.0); the longest of cha / tpa / via is the demo's length
void cTaskPacman::SetDemoClip(int n)
{
	DeleteModel(0);
	LoadModel();
	cKingdomModel* orbit = pModel(0);
	char name[0x80];
	std::snprintf(name, sizeof name, "m01m_pac_ob_a%x0_cha.brres", n);
	orbit->RegistAnm(0u, name);
	orbit->SetUpdateRateAnmChr(0u, 1.0f);
	static const char* const AN[5] = {"m01m_pac_a%x0_cha.brres", "m01m_pac_a%x0_cla.brres", "m01m_pac_a%x0_tsa.brres", "m01m_pac_a%x0_tpa.brres",
	                                  "m01m_pac_a%x0_via.brres"};
	cKingdomModel* pac = orbit->pExpandMdl(0);
	m_fDemoMax = 0.0f;
	m_nDemoKind = 0;
	for (int i = 0; pac != nullptr && i < 5; i++)
	{
		std::snprintf(name, sizeof name, AN[i], n);
		std::size_t size = 0;
		if (wii::ArchiveStore::Find(name, size) == nullptr) continue;
		pac->RegistAnm(0u, name);
		float max = 0.0f;
		if (i == 0) { pac->SetUpdateRateAnmChr(0u, 1.0f); max = pac->GetFrameMaxAnmChr(0u); }
		else if (i == 3) { pac->SetUpdateRateAnmTexPat(0u, 1.0f); max = pac->GetFrameMaxAnmTexPat(0u); }
		else if (i == 4) { pac->SetUpdateRateAnmVis(0u, 1.0f); max = pac->GetFrameMaxAnmVis(0u); }
		if (m_fDemoMax < max) { m_fDemoMax = max; m_nDemoKind = i; }
	}
}

void cTaskPacman::DemoFinish()
{
	m_bDemoDone = true;
	m_bGoalDone = true;
	if (m_nStage != 1) return;   // the goal demos' Term (0x800c961c) does nothing
	DeleteModel(0);
	LoadModel();
	SetStartClip();   // 0x800f5ab0
}

// the world position of Pac-Man's node ef_body: orbit world * pac_orbit * the expanded model's local * ef_body (0x800e0820 binds the effect
// to that ScnObj node)
bool cTaskPacman::NodeWorld(const char* node, float out[3])
{
	cKingdomModel* orbit = pModel(0);
	cKingdomModel* pac = orbit != nullptr ? orbit->pExpandMdl(0) : nullptr;
	float w[16], l[16], a[12], b[12];
	if (pac == nullptr || !orbit->NodeMatrix("pac_orbit", a) || !pac->NodeMatrix(node, b)) return false;
	orbit->CalcWorld(w);
	pac->CalcWorld(l);
	// p = W * A * L * B * (0, 0, 0, 1); A, B 3x4 row major, W, L column major
	float p[3] = {b[3], b[7], b[11]};
	float q[3];
	for (int i = 0; i < 3; i++) q[i] = l[i] * p[0] + l[4 + i] * p[1] + l[8 + i] * p[2] + l[12 + i];
	for (int i = 0; i < 3; i++) p[i] = a[i * 4] * q[0] + a[i * 4 + 1] * q[1] + a[i * 4 + 2] * q[2] + a[i * 4 + 3];
	for (int i = 0; i < 3; i++) out[i] = w[i] * p[0] + w[4 + i] * p[1] + w[8 + i] * p[2] + w[12 + i];
	return true;
}

// cDemoPacmanW1S3::Start 0x801bf138: vtable +0xcc (the goal clips 0x800f5860) and three effects at ef_body (DAT_80478550) connected to the
// app's task root; W2S3 / W3S3 below
// cDemoPacmanBoss::Start 0x801c0358 (stage 10): vtable +0xcc = 0x800d1054: the effect handle +0x410 detached, the a50 clips like 0x800f5860,
// the jump offset +0x2d0 = (0, 0, 0, 1)
// the world matrix of a node of the expanded Pac-Man: orbit world * pac_orbit * the expanded model's local * node
bool cTaskPacman::NodeMtx(const char* node, float m[16])
{
	cKingdomModel* orbit = pModel(0);
	cKingdomModel* pac = orbit != nullptr ? orbit->pExpandMdl(0) : nullptr;
	float a[12], b[12], w[16], l[16], am[16], bm[16];
	if (pac == nullptr || !orbit->NodeMatrix("pac_orbit", a) || !pac->NodeMatrix(node, b)) return false;
	orbit->CalcWorld(w);
	pac->CalcWorld(l);
	cTaskNodeEffect::FromNode(a, am);
	cTaskNodeEffect::FromNode(b, bm);
	cTaskNodeEffect::Mul(w, am, m);
	cTaskNodeEffect::Mul(m, l, m);
	cTaskNodeEffect::Mul(m, bm, m);
	return true;
}

// ...; cDemoPacmanW2S3::Start 0x801bf768: the clips, an effect at ef_body (0x800e0820, PTR_s_ef_body_80478550) and an unbound one
// (0x800e06dc), both connected to the app's task root; cDemoPacmanW3S3::Start 0x801bfd8c: the clips and four effects at ef_top
// (PTR_s_ef_top_80478554)
void cTaskPacman::GoalInit()
{
	SetDemoClip(m_nStage == 3 ? 2 : m_nStage == 6 ? 3 : m_nStage == 10 ? 5 : 4);
	m_fGoalLast = 0.0f;
	m_nGoalSub = 0;
	if (m_nStage == 10) for (float& v : m_vJumpOfs) v = 0.0f;
	const char* node = m_nStage == 3 || m_nStage == 6 ? "ef_body" : m_nStage == 9 ? "ef_top" : nullptr;
	const int n = m_nStage == 3 ? 3 : m_nStage == 6 ? 2 : m_nStage == 9 ? 4 : 0;
	for (int i = 0; i < n; i++)
	{
		cTaskNodeEffect*& e = m_pGoalEffect[i];
		std::string nd = node;
		e = new cTaskNodeEffect([this, nd](float o[3]) { return NodeWorld(nd.c_str(), o); });
		e->SetMtx([this, nd](float m[16]) { return NodeMtx(nd.c_str(), m); });   // the node's whole matrix (0x800e11fc)
		ConnectChild(e);
	}
}

// cDemoPacmanW1S3::Execute 0x801bf294: while the longest clip runs, at their Demo_s3.csv frames: m01se_23kiraan (s_shine01), m01se_18jump
// (s_jump01), m01se_29kamituki (s_set01), PTK_LINE01 (0x19, e_line01: one-time, dependent), PTK_JUMP01 (0x1a, e_jump01: looping), PTK_SHINE01
// (0x16, e_shine01: one-time); -1 = done. Native: a wrap of the clip counts as its end (UNVERIFIED)
bool cTaskPacman::GoalCheck()
{
	cKingdomModel* pac = pModel(0)->pExpandMdl(0);
	if (pac == nullptr) return false;
	const float f = m_nDemoKind == 3 ? pac->GetFrameAnmTexPat(0u) : m_nDemoKind == 4 ? pac->GetFrameAnmVis(0u) : pac->GetFrameAnmChr(0u);
	if (f >= m_fDemoMax || f < m_fGoalLast)
	{
		// cDemoPacmanBoss::Execute 0x801c0374: at the end CHAR_PACMAN_ANM_TPA_05 (0x456b6a5b) and CHAR_PACMAN_ANM_CHA_06 (0x9221d581) in
		// slot 0 (FUN_8001a4d8(pac, 0, crc, 0, 1))
		if (m_nStage == 10)
		{
			pac->RegistAnm(0u, "CHAR_PACMAN_ANM_TPA_05");
			pac->RegistAnm(0u, "CHAR_PACMAN_ANM_CHA_06");
		}
		return false;
	}
	m_fGoalLast = f;
	auto due = [f](float& t) { if (t != -1.0f && t <= f) { t = -1.0f; return true; } return false; };   // FLOAT_8047cf18 = -1: done
	if (m_nStage == 6)
	{
		// cDemoPacmanW2S3::Execute 0x801bf848: on its first frame Pac-Man's model slot 20 = the Boss Galaga (CHAR_ENEMY_BOSS2_MDL 0x29fa50a0) with
		// m01m_g03_2_a30_cha at 1.0 (FLOAT_8047cf28); at the Demo_s6.csv frames: s_tractor01 m01se_25beam, s_fly01 m01se_30boss_fly, s_appear01
		// m01se_24appearance, e_abduct01 effect 0x26 at Pac-Man's ef_body, e_tractor01 the unbound effect bound to the boss's ef_mouth (0x800e0ecc)
		// and 0x25 (both one-time, dependent, not clipped)
		if (m_nGoalSub == 0)
		{
			cKingdomModel* boss = CreateModel(20u);   // FUN_8009991c(+0x88, 0x14)
			boss->RegistModel("CHAR_ENEMY_BOSS2_MDL", 0x36f);
			boss->RegistAnm(0u, "m01m_g03_2_a30_cha.brres");   // s_m01m_g03_2_a30_cha_brres_8037dfc0
			boss->SetUpdateRateAnmChr(0u, 1.0f);
			m_nGoalSub++;
		}
		if (due(m_fGoalTime2[3])) snd::Play("m01se_25beam");
		if (due(m_fGoalTime2[2])) snd::Play("m01se_30boss_fly");
		if (due(m_fGoalTime2[4])) snd::Play("m01se_24appearance");
		if (due(m_fGoalTime2[0]) && m_pGoalEffect[0]) m_pGoalEffect[0]->Start(0x26u, true, true, false);
		if (due(m_fGoalTime2[1]) && m_pGoalEffect[1])
		{
			m_pGoalEffect[1]->SetPos([this](float o[3]) { float m[16]; if (!cTaskNodeEffect::ModelNodeMtx(pModel(20u), "ef_mouth", m)) return false;
			                                              o[0] = m[12]; o[1] = m[13]; o[2] = m[14]; return true; });
			m_pGoalEffect[1]->SetMtx([this](float m[16]) { return cTaskNodeEffect::ModelNodeMtx(pModel(20u), "ef_mouth", m); });   // PTR_s_ef_mouth_80478558
			m_pGoalEffect[1]->Start(0x25u, true, true, false);
		}
		return true;
	}
	if (m_nStage == 9)
	{
		// cDemoPacmanW3S3::Execute 0x801bff44: e_question01 / 02 effect 0x2d + m01se_37hatena, e_exclamation01 / 02 effect 0x2e + m01se_41bikkuri
		// (one-time, dependent, clipped; each its own effect at ef_top), s_pac_bound_01..10 m01se_39bound
		const unsigned slot[4] = {0x2du, 0x2du, 0x2eu, 0x2eu};
		for (int i = 0; i < 4; i++)
			if (due(m_fGoalTime2[i]))
			{
				if (m_pGoalEffect[i]) m_pGoalEffect[i]->Start(slot[i], true, true, true);
				snd::Play(i < 2 ? "m01se_37hatena" : "m01se_41bikkuri");
			}
		for (int i = 4; i < 14; i++)
			if (due(m_fGoalTime2[i])) snd::Play("m01se_39bound");
		return true;
	}
	if (m_nStage != 3) return true;
	if (due(m_fGoalTime[3])) snd::Play("m01se_23kiraan");
	if (due(m_fGoalTime[4])) snd::Play("m01se_18jump");
	if (due(m_fGoalTime[5])) snd::Play("m01se_29kamituki");
	if (due(m_fGoalTime[2]) && m_pGoalEffect[2]) m_pGoalEffect[2]->Start(0x19u, true, true, true);
	if (due(m_fGoalTime[0]) && m_pGoalEffect[0]) m_pGoalEffect[0]->Start(0x1au, false, false, true);
	if (due(m_fGoalTime[1]) && m_pGoalEffect[1]) m_pGoalEffect[1]->Start(0x16u, true, false, true);
	return true;
}

// DOL 0x800f4f20 [audited: the remote rumble (0x80021a14) is not ported, the hit cue goes through the unpositioned snd::Play]: enemies (0xb), the gate (0xd), hit shots (7), bombs (8), spikes (0xe / 0xf) hurt while not invincible: the hit clips
// (vtable +0xa0), effect 0x10, the invincible time, the cue; the remote rumble (0x80021a14) is not ported. A cookie (4) heals.
void cTaskPacman::HitCallback(cObject* other)
{
	const int k = other->nKind();
	if (k == 4)
	{
		// a cookie (cTaskCookie): m01se_23kiraan (DAT_8047b69c), effect 0x11 at Pac-Man (+0x350: one-time, dependent, not clipped), the life
		// gauge heals when idle (0x800eb6b0, +0x21c = 1)
		snd::Play("m01se_23kiraan");
		if (m_pCookieEffect == nullptr) m_pCookieEffect = new cKingdomPtkHnd();
		m_pCookieEffect->Generate(0x11u, true, true, false, 0.0f, Vector4(m_v20.x, m_v20.y, m_v20.z, 1.0f));
		if (m_pLife != nullptr) m_pLife->Heal();
		if (std::getenv("MEGAMIX_G01DBG")) std::fprintf(stderr, "g01 pacman eats a cookie (frame %d)\n", devtools::FrameNo()); // devtools: frame number
		return;
	}
	if (!(k == 0xb || k == 0xd || k == 7 || k == 8 || k == 0xe || k == 0xf)) return;
	if (m_fInvincible > 0.0f) return;
	if (devtools::G01Invincible()) return; // devtools: --g01-invincible (not 1:1)
	if (m_pLife != nullptr) m_pLife->Damage();   // 0x800eb6b0 -> +0x21c = 2
	HitClip();   // vtable +0xa0
	if (m_pHitEffect == nullptr) m_pHitEffect = new cKingdomPtkHnd();
	m_pHitEffect->Generate((uint)cTaskEffect::ePACMAN_HIT01, true, true, true, 0.0f, Vector4(m_v20.x, m_v20.y, m_v20.z, 1.0f));
	m_fInvincible = m_fMaxInvincible;
	// DAT_8047b694 / 690 (gauge +0x224 == 0): the death cue on Pac-Man's actor (+0x460; nothing in the DOL moves it from (0, 0, 0): UNVERIFIED),
	// the hit cue through FUN_801c15ac (unpositioned)
	if (m_pLife != nullptr && m_pLife->nLife() == 0) m_SndActor.Play("m01se_14p_dead");
	else snd::Play("m01se_13p_hit");
}

// DOL 0x800f5d94 [audited] (vtable +0xa0): the hit clips CHAR_PACMAN_ANM_VIS_02 (0x01273414) and CHAR_PACMAN_ANM_TPA_02 (0xdb0ffff8) in slot 0
void cTaskPacman::HitClip()
{
	cKingdomModel* orbit = pModel(0);
	if (cKingdomModel* pac = orbit != nullptr ? orbit->pExpandMdl(0) : nullptr)
	{
		pac->RegistAnm(0u, "CHAR_PACMAN_ANM_VIS_02");
		pac->RegistAnm(0u, "CHAR_PACMAN_ANM_TPA_02");
	}
}

// DOL 0x800f5e00 [audited] (vtable +0xa4): the hit clips released (vis FUN_8001ad60, pattern FUN_8001b1bc)
void cTaskPacman::HitEnd()
{
	cKingdomModel* orbit = pModel(0);
	if (cKingdomModel* pac = orbit != nullptr ? orbit->pExpandMdl(0) : nullptr)
	{
		pac->ReleaseAnmVis(0u);
		pac->ReleaseAnmTexPat(0u);
	}
}

// DOL 0x800f41d4 [audited]: the orbit skeleton (CRC 0xdab12f18) + Pac-Man (0xf40fa7fc) at "pac_orbit"; both light set 1 (characters), fog 0
void cTaskPacman::LoadModel()
{
	cKingdomModel* orbit = CreateModel(0);
	orbit->RegistModel("CHAR_PACMAN_ORBIT_MDL", 0x36f);
	orbit->SetLightSetIdx(1);
	orbit->SetFogIdx(0);
	orbit->IsDrawShadowDepth(true);   // vtable +0x20 (1) at 0x800f4230
	orbit->RegistExpand(0, "pac_orbit", "CHAR_PACMAN_MDL", 0x36f);
	if (cKingdomModel* pac = orbit->pExpandMdl(0))
	{
		pac->SetLightSetIdx(1);
		pac->SetFogIdx(0);
		pac->IsDrawShadowDepth(true);   // vtable +0x20 (1) at 0x800f427c
		// 0x800f4388 sets the expanded model's translate to the jump offset + pac_move with y negated: Pac-Man rests one radius above the
		// course locator pac_orbit (which runs along the track surface)
		if (const wii::Mdl0* mdl = pac->pMdl0())
			for (const wii::Mdl0Node& n : mdl->nodes)
				if (n.name == "pac_move") m_fLift = -n.translate[1];
		pac->vTrans() = Vector4(0.0f, m_fLift, 0.0f, 1.0f);
	}
	if (const wii::Mdl0* mdl = orbit->pMdl0())
		for (const wii::Mdl0Node& n : mdl->nodes)
			if (n.name == "boss_bite") for (int i = 0; i < 3; i++) m_vBiteNode[i] = n.translate[i];
}

// DOL 0x800f5ab0 [audited]: stage 1 has its own start clip, the others the world's
void cTaskPacman::SetStartClip()
{
	DeleteModel(0);
	LoadModel();   // vtable +0xac = 0x800f41d4: the model again
	cKingdomModel* orbit = pModel(0);
	char name[0x100];
	if (m_nStage == 1)
		std::snprintf(name, sizeof name, "m01m_s1s_pac_ob_cha.brres");
	else
		std::snprintf(name, sizeof name, "m01m_w%xstart_ob_cha.brres", m_nWorld);
	orbit->RegistAnm(0u, name);
	std::snprintf(name, sizeof name, "m01m_w%xstart_p_cha.brres", m_nWorld);
	if (cKingdomModel* pac = orbit->pExpandMdl(0)) pac->RegistAnm(0u, name);
	orbit->SetFrameAnmChr(0u, 0.0f);
	if (cKingdomModel* pac = orbit->pExpandMdl(0)) pac->SetFrameAnmChr(0u, 0.0f);
	orbit->SetUpdateRateAnmChr(0u, 1.0f);
}

// DOL 0x800f5bf0 [audited]: the course clip and the run clip CHAR_PACMAN_ANM_CHA_01 (0x0c454022), then SyncCourse
void cTaskPacman::SetCourseClip()
{
	DeleteModel(0);
	LoadModel();   // vtable +0xac = 0x800f41d4: the model again
	cKingdomModel* orbit = pModel(0);
	char name[0x80];
	std::snprintf(name, sizeof name, "m01m_s%x_pac_ob_cha.brres", m_nStage);
	orbit->RegistAnm(0u, name);
	if (cKingdomModel* pac = orbit->pExpandMdl(0)) pac->RegistAnm(0u, "CHAR_PACMAN_ANM_CHA_01");
	SyncCourse();
}

// DOL 0x800f42c8 [audited]
void cTaskPacman::SyncCourse()
{
	cKingdomModel* orbit = pModel(0);
	cKingdomModel* pac = orbit->pExpandMdl(0);
	orbit->SetFrameAnmChr(0u, m_pScene->fFrame());
	if (pac != nullptr) pac->SetFrameAnmChr(0u, m_pScene->fFrame());
	orbit->SetUpdateRateAnmChr(0u, m_pScene->fRate());
	if (pac != nullptr) pac->SetUpdateRateAnmChr(0u, m_pScene->fPacmanSpeed());
	m_fLastFrame = m_pScene->fFrame();
}

void cTaskPacman::ReadyStart()
{
	m_nState = 3;
	m_nStep = 0;
	SetStartClip();
}

// DOL 0x800f53cc [audited: frame/rates pinned every tick instead of by the scene flags +0x1fa/+0x1f9/+0x1fb; enemy target computed from the pose instead of copying +0x20]. The DOL re-sends the frame / rates only when the scene flags a change (+0x1fa / +0x1f9 / +0x1fb); natively the orbit clip is
// pinned to the scene frame every tick (the same value: both clocks start together and advance by the same rate).
void cTaskPacman::GameStep()
{
	PreGameStep();
	if (m_pScene->IsCourseEnd())
	{
		m_nState = 8;   // vtable +0x100 = 0x800f3e7c: the goal run, not collidable
		m_nStep = 0;
		m_b14 = false;
	}
	cKingdomModel* orbit = pModel(0);
	cKingdomModel* pac = orbit->pExpandMdl(0);
	orbit->SetFrameAnmChr(0u, m_pScene->fFrame());
	orbit->SetUpdateRateAnmChr(0u, 0.0f);
	if (pac != nullptr) pac->SetUpdateRateAnmChr(0u, m_pScene->fPacmanSpeed());
	m_fLastFrame = m_pScene->fFrame();
	if (0.0f < m_fInvincible)   // the invincible time; the hit effect follows Pac-Man (0x8008a49c); at its end the hit clips go (+0xa4)
	{
		m_fInvincible -= 1.0f;
		if (m_pHitEffect != nullptr) m_pHitEffect->vSetTrans(Vector4(m_v20.x, m_v20.y, m_v20.z, 1.0f));
		if (m_fInvincible <= 0.0f)
		{
			HitEnd();
			if (m_pHitEffect != nullptr && m_pHitEffect->IsValid()) m_pHitEffect->Detach(false);   // FUN_8008a508(+0x300, 0)
		}
	}
	if (m_pCookieEffect != nullptr && m_pCookieEffect->IsValid()) m_pCookieEffect->vSetTrans(Vector4(m_v20.x, m_v20.y, m_v20.z, 1.0f));   // +0x350
	JumpStep();      // vtable +0xbc
	UpdateFrame();   // vtable +0xb8
	// the end of 0x800f53cc: enemy manager +0x1c0 = Pac-Man's position m_vPos, from the pose 0x800f4388:
	//   m_vPos = T(CHR "position" translate) * T(model 0 +0x00) * Rz Ry Rx(CHR "pac_orbit" rotate) * (node pac_orbit + node pac_move with y negated)
	// natively with the bind offsets (pac_orbit 0, pac_move (0, -0.3873, 0) -> +0.3873; UNVERIFIED: 0x8001dd54 may return the animated value)
	if (cTaskEnemyMan* man = cTaskEnemyMan::s_pInstance)
	{
		wii::BoneKey pos, rot;
		auto chr = orbit->AnmChrOf(0u);
		if (chr != nullptr && chr->Sample("position", m_pScene->fFrame(), pos) && pos.hasTrans)
		{
			// R = Rz Ry Rx of the CHR "pac_orbit" rotation (identity without one)
			float rx = 0.0f, ry = 0.0f, rz = 0.0f;
			if (chr->Sample("pac_orbit", m_pScene->fFrame(), rot) && rot.hasRotate)
			{
				const float d2r = 0.017453292f;
				rx = rot.rotate[0] * d2r; ry = rot.rotate[1] * d2r; rz = rot.rotate[2] * d2r;
			}
			m_vOrbitRot[0] = rx; m_vOrbitRot[1] = ry; m_vOrbitRot[2] = rz;   // +0x280 (FLOAT_8047b6bc = pi / 180)
			auto rotate = [rx, ry, rz](const float in[3], float out[3]) {
				float t[3], v[3];
				t[0] = in[0]; t[1] = std::cos(rx) * in[1] - std::sin(rx) * in[2]; t[2] = std::sin(rx) * in[1] + std::cos(rx) * in[2];   // Rx
				v[0] = std::cos(ry) * t[0] + std::sin(ry) * t[2]; v[1] = t[1]; v[2] = -std::sin(ry) * t[0] + std::cos(ry) * t[2];       // Ry
				out[0] = std::cos(rz) * v[0] - std::sin(rz) * v[1]; out[1] = std::sin(rz) * v[0] + std::cos(rz) * v[1]; out[2] = v[2];  // Rz
			};
			const float lift[3] = {0.0f, 0.3873f, 0.0f};
			float off[3];
			rotate(lift, off);
			const float l = std::sqrt(off[0] * off[0] + off[1] * off[1] + off[2] * off[2]);
			if (l > 0.0f) for (int i = 0; i < 3; i++) m_vUp[i] = off[i] / l;
			for (int i = 0; i < 3; i++) man->m_vTarget[i] = pos.trans[i] + off[i];
			// +0x2f0: R * (0, 0, 1) (DAT_80374e80), normalized, scaled by -3 (DAT_80374e00), added to the position
			const float fwd[3] = {0.0f, 0.0f, 1.0f};
			float z[3];
			rotate(fwd, z);
			const float zl = std::sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
			for (int i = 0; i < 3; i++) m_vAimPoint[i] = man->m_vTarget[i] + (zl > 0.0f ? -3.0f * z[i] / zl : 0.0f);
			// +0x290: the orbit model's node boss_bite (PTR_s_boss_bite_804779a8, FUN_8001dd54: natively its bind translate) through the same
			// T(position) * R: where the boss bites and the rescued fighter flies to
			float b[3];
			rotate(m_vBiteNode, b);
			for (int i = 0; i < 3; i++) m_vBite[i] = pos.trans[i] + b[i];
		}
		m_v20 = Vector4(man->m_vTarget[0], man->m_vTarget[1], man->m_vTarget[2], 1.0f);   // cObject +0x20 = m_vPos
	}
	// 0x800f4388: up (+0x2a0) = normalize(R(pac_orbit) * (0, -pac_move y, 0)); the expanded model's translate = jump offset + (0, lift, 0)
	if (cKingdomModel* pac = orbit->pExpandMdl(0))
		pac->vTrans() = Vector4(m_vJumpOfs[0], m_vJumpOfs[1] + m_fLift, m_vJumpOfs[2], 1.0f);
}

// cTaskPacman's Init 0x800f2e4c (kind 3, state 2, collidable, the counters, PacmanParam.csv GRAVITY / JUMP_START_SPEED / JUMP_MAX_RIGIDITY /
// MAX_INVINCIBLE, the sound actor) is the constructor here; Term 0x800f3070 frees the parameters and the sound actor; the demo interface's goal
// start +0xe8 = 0x800f3d94 is DemoGoalStart. cTaskBossPacman's Init 0x800d0be4 (segment +0x464 / +0x468 = 0) is its member initialisers.
cTaskPacman* cTaskPacman::s_pInstance = nullptr;

// DOL 0x800f3ed0 [audited]
void cTaskPacman::RequestJump()
{
	if (m_pScene != nullptr && m_pScene->b240()) return;
	m_bJumpReq = true;
}

// DOL 0x800f4be8 [audited]: 0 a request starts it; 1 speed JUMP_START_SPEED, height 0, PTK_PAC_JUMPSTART01 (0x41), m01se_18jump; 2 speed -= GRAVITY,
// height += speed, offset = height * up, landed when the offset no longer points up: PTK_PAC_JUMPEND01 (0x42), 3; 3 JUMP_MAX_RIGIDITY
// frames, then 0 once the request is gone
void cTaskPacman::JumpStep()
{
	const Vector4 pos(m_v20.x, m_v20.y, m_v20.z, 1.0f);
	if (m_nJump == 0)
	{
		if (m_bJumpReq) m_nJump = 1;
	}
	else if (m_nJump == 1)
	{
		m_nJump = 2;
		m_fJumpVel = m_fJumpStart;
		m_fJumpH = 0.0f;
		(new cKingdomPtkHnd())->Generate(0x41u, true, false, true, 0.0f, pos);
		snd::Play("m01se_18jump");   // DAT_8047b698
	}
	else if (m_nJump == 2)
	{
		m_fJumpVel -= m_fGravity;
		m_fJumpH += m_fJumpVel;
		for (int i = 0; i < 3; i++) m_vJumpOfs[i] = m_fJumpH * m_vUp[i];
		float n[3] = {m_vJumpOfs[0], m_vJumpOfs[1], m_vJumpOfs[2]};
		const float l = std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
		const float dot = l > 0.0f ? (m_vUp[0] * n[0] + m_vUp[1] * n[1] + m_vUp[2] * n[2]) / l : 0.0f;
		if (dot <= 0.0f)
		{
			for (float& v : m_vJumpOfs) v = 0.0f;
			m_nJump = 3;
			m_fJumpWait = 0.0f;
			(new cKingdomPtkHnd())->Generate(0x42u, true, false, true, 0.0f, pos);
		}
	}
	if (std::getenv("MEGAMIX_G01DBG") && m_nJump != 0) std::fprintf(stderr, "g01 jump state %d height %.3f\n", m_nJump, m_fJumpH);
	if (m_nJump == 3)
	{
		if (m_fJumpWait < m_fRigidity) m_fJumpWait += 1.0f;
		else if (!m_bJumpReq) m_nJump = 0;
	}
}

// DOL 0x800f5e4c [audited: demo states 0/1 approximate the cDemo objects]
bool cTaskPacman::Execute(cJetTask* pParam)
{
	if (!m_b08 || cG01GalagaSys::IsPause()) return true;   // +8 and DAT_80479118: nothing while hidden / paused
	if (std::getenv("MEGAMIX_G01DBG"))
	{
		static int last = -1;
		const int now = m_nState * 16 + m_nStep;
		if (now != last) std::fprintf(stderr, "g01 pacman state %d step %d (goal node %d)\n", m_nState, m_nStep, m_nGoalNode);
		last = now;
	}
	switch (m_nState)
	{
	case 1:   // the goal demo: +0xb4 = 0x800f3db0: 0x801bce94 on the demo object (Start +0x1c on the first frame, then while Execute +0xc
	          // holds; the base objects' check 0x800f6030 is 0), the clips advance (+0xb8) while it is not done; cTaskBossPacman's +0xb4 =
	          // 0x800d0d8c advances them every frame
		if (m_nGoalStep == 0)
		{
			if (m_nStage == 3 || m_nStage == 6 || m_nStage == 9 || m_nStage == 10) GoalInit();
			m_nGoalStep = 1;
		}
		else if (!m_bGoalDone)
		{
			const bool has = m_nStage == 3 || m_nStage == 6 || m_nStage == 9 || m_nStage == 10;
			if (!has || !GoalCheck()) m_bGoalDone = true;
		}
		if (!m_bGoalDone || m_nStage == 10) UpdateFrame();
		break;
	case 0:   // the opening demo (+0xb0 = 0x800f3d40): Init (+0x14, cDemoPacmanW1S1 0x801beeb4 -> +0xc8) on the first frame, then while IsWaitEnd
	          // (+8: W1S1 0x801beed0 = the longest a10 clip runs, cAnmCheck 0x801bcd20; the others 0x800f6038 = 0) holds; the clips advance
	          // (+0xb8) while it is not done
		if (m_nDemoStep == 0)
		{
			if (m_nStage == 1)
			{
				SetDemoClip(1);
				m_fDemoLast = 0.0f;
				m_fDemoLastOrbit = 0.0f;
			}
			m_nDemoStep = 1;
		}
		else if (!m_bDemoDone)
		{
			if (m_nStage != 1)
			{
				m_bDemoDone = true;
			}
			else
			{
				cKingdomModel* pac = pModel(0)->pExpandMdl(0);
				float f = 0.0f;
				if (pac != nullptr) f = m_nDemoKind == 3 ? pac->GetFrameAnmTexPat(0u) : m_nDemoKind == 4 ? pac->GetFrameAnmVis(0u) : pac->GetFrameAnmChr(0u);
				// native: also the orbit skeleton's own a10 clip (the expanded model's clips may not advance with the parent's UpdateFrame here;
				// UNVERIFIED stand-in for cAnmCheck 0x801bcd20 on the expanded model)
				const float of = pModel(0)->GetFrameAnmChr(0u), omax = pModel(0)->GetFrameMaxAnmChr(0u);
				if (f >= m_fDemoMax || f < m_fDemoLast || of >= omax || of < m_fDemoLastOrbit) m_bDemoDone = true;   // a wrap counts as the end
				m_fDemoLast = f;
				m_fDemoLastOrbit = of;
			}
		}
		if (!m_bDemoDone) UpdateFrame();
		break;
	case 3:
	{
		cKingdomModel* orbit = pModel(0);
		if (orbit->GetFrameAnmChr(0u) < orbit->GetFrameMaxAnmChr(0u))
		{
			UpdateFrame();   // vtable +0xb8
		}
		else
		{
			SetCourseClip();   // vtable +0xd4
			m_nState = 4;
		}
		break;
	}
	case 5:
		GameStep();
		break;
	case 8:
		GoalRun();   // +0x84
		break;
	case 9:
		UpdateFrame();   // +0xb8 = 0x800f4b14
		break;
	case 6:   // miss
		MissStep();   // +0x80
		break;
	default:
		break;
	}
	m_bJumpReq = false;   // 0x800f5fbc: the request lives one frame
	const bool bRet = cTaskModel::Execute(pParam);
	UpdateWorldAABB();    // 0x801c1950
	return bRet;
}


// DOL 0x800f600c [audited]
void cTaskPacman::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08 || !m_b09) return;
	cTaskModel::Draw(nStatus, pParam);
}


// ---- the goal run -------------------------------------------------------------------------------------------------------------------

namespace
{
// DOL 0x801c1b24 (the same helper as cTaskEnemy.cpp's, which is file-local there): a + (b - a) t, w = 1
void OrbitLerp(float t, float out[4], const float a[4], const float b[4])
{
	for (int i = 0; i < 3; i++) out[i] = a[i] + (b[i] - a[i]) * t;
	out[3] = 1.0f;
}
constexpr float DEG2RAD_GOAL = 0.017453292f;   // FLOAT_8047b6bc
const wii::Mdl0UserItem* UserItem(const wii::Mdl0Node* n, const char* name)
{
	if (n == nullptr) return nullptr;
	for (const wii::Mdl0UserItem& u : n->userData)
		if (u.name == name) return &u;
	return nullptr;
}
}

// DOL 0x800f5708 [audited]: step 0 stop (+0x88), 1 next node (+0x8c: none left -> stage 9 straight to state 9, else step 3), 2 move (+0x90, then 1),
// 3 arrival clips (+0x94), 4 their blend (+0x98, then state 9 via +0x104); the clips advance every frame (+0xb8)
void cTaskPacman::GoalRun()
{
	if (m_nStep == 0)
	{
		if (GoalStop()) m_nStep = 1;
	}
	else if (m_nStep == 1)
	{
		if (!GoalNext()) m_nStep = 2;
		else if (KingdomGlobal::GalagaSys->nStage() == 9) { m_nState = 9; m_nStep = 0; }   // +0x104 = 0x800f3ea8
		else m_nStep = 3;
	}
	else if (m_nStep == 2)
	{
		if (GoalMove()) m_nStep = 1;
	}
	else if (m_nStep == 3)
	{
		GoalClip();
		m_nStep++;
	}
	else if (m_nStep == 4 && GoalBlend())
	{
		m_nState = 9;   // +0x104
		m_nStep = 0;
	}
	UpdateFrame();   // +0xb8
}

// DOL 0x800f30f4 [audited]: once Pac-Man is on the ground (jump state 0 or 3): the invincible time ends (+0x2e4 = -1; +0xa4 = 0x800f5e00 releases the
// hit clips; the hit effect detached), the orbit and Pac-Man clips stop (rate 0), +0x3b0 = Rz Ry Rx of the orbit clip's
// pac_orbit rotation now, and the scene goes to its goal camera (0x800f6ad0, state 7)
bool cTaskPacman::GoalStop()
{
	JumpStep();   // +0xbc
	if (m_nJump != 0 && m_nJump != 3) return false;
	if (0.0f < m_fInvincible)
	{
		m_fInvincible = -1.0f;   // FLOAT_8047b6b8
		HitEnd();                // +0xa4
		if (m_pHitEffect != nullptr && m_pHitEffect->IsValid()) m_pHitEffect->Detach(false);   // FUN_8008a508(+0x300, 0)
	}
	cKingdomModel* orbit = pModel(0);
	cKingdomModel* pac = orbit != nullptr ? orbit->pExpandMdl(0) : nullptr;
	if (orbit != nullptr) orbit->SetUpdateRateAnmChr(0u, 0.0f);   // FUN_8001cad4
	if (pac != nullptr) pac->SetUpdateRateAnmChr(0u, 0.0f);
	float r[3] = {0.0f, 0.0f, 0.0f};
	if (orbit != nullptr)
		if (std::shared_ptr<wii::ChrAnim> chr = orbit->AnmChrOf(0u))
		{
			wii::BoneKey k;
			if (chr->Sample("pac_orbit", orbit->GetFrameAnmChr(0u), k) && k.hasRotate)
				for (int i = 0; i < 3; i++) r[i] = k.rotate[i] * DEG2RAD_GOAL;   // ChrAnmResult::GetRotateDeg
		}
	// PSMTX44Identity, then * Rz, * Ry, * Rx (FUN_80298d00 'Z' / 'Y' / 'X', each only when its angle is not 0)
	const float cx = std::cos(r[0]), sx = std::sin(r[0]), cy = std::cos(r[1]), sy = std::sin(r[1]), cz = std::cos(r[2]), sz = std::sin(r[2]);
	const float m[9] = {cz * cy, cz * sy * sx - sz * cx, cz * sy * cx + sz * sx,
	                    sz * cy, sz * sy * sx + cz * cx, sz * sy * cx - cz * sx,
	                    -sy, cy * sx, cy * cx};
	for (int i = 0; i < 9; i++) m_mGoalRot[i] = m[i];
	if (m_pScene != nullptr) m_pScene->GoalStart();   // FUN_800f6ad0
	return true;
}

// DOL 0x800f3394 [audited: port guards the roll-rate division]: the frame counter to 0; the goal skeleton's (model 1) first nodes whose names contain pac_goal_%03d for the index and the next
// one (from / to); the to node's user data pac_trans_frame (frames, f32) and pac_roll_anm (s32): rolling, Pac-Man's roll clip is set to run
// out (plus a whole turn when less than half is left) over the trip (stages 7 / 8: stop unless at least 0.75 frames in), else it stops.
// true when a node is missing (the run is over).
bool cTaskPacman::GoalNext()
{
	m_fGoalPos = 0.0f;
	cKingdomModel* goal = pModel(1);
	const wii::Mdl0* skel = goal != nullptr ? goal->pMdl0() : nullptr;
	bool found = false;
	for (int k = 0; k < 2; k++)
	{
		char key[0x80];
		std::snprintf(key, sizeof key, "pac_goal_%03d", m_nGoalNode + k);   // s_pac_goal__03d_80375020
		found = false;
		if (skel != nullptr)
			for (const wii::Mdl0Node& n : skel->nodes)
				if (n.name.find(key) != std::string::npos) { m_pGoalNode[k] = &n; found = true; break; }
	}
	if (!found) return true;
	if (const wii::Mdl0UserItem* u = UserItem(m_pGoalNode[1], "pac_trans_frame"))   // PTR_s_pac_trans_frame_804779ac
		if (!u->floats.empty()) m_fGoalFrames = u->floats[0];
	m_bGoalRoll = false;
	if (const wii::Mdl0UserItem* u = UserItem(m_pGoalNode[1], "pac_roll_anm"))   // PTR_s_pac_roll_anm_804779b0
		m_bGoalRoll = !u->ints.empty() && u->ints[0] != 0;
	cKingdomModel* orbit = pModel(0);
	cKingdomModel* pac = orbit != nullptr ? orbit->pExpandMdl(0) : nullptr;
	if (pac != nullptr)
	{
		if (m_bGoalRoll)
		{
			const float max = pac->GetFrameMaxAnmChr(0u), frame = pac->GetFrameAnmChr(0u);   // FUN_8001c630 / FUN_8001c2e0
			float left = max - frame;
			const int stage = KingdomGlobal::GalagaSys->nStage();
			if (stage == 7 || stage == 8)
			{
				if (frame < 0.75f) left = 0.0f;   // FLOAT_8047b6c4
			}
			else
			{
				if (left < max * 0.5f) left += max;   // FLOAT_8047b6c0
				left += max;
			}
			pac->SetUpdateRateAnmChr(0u, m_fGoalFrames > 0.0f ? left / m_fGoalFrames : 0.0f);
		}
		else
		{
			pac->SetUpdateRateAnmChr(0u, 0.0f);
		}
	}
	m_nGoalNode++;
	return false;
}

// DOL 0x800f386c [audited: port guards the lerp division]: one frame further (at most the trip length); the orbit model's translate = lerp of the two nodes' translates turned by
// +0x3b0, Pac-Man's rotate = lerp of their rotates (deg -> rad) (0x801c1b24); true when the trip is done
bool cTaskPacman::GoalMove()
{
	m_fGoalPos += 1.0f;   // FLOAT_8047b6b4
	if (m_fGoalFrames <= m_fGoalPos) m_fGoalPos = m_fGoalFrames;
	float pos[2][4], rot[2][4];
	for (int k = 0; k < 2; k++)
	{
		const wii::Mdl0Node* n = m_pGoalNode[k];
		const float t[3] = {n != nullptr ? n->translate[0] : 0.0f, n != nullptr ? n->translate[1] : 0.0f, n != nullptr ? n->translate[2] : 0.0f};   // ResNode +0x38
		for (int i = 0; i < 3; i++) pos[k][i] = m_mGoalRot[i * 3 + 0] * t[0] + m_mGoalRot[i * 3 + 1] * t[1] + m_mGoalRot[i * 3 + 2] * t[2];
		pos[k][3] = 1.0f;
		for (int i = 0; i < 3; i++) rot[k][i] = n != nullptr ? n->rotate[i] * DEG2RAD_GOAL : 0.0f;   // ResNode +0x2c
		rot[k][3] = 0.0f;
	}
	const float f = m_fGoalFrames > 0.0f ? m_fGoalPos / m_fGoalFrames : 1.0f;
	cKingdomModel* orbit = pModel(0);
	if (orbit != nullptr)
	{
		float out[4];
		OrbitLerp(f, out, pos[0], pos[1]);
		orbit->vTrans() = Vector4(out[0], out[1], out[2], 1.0f);
		if (cKingdomModel* pac = orbit->pExpandMdl(0))
		{
			OrbitLerp(f, out, rot[0], rot[1]);
			pac->vRotate() = Vector4(out[0], out[1], out[2], 0.0f);
		}
	}
	return m_fGoalFrames <= m_fGoalPos;
}

// DOL 0x800f3b44 [audited]: Pac-Man's slot 0 clip rate 1 then 0 (as the DOL does), CHAR_PACMAN_ANM_CHA_04 in slot 1; not on stage 9: PTK_GOAL01 (0x2b)
// on the handle +0x410 (FUN_80021a9c: not one-time, not dependent, clipped) at Pac-Man (+0x20), turned by the pac_orbit rotation (+0x280)
void cTaskPacman::GoalClip()
{
	cKingdomModel* orbit = pModel(0);
	cKingdomModel* pac = orbit != nullptr ? orbit->pExpandMdl(0) : nullptr;
	if (pac != nullptr)
	{
		pac->SetUpdateRateAnmChr(0u, 1.0f);
		pac->SetUpdateRateAnmChr(0u, 0.0f);
		pac->RegistAnm(1u, "CHAR_PACMAN_ANM_CHA_04");   // 0x7c2fb4ad, FUN_8001a4d8(pac, 1, crc, 0, 1)
	}
	if (KingdomGlobal::GalagaSys->nStage() != 9)
	{
		if (m_pGoalPtk == nullptr) m_pGoalPtk = new cKingdomPtkHnd();
		m_pGoalPtk->Generate(0x2bu, false, false, true, 0.0f, Vector4(m_v20.x, m_v20.y, m_v20.z, 1.0f),
		                        Vector4(m_vOrbitRot[0], m_vOrbitRot[1], m_vOrbitRot[2], 1.0f));
	}
}

// DOL 0x800f3c14 [audited]: for 5 frames slot 0 / 1 weights 1 - 0.2 t / 0.2 t (FUN_8001d994); then, when the slot 1 clip ended (FUN_8001c630 max < FUN_8001c2e0
// frame; natively "<="), CHAR_PACMAN_ANM_CHA_05 (0x0b28843b) and CHAR_PACMAN_ANM_TPA_05 in slot 0 and slot 1 released (FUN_8001ab2c)
bool cTaskPacman::GoalBlend()
{
	cKingdomModel* orbit = pModel(0);
	cKingdomModel* pac = orbit != nullptr ? orbit->pExpandMdl(0) : nullptr;
	if (pac == nullptr) return true;
	if (5.0f <= m_fGoalPos)   // FLOAT_8047b6c8
	{
		if (pac->GetFrameMaxAnmChr(1u) <= pac->GetFrameAnmChr(1u))
		{
			pac->RegistAnm(0u, "CHAR_PACMAN_ANM_CHA_05");
			pac->RegistAnm(0u, "CHAR_PACMAN_ANM_TPA_05");
			pac->ReleaseAnmChr(1u);
			return true;
		}
	}
	else
	{
		const float w = 0.2f * m_fGoalPos;   // FLOAT_8047b6cc
		pac->SetWeightAnmChr(0u, 1.0f - w);
		pac->SetWeightAnmChr(1u, w);
		m_fGoalPos += 1.0f;
	}
	return false;
}


// ---- the miss -----------------------------------------------------------------------------------------------------------------------

// DOL 0x800f5590 [audited: frame/rates pinned (SyncCourse) instead of by scene flags]: step 0 the clips follow the scene while it slows down (natively pinned every frame, as GameStep); once the scene's rate is
// 0 (0x800f7df8) the Galaga system stops everything (+0x09, 0x800afe34) and the miss clips start (+0x9c); step 1 waits for their end
// (+0xa8) -> state 7; the clips advance every frame (+0xb8)
void cTaskPacman::MissStep()
{
	if (m_nStep == 0)
	{
		SyncCourse();
		if (m_pScene == nullptr || m_pScene->fRate() <= 0.0f)   // FLOAT_8047b6b0
		{
			KingdomGlobal::GalagaSys->bUnk9(true);
			MissClip();
			m_nStep++;
		}
	}
	else if (m_nStep == 1 && IsMissClipEnd())
	{
		m_nState = 7;
	}
	UpdateFrame();   // +0xb8
}

// DOL 0x800f5ca4 [audited]
void cTaskPacman::MissClip()
{
	cKingdomModel* orbit = pModel(0);
	cKingdomModel* pac = orbit != nullptr ? orbit->pExpandMdl(0) : nullptr;
	if (pac == nullptr) return;
	pac->RegistAnm(0u, "CHAR_PACMAN_ANM_CHA_03");   // 0xe24b210e
	pac->RegistAnm(0u, "CHAR_PACMAN_ANM_TPA_03");   // 0xac08cf6e
	pac->SetUpdateRateAnmChr(0u, 1.0f);             // FUN_8001cad4 (FLOAT_8047b6b4)
	pac->SetUpdateRateAnmTexPat(0u, 1.0f);          // FUN_8001ce94
}

// DOL 0x800f5d30 [audited]: the bone clip ran out (FUN_8001c630 max < FUN_8001c2e0 frame; natively "<=")
bool cTaskPacman::IsMissClipEnd()
{
	cKingdomModel* orbit = pModel(0);
	cKingdomModel* pac = orbit != nullptr ? orbit->pExpandMdl(0) : nullptr;
	return pac == nullptr || pac->GetFrameMaxAnmChr(0u) <= pac->GetFrameAnmChr(0u);
}

}
