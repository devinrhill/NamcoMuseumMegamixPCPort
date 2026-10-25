#include "kingdom/game/g03/cTaskConsCombo.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskConsNum.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskConsCombo::cTaskConsCombo()
{
	SetName("cTaskConsCombo");
	Init();
}

cTaskConsCombo::~cTaskConsCombo()
{
	Term();
}

void cTaskConsCombo::Init()
{
	Regist(0u, "BIN_ANT_GAME");
	SetState(0u);
	m_nCombo = 0u;
	ConnectChild(m_pTaskNum = new cTaskConsNum(2u));
	int array[2] = {0, 1};
	CustomKingdomUtil::Assert(2 == ::Constants::ARRAY_SIZE(array), "error");
	for (uint num = 0u; num < 2; num++)
	{
		m_pTaskNum->SetPosition(num, 14, array[num]);
	}
}

void cTaskConsCombo::Term()
{
}

void cTaskConsCombo::Start(uint now)
{
	m_nCombo = now;
	SetState(1u);
	SetAnim(0u, 215);
	Visible(0u);
}

void cTaskConsCombo::End()
{
	if (nGetState() != 0 && nGetState() != 3)
	{
		m_pTaskNum->End();
		SetState(3u);
	}
}

void cTaskConsCombo::Update(uint now)
{
	if (now == 0)
	{
		return;
	}
	m_nWaitCnt = 0u;
	if (now > m_nCombo)
	{
		if (99 < now)
		{
			now = 99u;
		}
		m_nCombo = now;
		if (nGetState() != 1 && nGetState() != 2)
		{
			Start(now);
		}
	}
}

bool cTaskConsCombo::Execute(cJetTask* pParam)
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
				SetAnim(0u, 216);
				m_pTaskNum->Start(m_nCombo);
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
		{
			uint num = nGetStep();
			if (num != 0)
			{
				if (num != 1)
				{
					break;
				}
			}
			else
			{
				m_nWaitCnt = 0u;
				SetStep(1u);
			}
			m_pTaskNum->Set(m_nCombo);
			m_nWaitCnt++;
			if (60 < m_nWaitCnt)
			{
				End();
			}
			break;
		}
	case 3u:
		switch (nGetStep())
		{
		case 0u:
			if (m_pTaskNum->IsAnimEnd())
			{
				SetAnim(0u, 217);
				SetStep(1u);
			}
			break;
		case 1u:
			if (IsAnimEnd(0u))
			{
				Invisible(0u);
				SetState(0u);
				m_nCombo = 0u;
			}
			break;
		}
		break;
	}
	return cTaskConsoleBase::Execute(pParam);
}

bool cTaskConsCombo::IsInvisible()
{
	return nGetState() == 0;
}

}
