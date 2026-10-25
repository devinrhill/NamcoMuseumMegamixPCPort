#include "kingdom/game/g03/cTaskConsCookie.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskConsNum.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

cTaskConsCookie::cTaskConsCookie()
{
	SetName("cTaskConsCookie");
	Init();
}

cTaskConsCookie::~cTaskConsCookie()
{
	Term();
}

bool cTaskConsCookie::IsUpdate(uint dat)
{
	return m_nCookie != dat;
}

void cTaskConsCookie::Init()
{
	Regist(0u, "BIN_ANT_GAME");
	Regist(1u, "BIN_ANT_GAME", 2, 3);
	SetState(0u);
	m_nCookieMax = 0u;
	m_nCookie = 0u;
	m_bComplete = false;
	ConnectChild(m_pTaskNumMax = new cTaskConsNum(3u));
	ConnectChild(m_pTaskNum = new cTaskConsNum(3u));
	int array[3] = {6, 5, 4};
	CustomKingdomUtil::Assert(3 == ::Constants::ARRAY_SIZE(array), "error");
	for (uint num = 0u; num < 3; num++)
	{
		m_pTaskNumMax->SetPosition(num, 2, array[num]);
	}
	int array2[3] = {2, 1, 0};
	CustomKingdomUtil::Assert(3 == ::Constants::ARRAY_SIZE(array2), "error");
	for (uint num2 = 0u; num2 < 3; num2++)
	{
		m_pTaskNum->SetPosition(num2, 2, array2[num2]);
	}
}

void cTaskConsCookie::Term()
{
}

void cTaskConsCookie::Start(uint max, uint now)
{
	if (max == now)
	{
		m_bComplete = true;
	}
	m_nCookieMax = max;
	m_nCookie = now;
	SetState(1u);
	if (IsComplete())
	{
		SetAnim(0u, 188);
	}
	else
	{
		SetAnim(0u, 42);
	}
	Visible(0u);
}

void cTaskConsCookie::End()
{
	if (nGetState() != 3 && nGetState() != 0)
	{
		m_pTaskNumMax->End();
		m_pTaskNum->End();
		SetAnim(1u, 38);
		SetState(3u);
	}
}

void cTaskConsCookie::Update(uint max, uint now)
{
	m_nCookieMax = max;
	m_nCookie = now;
	if (IsComplete() && !m_bComplete)
	{
		if (nGetState() != 2)
		{
			Start(max, m_nCookie);
		}
		else
		{
			ChangeAnim(0u, 189);
			SetState(2u);
		}
		SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE06_HISCORE);
		m_bComplete = true;
	}
	else if (nGetState() != 1 && nGetState() != 2)
	{
		Start(max, m_nCookie);
	}
}

bool cTaskConsCookie::IsComplete()
{
	if (m_nCookieMax == m_nCookie)
	{
		return true;
	}
	return false;
}

bool cTaskConsCookie::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	switch (nGetState())
	{
	case 1u:
		switch (nGetStep())
		{
		case 0u:
			if (IsAnimEnd(0u))
			{
				if (IsComplete())
				{
					SetAnim(0u, 189);
				}
				else
				{
					SetAnim(0u, 43);
				}
				SetAnim(1u, 36);
				Visible(1u);
				m_pTaskNumMax->Start(m_nCookieMax);
				m_pTaskNum->Start(m_nCookie);
				SetStep(1u);
			}
			break;
		case 1u:
			if (IsAnimEnd(1u))
			{
				SetAnim(1u, 37);
				SetState(2u);
			}
			break;
		}
		break;
	case 2u:
		m_pTaskNum->Set(m_nCookie);
		break;
	case 3u:
		switch (nGetStep())
		{
		case 0u:
			if (IsAnimEnd(1u))
			{
				Invisible(1u);
				if (IsComplete())
				{
					SetAnim(0u, 190);
				}
				else
				{
					SetAnim(0u, 44);
				}
				SetStep(1u);
			}
			break;
		case 1u:
			if (IsAnimEnd(0u))
			{
				Invisible(0u);
				SetState(0u);
			}
			break;
		}
		break;
	}
	return cTaskConsoleBase::Execute(pParam);
}

}
