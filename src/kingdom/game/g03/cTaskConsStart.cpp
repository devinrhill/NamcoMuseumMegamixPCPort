#include "kingdom/game/g03/cTaskConsStart.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"

namespace kingdom::game::g03
{

cTaskConsStart::cTaskConsStart()
{
	SetName("cTaskConsStart");
	Init();
}

cTaskConsStart::~cTaskConsStart()
{
	Term();
}

void cTaskConsStart::Init()
{
	Regist(0u, "BIN_ANT_GAME");
	SetState(0u);
	m_nWaitCnt = 0u;
}

void cTaskConsStart::Term()
{
}

void cTaskConsStart::Start()
{
	SetState(1u);
	SetAnim(0u, 72);
	Visible(0u);
}

void cTaskConsStart::End()
{
	SetAnim(0u, 74);
	SetState(3u);
}

bool cTaskConsStart::Execute(cJetTask* pParam)
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
			SetAnim(0u, 73);
			SetState(2u);
			m_nWaitCnt = 0u;
		}
		break;
	case 2u:
		if (m_nWaitCnt < 60)
		{
			m_nWaitCnt++;
		}
		if ((60 == m_nWaitCnt) & IsAnimEnd(0u))
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
	}
	return cTaskConsoleBase::Execute(pParam);
}

}
