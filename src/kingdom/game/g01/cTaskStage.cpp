#include "kingdom/game/g01/cTaskStage.hpp"

#include <cctype>
#include <algorithm>
#include <cstdio>
#include <cstring>

#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskLensFlare.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskCollisionMan.hpp"
#include "kingdom/game/g01/cTaskEnemySpikeMaster.hpp"
#include "kingdom/game/g01/cTaskGoalBoss.hpp"
#include "kingdom/game/g01/cTaskSS.hpp"
#include "kingdom/game/g01/cTaskGimmick.hpp"
#include "kingdom/game/g01/cTaskStart.hpp"
#include "kingdom/game/g01/cTaskGoal.hpp"
#include "kingdom/game/g01/cTaskEnemyGate.hpp"
#include "kingdom/game/g01/cTaskEnemySpike.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g01
{

namespace
{
constexpr float DEG2RAD = 0.017453292f;     // FLOAT_8047b370
constexpr float ANM_SLOW = 0.1f;            // FLOAT_8047b374 / FLOAT_8047b4f8: bone, colour and texture srt clips
constexpr float ANM_PAT = 1.0f;             // FLOAT_8047b378 / FLOAT_8047b4fc: texture pattern clips

bool Exists(const std::string& name)
{
	std::size_t size = 0;
	return wii::ArchiveStore::Find(name, size) != nullptr;
}

// cObject +0x20 / +0x30 / +0x40 from the node (DOL 0x800c8b78 [audited: +0x224 stage and virtual +0x74 (clip start) not mirrored]: translate +0x38, rotate +0x2c * DEG2RAD, scale +0x20)
void PlaceFromNode(cKingdomModel* m, const wii::Mdl0Node& node)
{
	m->vTrans() = Vector4(node.translate[0], node.translate[1], node.translate[2], 1.0f);
	m->vRotate() = Vector4(node.rotate[0] * DEG2RAD, node.rotate[1] * DEG2RAD, node.rotate[2] * DEG2RAD, 0.0f);
	m->vScale() = Vector4(node.scale[0], node.scale[1], node.scale[2], 1.0f);
}

// the a01 clips of a stage model: [0] cha (bone) [1] cla (colour) [2] tpa (texture pattern) [3] tsa (texture srt)
void RegistA01(cKingdomModel* m, const char* const fmt[4], int nStage)
{
	for (int i = 0; i < 4; i++)
	{
		char name[0x80];
		std::snprintf(name, sizeof name, fmt[i], nStage);
		if (!Exists(name)) continue;   // 0x8000bc70 (SearchData) == 0: skipped
		m->RegistAnm(0u, name);
		switch (i)
		{
		case 0: m->SetUpdateRateAnmChr(0u, ANM_SLOW); break;      // 0x8001cad4
		case 1: m->SetUpdateRateMatClr(0u, ANM_SLOW); break;      // 0x8001cd4c
		case 2: m->SetUpdateRateAnmTexPat(0u, ANM_PAT); break;    // 0x8001ce94
		case 3: m->SetUpdateRateAnmTexSrt(0u, ANM_SLOW); break;   // 0x8001cfdc
		}
	}
}
}

// ---- cTaskBg ------------------------------------------------------------------------------------------------------------------------

void cTaskBg::Setup(const wii::Mdl0Node& node, int nStage)
{
	SetName("g01::cTaskBg");
	nDrawPriority(0x10001u);   // cTaskBg::Init 0x800c898c (vtable +0x6c)
	m_nStage = nStage;
	LoadModel();
	PlaceFromNode(pModel(0), node);
	m_vT = pModel(0)->vTrans();
	m_vR = pModel(0)->vRotate();
	m_vS = pModel(0)->vScale();
	if (nStage == 1 || nStage == 10)   // 0x800c89dc: cDemoBgW1S1 / cDemoBgBoss registered with the demo manager
		if (cDemoMan* dm = KingdomGlobal::GalagaSys->pDemoMan()) dm->Regist(this);
}

// DOL 0x800c8fe8 [audited]
void cTaskBg::LoadDemoModel()
{
	static const char* const A10[5] = {"m01b_s1_bg_a10_cha.brres", "m01b_s1_bg_a10_cla.brres", "m01b_s1_bg_a10_tsa.brres", "m01b_s1_bg_a10_tpa.brres",
	                                   "m01b_s1_bg_a10_via.brres"};
	DeleteModel(0);
	char name[0x80];
	std::snprintf(name, sizeof name, "m01b_s%x_bg.brres", m_nStage);
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(name, 0x36f);
	m->SetLightSetIdx(0);
	m->SetFogIdx(m_nStage == 2 ? 3 : 0);
	m_fDemoMax = 0.0f;
	for (int i = 0; i < 5; i++)
	{
		if (!Exists(A10[i])) continue;
		m->RegistAnm(0u, A10[i]);
		if (i == 0)
		{
			m->SetUpdateRateAnmChr(0u, 1.0f);   // FLOAT_8047b378
			m_fDemoMax = std::max(m_fDemoMax, m->GetFrameMaxAnmChr(0u));
		}
	}
	m->vTrans() = m_vT;
	m->vRotate() = m_vR;
	m->vScale() = m_vS;
}

// DOL 0x800c91a0 [audited]: the model again (light set 0, fog 3 on stage 2 else 0), m01b_s%x_bg_a50_cha (the DOL's table names the cha clip five
// times; only the first sets rate 1.0 and the length), the node's SRT
void cTaskBg::LoadGoalModel()
{
	DeleteModel(0);
	char name[0x80];
	std::snprintf(name, sizeof name, "m01b_s%x_bg.brres", m_nStage);
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(name, 0x36f);
	m->SetLightSetIdx(0);
	m->SetFogIdx(m_nStage == 2 ? 3 : 0);
	m_fDemoMax = 0.0f;
	std::snprintf(name, sizeof name, "m01b_s%x_bg_a%d0_cha.brres", m_nStage, 5);   // PTR_s_m01b_s_x_bg_a_d0_cha_brres_80371524
	if (Exists(name))
	{
		m->RegistAnm(0u, name);
		m->SetUpdateRateAnmChr(0u, 1.0f);   // FLOAT_8047b378
		m_fDemoMax = m->GetFrameMaxAnmChr(0u);
	}
	m->vTrans() = m_vT;
	m->vRotate() = m_vR;
	m->vScale() = m_vS;
}

void cTaskBg::DemoFinish()
{
	m_bDemoDone = true;
	m_bGoalDone = true;
	if (m_nStage != 1) return;   // cDemoBgBoss's Term (0x800c961c) does nothing
	DeleteModel(0);
	LoadModel();
	pModel(0)->vTrans() = m_vT;
	pModel(0)->vRotate() = m_vR;
	pModel(0)->vScale() = m_vS;
}

// DOL 0x800c8ddc [audited]
void cTaskBg::LoadModel()
{
	static const char* const A01[4] = {"m01b_s%x_bg_a01_cha.brres", "m01b_s%x_bg_a01_cla.brres", "m01b_s%x_bg_a01_tpa.brres",
	                                   "m01b_s%x_bg_a01_tsa.brres"};
	char name[0x80];
	std::snprintf(name, sizeof name, "m01b_s%x_bg.brres", m_nStage);
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(name, 0x36f);
	m->SetLightSetIdx(0);
	m->SetFogIdx((m_nStage - 1) / 3 == 2 ? 3 : 0);
	RegistA01(m, A01, m_nStage);
	m_nState = 2;   // no demo object: state 2 just plays the model
}

// DOL 0x800c9388 [audited]: states 0 / 1 run the demo object first; state 2 updates the clips (0x8009a478)
bool cTaskBg::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (m_nState == 0)   // the opening demo (0x801bce00): Init on the first frame (cDemoBgW1S1 0x801bcf28 -> +0x7c), then while IsWaitEnd holds
	{                    // (W1S1 0x801bcf60: the longest a10 clip runs; cDemoBgBoss 0x800c962c = 0); the clips advance while it is not done
		if (m_nDemoStep == 0)
		{
			if (m_nStage == 1)
			{
				LoadDemoModel();
				m_fDemoLast = 0.0f;
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
				// native: a clip that loops wraps instead of reaching its max; a wrap counts as the end too
				const float f = pModel(0)->GetFrameAnmChr(0u);
				if (f >= m_fDemoMax || f < m_fDemoLast) m_bDemoDone = true;
				m_fDemoLast = f;
			}
		}
		if (!m_bDemoDone) UpdateFrame();
	}
	else if (m_nState == 1)   // the goal demo (0x801bce94): Start (cDemoBgBoss 0x801bcf8c -> +0x80; W1S1's is the base 0x800c9618), then while
	{                         // the check holds (0x801bcfa8: cAnmCheck; the base 0x800c9624 = 0)
		if (m_nDemoStep == 0)
		{
			if (m_nStage == 10) LoadGoalModel();
			m_nDemoStep = 1;
		}
		else if (!m_bGoalDone && (m_nStage != 10 || m_fDemoMax <= pModel(0)->GetFrameAnmChr(0u)))
		{
			m_bGoalDone = true;   // max < frame natively max <= frame (the one-time clip stops at max)
		}
		if (!m_bGoalDone) UpdateFrame();
	}
	if (m_nState == 2) UpdateFrame();
	return cTaskModel::Execute(pParam);
}

