#include "kingdom/game/g01/cTaskStart.hpp"

#include <cstdio>
#include <cstring>

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "snd/Mixer.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g01
{

namespace
{
constexpr float DEG2RAD = 0.017453292f;   // FLOAT_8047b740
constexpr uint ELV01 = 0x15;              // PTK_ELV01

bool Exists(const char* name)
{
	std::size_t size = 0;
	return wii::ArchiveStore::Find(name, size) != nullptr;
}

// clip kind k (0 cha, 1 cla, 2 tsa, 3 tpa, 4 via) of a model: register, rate, frame and length
void Clip(cKingdomModel* m, int k, const char* name, float rate)
{
	m->RegistAnm(0u, name);   // 0x8001a46c
	switch (k)
	{
	case 0: m->SetUpdateRateAnmChr(0u, rate); break;      // 0x8001cad4
	case 1: m->SetUpdateRateMatClr(0u, rate); break;      // 0x8001cd4c
	case 2: m->SetUpdateRateAnmTexSrt(0u, rate); break;   // 0x8001cfdc
	case 3: m->SetUpdateRateAnmTexPat(0u, rate); break;   // 0x8001ce94
	case 4: m->SetUpdateRateAnmVis(0u, rate); break;      // 0x8001cc1c
	}
}

float FrameMax(cKingdomModel* m, int k)
{
	switch (k)
	{
	case 0: return m->GetFrameMaxAnmChr(0u);      // 0x8001c630
	case 1: return m->GetFrameMaxMatClr(0u);      // 0x8001c804
	case 2: return m->GetFrameMaxAnmTexSrt(0u);   // 0x8001c9e4
	case 3: return m->GetFrameMaxAnmTexPat(0u);   // 0x8001c8f4
	default: return m->GetFrameMaxAnmVis(0u);     // 0x8001c720
	}
}

float Frame(cKingdomModel* m, int k)
{
	switch (k)
	{
	case 0: return m->GetFrameAnmChr(0u);
	case 1: return m->GetFrameMatClr(0u);
	case 2: return m->GetFrameAnmTexSrt(0u);
	case 3: return m->GetFrameAnmTexPat(0u);
	default: return m->GetFrameAnmVis(0u);
	}
}
}

// DOL 0x800faf04
cTaskStart::cTaskStart()
{
	SetName("g01::cTaskStart");
}

cTaskStart::~cTaskStart() = default;

// DOL 0x800fb0e4 / 0x800fb188: for n = 1, 2 ... while m01g_s%x_s%02d.brres exists, the first skeleton node containing m01g_s%x_s%02d_
// gives the placement (translate, rotate * DEG2RAD, scale). Stage 1 registers its demo object with cDemoMan, the others load the a01
// models at once. State 2 (no demo running).
void cTaskStart::Setup(int nStage, const wii::Mdl0& skel)
{
	m_nStage = nStage;
	for (int n = 1;; n++)
	{
		char file[0x80], key[0x80];
		std::snprintf(file, sizeof file, "m01g_s%x_s%02d.brres", nStage, n);   // DAT_80477a18
		if (!Exists(file)) break;
		std::snprintf(key, sizeof key, "m01g_s%x_s%02d_", nStage, n);         // DAT_80477a14
		for (const wii::Mdl0Node& node : skel.nodes)
		{
			if (std::strstr(node.name.c_str(), key) == nullptr) continue;
			Place p;
			p.t = Vector4(node.translate[0], node.translate[1], node.translate[2], 1.0f);   // w = DAT_80375cf4 (1.0)
			p.r = Vector4(node.rotate[0] * DEG2RAD, node.rotate[1] * DEG2RAD, node.rotate[2] * DEG2RAD, 1.0f);
			p.s = Vector4(node.scale[0], node.scale[1], node.scale[2], 1.0f);
			p.nIndex = n;
			m_Place.push_back(p);
			break;   // the loop counter is overwritten with the node: only the first node counts
		}
	}
	if (nStage == 1)
	{
		if (cDemoMan* dm = KingdomGlobal::GalagaSys->pDemoMan()) dm->Regist(this);   // 0x801c0afc + 0x801be964
		// Demo_s1.csv: e_elevator01 / s_elevator01 / s_elevator_end01, column 1 = the frame (0x801c0afc)
		kingdom::cKingdomCSV csv;
		if (csv.RegistData("Demo_s1.csv"))
		{
			const char* key[3] = {"e_elevator01", "s_elevator01", "s_elevator_end01"};
			float* out[3] = {&m_fEffectTime, &m_fSeTime, &m_fSeEndTime};
			for (int i = 0; i < 3; i++) csv.F32(csv.SearchRow(0, key[i], 0, 0), 1, *out[i]);
		}
	}
	else
		LoadGame();
	m_nState = 2;
}

// DOL 0x800fb97c: model slot i = m01g_s%x_s%02d.brres of placement i at its SRT, light set 0, fog 3
void cTaskStart::LoadModels()
{
	for (std::size_t i = 0; i < m_Place.size(); i++)
	{
		char file[0x80];
		std::snprintf(file, sizeof file, "m01g_s%x_s%02d.brres", m_nStage, m_Place[i].nIndex);
		cKingdomModel* m = CreateModel((uint)i);   // replaces the slot (0x8009991c)
		m->RegistModel(file, 0x36f);
		m->SetLightSetIdx(0);
		m->vTrans() = m_Place[i].t;
		m->vRotate() = m_Place[i].r;
		m->vScale() = m_Place[i].s;
		m->SetFogIdx(3);
	}
}

// DOL 0x800fbcb8: the a01 clips (cha / cla / tsa at 0.1 before world 3, else 1.0; tpa / via at 1.0)
void cTaskStart::LoadGame()
{
	static const char* const A01[5] = {"m01g_s%x_s%02d_a01_cha.brres", "m01g_s%x_s%02d_a01_cla.brres", "m01g_s%x_s%02d_a01_tsa.brres",
	                                   "m01g_s%x_s%02d_a01_tpa.brres", "m01g_s%x_s%02d_a01_via.brres"};
	LoadModels();
	const float rate = KingdomGlobal::GalagaSys->nWorld() < 3 ? 0.1f : 1.0f;   // FLOAT_8047b748 / FLOAT_8047b744
	for (std::size_t i = 0; i < m_Place.size(); i++)
		for (int k = 0; k < 5; k++)
		{
			char name[0x80];
			std::snprintf(name, sizeof name, A01[k], m_nStage, m_Place[i].nIndex);
			if (Exists(name)) Clip(pModel((uint)i), k, name, k < 3 ? rate : 1.0f);
		}
}

// DOL 0x800fbab0: the stage 1 demo clips m01g_s1_s%02d_a10_* at 1.0; the longest clip of all is the demo's (cAnmCheck: model, kind, max)
void cTaskStart::LoadDemo()
{
	static const char* const A10[5] = {"m01g_s1_s%02d_a10_cha.brres", "m01g_s1_s%02d_a10_cla.brres", "m01g_s1_s%02d_a10_tsa.brres",
	                                   "m01g_s1_s%02d_a10_tpa.brres", "m01g_s1_s%02d_a10_via.brres"};
	LoadModels();
	m_nChkModel = -1;
	m_fChkMax = 0.0f;
	for (std::size_t i = 0; i < m_Place.size(); i++)
		for (int k = 0; k < 5; k++)
		{
			char name[0x80];
			std::snprintf(name, sizeof name, A10[k], m_Place[i].nIndex);
			if (!Exists(name)) continue;
			cKingdomModel* m = pModel((uint)i);
			Clip(m, k, name, 1.0f);
			const float max = FrameMax(m, k);
			if (m_fChkMax < max)
			{
				m_nChkModel = (int)i;
				m_nChkKind = k;
				m_fChkMax = max;
			}
		}
}

// cDemoStartW1S1::Init 0x801c0e3c: the demo clips; the elevator effect position = node m01e_22_elv01_001 (DAT_804785b8) of
// m01e_s1_d_s (DAT_804785b4, loaded into model slot 10 by the DOL; a skeleton without shapes), w = 1.0 (FLOAT_8047cf38)
void cTaskStart::DemoInit()
{
	LoadDemo();
	m_fChkLast = 0.0f;
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find("m01e_s1_d_s.brres", size);
	wii::Brres b;
	wii::Mdl0 skel;
	if (d != nullptr && b.Load(d, size) && skel.Load(b, "m01e_s1_d_s"))
		for (const wii::Mdl0Node& node : skel.nodes)
			if (node.name == "m01e_22_elv01_001") m_vEffect = Vector4(node.translate[0], node.translate[1], node.translate[2], 1.0f);
}

// cDemoStartW1S1::IsWaitEnd 0x801c0fac: true while the longest clip runs; on the way PTK_ELV01 (onetime, dependent, not clipped) and the
// two elevator sounds start once their Demo_s1.csv frame is reached (cAnmCheck 0x801bcd20 / frame 0x801bccbc; native: a wrap counts as
// the end, UNVERIFIED)
bool cTaskStart::DemoCheck()
{
	if (m_nChkModel < 0) return false;
	const float f = Frame(pModel((uint)m_nChkModel), m_nChkKind);
	if (f >= m_fChkMax || f < m_fChkLast) return false;
	m_fChkLast = f;
	if (m_fEffectTime != -1.0f && m_fEffectTime <= f)
	{
		m_fEffectTime = -1.0f;
		m_Effect.Generate(ELV01, true, true, false, 0.0f, m_vEffect);
	}
	if (m_fSeTime != -1.0f && m_fSeTime <= f)
	{
		m_fSeTime = -1.0f;
		m_nSe = snd::Play("m01se_16elevator");     // DAT_8047cf30
	}
	if (m_fSeEndTime != -1.0f && m_fSeEndTime <= f)
	{
		m_fSeEndTime = -1.0f;
		m_nSeEnd = snd::Play("m01se_17elevator2");  // DAT_8047cf34
	}
	return true;
}

// cDemoStartW1S1::Term 0x801c0f20: done, the game clips, the effect and the sounds stop
void cTaskStart::DemoFinish()
{
	m_bDemoDone = true;
	LoadGame();
	if (m_Effect.IsValid()) m_Effect.Detach(true);   // 0x8008a508(hnd, 1)
	if (m_nSe >= 0 && snd::IsPlaying(m_nSe)) snd::Stop(m_nSe);
	if (m_nSeEnd >= 0 && snd::IsPlaying(m_nSeEnd)) snd::Stop(m_nSeEnd);
	m_nSe = m_nSeEnd = -1;
}

// DOL 0x800fbeec: state 0 runs the demo object (0x801bce00: Init, then until its check fails), then the clips advance (0x8009a478)
bool cTaskStart::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (m_nState == 0)
	{
		if (m_nDemoState == 0)
		{
			DemoInit();
			m_nDemoState = 1;
		}
		else if (m_nDemoState == 1)
		{
			if (!m_bDemoDone) { if (!DemoCheck()) m_bDemoDone = true; }
			else m_nDemoState = 2;
		}
	}
	UpdateFrame();
	return cTaskModel::Execute(pParam);
}

}
