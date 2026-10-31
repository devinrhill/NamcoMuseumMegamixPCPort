#include "kingdom/MslRand.hpp"
#include "kingdom/game/g01/cTaskSS.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"

#include <cstdio>
#include <cstdlib>

#include "wii/Mdl0.hpp"

namespace kingdom::game::g01
{

// cTaskSS: its Draw is the model task's (0x8009aad0).

namespace
{
constexpr float DEG2RAD = 0.017453292f;   // FLOAT_8047b728
constexpr float STAR_SCALE = 10.0f;       // FLOAT_8047b72c
constexpr unsigned STAR_SLOT = 0x20;
}

float cTaskSS::NextWait()
{
	return (float)((unsigned)kingdom::MslRand() % 60000u + 40000u) / 1000.0f;   // rand 0x80211048, FLOAT_8047b710
}

// DOL 0x800f8f74 [audited]: the model m01e_53_star01.brres (FUN_8001f6c0), its node count, the first wait, active, state 1 (+0xa8 = 0x800f9450)
cTaskSS::cTaskSS()
{
	SetName("g01::cTaskSS");
	cKingdomModel* m = CreateModel(0);
	m->RegistModel("m01e_53_star01.brres", 0x36f);   // s_m01e_53_star01_brres_8037532c
	if (const wii::Mdl0* mdl = m->pMdl0()) m_nNodes = (int)mdl->nodes.size();
	m_fWait = NextWait();
	m_b08 = true;
	m_nState = 1;
	m_nStep = 0;
}

// DOL 0x800f91e8 [audited]: counting down; below 0 the previous star is detached (FUN_8008a508(+0x2f0, 0)) and slot 0x20 starts at a random node
// 2 .. n - 1 (translate, rotate in degrees -> radians, scale 10) (FUN_80021a9c: one-time, depend, clipping, light 0, fog -1), next wait
void cTaskSS::Step()
{
	if (0.0f <= m_fWait)   // FLOAT_8047b720
	{
		m_fWait -= 1.0f;   // FLOAT_8047b724
		return;
	}
	cKingdomModel* m = pModel(0);
	const wii::Mdl0* mdl = m != nullptr ? m->pMdl0() : nullptr;
	if (mdl != nullptr && m_nNodes > 2)
	{
		const unsigned span = (unsigned)(m_nNodes - 2);
		const wii::Mdl0Node& n = mdl->nodes[(unsigned)kingdom::MslRand() % span + 2u];
		if (m_Hnd.IsValid()) m_Hnd.Detach(false);
		const Vector4 pos(n.translate[0], n.translate[1], n.translate[2], 1.0f);
		const Vector4 rot(n.rotate[0] * DEG2RAD, n.rotate[1] * DEG2RAD, n.rotate[2] * DEG2RAD, 1.0f);
		const Vector4 scale(STAR_SCALE, STAR_SCALE, STAR_SCALE, STAR_SCALE);
		const bool ok = m_Hnd.Generate(STAR_SLOT, true, true, true, 0.0f, pos, rot, scale, 0, -1);
		if (std::getenv("MEGAMIX_G01DBG")) std::fprintf(stderr, "g01 shooting star at %s (%.1f %.1f %.1f) %s\n", n.name.c_str(), pos.x, pos.y, pos.z, ok ? "ok" : "FAILED");
	}
	m_fWait = NextWait();
}

// DOL 0x800f9184 [audited] (+0x70 0x800f90b8 sets state 0 / step 0, +0x74 0x800f9450 state 1 / step 0 [audited])
bool cTaskSS::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (m_b08 && m_nState == 1) Step();
	return true;
}

}