// ---- cTaskCourse --------------------------------------------------------------------------------------------------------------------

// DOL 0x800dfd4c [audited: FUN_8001e16c(model) after the fog index not ported (unknown); copies SRT to +0x60/+0x70 too] (the part after the animations, which copies the node's SRT like cTaskBg, is UNVERIFIED: read only up to the clip loop)
void cTaskCourse::Setup(const wii::Mdl0Node& node, int nStage)
{
	SetName("g01::cTaskCourse");
	static const char* const A01[4] = {"m01b_s%x_cou_a01_chr.brres", "m01b_s%x_cou_a01_cla.brres", "m01b_s%x_cou_a01_tpa.brres",
	                                   "m01b_s%x_cou_a01_tsa.brres"};
	char name[0x80];
	std::snprintf(name, sizeof name, "m01b_s%x_cou.brres", nStage);
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(name, 0x36f);
	m->SetLightSetIdx(0);
	m->SetFogIdx(2);
	RegistA01(m, A01, nStage);
	PlaceFromNode(m, node);
}

// DOL 0x800e0128 [audited]: (not paused) the clips (FUN_8001ded4) and the model task. Its Draw 0x800e0188 and cTaskBg's Draw
// 0x800c9450 are the plain model draw (no override natively).
bool cTaskCourse::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	UpdateFrame();
	return cTaskModel::Execute(pParam);
}

