#include "kingdom/game/g03/cTaskConsPCookie.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskConsPCookie::cTaskConsPCookie()
{
	SetName("cTaskConsPCookie");
	Init();
}

cTaskConsPCookie::~cTaskConsPCookie()
{
	Term();
}

void cTaskConsPCookie::Init()
{
	m_nCookie = 0;
	m_nCookieOld = 0;
	Regist(0u, "BIN_ANT_GAME");
	SetState(0u);
}

void cTaskConsPCookie::Term()
{
}

void cTaskConsPCookie::Start()
{
	m_nCookie = 1;
	m_nCookieOld = 1;
	SetState(1u);
	SetAnim(0u, 84);
	Visible(0u);
}

void cTaskConsPCookie::End()
{
	if (nGetState() != 4 && nGetState() != 0)
	{
		SetState(4u);
		SetAnim(0u, nGetAnimCookieFlameOut(m_nCookieOld));
	}
}

bool cTaskConsPCookie::Execute(cJetTask* pParam)
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
			ChangeAnim(0u, nGetAnimCookieStay(m_nCookieOld));
			SetState(2u);
		}
		break;
	case 2u:
		Update();
		break;
	case 3u:
		if (IsAnimEnd(0u))
		{
			ChangeAnim(0u, nGetAnimCookieStay(m_nCookieOld));
			SetState(2u);
		}
		break;
	case 4u:
		if (IsAnimEnd(0u))
		{
			Invisible(0u);
			SetState(0u);
		}
		break;
	}
	return cTaskConsoleBase::Execute(pParam);
}

void cTaskConsPCookie::Update()
{
	if (m_nCookie != 0 && m_nCookieOld < m_nCookie)
	{
		UpCookie();
	}
}

void cTaskConsPCookie::UpCookie()
{
	if (m_nCookieOld < 3)
	{
		m_nCookieOld++;
	}
	ChangeAnim(0u, nGetAnimCookieUP(m_nCookieOld));
}

void cTaskConsPCookie::Update(int Cookie, uint HP)
{
	if (m_nCookie < Cookie && Cookie == 1)
	{
		Start();
	}
	if (Cookie < m_nCookie)
	{
		End();
	}
	if (HP == 0)
	{
		End();
	}
	m_nCookie = Cookie;
}

int cTaskConsPCookie::nGetAnimCookieStay(int Cookie)
{
	int array[4] = {0, 85, 88, 91};
	CustomKingdomUtil::Assert(Cookie < ::Constants::ARRAY_SIZE(array), "over");
	return array[Cookie];
}

int cTaskConsPCookie::nGetAnimCookieUP(int Cookie)
{
	int array[4] = {0, -1, 88, 91};
	CustomKingdomUtil::Assert(Cookie < ::Constants::ARRAY_SIZE(array), "over");
	return array[Cookie];
}

int cTaskConsPCookie::nGetAnimCookieFlameOut(int Cookie)
{
	int array[4] = {0, 86, 89, 92};
	CustomKingdomUtil::Assert(Cookie < ::Constants::ARRAY_SIZE(array), "over");
	return array[Cookie];
}

}
