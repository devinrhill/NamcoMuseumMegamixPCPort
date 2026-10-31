#include "kingdom/game/g01/cTaskNodeEffect.hpp"

#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/cKingdomModel.hpp"

namespace kingdom::game::g01
{

void cTaskNodeEffect::FromNode(const float n[12], float out[16])
{
	for (int r = 0; r < 3; r++)
		for (int k = 0; k < 4; k++) out[k * 4 + r] = n[r * 4 + k];
	out[3] = out[7] = out[11] = 0.0f;
	out[15] = 1.0f;
}

void cTaskNodeEffect::Mul(const float a[16], const float b[16], float out[16])
{
	float t[16];
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
		{
			float v = 0.0f;
			for (int k = 0; k < 4; k++) v += a[k * 4 + r] * b[c * 4 + k];
			t[c * 4 + r] = v;
		}
	for (int i = 0; i < 16; i++) out[i] = t[i];
}

bool cTaskNodeEffect::ModelNodeMtx(kingdom::cKingdomModel* m, const char* node, float out[16])
{
	float w[16], n[12], nm[16];
	if (m == nullptr || !m->NodeMatrix(node, n)) return false;
	m->CalcWorld(w);
	FromNode(n, nm);
	Mul(w, nm, out);
	return true;
}

cTaskNodeEffect::cTaskNodeEffect(PosFn fn) : m_Pos(std::move(fn))
{
	SetName("g01::cTaskNodeEffect");
}

// DOL 0x800e0fe4 [audited] (delay 0.0)
void cTaskNodeEffect::Start(uint nSlot, bool bOnetime, bool bDepend, bool bClipping)
{
	float p[3] = {0, 0, 0};
	if (m_Pos) m_Pos(p);
	if (m_Mtx)
	{
		float c[16];
		if (m_Mtx(c)) { p[0] = c[12]; p[1] = c[13]; p[2] = c[14]; }
	}
	m_Hnd.Generate(nSlot, bOnetime, bDepend, bClipping, 0.0f, Vector4(p[0], p[1], p[2], 1.0f));
	m_bStarted = true;
	ApplyMtx();
}

// the node's world matrix as the handle's (0x8008a324 with the handle's own translate 0): the native handle space has x negated (vSetTrans,
// undone by the particle push), so row 0 of the row-major matrix is negated here
void cTaskNodeEffect::ApplyMtx()
{
	float c[16];
	if (!m_Mtx || !m_Mtx(c)) return;
	Matrix4x4 w;
	for (int r = 0; r < 4; r++)
		for (int k = 0; k < 4; k++) w.m[r * 4 + k] = c[k * 4 + r];
	for (int k = 0; k < 4; k++) w.m[k] = -w.m[k];
	m_Hnd.vSetTrans(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	m_Hnd.UpdateMatrix(w);
}

void cTaskNodeEffect::Stop(bool bKill)
{
	if (m_bStarted && m_Hnd.IsValid()) m_Hnd.Detach(bKill);
	m_bStarted = false;
}

bool cTaskNodeEffect::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	float p[3];
	if (m_bStarted && m_Hnd.IsValid())
	{
		if (m_Mtx) ApplyMtx();
		else if (m_Pos && m_Pos(p)) m_Hnd.vSetTrans(Vector4(p[0], p[1], p[2], 1.0f));
	}
	return cTaskKingdom::Execute(pParam);
}

}