// ---- cTaskStage ---------------------------------------------------------------------------------------------------------------------

// DOL 0x800f9484 [audited]
cTaskStage::cTaskStage()
{
	SetName("g01::cTaskStage");
	nDrawPriority(0x10000u);
	m_nState = 0;
	m_nStep = 0;
}

// DOL 0x800fa80c [audited: Setup compared by structure only (children, effects, shadow, goal, gimmick setups); the effects emitters 0x800f94ec not line-compared]. The DOL keeps the skeleton (+0x21c, m01b_s%x_bg_s.brres) and the lens model (+0x220, m01e_s%x_lens_s.brres) as its own models,
// makes a cTaskBg per "m01b_s%x_bg_" node and a cTaskCourse per "m01b_s%x_cou_" node, then the effects (0x800f94ec: star / ginga / mist / water
// emitters), the shadow (+0x224, 0x800fb0e4), the goal (+0x228, cTaskGoal / cTaskGoalBoss for stage 9), the gimmick setups (0x800f9e8c ...
// 0x800fa578), cTaskSS 0x800f8f74 for stages < 7 and the lens flare (+0x230).
void cTaskStage::Setup(int nStage)
{
	m_nStage = nStage;
	char name[0x80];
	std::snprintf(name, sizeof name, "m01b_s%x_bg_s.brres", nStage);
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find(name, size);
	wii::Brres brres;
	wii::Mdl0 skel;
	char mdl[0x80];
	std::snprintf(mdl, sizeof mdl, "m01b_s%x_bg_s", nStage);
	if (d == nullptr || !brres.Load(d, size) || !skel.Load(brres, mdl))
	{
		std::fprintf(stderr, "g01::cTaskStage: %s not found\n", name);
		return;
	}
	char bgKey[0x80], couKey[0x80];
	std::snprintf(bgKey, sizeof bgKey, "m01b_s%x_bg_", nStage);
	std::snprintf(couKey, sizeof couKey, "m01b_s%x_cou_", nStage);
	for (const wii::Mdl0Node& node : skel.nodes)
	{
		if (std::strstr(node.name.c_str(), bgKey) != nullptr)
		{
			cTaskBg* p = new cTaskBg();
			p->Setup(node, nStage);
			ConnectChild(p);
		}
	}
	for (const wii::Mdl0Node& node : skel.nodes)
	{
		if (std::strstr(node.name.c_str(), couKey) != nullptr)
		{
			cTaskCourse* p = new cTaskCourse();
			p->Setup(node, nStage);
			ConnectChild(p);
		}
	}
	// 0x800f94ec: the background emitters (looping effects at the m01e_* nodes of the skeleton)
	for (const wii::Mdl0Node& node : skel.nodes) SetupEffect(node);
	// +0x224: the start area (0x800faf04 / 0x800fb0e4)
	cTaskStart* start = new cTaskStart();
	start->Setup(nStage, skel);
	ConnectChild(start);
	SetupGimmick(skel, nStage);   // 0x800f9e8c
	SetupMComet(skel);            // 0x800f97a4
	SetupBreakComet(nStage);      // 0x800f99fc
	SetupGate(skel, nStage);      // 0x800fa0a4
	SetupSpike(skel, nStage);     // 0x800fa2fc
	SetupSpikeMaster(skel, nStage);   // 0x800fa578
	if (nStage < 7) ConnectChild(new cTaskSS());   // 0x800fa80c: the shooting stars (0x800f8f74), worlds 1 and 2
	// +0x228: the goal area (0x800ee050 + vtable +0x7c = 0x800eeedc; stage 9: cTaskGoalBoss, setup 0x800efd6c)
	cTaskGoal* goal = nStage == 9 ? new cTaskGoalBoss() : new cTaskGoal();
	goal->Setup(nStage, skel);
	ConnectChild(goal);
	// +0x230: the lens flare (0x80096d40 with the app's scene root; 0x80097f54 ANT_LENSFLARE (0x08a27694)), its light at node light_001
	// (PTR_s_light_001_804779f4, FUN_8001dd54: natively the bind pose position) of the lens model (+0x220, m01e_s%x_lens_s.brres), size 16
	// (FLOAT_8047b73c, 0x80098030)
	kingdom::cTaskLensFlare* flare = new kingdom::cTaskLensFlare();
	flare->Load("ANT_LENSFLARE");
	ConnectChild(flare);
	std::snprintf(name, sizeof name, "m01e_s%x_lens_s.brres", nStage);   // s_m01e_s_x_lens_s_brres_80375caa
	std::snprintf(mdl, sizeof mdl, "m01e_s%x_lens_s", nStage);
	const u8* ld = wii::ArchiveStore::Find(name, size);
	wii::Brres lb;
	wii::Mdl0 lens;
	if (ld != nullptr && lb.Load(ld, size) && lens.Load(lb, mdl))
		for (const wii::Mdl0Node& node : lens.nodes)
			if (node.name == "light_001")
			{
				const float pos[3] = {node.model[3], node.model[7], node.model[11]};
				flare->SetLight(pos);
			}
	flare->SetSize(16.0f);
}

