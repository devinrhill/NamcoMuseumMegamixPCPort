#include "kingdom/game/g01/cTaskShootMarker.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"

#include "gfx/Gfx.hpp"
#include "kingdom/NativeScene.hpp"

namespace kingdom::game::g01
{

namespace
{
constexpr int ANM_APPEAR = 0x3d;
constexpr int ANM_LOOP = 0x3e;
constexpr int ANM_OUT = 0x3f;
bool AnimDone(cKingdomAnita* a) { return a->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END); }
}

// DOL 0x800f84d4 [audited]: priority 0x80000001, ANT_SCORE anita 0 hidden, owner +0x250, offset / rot / scale, state 1, inactive
cTaskShootMarker::cTaskShootMarker(const cObject* pOwner, const Vector4* pOffset, const Vector4* pRot, const Vector4* pScale) : m_pOwner(pOwner)
{
	SetName("g01::cTaskShootMarker");
	nDrawPriority(0x80000001u);
	pAnita(0)->RegistData("ANT_SCORE");
	pAnita(0)->SetState(1u, false);
	if (pOffset != nullptr) m_vOffset = *pOffset;
	if (pRot != nullptr) m_vRot = *pRot;
	if (pScale != nullptr) m_vScale = *pScale;   // else ONE (0x8013e968)
	m_nState = 1;
	m_nStep = 0;
	m_b08 = false;
}

// DOL 0x800f88a4 [audited: native OpenGL clip range instead of GX z/w > 0, viewport taken as the 640 x 480 screen]: the owner's +0x20 through S(w/2, -h/2) * proj * view (w, h = the camera's viewport), divided by w; false when off the
// viewport or in front of the near plane (GX z / w > 0); else +0x20 = layout coordinates (origin in the middle, y up).
// Native: the port's 3D pass uses OpenGL clip space (z / w in -1..1, near = -1), so "z / w > 0" (GX, near = 0) reads "z / w < -1 or w <= 0";
// the viewport is the 640 x 480 layout screen the 3D pass is fitted into.
bool cTaskShootMarker::Project()
{
	if (m_pOwner == nullptr) return false;
	float view[16], proj[16];
	if (!kingdom::NativeScene::HasCamera() || !kingdom::NativeScene::GetMatrices(view, proj)) return false;
	const float* p = m_pOwner->vPos();
	float e[4], c[4];
	for (int i = 0; i < 4; i++) e[i] = view[i] * p[0] + view[4 + i] * p[1] + view[8 + i] * p[2] + view[12 + i];
	for (int i = 0; i < 4; i++) c[i] = proj[i] * e[0] + proj[4 + i] * e[1] + proj[8 + i] * e[2] + proj[12 + i] * e[3];
	if (c[3] <= 0.0f || c[2] / c[3] < -1.0f) return false;
	const float w = (float)gfx::SCREEN_W, h = (float)gfx::SCREEN_H;
	const float x = 0.5f * w * (c[0] / c[3] + 1.0f), y = 0.5f * h * (1.0f - c[1] / c[3]);
	if (x < 0.0f || w < x || y < 0.0f || h < y) return false;
	m_v20 = Vector4(x - 0.5f * w, -y + 0.5f * h, c[2] / c[3], 1.0f);
	return true;
}

// DOL 0x800f8c08 [audited] (when active, not paused): state 0 appear -> loop (state 1), state 2 out -> hidden, inactive;
// then the projection places (or hides) the anita: +0x400 = offset + the projected point, +0x410 = rot, +0x420 = scale
bool cTaskShootMarker::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08) return true;
	cKingdomAnita* a = pAnita(0);
	if (m_nState == 0)
	{
		if (m_nStep == 0)
		{
			a->SetAnimNo(ANM_APPEAR, true);
			a->SetState(1u, true);
			m_nStep++;
		}
		else if (m_nStep == 1 && AnimDone(a))
		{
			a->SetAnimNo(ANM_LOOP, true);
			m_nState = 1;
			m_nStep = 0;
		}
	}
	else if (m_nState == 2)
	{
		if (m_nStep == 0)
		{
			a->SetAnimNo(ANM_OUT, true);
			m_nStep++;
		}
		else if (m_nStep == 1 && AnimDone(a))
		{
			a->SetState(1u, false);
			m_b08 = false;
		}
	}
	if (!Project())
	{
		a->SetState(1u, false);
	}
	else
	{
		a->SetState(1u, true);
		a->vTrans() = Vector4(m_vOffset.x + m_v20.x, m_vOffset.y + m_v20.y, m_vOffset.z + m_v20.z, a->vTrans().w);   // PSVECAdd: xyz only
		a->vRot() = m_vRot;
		a->vScale() = m_vScale;
	}
	return cTaskAnita::Execute(pParam);
}

// DOL 0x800f8e6c [audited]: only while active (an ortho over the viewport, then the anitas)
void cTaskShootMarker::Draw(int nStatus, cJetTask* pParam)
{
	if (m_b08) cTaskAnita::Draw(nStatus, pParam);
}

}
