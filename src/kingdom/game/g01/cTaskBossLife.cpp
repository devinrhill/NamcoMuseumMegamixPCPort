#include "kingdom/game/g01/cTaskBossLife.hpp"

#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskBoss.hpp"

namespace kingdom::game::g01
{

namespace
{
constexpr float BAR_FRAMES = 1000.0f;   // FLOAT_8047b430
constexpr float DRAIN = 2.0f;           // FLOAT_8047b448
constexpr float LOW = 0.2f;             // FLOAT_8047b44c
bool AnimDone(cKingdomAnita* a) { return a->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END); }
}

// DOL 0x801ba1f4 (cObject + cTaskAnita, vtable 0x80438db0) + 0x800cff48: priority 0x80000008, the boss, the life (+0x18) as maximum / shown /
// target, the three ANT_BOSS anitas (frame: anim 1, hidden; bar: anim 4, hidden; shake: anim 3 at its end, AnimUpdate(0)), 0 and 1 at +0x220
cTaskBossLife::cTaskBossLife(cTaskBoss* pBoss) : m_pBoss(pBoss)
{
	SetName("g01::cTaskBossLife");
	nDrawPriority(0x80000008u);
	m_b08 = false;
	m_fMax = m_fShown = m_fTarget = pBoss != nullptr ? (float)pBoss->nLife() : 1.0f;
	if (m_fMax <= 0.0f) m_fMax = 1.0f;
	for (uint i = 0; i < 3; i++) pAnita(i)->RegistData("ANT_BOSS");
	pAnita(0)->SetAnimNo(FrameAnm(1), true);
	pAnita(0)->SetState(1u, false);
	pAnita(1)->SetAnimNo(BarAnm(1), true);
	pAnita(1)->SetState(1u, false);
	pAnita(2)->SetAnimNo(FrameAnm(2), true);
	pAnita(2)->SetFrame(pAnita(2)->GetFrameNum(-1));   // 0x80141bd4(anita, -1): parked at its end (no shake until a change)
	pAnita(2)->AnimUpdate(0.0f);
	pAnita(2)->SetState(1u, false);
	// +0x220 = layer 0 of anita 0's pattern 0 (0x80014408(anita, +0x220, 0, 0)): the gauge's place at the left edge
	Vector4 base(0.0f, 0.0f, 0.0f, 1.0f);
	pAnita(0)->GetLayerPos(base, 0, 0);
	m_vBase[0] = base.x;
	m_vBase[1] = base.y;
	m_vBase[2] = base.z;
	for (uint i = 0; i < 2; i++) pAnita(i)->vTrans() = Vector4(m_vBase[0], m_vBase[1], m_vBase[2], 1.0f);
}

// DOL 0x800d02ac
int cTaskBossLife::FrameAnm(int n)
{
	switch (n)
	{
	case 0: return 0;
	case 1: return 1;
	case 4: return 2;
	case 2: return 3;
	case 3: return 5;
	default: return -1;
	}
}

// DOL 0x800d0308
int cTaskBossLife::BarAnm(int n)
{
	return n == 0 ? 6 : n == 1 ? 4 : -1;
}

// DOL 0x800d0228: the shake anitas (2) plays anim 3 and has not ended
bool cTaskBossLife::IsShaking()
{
	cKingdomAnita* a = pAnita(2);
	return a->GetAnimNo() == FrameAnm(2) && !a->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END);
}

// DOL 0x800d0438
void cTaskBossLife::AppearStep()
{
	if (m_nStep == 0)
	{
		pAnita(0)->SetState(1u, true);
		pAnita(1)->SetState(1u, true);
		pAnita(0)->SetAnimNo(FrameAnm(0), true);
		pAnita(1)->SetAnimNo(BarAnm(0), true);
		m_nStep++;
	}
	else if (m_nStep == 1 && AnimDone(pAnita(0)))
	{
		m_nState = 1;
		m_nStep = 0;
		pAnita(0)->SetAnimNo(FrameAnm(1), true);
		pAnita(0)->SetState(cJetAnita::JET_ANITA_STATE_ANIMATE, false);
		pAnita(1)->SetAnimNo(BarAnm(1), true);
		pAnita(1)->SetState(cJetAnita::JET_ANITA_STATE_ANIMATE, false);
	}
}

// DOL 0x800d05f8
void cTaskBossLife::GaugeStep()
{
	m_fTarget = m_pBoss != nullptr ? (float)m_pBoss->nLife() : 0.0f;
	if (m_fTarget != m_fShown)
	{
		m_fShown -= DRAIN;
		if (m_fShown < m_fTarget) m_fShown = m_fTarget;
		m_fRatio = m_fShown / m_fMax;
		pAnita(1)->SetFrame(-(BAR_FRAMES * m_fRatio - BAR_FRAMES));
		pAnita(1)->AnimUpdate(0.0f);   // 0x80141c44(0.0)
		if (!IsShaking()) pAnita(2)->SetAnimNo(FrameAnm(2), true);
	}
	// the frame: normal above 0.2 of the life, low below (reset 0: the current frame is kept)
	if (!IsShaking()) pAnita(0)->SetAnimNo(LOW < m_fRatio ? FrameAnm(1) : FrameAnm(4), false);
	Shake();
}

// DOL 0x800d07c4
void cTaskBossLife::OutStep()
{
	if (m_nStep == 0)
	{
		pAnita(1)->SetState(1u, false);
		pAnita(0)->SetState(cJetAnita::JET_ANITA_STATE_ANIMATE, true);
		pAnita(0)->SetAnimNo(FrameAnm(3), true);
		m_nStep++;
	}
	else if (m_nStep == 1 && AnimDone(pAnita(0)))
	{
		pAnita(0)->SetState(1u, false);
		m_b08 = false;
	}
}

// DOL 0x800d0334: while the shake plays, anitas 0 and 1 sit at base + the shake's layer 0 position
void cTaskBossLife::Shake()
{
	Vector4 ofs(0.0f, 0.0f, 0.0f, 0.0f);
	if (IsShaking()) pAnita(2)->GetLayerPos(ofs, 0, -1);   // 0x80014408(anita, out, 0, -1)
	for (uint i = 0; i < 2; i++) pAnita(i)->vTrans() = Vector4(m_vBase[0] + ofs.x, m_vBase[1] + ofs.y, m_vBase[2] + ofs.z, 1.0f);
}

// DOL 0x800d08bc (active, not paused); hidden while inactive (Draw 0x800d0970 draws only then)
bool cTaskBossLife::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (m_b08)
	{
		if (m_nState == 0) AppearStep();
		else if (m_nState == 1) GaugeStep();
		else if (m_nState == 2) OutStep();
	}
	return cTaskAnita::Execute(pParam);
}

}
