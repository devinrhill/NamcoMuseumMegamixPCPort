#include "kingdom/game/g03/cTaskConsCookieWarning.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

cTaskConsCookieWarning::cTaskConsCookieWarning()
{
	SetName("cTaskConsCookieWarning");
	Init();
}

void cTaskConsCookieWarning::Init()
{
	m_nWaitCnt = 0u;
	Regist(0u, "BIN_ANT_GAME");
	SetState(0u);
}

void cTaskConsCookieWarning::Term()
{
}

void cTaskConsCookieWarning::Start()
{
	StartWarning();
}

void cTaskConsCookieWarning::StartWarning()
{
	m_nWaitCnt = 0u;
	if (nGetState() != 1 && nGetState() != 2)
	{
		SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE029_ESA_TARINAI);
		SetState(1u);
		SetAnim(0u, 69);
		Visible(0u);
	}
}

void cTaskConsCookieWarning::StartGate()
{
	m_nWaitCnt = 0u;
	if (nGetState() != 4 && nGetState() != 5)
	{
		SetState(4u);
		SetAnim(0u, 66);
		Visible(0u);
	}
}

void cTaskConsCookieWarning::End()
{
	switch (nGetState())
	{
	case 1u:
	case 2u:
		EndWarning();
		break;
	case 4u:
	case 5u:
		EndGate();
		break;
	}
}

void cTaskConsCookieWarning::EndWarning()
{
	SetAnim(0u, 71);
	SetState(3u);
}

void cTaskConsCookieWarning::EndGate()
{
	SetAnim(0u, 68);
	SetState(6u);
}

bool cTaskConsCookieWarning::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	switch (nGetState())
	{
	case 1u:
		if (IsAnimEnd(0u))
		{
			SetAnim(0u, 70);
			SetState(2u);
			m_nWaitCnt = 0u;
		}
		break;
	case 2u:
		if (60 <= ++m_nWaitCnt)
		{
			End();
		}
		break;
	case 3u:
		if (IsAnimEnd(0u))
		{
			Invisible(0u);
			SetState(0u);
		}
		break;
	case 4u:
		if (IsAnimEnd(0u))
		{
			SetAnim(0u, 67);
			SetState(5u);
			m_nWaitCnt = 0u;
		}
		break;
	case 5u:
		if (60 <= ++m_nWaitCnt)
		{
			End();
		}
		break;
	case 6u:
		if (IsAnimEnd(0u))
		{
			Invisible(0u);
			SetState(0u);
		}
		break;
	}
	return cTaskConsoleBase::Execute(pParam);
}

}
