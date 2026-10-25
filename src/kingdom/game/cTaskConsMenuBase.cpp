#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "gfx/Gfx.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game
{

cTaskConsMenuManBase*& cTaskConsMenuBase::pTaskConsMenuMan()
{
	return m_pTaskConsMenuMan;
}

int& cTaskConsMenuBase::nId()
{
	return m_nId;
}

int& cTaskConsMenuBase::nState()
{
	return m_nState;
}

int& cTaskConsMenuBase::nStep()
{
	return m_nStep;
}

int cTaskConsMenuBase::nGetId()
{
	return nId();
}

// DOL 0x801456e4
void cTaskConsMenuBase::RestartAnim()
{
	pAnita(0u)->SetAnimNo(pAnita(0u)->GetAnimNo(), true, 0.0f);
}

void cTaskConsMenuBase::WaitState()
{
	nState() = 1;
	nStep() = 0;
}

bool cTaskConsMenuBase::IsWaitState()
{
	return nState() == 1;
}

void cTaskConsMenuBase::AppearState()
{
	nState() = 2;
	nStep() = 0;
}

void cTaskConsMenuBase::AppearEndState()
{
	nState() = 3;
	nStep() = 0;
}

bool cTaskConsMenuBase::IsAppearEndState()
{
	return nState() == 3;
}

void cTaskConsMenuBase::PointState()
{
	nState() = 4;
	nStep() = 0;
}

void cTaskConsMenuBase::DecideState()
{
	nState() = 5;
	nStep() = 0;
}

void cTaskConsMenuBase::DecideEndState()
{
	nState() = 6;
	nStep() = 0;
}

bool cTaskConsMenuBase::IsDecideEndState()
{
	return nState() == 6;
}

void cTaskConsMenuBase::DeleteState()
{
	nState() = 7;
	nStep() = 0;
}

void cTaskConsMenuBase::DeleteEndState()
{
	nState() = 8;
	nStep() = 0;
}

bool cTaskConsMenuBase::IsDeleteEndState()
{
	return nState() == 8;
}

cTaskConsMenuBase::cTaskConsMenuBase(cTaskConsMenuManBase* const& pConsMan)
{
	pTaskConsMenuMan() = pConsMan;
	nId() = -1;
	nState() = 0;
	nStep() = 0;
}

cTaskConsMenuBase::~cTaskConsMenuBase()
{
}

bool cTaskConsMenuBase::IsDpdHit(float x, float y)
{
	return false;
}

bool cTaskConsMenuBase::DpdHitPat(float x, float y, int pat)
{
	const float px = (float)(int)(x * (float)gfx::SCREEN_W * 0.5f);
	const float py = (float)(int)(-y * (float)gfx::SCREEN_H * 0.5f);
	return pAnita(0u)->IsHit(Vector4(px, py, 0.0f, 1.0f), pat);
}

int cTaskConsMenuBase::IsDpdHitId(float x, float y)
{
	if (IsDpdHit(x, y))
	{
		return nId();
	}
	return -1;
}

void cTaskConsMenuBase::WaitExec()
{
}

bool cTaskConsMenuBase::AppearExec()
{
	return true;
}

void cTaskConsMenuBase::PointExec()
{
}

bool cTaskConsMenuBase::DecideExec()
{
	return true;
}

bool cTaskConsMenuBase::DeleteExec()
{
	return true;
}

void cTaskConsMenuBase::ResetAnm()
{
	CustomKingdomUtil::Assert(pAnita(0u) != nullptr);
	pAnita(0u)->SetAnimNo(pAnita(0u)->GetAnimNo());
}

bool cTaskConsMenuBase::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::KingSys->pAppMain()->IsPause())
	{
		return true;
	}
	cTaskAnita::Execute(pParam);
	switch (nState())
	{
	case 1:
		WaitExec();
		break;
	case 2:
		if (AppearExec())
		{
			AppearEndState();
		}
		break;
	case 4:
		PointExec();
		break;
	case 5:
		if (DecideExec())
		{
			DecideEndState();
		}
		break;
	case 7:
		if (DeleteExec())
		{
			DeleteEndState();
		}
		break;
	default:
		CustomKingdomUtil::Assert("STATE ERROR");
		break;
	case 0:
	case 3:
	case 6:
	case 8:
		break;
	}
	return true;
}

}