// DOL 0x800f9e8c [audited]: for every name of the table (0x803759ac, in order), every skeleton node containing it gets a cTaskGimmick of that
// index; fog 0 before stage 7, else 3; world 3 sets fog 3 afterwards (0x8001e400)
void cTaskStage::SetupGimmick(const wii::Mdl0& skel, int nStage)
{
	for (int idx = 0; idx < 68; idx++)
		for (const wii::Mdl0Node& node : skel.nodes)
		{
			if (cTaskGimmick::Find(node.name.c_str()) != idx) continue;
			cTaskGimmick* p = new cTaskGimmick();
			p->Setup(node, idx, nStage < 7 ? 0 : 3);
			ConnectChild(p);
			if (KingdomGlobal::GalagaSys->nWorld() == 3) p->pModel(0)->SetFogIdx(3);
		}
}

// DOL 0x800f97a4 [audited]: the middle comets (table 0x80375548), fog 3, registered with the collision manager (0x800d41c8)
void cTaskStage::SetupMComet(const wii::Mdl0& skel)
{
	for (int idx = 0; idx < 12; idx++)
		for (const wii::Mdl0Node& node : skel.nodes)
		{
			if (cTaskGimmickMComet::Find(node.name.c_str()) != idx) continue;
			cTaskGimmickMComet* p = new cTaskGimmickMComet();
			p->Setup(node, idx, 3);
			ConnectChild(p);
			if (cTaskCollisionMan* col = KingdomGlobal::GalagaSys->pTaskCollisionMan()) col->Regist(p);
		}
}

