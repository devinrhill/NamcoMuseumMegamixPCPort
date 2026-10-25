#include "kingdom/game/g03/cTaskConsGate.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskConsNum.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskConsGate::cTaskConsGate()
{
	SetName("cTaskConsGate");
	Init();
}

cTaskConsGate::~cTaskConsGate()
{
	Term();
}

void cTaskConsGate::Init()
{
	Regist(0u, "BIN_ANT_GAME");
	SetState(0u);
	m_nGate = 0u;
	ConnectChild(m_pTaskNum = new cTaskConsNum(2u));
	int array[2] = {1, 0};
	CustomKingdomUtil::Assert(2 == ::Constants::ARRAY_SIZE(array), "error");
	for (uint num = 0u; num < 2; num++)
	{
		m_pTaskNum->SetPosition(num, 6, array[num]);
	}
}

void cTaskConsGate::Term()
{
}

void cTaskConsGate::Start(uint now)
{
	m_nGate = now;
	SetState(1u);
	SetAnim(0u, 93);
	Visible(0u);
}

void cTaskConsGate::End()
{
	if (nGetState() != 3 && nGetState() != 0)
	{
		m_pTaskNum->End();
		SetState(3u);
	}
}

void cTaskConsGate::Update(uint now)
{
	m_nGate = now;
}

bool cTaskConsGate::Execute(cJetTask* pParam)
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
				SetAnim(0u, 94);
				m_pTaskNum->Start(m_nGate);
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
		m_pTaskNum->Set(m_nGate);
		break;
	case 3u:
		switch (nGetStep())
		{
		case 0u:
			if (m_pTaskNum->IsAnimEnd())
			{
				SetAnim(0u, 95);
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
