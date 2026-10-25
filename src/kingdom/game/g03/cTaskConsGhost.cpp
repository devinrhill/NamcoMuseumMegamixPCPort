#include "kingdom/game/g03/cTaskConsGhost.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskConsNum.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskConsGhost::cTaskConsGhost()
{
	SetName("cTaskConsGhost");
	Init();
}

cTaskConsGhost::~cTaskConsGhost()
{
	Term();
}

bool cTaskConsGhost::IsUpdate(uint dat)
{
	return m_nGhost != dat;
}

void cTaskConsGhost::Init()
{
	Regist(0u, "BIN_ANT_GAME");
	SetState(0u);
	m_nGhost = 0u;
	ConnectChild(m_pTaskNum = new cTaskConsNum(2u));
	int array[2] = {1, 0};
	CustomKingdomUtil::Assert(2 == ::Constants::ARRAY_SIZE(array), "error");
	for (uint num = 0u; num < 2; num++)
	{
		m_pTaskNum->SetPosition(num, 3, array[num]);
	}
}

void cTaskConsGhost::Term()
{
}

void cTaskConsGhost::Start(uint now)
{
	m_nGhost = now;
	if (nGetState() != 1 && nGetState() != 2)
	{
		SetState(1u);
		SetAnim(0u, 45);
		Visible(0u);
	}
}

void cTaskConsGhost::End()
{
	if (nGetState() != 3 && nGetState() != 0)
	{
		m_pTaskNum->End();
		SetState(3u);
	}
}

void cTaskConsGhost::Update(uint now)
{
	m_nGhost = now;
}

bool cTaskConsGhost::Execute(cJetTask* pParam)
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
				SetAnim(0u, 46);
				m_pTaskNum->Start(m_nGhost);
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
		m_pTaskNum->Set(m_nGhost);
		break;
	case 3u:
		switch (nGetStep())
		{
		case 0u:
			if (m_pTaskNum->IsAnimEnd())
			{
				SetAnim(0u, 47);
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
