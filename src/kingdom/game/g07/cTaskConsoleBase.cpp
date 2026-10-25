#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

int cTaskConsoleBase::ANIM_NOT_USED = -1;

cTaskConsoleBase::cTaskConsoleBase()
{
	Create();
	Init();
}

cTaskConsoleBase::~cTaskConsoleBase()
{
	Free();
}

void cTaskConsoleBase::Free()
{
	Term();
}

Vector4& cTaskConsoleBase::vTrans()
{
	return m_vTrans;
}

float& cTaskConsoleBase::X()
{
	return m_vTrans.x;
}

float& cTaskConsoleBase::Y()
{
	return m_vTrans.y;
}

int& cTaskConsoleBase::nApearAnm()
{
	return m_nApearAnim;
}

int& cTaskConsoleBase::nLoopAnm()
{
	return m_nLoopAnim;
}

int& cTaskConsoleBase::nEndAnm()
{
	return m_nEndAnim;
}

int cTaskConsoleBase::nStep()
{
	return m_nStep;
}

void cTaskConsoleBase::StepChange(int set)
{
	m_nStep = set;
	m_bStepChange = true;
}

bool cTaskConsoleBase::InitStep()
{
	return m_nStep != m_nStepOld;
}

bool cTaskConsoleBase::UpdateStep()
{
	return m_nStep == m_nStepOld;
}

bool cTaskConsoleBase::EndStep()
{
	return m_bStepChange;
}

bool cTaskConsoleBase::IsStateChange()
{
	return m_nStateOld != m_nState;
}

void cTaskConsoleBase::SetAnm(uint set, int nAnmNo)
{
	pAnita(set)->SetAnimNo(nAnmNo);
}

bool cTaskConsoleBase::SetAnmDiffCurrent(uint set, int nAnmNo)
{
	if (!IsAnmNoSame(set, nAnmNo))
	{
		SetAnm(set, nAnmNo);
		return true;
	}
	return false;
}

bool cTaskConsoleBase::IsAnmEnd(uint set)
{
	return (pAnita(set)->GetState() & 0x10000) != 0;
}

void cTaskConsoleBase::AnimationLoopFromApear(uint set)
{
	if (IsAnmNoSame(set, nApearAnm()) && IsAnmEnd(set))
	{
		SetAnm(set, nLoopAnm());
	}
}

void cTaskConsoleBase::AnimationLoopFromApear(uint set, int nLoopAnimation)
{
	if (IsAnmEnd(set))
	{
		SetAnmDiffCurrent(set, nLoopAnimation);
	}
}

void cTaskConsoleBase::Init()
{
	m_nStep = 0;
	m_nStepOld = m_nStep - 1;
	m_nApearAnim = (m_nLoopAnim = (m_nEndAnim = ANIM_NOT_USED));
	::Constants::jetGeoVecUnit(m_vTrans);
	m_nState = (m_nStateOld = -1);
	SetName("cTaskConsoleBase");
}

void cTaskConsoleBase::Term()
{
}

void cTaskConsoleBase::Create()
{
	nDrawPriority(++KingdomGlobal::g_pMotosGame->pTaskConsoleMan()->nPriority());
}

void cTaskConsoleBase::Visible(uint set)
{
	CustomKingdomUtil::Assert(pAnita(set) != nullptr, "nullpo");
	pAnita(set)->SetState(3u);
}

void cTaskConsoleBase::Invisible(uint set)
{
	CustomKingdomUtil::Assert(pAnita(set) != nullptr, "nullpo");
	pAnita(set)->SetState(3u, false);
}

bool cTaskConsoleBase::IsAnmNoSame(uint set, int nAnmNo)
{
	CustomKingdomUtil::Assert(pAnita(set) != nullptr, "nullpo");
	return pAnita(set)->GetAnimNo() == nAnmNo;
}

bool cTaskConsoleBase::Execute(cJetTask* pParam)
{
	m_nStateOld = m_nState;
	m_nState = KingdomGlobal::g_pMotosGame->nStep(0);
	m_nStepOld = m_nStep;
	m_bStepChange = false;
	if (KingdomGlobal::g_pMotosGame->IsPause())
	{
		return true;
	}
	return cTaskAnita::Execute(pParam);
}

bool cTaskConsoleBase::BaseExecute(cJetTask* pParam)
{
	return cTaskAnita::Execute(pParam);
}

// C# base.Draw (the decompiler printed it as a virtual Draw call: every console Draw -> BaseDraw -> Draw looped forever)
void cTaskConsoleBase::BaseDraw(int nStatus, cJetTask* pParam)
{
	cTaskAnita::Draw(nStatus, pParam);
}

}
