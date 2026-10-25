#include "kingdom/game/g01/cTaskConsoleReadyString.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"

namespace kingdom::game::g01
{

// DOL 0x800d7908 (priority 0x80000002) + 0x800d7924: ANT_SCORE anita 0 at rate 60 (bit 0x20000 cleared, +0x14 = FLOAT_8047b4b8), anim 0 at
// frame 0, hidden; the console manager starts it at once (0x800eb3b0: state 0, active)
cTaskConsoleReadyString::cTaskConsoleReadyString()
{
	SetName("g01::cTaskConsoleReadyString");
	nDrawPriority(0x80000002u);
	pAnita(0)->RegistData("ANT_SCORE");
	pAnita(0)->SetFrameRate(60.0f);
	pAnita(0)->SetAnimNo(0, true, 0.0f);
	pAnita(0)->SetState(1u, false);
	m_nState = 0;
	m_nStep = 0;
	m_b08 = true;
}

// DOL 0x800d79ec: sub-step 0 anim 0 from the start, visible, shown (+9); sub-step 1 once hidden or at the anim end: hidden, done
bool cTaskConsoleReadyString::ReadyStep()
{
	cKingdomAnita* a = pAnita(0);
	if (m_nSub == 0)
	{
		a->SetAnimNo(0, true, 0.0f);   // FLOAT_8047b4bc
		a->SetState(1u, true);
		m_b09 = true;
		m_nSub++;
	}
	else if (m_nSub == 1 && (!a->IsState(1u) || a->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END)))
	{
		a->SetState(1u, false);
		return true;
	}
	return false;
}

// DOL 0x800eb400 (active, not paused): state 1 runs the ready step, then state 2 (the other states' slots are empty for this console:
// 0x800d7ad4 / 0x800d7ad8 / 0x800d549c); the anitas update (FUN_80014d58)
bool cTaskConsoleReadyString::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (!m_b08) return true;
	if (m_nState == 1 && m_nStep == 0 && ReadyStep())
	{
		m_nState = 2;
		m_nStep++;
	}
	return cTaskAnita::Execute(pParam);
}

// DOL 0x800eb548: only while active (+8) and shown (+9)
void cTaskConsoleReadyString::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08 || !m_b09) return;
	cTaskAnita::Draw(nStatus, pParam);   // FUN_80014df8
}

}
