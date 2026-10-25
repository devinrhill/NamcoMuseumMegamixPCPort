// Particle system natively: cJetPtkMan keeps one gfx::PtkSim (port of the jet particle simulation, see gfx/PtkSim.hpp) per data slot; every
// Generate() spawns an emitter in the slot's sim and returns a cJetPtkEmi that mirrors its flags / world matrix into it each frame.
// The converted C# classes (cJetPtkMan.cpp, jet/ptk/*) stay out of the build. UNVERIFIED against the DOL beyond what PtkSim documents.
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <set>

#include "cJetPtkMan.hpp"
#include "jet/ptk/cJetPtkEmi.hpp"
#include "CommonUtil.hpp"
#include "gfx/PtkSim.hpp"

namespace
{
struct Slot
{
	std::unique_ptr<gfx::PtkSim> sim;
	bool alphaTestOverride = false;
};
struct Live { jet::ptk::cJetPtkEmi* emi; gfx::PtkSim* sim; unsigned id; unsigned slot; };
struct State
{
	std::vector<Slot> slots;
	std::vector<Live> live;
};
std::map<const cJetPtkMan*, State> g_state;
std::map<const jet::ptk::cJetPtkEmi*, Live> g_emiLive;
// Managers that stepped since the last draw are drawn: a game can run several (Pac-Motos: cTaskEffect for the hits + cTaskBGEffect for the
// mist), and managers of a previous stage (tasks released, not deleted) stop stepping and so stop drawing. Drawing only the manager that
// stepped last hid Pac-Motos' collision sparks behind the BG mist manager (27th session). A frame where none stepped (pause) draws the last set.
std::set<const cJetPtkMan*> g_stepped, g_drawn;

// the owner's handle -> emitter world matrix -> sim (12 floats)
void PushMatrix(Live& l, int active, int wait, int deadReserve, int visible)
{
	auto* e = l.emi;
	e->UpdateMatrix();
	float m[12];
	for (int i = 0; i < 12; i++) m[i] = e->pWorldMtx().m[i];
	// cJetPtkHnd negates x / rotY (Unity -> Wii coordinates), but the native 3D pass draws in the converted code's own (Unity) space: mirror back
	for (int i = 0; i < 4; i++) m[i] = -m[i];
	l.sim->Control(l.id, m, active, wait, deadReserve, visible);
}
}

cJetPtkMan::cJetPtkMan() { g_state[this]; }
cJetPtkMan::cJetPtkMan(uint nSlot, uint, uint, uint) { g_state[this].slots.resize(nSlot); }
cJetPtkMan::~cJetPtkMan() { Term(); g_state.erase(this); g_stepped.erase(this); g_drawn.erase(this); }
void cJetPtkMan::Term()
{
	Genocide();
	g_state[this].slots.clear();
}
void cJetPtkMan::Create(uint nSlot, uint, uint, uint)
{
	Genocide();
	g_state[this].slots.clear();
	g_state[this].slots.resize(nSlot);
}
void cJetPtkMan::SetData(uint nSlotNo, std::vector<byte>& bin)
{
	State& st = g_state[this];
	if (nSlotNo >= st.slots.size()) st.slots.resize(nSlotNo + 1);
	wii::PtkFile f;
	if (!f.Parse(bin.data(), bin.size())) return;
	st.slots[nSlotNo].sim = std::make_unique<gfx::PtkSim>(std::move(f));
}
void cJetPtkMan::SetOverrideAlphaTest(uint nSlotNo)
{
	State& st = g_state[this];
	if (nSlotNo >= st.slots.size()) st.slots.resize(nSlotNo + 1);
	st.slots[nSlotNo].alphaTestOverride = true; // UNVERIFIED: PtkSim has no alpha test override yet (ptc_alpha_test_func = 5 for "ef_steam" data)
}
bool cJetPtkMan::Generate(cJetPtkHnd* pHnd, uint nSlotNo, bool bOneTime, bool bDepend, bool, float fDelay, int nLightSetIdx, int nFogIdx, int, float)
{
	State& st = g_state[this];
	if (nSlotNo >= st.slots.size() || !st.slots[nSlotNo].sim)
	{
		if (std::getenv("MEGAMIX_PTKLOG")) std::fprintf(stderr, "ptk: slot %u has no data (%zu slots)\n", nSlotNo, st.slots.size());
		return false;
	}
	gfx::PtkSim* sim = st.slots[nSlotNo].sim.get();
	auto* emi = new jet::ptk::cJetPtkEmi();
	emi->bActive(true);
	emi->bVisible(true);
	emi->bUse(true);
	emi->nLightSetIdx(nLightSetIdx);
	emi->nFogIdx(nFogIdx);
	const float ident[12] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0};
	const unsigned id = sim->Spawn(ident, bOneTime, bDepend, fDelay);
	if (pHnd) emi->AttachHnd(pHnd);
	if (std::getenv("MEGAMIX_PTKLOG")) std::fprintf(stderr, "ptk: slot %u emitter %u (gens %zu)\n", nSlotNo, id, sim->File().gens.size());
	const Live l{emi, sim, id, nSlotNo};
	st.live.push_back(l);
	g_emiLive[emi] = l;
	return true;
}
void cJetPtkMan::Genocide()
{
	State& st = g_state[this];
	for (Live& l : st.live)
	{
		l.emi->DetachHnd();
		l.sim->Control(l.id, nullptr, -1, -1, 1, -1);
		g_emiLive.erase(l.emi);
		delete l.emi;
	}
	st.live.clear();
}
void cJetPtkMan::AnimUpdate(float fStep)
{
	g_stepped.insert(this);
	State& st = g_state[this];
	for (Live& l : st.live)
	{
		auto* e = l.emi;
		// wait / deadReserve only go owner -> sim when the owner sets them (Detach): the sim sets wait itself when a one-time emitter's life ends,
		// and pushing the owner's `false` every frame restarted it (wall-hit stars spawned forever)
		PushMatrix(l, e->IsActive(), e->IsWait() ? 1 : -1, e->IsDeadReserve() ? 1 : -1, e->IsVisible());
	}
	for (Slot& s : st.slots) if (s.sim) s.sim->Step(fStep);
	if (std::getenv("MEGAMIX_PTKLOG"))
	{
		static int n = 0;
		if (++n % 60 == 0)
		{
			size_t p = 0;
			for (Slot& s : st.slots) if (s.sim) p += s.sim->ParticleCount();
			std::fprintf(stderr, "ptk: frame %d emitters %zu particles %zu\n", n, st.live.size(), p);
		}
	}
	// emitters the sim has finished are released (a handle still pointing at one is detached first)
	st.live.erase(std::remove_if(st.live.begin(), st.live.end(), [](Live& l) {
		if (l.sim->Alive(l.id)) return false;
		if (std::getenv("MEGAMIX_PTKLOG")) std::fprintf(stderr, "ptk: end slot %u emitter %u\n", l.slot, l.id);
		l.emi->DetachHnd();
		g_emiLive.erase(l.emi);
		delete l.emi;
		return true;
	}), st.live.end());
}
void cJetPtkMan::Draw(Camera*, Vector4, Vector4, float) {} // drawn by NativeScene::Flush (gfx::DrawParticles) inside the 3D pass
uint cJetPtkMan::nUseEmi() { return (uint)g_state[this].live.size(); }

