#include "kingdom/game/g03/cTaskConsHaveCookie.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskConsNum.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskConsHaveCookie::cTaskConsHaveCookie()
{
	SetName("cTaskConsHaveCookie");
	Init();
}

cTaskConsHaveCookie::~cTaskConsHaveCookie()
{
	Term();
}

bool cTaskConsHaveCookie::IsUpdate(uint dat)
{
	return m_nCookie != dat;
}

void cTaskConsHaveCookie::Init()
{
	Regist(0u, "BIN_ANT_GAME");
	SetState(0u);
	m_nCookie = 0u;
	ConnectChild(m_pTaskNum = new cTaskConsNum(3u));
	int array[3] = {2, 1, 0};
	CustomKingdomUtil::Assert(3 == ::Constants::ARRAY_SIZE(array), "error");
	for (uint num = 0u; num < 3; num++)
	{
		m_pTaskNum->SetPosition(num, 15, array[num]);
	}
}

void cTaskConsHaveCookie::Term()
{
}

void cTaskConsHaveCookie::Start(uint have)
{
	m_nCookie = have;
	if (nGetState() != 1 && nGetState() != 2)
	{
		SetState(1u);
		SetAnim(0u, 218);
		Visible(0u);
	}
}

void cTaskConsHaveCookie::End()
{
	if (nGetState() != 3 && nGetState() != 0)
	{
		m_pTaskNum->End();
		SetState(3u);
	}
}

void cTaskConsHaveCookie::Update(uint have)
{
	m_nCookie = have;
	if (nGetState() != 1 && nGetState() != 2)
	{
		Start(m_nCookie);
	}
}

bool cTaskConsHaveCookie::Execute(cJetTask* pParam)
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
				SetAnim(0u, 219);
				m_pTaskNum->Start(m_nCookie);
				SetStep(1u);
			}
			break;
		case 1u:
			if (m_pTaskNum->IsAnimEnd())
			{
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
			if (m_pTaskNum->IsAnimEnd())
			{
				SetAnim(0u, 220);
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
