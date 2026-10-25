#include "kingdom/game/g01/cTaskBomb.hpp"

#include <algorithm>

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskCamera.hpp"

namespace kingdom::game::g01
{

// DOL 0x800c9674: priority 0x10000, kind 0xc, the parameters from BombParam.csv (DAT_8047b380; rows AREA / INTERVAL / ENDURANCE, column 1),
// radius / counters 0 (FLOAT_8047b398), sphere (0, 0, 0, 1). The pool (0x800e6b9c) makes it inactive and registers it for collision.
cTaskBomb::cTaskBomb()
{
	SetName("g01::cTaskBomb");
	nDrawPriority(0x10000u);
	m_nKind = 0xc;
	m_b08 = false;
	kingdom::cKingdomCSV csv;
	if (csv.RegistData("BombParam.csv"))
	{
		csv.F32(csv.SearchRow(0, "AREA", 0, 0), 1, m_Param.fArea);             // DAT_8047b38c
		csv.F32(csv.SearchRow(0, "INTERVAL", 0, 0), 1, m_Param.fInterval);     // DAT_8047b390
		csv.F32(csv.SearchRow(0, "ENDURANCE", 0, 0), 1, m_Param.fEndurance);   // DAT_8047b394
	}
}

// position = camera aim point (pTaskCamera +0x220) + offset; the effect follows (0x8008a49c); the sphere's centre = the effect's translate;
// the radius grows by AREA / INTERVAL per frame up to AREA (the first step leaves it 0)
void cTaskBomb::Follow()
{
	const cTaskCamera* cam = KingdomGlobal::GalagaSys->pTaskCamera();
	const float* aim = cam != nullptr ? cam->vAim() : nullptr;
	for (int i = 0; i < 3; i++) (&m_v20.x)[i] = m_vOffset[i] + (aim != nullptr ? aim[i] : 0.0f);
	m_Hnd.vSetTrans(Vector4(m_v20.x, m_v20.y, m_v20.z, 1.0f));
	for (int i = 0; i < 3; i++) m_vCenter[i] = (&m_v20.x)[i];
	if (m_Param.fArea <= m_fRadius)
	{
		m_fRadius = m_Param.fArea;
	}
	else
	{
		m_fRadius = m_fGrow * (m_Param.fArea / m_Param.fInterval);
		m_fGrow += 1.0f;   // FLOAT_8047b39c
	}
	m_fSphereR = m_fRadius;
}

// DOL 0x800c9a78: counters 0, owner, offset, effect 0x2a (one-time, dependent, clipped; FUN_80021a9c), the first step, collidable, active
void cTaskBomb::Start(cTaskConsolePlayer* pOwner, const float vOffset[3])
{
	m_fRadius = m_fGrow = m_fAge = 0.0f;
	m_pOwner = pOwner;
	for (int i = 0; i < 3; i++) m_vOffset[i] = vOffset[i];
	m_Hnd.Generate(0x2au, true, true, true, 0.0f, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	Follow();
	m_b14 = true;   // +0x1d4
	m_b08 = true;   // +0x1c8
}

// DOL 0x800c9bc0 (active, not paused): follow and grow; past ENDURANCE frames (+0x238 > param +8) the owner is cleared and the bomb is free
bool cTaskBomb::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	(void)pParam;
	if (!m_b08) return true;
	Follow();
	if (m_Param.fEndurance < m_fAge)
	{
		m_pOwner = nullptr;
		m_b14 = false;
		m_b08 = false;
	}
	m_fAge += 1.0f;
	return true;
}

// DOL 0x800c9cf4: an enemy (kind 0xb) whose world box (+0x68 / +0x74) the sphere touches (nw4r::math::IntersectionSphereAABB)
bool cTaskBomb::HitCheck(cObject* other)
{
	if (other->nKind() != 0xb) return false;
	const cObjectAABB* box = dynamic_cast<const cObjectAABB*>(other);
	if (box == nullptr) return false;
	const float* mn = box->vWorldMin();
	const float* mx = box->vWorldMax();
	float d2 = 0.0f;
	for (int i = 0; i < 3; i++)
	{
		const float c = std::clamp(m_vCenter[i], mn[i], mx[i]);
		d2 += (m_vCenter[i] - c) * (m_vCenter[i] - c);
	}
	return d2 <= m_fSphereR * m_fSphereR;
}

}