// DOL 0x800f99fc [audited]: GimmickParam_s%x.csv BLUE_SCORE / RED_SCORE / GREEN_SCORE, then a break comet per CometGimmick_s%x.csv row (not empty, not
// a ';' comment): type, clip rate, scale xyz, rotation xyz (degrees * FLOAT_8047b738), position xyz; fog 3; registered for collision
void cTaskStage::SetupBreakComet(int nStage)
{
	char name[0x80];
	int score[3] = {};
	std::snprintf(name, sizeof name, "GimmickParam_s%x.csv", nStage);
	{
		kingdom::cKingdomCSV csv;
		if (csv.RegistData(name))
		{
			const char* key[3] = {"BLUE_SCORE", "RED_SCORE", "GREEN_SCORE"};
			for (int i = 0; i < 3; i++) csv.S32(csv.SearchRow(0, key[i], 0, 0), 1, score[i]);
		}
	}
	std::snprintf(name, sizeof name, "CometGimmick_s%x.csv", nStage);
	kingdom::cKingdomCSV csv;
	if (!csv.RegistData(name)) return;
	for (int row = 0; row < csv.nRow(); row++)
	{
		const std::string first = csv.Cell(row, 0);
		if (first.empty() || first[0] == ';') continue;
		int type = 0;
		float rate = 1.0f, v[9] = {};
		csv.S32(row, 0, type);
		csv.F32(row, 1, rate);
		for (int i = 0; i < 9; i++) csv.F32(row, 2 + i, v[i]);
		const Vector4 scale(v[0], v[1], v[2], 1.0f);                                  // FLOAT_8047b734
		const Vector4 rot(v[3] * DEG2RAD, v[4] * DEG2RAD, v[5] * DEG2RAD, 0.0f);      // FLOAT_8047b730
		const Vector4 pos(v[6], v[7], v[8], 1.0f);
		cTaskGimmickBreakComet* p = new cTaskGimmickBreakComet();
		p->Setup(type, rate, pos, rot, scale, (unsigned)type < 3 ? score[type] : 0, 3);
		ConnectChild(p);
		if (cTaskCollisionMan* col = KingdomGlobal::GalagaSys->pTaskCollisionMan()) col->Regist(p);
	}
}

// DOL 0x800fa0a4 [audited]: an enemy gate at the first node containing m01m_g10_%03d for n = 1, 2 ... (every node is scanned for each n; n
// counts up per gate found), registered with the collision manager
void cTaskStage::SetupGate(const wii::Mdl0& skel, int nStage)
{
	int n = 1;
	for (const wii::Mdl0Node& node : skel.nodes)
	{
		char key[0x40];
		std::snprintf(key, sizeof key, "m01m_g10_%03d", n);   // DAT_80375c6c
		if (std::strstr(node.name.c_str(), key) == nullptr) continue;
		cTaskEnemyGate* p = new cTaskEnemyGate();
		p->SetupGate(node, nStage, n);
		ConnectChild(p);
		if (cTaskCollisionMan* col = KingdomGlobal::GalagaSys->pTaskCollisionMan()) col->Regist(p);
		n++;
	}
}

