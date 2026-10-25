#include "kingdom/game/g01/cTaskConsoleLife.hpp"

#include "KingdomGlobal.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"

namespace kingdom::game::g01
{

namespace
{
const int LIFE_ANM[7] = {1, 8, 0xf, 0x16, 0x1d, 0x24, 0x2b};   // DOL 0x80371eb0
bool AnimDone(cKingdomAnita* a) { return !a->IsState(1u) || a->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END); }
}

// DOL 0x800d4b5c (life 6, priority 0x80000007), 0x800d4c34 (ANT_LIFE, wait anim, hidden), 0x800d4d38 (the life of the stage / multi work)
cTaskConsoleLife::cTaskConsoleLife()
{
	SetName("g01::cTaskConsoleLife");
	nDrawPriority(0x80000007u);
	m_b08 = true;   // the console manager starts every console at once (0x800eb3b0: state 0, +8 = 1)
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	m_nLife = (!sys->IsMulti() ? sys->pStageWork()->n0c : sys->pMultiWork()->n[1]) - 1;
	if (m_nLife < 0) m_nLife = 0;
	if (m_nLife > 6) m_nLife = 6;
	pAnita(0)->RegistData("ANT_LIFE");
	pAnita(0)->SetAnimNo(Anm(0), true);
	pAnita(0)->SetState(1u, false);
}

// DOL 0x800d4b8c
int cTaskConsoleLife::Anm(int kind) const
{
	static const int OFS[5] = {0, -1, 1, 2, 3};
	return LIFE_ANM[m_nLife] + OFS[(unsigned)kind < 5 ? kind : 0];
}

// DOL 0x800d4d9c
void cTaskConsoleLife::AppearExec()
{
	if (m_nAppearStep == 0)
	{
		pAnita(0)->SetAnimNo(0x31, true);
		pAnita(0)->SetState(1u, true);
		m_nAppearStep++;
	}
	else if (m_nAppearStep == 1 && AnimDone(pAnita(0)))
	{
		pAnita(0)->SetAnimNo(Anm(0), true);
		m_bAppear = false;
	}
}

// DOL 0x800eb6b0 (true when the gauge is idle) + the caller's write of state 2
bool cTaskConsoleLife::Damage()
{
	if (m_nState2 != 0) return false;
	m_nState2 = 2;
	m_nStep2 = 0;
	return true;
}

// DOL 0x800eb6b0 + the caller's write of state 1 (cTaskPacman::HitCallback 0x800f4f20, kind 4)
bool cTaskConsoleLife::Heal()
{
	if (m_nState2 != 0) return false;
	m_nState2 = 1;
	m_nStep2 = 0;
	return true;
}

// DOL 0x800d4ea8
void cTaskConsoleLife::StateExec()
{
	cKingdomAnita* a = pAnita(0);
	if (m_nState2 == 1)
	{
		if (m_nStep2 == 0)
		{
			if (m_nLife == 6)
			{
				m_nState2 = 0;
				return;
			}
			m_nLife++;
			a->SetAnimNo(Anm(1), true);
			m_nStep2++;
		}
		else if (m_nStep2 == 1 && a->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END))
		{
			a->SetAnimNo(Anm(0), true);
			m_nState2 = 0;
			m_nStep2 = 0;
		}
	}
	else if (m_nState2 == 2)
	{
		if (m_nStep2 == 0)
		{
			a->SetAnimNo(Anm(2), true);
			m_nStep2++;
		}
		else if (m_nStep2 == 1 && AnimDone(a))
		{
			a->SetAnimNo(Anm(3), true);
			m_nStep2++;
		}
		else if (m_nStep2 == 2 && AnimDone(a))
		{
			if (m_nLife == 0)
			{
				a->SetAnimNo(0x33, true);
				m_bMiss = true;
				m_nStep2++;
			}
			else
			{
				m_nLife--;
				a->SetAnimNo(Anm(0), true);
				m_nState2 = 0;
				m_nStep2 = 0;
			}
		}
		else if (m_nStep2 == 3 && AnimDone(a))
		{
			m_nState2 = 0;
			m_nStep2 = 0;
		}
	}
}

// DOL 0x800d54a4 -> 0x800d54e4: the life bonus +0x228 = (life + 1) * 1000; then console state 4
void cTaskConsoleLife::StartResult()
{
	m_nBonus = (m_nLife + 1) * 1000;
	m_bResult = true;
}

bool cTaskConsoleLife::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (m_bResult)   // console base 0x800eb400 state 4 -> 0x800d51b8: the gauge's out anim (Anm(4)), done at once (state 5)
	{
		pAnita(0)->SetAnimNo(Anm(4), false);
		m_bResult = false;
	}
	if (m_bAppear) AppearExec();
	StateExec();
	return cTaskAnita::Execute(pParam);
}


// DOL 0x800eb548 (cTaskGameConsoleBase): only while active (+8) and shown (+9), the anitas over a 640 x 480 ortho
void cTaskConsoleLife::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08 || !m_b09) return;
	cTaskAnita::Draw(nStatus, pParam);   // FUN_80014df8
}

}
