#include "kingdom/game/g01/cTaskGoalBoss.hpp"

#include <cstdio>
#include <cstring>

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g01/cTaskNodeEffect.hpp"
#include "snd/Mixer.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "wii/Archive.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g01
{

namespace
{
constexpr float DEG2RAD = 0.017453292f;   // FLOAT_8047b620
constexpr float RATE_SLOW = 0.1f;         // FLOAT_8047b624
constexpr float RATE = 1.0f;              // FLOAT_8047b628

bool Exists(const char* name)
{
	std::size_t size = 0;
	return wii::ArchiveStore::Find(name, size) != nullptr;
}
}

cTaskGoalBoss::cTaskGoalBoss()
{
	SetName("g01::cTaskGoalBoss");
}

// DOL 0x800efd6c [audited]: the stage (+0x224), the placements (+0x6c = 0x800ef130: every skeleton node containing m01g_g09_ (DAT_8047790c), its
// translate, rotate (deg -> rad) and scale); stage 9: the goal demo object (cDemoGoalW3S3 0x801bdde0) registered with cDemoMan; the game
// clips (+0x78)
void cTaskGoalBoss::Setup(int nStage, const wii::Mdl0& skel)
{
	m_nStage = nStage;
	m_Place.clear();
	for (const wii::Mdl0Node& node : skel.nodes)
	{
		if (std::strstr(node.name.c_str(), "m01g_g09_") == nullptr) continue;
		Place p;
		p.t = Vector4(node.translate[0], node.translate[1], node.translate[2], 1.0f);
		p.r = Vector4(node.rotate[0] * DEG2RAD, node.rotate[1] * DEG2RAD, node.rotate[2] * DEG2RAD, 1.0f);
		p.s = Vector4(node.scale[0], node.scale[1], node.scale[2], 1.0f);
		p.nSet = 9;
		p.nIndex = 0;
		m_Place.push_back(p);
	}
	if (nStage == 9)
	{
		if (cDemoMan* dm = KingdomGlobal::GalagaSys->pDemoMan()) dm->Regist(this);
		// cDemoGoalW3S3 0x801bdde0: a cTaskAnita (priority 0x8000000a) with ANT_LINE (0x7fb8879b) as anitas 0..2, hidden; Demo_s9.csv
		// (s_Demo_s9_csv_8037dc64), column 1 of each key (+0x24 + 4 i, the jump table at 0x8044ab98)
		m_pLine = new kingdom::cTaskAnita();
		m_pLine->nDrawPriority(0x8000000au);
		for (uint i = 0; i < 3; i++)
		{
			m_pLine->pAnita(i)->RegistData("ANT_LINE");
			m_pLine->pAnita(i)->SetState(1u, false);
		}
		ConnectChild(m_pLine);
		kingdom::cKingdomCSV d;
		if (d.RegistData("Demo_s9.csv"))
		{
			const char* key[11] = {"e_bite01", "s_appear01", "s_boss_voice1_01", "s_boss_voice2_01", "s_boss_trance_01", "s_quake_01",
			                       "s_ensyutu_01", "a_line01", "a_line02", "a_line03", "b_boss01"};
			for (int i = 0; i < 11; i++) d.F32(d.SearchRow(0, key[i], 0, 0), 1, m_fDemo[i]);
		}
	}
	else
		m_bDemoDone = m_bGoalDone = true;
	LoadGame();
}

// DOL 0x800ef8d4 [audited]: model slot i = m01g_g09.brres (DAT_80477910) at its placement, light set 0, fog 3
void cTaskGoalBoss::LoadModels()
{
	for (std::size_t i = 0; i < m_Place.size(); i++)
	{
		cKingdomModel* m = CreateModel((uint)i);
		m->RegistModel("m01g_g09.brres", 0x36f);
		m->SetLightSetIdx(0);
		m->vTrans() = m_Place[i].t;
		m->vRotate() = m_Place[i].r;
		m->vScale() = m_Place[i].s;
		m->SetFogIdx(3);
	}
}

// DOL 0x800efa00 [audited]: the models (+0x70), then the a01 clips {cha 0.1, cla 1, tsa 0.1, tpa 1, via 1} (PTR_s_m01g_g09_a01_cha_brres_80374970 ..)
void cTaskGoalBoss::LoadGame()
{
	static const char* const A01[5] = {"m01g_g09_a01_cha.brres", "m01g_g09_a01_cla.brres", "m01g_g09_a01_tsa.brres", "m01g_g09_a01_tpa.brres",
	                                   "m01g_g09_a01_via.brres"};
	LoadModels();
	for (std::size_t i = 0; i < m_Place.size(); i++)
	{
		cKingdomModel* m = pModel((uint)i);
		for (int k = 0; k < 5; k++)
		{
			if (!Exists(A01[k])) continue;
			m->RegistAnm(0u, A01[k]);
			switch (k)
			{
			case 0: m->SetUpdateRateAnmChr(0u, RATE_SLOW); break;
			case 1: m->SetUpdateRateMatClr(0u, RATE); break;
			case 2: m->SetUpdateRateAnmTexSrt(0u, RATE_SLOW); break;
			case 3: m->SetUpdateRateAnmTexPat(0u, RATE); break;
			case 4: m->SetUpdateRateAnmVis(0u, RATE); break;
			}
		}
	}
}

// DOL 0x800efb88 [audited]: the models again, the a40 clips at 1.0; the longest one is timed (cAnmCheck)
void cTaskGoalBoss::LoadDemo()
{
	static const char* const A40[5] = {"m01g_g09_a40_cha.brres", "m01g_g09_a40_cla.brres", "m01g_g09_a40_tsa.brres", "m01g_g09_a40_tpa.brres",
	                                   "m01g_g09_a40_via.brres"};
	LoadModels();
	m_nChkModel = -1;
	m_fChkMax = 0.0f;
	for (std::size_t i = 0; i < m_Place.size(); i++)
	{
		cKingdomModel* m = pModel((uint)i);
		for (int k = 0; k < 5; k++)
		{
			if (!Exists(A40[k])) continue;
			m->RegistAnm(0u, A40[k]);
			float max = 0.0f;
			switch (k)
			{
			case 0: m->SetUpdateRateAnmChr(0u, RATE); max = m->GetFrameMaxAnmChr(0u); break;
			case 1: m->SetUpdateRateMatClr(0u, RATE); max = m->GetFrameMaxMatClr(0u); break;
			case 2: m->SetUpdateRateAnmTexSrt(0u, RATE); max = m->GetFrameMaxAnmTexSrt(0u); break;
			case 3: m->SetUpdateRateAnmTexPat(0u, RATE); max = m->GetFrameMaxAnmTexPat(0u); break;
			case 4: m->SetUpdateRateAnmVis(0u, RATE); max = m->GetFrameMaxAnmVis(0u); break;
			}
			if (m_fChkMax < max) { m_fChkMax = max; m_nChkModel = (int)i; m_nChkKind = k; }
		}
	}
}

// cDemoGoalW3S3::Execute 0x801be36c: false once the longest a40 clip ended; before that, at the Demo_s9.csv frames: m01se_26boss_01,
// m01se_27boss_02, m01se_40trance, m01se_38quake, m01se_28ensyutu, m01se_24appearance; e_bite01 m01se_29kamituki and a new effect at node
// ef_scissors of model 0 (0x800e0a38: slot 0x35, one-time, dependent, clipped; connected to the app's task root); a_line01..03 ANT_LINE i
// plays its animation i from the start and shows; b_boss01 m01bgm_Boss1 (as a plain cue, 0x80025988)
bool cTaskGoalBoss::GoalCheck()
{
	if (!ClipCheck()) return false;
	const float f = m_fChkLast;
	auto due = [f](float& t) { if (t != -1.0f && t <= f) { t = -1.0f; return true; } return false; };   // FLOAT_8047cf18
	if (due(m_fDemo[2])) snd::Play("m01se_26boss_01");
	if (due(m_fDemo[3])) snd::Play("m01se_27boss_02");
	if (due(m_fDemo[4])) snd::Play("m01se_40trance");
	if (due(m_fDemo[5])) snd::Play("m01se_38quake");
	if (due(m_fDemo[6])) snd::Play("m01se_28ensyutu");
	if (due(m_fDemo[1])) snd::Play("m01se_24appearance");
	if (due(m_fDemo[0]))
	{
		snd::Play("m01se_29kamituki");
		cTaskNodeEffect* e = new cTaskNodeEffect([this](float o[3]) {
			float m[16];
			if (!cTaskNodeEffect::ModelNodeMtx(pModel(0), "ef_scissors", m)) return false;
			o[0] = m[12]; o[1] = m[13]; o[2] = m[14];
			return true;
		});
		e->SetMtx([this](float m[16]) { return cTaskNodeEffect::ModelNodeMtx(pModel(0), "ef_scissors", m); });   // DAT_8047a90c
		ConnectChild(e);
		e->Start(0x35u, true, true, true);
	}
	for (uint i = 0; i < 3; i++)
		if (due(m_fDemo[7 + i]) && m_pLine != nullptr)
		{
			m_pLine->pAnita(i)->SetAnimNo((int)i, true);
			m_pLine->pAnita(i)->SetState(1u, true);
		}
	if (due(m_fDemo[10])) snd::Play("m01bgm_Boss1");
	return true;
}

// DOL 0x800efe1c [audited: state 0 (the demo object's start tick 0x801bce00) and the goal tick 0x801bce94 live in cDemoMan / GoalInit and were not compared] (not paused): the model task; state 0 / 1 = the demo object's start / goal paths; the clips advance (FUN_8009a478)
bool cTaskGoalBoss::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	cTaskModel::Execute(pParam);
	if (m_nState == 1)
	{
		if (m_nGoalStep == 0)
		{
			if (m_nStage == 9) GoalInit();
			else m_bGoalDone = true;
			m_nGoalStep = 1;
		}
		else if (!m_bGoalDone && !GoalCheck())
			m_bGoalDone = true;
	}
	UpdateFrame();
	return true;
}

}