// DOL 0x800fa578 [audited]: a spike master at the first node containing m01m_g12_%03d for n = 1, 2 ... (stops at the first n without a node), set
// up with row n - 1 (+0xd0), registered for collision (0x800d41c8)
void cTaskStage::SetupSpikeMaster(const wii::Mdl0& skel, int nStage)
{
	for (int n = 1;; n++)
	{
		char key[0x40];
		std::snprintf(key, sizeof key, "m01m_g12_%03d", n);   // s_m01m_g12__03d_80375c88
		const wii::Mdl0Node* found = nullptr;
		for (const wii::Mdl0Node& node : skel.nodes)
			if (std::strstr(node.name.c_str(), key) != nullptr) { found = &node; break; }
		if (found == nullptr) break;
		cTaskEnemySpikeMaster* p = new cTaskEnemySpikeMaster();
		p->Setup(*found, nStage, n - 1);
		ConnectChild(p);
		if (cTaskCollisionMan* col = KingdomGlobal::GalagaSys->pTaskCollisionMan()) col->Regist(p);
	}
}

// DOL 0x800fa2fc [audited]: a spike at the first node containing m01m_g11_%03d for n = 1, 2 ... (stops at the first n without a node), registered
// with the collision manager; its CSV row is n - 1
void cTaskStage::SetupSpike(const wii::Mdl0& skel, int nStage)
{
	for (int n = 1;; n++)
	{
		char key[0x40];
		std::snprintf(key, sizeof key, "m01m_g11_%03d", n);   // s_m01m_g11__03d_80375c7a
		const wii::Mdl0Node* found = nullptr;
		for (const wii::Mdl0Node& node : skel.nodes)
			if (std::strstr(node.name.c_str(), key) != nullptr) { found = &node; break; }
		if (found == nullptr) break;
		cTaskEnemySpike* p = new cTaskEnemySpike();
		p->SetupSpike(*found, nStage, n - 1);
		ConnectChild(p);
		if (cTaskCollisionMan* col = KingdomGlobal::GalagaSys->pTaskCollisionMan()) col->Regist(p);
	}
}

// DOL 0x800f94ec [audited]: node name part -> effect slot (tables 0x80375418 / 0x80375438): STAR01, GALAXY01, GALAXY02, MIST01, WATER04, WATER01,
// WATER02. Generate(slot, loop, not dependent, clipped) at the node's translate / rotate / scale (0x8001de14 / 0x8001dc94 / 0x8001db14;
// UNVERIFIED: read here from the node's local SRT, the rotation converted to radians).
void cTaskStage::SetupEffect(const wii::Mdl0Node& node)
{
	static const struct { const char* name; uint slot; } EFFECT[7] = {
		{"m01e_51_star01_", 0x1d}, {"m01e_52_ginga01_", 0x1e}, {"m01e_58_ginga01_", 0x1f}, {"m01e_56_mist01_", 0x23},
		{"m01e_57_water01_", 0x24}, {"m01e_47_water01_", 0x27}, {"m01e_48_water01_", 0x28}};
	for (const auto& e : EFFECT)
	{
		if (std::strstr(node.name.c_str(), e.name) == nullptr) continue;
		cKingdomPtkHnd* h = new cKingdomPtkHnd();
		const Vector4 t(node.translate[0], node.translate[1], node.translate[2], 1.0f);
		const Vector4 r(node.rotate[0] * DEG2RAD, node.rotate[1] * DEG2RAD, node.rotate[2] * DEG2RAD, 0.0f);
		const Vector4 s(node.scale[0], node.scale[1], node.scale[2], 1.0f);
		h->Generate(e.slot, false, false, true, 0.0f, t, r, s);
		m_Effect.push_back(h);
	}
}

// DOL 0x800fad60 [audited]: state 0 / 1 start the +0x22c object's demo / game animation (vtable +0xa8 / +0xac) once. +0x22c is dead in the DOL: only the
// ctor (0x800f94cc) stores it (0), Setup 0x800fa80c never fills it (a scan of every "stw rX, 0x22c(rY)" in the g01 code), so only the step advances.
// (This is not the missing Pac-Man shadow on the goal platform.)
bool cTaskStage::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	(void)pParam;
	if ((m_nState == 0 || m_nState == 1) && m_nStep == 0) m_nStep++;
	return true;
}

}
