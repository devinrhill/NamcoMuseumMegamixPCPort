#include "kingdom/game/g03/cTaskConsTimer.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskConsNum.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskConsTimer::cTaskConsTimer()
{
	SetName("cTaskConsTimer");
	Init();
}

cTaskConsTimer::~cTaskConsTimer()
{
	Term();
}

void cTaskConsTimer::Init()
{
	Regist(0u, "BIN_ANT_GAME");
	Regist(1u, "BIN_ANT_GAME", 0, 1);
	Regist(2u, "BIN_ANT_GAME", 0, 4);
	SetState(0u);
	m_nTime = 0u;
	ConnectChild(m_pTaskMin = new cTaskConsNum(1u));
	ConnectChild(m_pTaskSec = new cTaskConsNum(2u));
	ConnectChild(m_pTaskMSec = new cTaskConsNum(2u));
	int array[1]{};
	CustomKingdomUtil::Assert(1 == ::Constants::ARRAY_SIZE(array), "error");
	for (uint num = 0u; num < 1; num++)
	{
		m_pTaskMin->SetPosition(num, 0, array[num]);
	}
	int array2[2] = {3, 2};
	CustomKingdomUtil::Assert(2 == ::Constants::ARRAY_SIZE(array2), "error");
	for (uint num2 = 0u; num2 < 2; num2++)
	{
		m_pTaskSec->SetPosition(num2, 0, array2[num2]);
	}
	int array3[2] = {6, 5};
	CustomKingdomUtil::Assert(2 == ::Constants::ARRAY_SIZE(array3), "error");
	for (uint num3 = 0u; num3 < 2; num3++)
	{
		m_pTaskMSec->SetPosition(num3, 0, array3[num3]);
	}
}

void cTaskConsTimer::Term()
{
}

void cTaskConsTimer::Start(uint now)
{
	m_nTime = now;
	SetState(1u);
	SetAnim(0u, 39);
	Visible(0u);
}

void cTaskConsTimer::End()
{
	m_pTaskMin->End();
	m_pTaskSec->End();
	m_pTaskMSec->End();
	SetAnim(1u, 32);
	SetAnim(2u, 32);
	SetState(4u);
}

void cTaskConsTimer::Update(uint now)
{
	if (599999 < now)
	{
		now = 599999u;
	}
	m_nTime = now;
}

bool cTaskConsTimer::Execute(cJetTask* pParam)
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
			SetAnim(0u, 40);
			SetAnim(1u, 30);
			Visible(1u);
			SetAnim(2u, 30);
			Visible(2u);
			uint num = m_nTime / 1000 / 60 % 60;
			uint num2 = m_nTime / 1000 % 60;
			uint num3 = m_nTime / 10 % 100;
			m_pTaskMin->Start(num);
			m_pTaskSec->Start(num2);
			m_pTaskMSec->Start(num3);
			SetState(2u);
		}
		break;
	case 2u:
		if (IsAnimEnd(1u))
		{
			SetAnim(1u, 31);
			SetAnim(2u, 31);
			SetState(3u);
		}
		break;
	case 3u:
		{
			uint num4 = m_nTime / 1000 / 60 % 60;
			uint num5 = m_nTime / 1000 % 60;
			uint num6 = m_nTime / 10 % 100;
			m_pTaskMin->Set(num4);
			m_pTaskSec->Set(num5);
			m_pTaskMSec->Set(num6);
			break;
		}
	case 4u:
		if (IsAnimEnd(1u))
		{
			Invisible(1u);
			Invisible(2u);
			SetAnim(0u, 41);
			SetState(5u);
		}
		break;
	case 5u:
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
