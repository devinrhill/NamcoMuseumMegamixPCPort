#include "kingdom/game/g01/cTaskGoal.hpp"

#include <cstdio>
#include <cstring>

#include "KingdomGlobal.hpp"
#include "snd/Mixer.hpp"
#include "kingdom/game/g01/cTaskNodeEffect.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "wii/Archive.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g01
{

namespace
{
constexpr float DEG2RAD = 0.017453292f;   // FLOAT_8047b610

bool Exists(const char* name)
{
	std::size_t size = 0;
	return wii::ArchiveStore::Find(name, size) != nullptr;
}
}

// DOL 0x800ee050 [audited]: state 2 (no demo running)
cTaskGoal::cTaskGoal()
{
	SetName("g01::cTaskGoal");
	m_nState = 2;
}

// DOL 0x800eeedc [audited] / 0x800ee1e4
void cTaskGoal::Setup(int nStage, const wii::Mdl0& skel)
{
	m_nStage = nStage;
	const int set = nStage < 7 ? 1 : 7;
	for (int n = 1;; n++)
	{
		char file[0x80], key[0x80];
		std::snprintf(file, sizeof file, "m01g_s%x_g%02d.brres", set, n);   // DAT_804778f8
		if (!Exists(file)) break;
		std::snprintf(key, sizeof key, "m01g_s%x_g%02d_", set, n);          // DAT_804778f4
		for (const wii::Mdl0Node& node : skel.nodes)
		{
			if (std::strstr(node.name.c_str(), key) == nullptr) continue;
			Place p;
			p.t = Vector4(node.translate[0], node.translate[1], node.translate[2], 1.0f);   // w = DAT_80374614 (1.0)
			p.r = Vector4(node.rotate[0] * DEG2RAD, node.rotate[1] * DEG2RAD, node.rotate[2] * DEG2RAD, 1.0f);
			p.s = Vector4(node.scale[0], node.scale[1], node.scale[2], 1.0f);
			p.nSet = set;
			p.nIndex = n;
			m_Place.push_back(p);
			break;   // the loop counter is overwritten with the node: only the first node counts
		}
	}
	if (nStage == 3)   // the goal demo object 0x801bd5ac, registered with cDemoMan
	{
		if (cDemoMan* dm = KingdomGlobal::GalagaSys->pDemoMan()) dm->Regist(this);
		kingdom::cKingdomCSV csv;
		if (csv.RegistData("Demo_s3.csv"))
		{
			const char* key[7] = {"e_power01", "e_jet01", "e_spark01", "s_seridasi01", "s_catapult01", "s_booster01", "s_catapult_start01"};
			for (int i = 0; i < 7; i++) csv.F32(csv.SearchRow(0, key[i], 0, 0), 1, m_fGoalTime[i]);
		}
	}
	else
		m_bDemoDone = m_bGoalDone = true;
	LoadGame();   // vtable +0x78 in both paths
}

// DOL 0x800ee9fc [audited]: model slot i = m01g_s%x_g%02d.brres at its SRT, light set 0, fog 3
void cTaskGoal::LoadModels()
{
	for (std::size_t i = 0; i < m_Place.size(); i++)
	{
		char file[0x80];
		std::snprintf(file, sizeof file, "m01g_s%x_g%02d.brres", m_Place[i].nSet, m_Place[i].nIndex);
		cKingdomModel* m = CreateModel((uint)i);
		m->RegistModel(file, 0x36f);
		m->SetLightSetIdx(0);
		m->vTrans() = m_Place[i].t;
		m->vRotate() = m_Place[i].r;
		m->vScale() = m_Place[i].s;
		m->SetFogIdx(3);
	}
}

// DOL 0x800eeb30 [audited]: the a01 clips {cha, cla, tsa, tpa, via}
void cTaskGoal::LoadGame()
{
	static const char* const A01[5] = {"m01g_s%x_g%02d_a01_cha.brres", "m01g_s%x_g%02d_a01_cla.brres", "m01g_s%x_g%02d_a01_tsa.brres",
	                                   "m01g_s%x_g%02d_a01_tpa.brres", "m01g_s%x_g%02d_a01_via.brres"};
	LoadModels();
	const float rate = KingdomGlobal::GalagaSys->nWorld() < 3 ? 0.1f : 1.0f;   // FLOAT_8047b614 / FLOAT_8047b618
	for (std::size_t i = 0; i < m_Place.size(); i++)
	{
		cKingdomModel* m = pModel((uint)i);
		for (int k = 0; k < 5; k++)
		{
			char name[0x80];
			std::snprintf(name, sizeof name, A01[k], m_Place[i].nSet, m_Place[i].nIndex);
			if (!Exists(name)) continue;
			m->RegistAnm(0u, name);   // 0x8001a46c
			switch (k)
			{
			case 0: m->SetUpdateRateAnmChr(0u, rate); break;
			case 1: m->SetUpdateRateMatClr(0u, rate); break;
			case 2: m->SetUpdateRateAnmTexSrt(0u, rate); break;
			case 3: m->SetUpdateRateAnmTexPat(0u, 1.0f); break;
			case 4: m->SetUpdateRateAnmVis(0u, 1.0f); break;
			}
		}
	}
}

// DOL 0x800eecf0 [audited]: the a20 clips {cha, cla, tsa, tpa, via} at 1.0; the longest one is the demo's (cAnmCheck)
void cTaskGoal::LoadDemo()
{
	static const char* const A20[5] = {"m01g_s%x_g%02d_a20_cha.brres", "m01g_s%x_g%02d_a20_cla.brres", "m01g_s%x_g%02d_a20_tsa.brres",
	                                   "m01g_s%x_g%02d_a20_tpa.brres", "m01g_s%x_g%02d_a20_via.brres"};
	LoadModels();
	m_nChkModel = -1;
	m_fChkMax = 0.0f;
	for (std::size_t i = 0; i < m_Place.size(); i++)
	{
		cKingdomModel* m = pModel((uint)i);
		for (int k = 0; k < 5; k++)
		{
			char name[0x80];
			std::snprintf(name, sizeof name, A20[k], m_Place[i].nSet, m_Place[i].nIndex);
			if (!Exists(name)) continue;
			m->RegistAnm(0u, name);
			float max = 0.0f;
			switch (k)
			{
			case 0: m->SetUpdateRateAnmChr(0u, 1.0f); max = m->GetFrameMaxAnmChr(0u); break;
			case 1: m->SetUpdateRateMatClr(0u, 1.0f); max = m->GetFrameMaxMatClr(0u); break;
			case 2: m->SetUpdateRateAnmTexSrt(0u, 1.0f); max = m->GetFrameMaxAnmTexSrt(0u); break;
			case 3: m->SetUpdateRateAnmTexPat(0u, 1.0f); max = m->GetFrameMaxAnmTexPat(0u); break;
			case 4: m->SetUpdateRateAnmVis(0u, 1.0f); max = m->GetFrameMaxAnmVis(0u); break;
			}
			if (m_fChkMax < max) { m_fChkMax = max; m_nChkModel = (int)i; m_nChkKind = k; }
		}
	}
}

// cDemoGoalW1S3::Start 0x801bd904: vtable +0x74, then model slot 20 (DAT_80478530) = the effect skeleton m01e_s3_d_s with its a20 bone clip
// at 1.0 (FLOAT_8047ceb8); the three effects at its nodes m01e_24_power01_001 / m01e_25_jet01_001 / m01e_29_spark01_001 (0x800e0a38)
void cTaskGoal::GoalInit()
{
	LoadDemo();
	cKingdomModel* d = CreateModel(20u);
	d->RegistModel("m01e_s3_d_s.brres", 0x36f);
	d->RegistAnm(0u, "m01e_s3_d_s_a20_cha.brres");
	d->SetUpdateRateAnmChr(0u, 1.0f);
	static const char* const NODE[3] = {"m01e_24_power01_001", "m01e_25_jet01_001", "m01e_29_spark01_001"};
	for (int i = 0; i < 3; i++)
	{
		const char* node = NODE[i];
		m_pEffect[i] = new cTaskNodeEffect([this, node](float o[3]) {
			cKingdomModel* m = pModel(20u);
			float n[12], w[16];
			if (m == nullptr || !m->NodeMatrix(node, n)) return false;
			m->CalcWorld(w);
			for (int k = 0; k < 3; k++) o[k] = w[k] * n[3] + w[4 + k] * n[7] + w[8 + k] * n[11] + w[12 + k];
			return true;
		});
		m_pEffect[i]->SetMtx([this, node](float m[16]) { return cTaskNodeEffect::ModelNodeMtx(pModel(20u), node, m); });   // 0x800e11fc
		ConnectChild(m_pEffect[i]);
	}
	m_fChkLast = 0.0f;
}

// cDemoGoalW1S3::Execute 0x801bd9ac: while the longest clip runs, at the Demo_s3.csv frames: m01se_19seridasi, m01se_20seridasi2,
// m01se_21charge, m01se_22hassya; PTK_POWER01 (0x17), PTK_JET01 (0x18), PTK_SPARK01 (0x1b) at their nodes (one-time, dependent, clipped)
bool cTaskGoal::ClipCheck()
{
	if (m_nChkModel < 0) return false;
	cKingdomModel* m = pModel((uint)m_nChkModel);
	float f = 0.0f;
	switch (m_nChkKind)
	{
	case 0: f = m->GetFrameAnmChr(0u); break;
	case 1: f = m->GetFrameMatClr(0u); break;
	case 2: f = m->GetFrameAnmTexSrt(0u); break;
	case 3: f = m->GetFrameAnmTexPat(0u); break;
	default: f = m->GetFrameAnmVis(0u); break;
	}
	if (f >= m_fChkMax || f < m_fChkLast) return false;
	m_fChkLast = f;
	return true;
}

bool cTaskGoal::GoalCheck()
{
	if (m_nChkModel < 0) return false;
	cKingdomModel* m = pModel((uint)m_nChkModel);
	float f = 0.0f;
	switch (m_nChkKind)
	{
	case 0: f = m->GetFrameAnmChr(0u); break;
	case 1: f = m->GetFrameMatClr(0u); break;
	case 2: f = m->GetFrameAnmTexSrt(0u); break;
	case 3: f = m->GetFrameAnmTexPat(0u); break;
	default: f = m->GetFrameAnmVis(0u); break;
	}
	if (f >= m_fChkMax || f < m_fChkLast) return false;   // native: a wrap is an end too (UNVERIFIED)
	m_fChkLast = f;
	auto due = [f](float& t) { if (t != -1.0f && t <= f) { t = -1.0f; return true; } return false; };
	if (due(m_fGoalTime[3])) snd::Play("m01se_19seridasi");
	if (due(m_fGoalTime[4])) snd::Play("m01se_20seridasi2");
	if (due(m_fGoalTime[5])) snd::Play("m01se_21charge");
	if (due(m_fGoalTime[6])) snd::Play("m01se_22hassya");
	if (due(m_fGoalTime[0]) && m_pEffect[0]) m_pEffect[0]->Start(0x17u, true, true, true);
	if (due(m_fGoalTime[1]) && m_pEffect[1]) m_pEffect[1]->Start(0x18u, true, true, true);
	if (due(m_fGoalTime[2]) && m_pEffect[2]) m_pEffect[2]->Start(0x1bu, true, true, true);
	return true;
}

// DOL 0x800eefa4 [audited: the demo object's ticks 0x801bce00 / 0x801bce94 live in GoalInit / GoalCheck and were not compared]: the demo object's state 1 path (0x801bce94: Init, then while the check holds), then the clips
bool cTaskGoal::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	cTaskModel::Execute(pParam);
	if (m_nState == 1)
	{
		if (m_nGoalStep == 0)
		{
			if (m_nStage == 3) GoalInit();
			else m_bGoalDone = true;
			m_nGoalStep = 1;
		}
		else if (!m_bGoalDone && !GoalCheck())
			m_bGoalDone = true;
	}
	UpdateFrame();   // 0x8009a478
	return true;
}

}