namespace gfx
{
void DrawParticles()
{
	if (!g_stepped.empty()) { g_drawn = g_stepped; g_stepped.clear(); }
	for (const cJetPtkMan* man : g_drawn)
	{
		auto it = g_state.find(man);
		if (it == g_state.end()) continue;
		// owners move their handles in Draw (Pac-Man's dash trail, cTaskCharPacman::Draw): take those positions now, not one frame late at the next AnimUpdate
		for (Live& l : it->second.live) PushMatrix(l, -1, -1, -1, -1);
		for (Slot& s : it->second.slots) if (s.sim) s.sim->Draw();
	}
}
}

namespace jet::ptk
{
cJetPtkEmi::cJetPtkEmi() { m_mWorldMtx = Matrix4x4::identity(); m_mBillBoardMtx = Matrix4x4::identity(); m_vScale = Vector4::one; }
cJetPtkEmi::~cJetPtkEmi() { DetachHnd(); }
// C# cJetPtkEmi.UpdateMatrix: a handle moved since the last frame (vSetTrans / vSetRotate / vSetScale) rebuilds the world matrix; the empty stub kept
// every emitter where it was generated (the dash trail stayed behind Pac-Man)
void cJetPtkEmi::UpdateMatrix()
{
	if (m_pHnd != nullptr && m_pHnd->IsUpdate()) m_pHnd->UpdateMatrix();
}
bool cJetPtkEmi::ClippingExec(Camera*, Vector4, Vector4, float) { return true; }
void cJetPtkEmi::AttachHnd(cJetPtkHnd* pHnd)
{
	if (pHnd != nullptr)
	{
		if (pHnd->pEmi() != this && pHnd->pEmi() != nullptr) pHnd->pEmi()->DetachHnd();
		pHnd->pEmi(this);
	}
	m_pHnd = pHnd;
}
void cJetPtkEmi::DetachHnd()
{
	if (m_pHnd != nullptr && m_pHnd->pEmi() == this) m_pHnd->pEmi(nullptr);
	m_pHnd = nullptr;
}
static void SetFlag(uint& f, uint bit, bool b) { if (b) f |= bit; else f &= ~bit; }
void cJetPtkEmi::bWait(bool b) { SetFlag(m_nFlags, 16u, b); }
bool cJetPtkEmi::IsWait() { return (m_nFlags & 16u) != 0; }
void cJetPtkEmi::bUse(bool b) { SetFlag(m_nFlags, 32u, b); }
bool cJetPtkEmi::IsUse() { return (m_nFlags & 32u) != 0; }
void cJetPtkEmi::bDeadReserve(bool b) { SetFlag(m_nFlags, 64u, b); }
bool cJetPtkEmi::IsDeadReserve() { return (m_nFlags & 64u) != 0; }
void cJetPtkEmi::bActive(bool b) { SetFlag(m_nFlags, 1u, b); }
bool cJetPtkEmi::IsActive() { return (m_nFlags & 1u) != 0; }
void cJetPtkEmi::bVisible(bool b) { SetFlag(m_nFlags, 2u, b); }
bool cJetPtkEmi::IsVisible() { return (m_nFlags & 2u) != 0; }
void cJetPtkEmi::nFogIdx(int i) { m_nFogIdx = i; }
int cJetPtkEmi::nFogIdx() { return m_nFogIdx; }
void cJetPtkEmi::nLightSetIdx(int i) { m_nLightSetIdx = i; }
int cJetPtkEmi::nLightSetIdx() { return m_nLightSetIdx; }
Matrix4x4& cJetPtkEmi::pWorldMtx() { return m_mWorldMtx; }
}

void CommonUtil::SetLayer(GameObject*, string) {}
void CommonUtil::SetLayer(GameObject*, int) {}
void CommonUtil::GetChildren(GameObject*, std::vector<GameObject*>&) {}
