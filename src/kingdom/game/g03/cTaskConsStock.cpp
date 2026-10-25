#include "kingdom/game/g03/cTaskConsStock.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskConsNum.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskConsStock::cTaskConsStock()
{
	SetName("cTaskConsStock");
	Init();
}

cTaskConsStock::~cTaskConsStock()
{
	Term();
}

bool cTaskConsStock::IsUpdate(uint dat)
{
	return m_nStock != dat;
}

void cTaskConsStock::Init()
{
	Regist(0u, "BIN_ANT_GAME");
	Regist(1u, "BIN_ANT_GAME", 4);
	SetState(0u);
	m_nStock = 0u;
	ConnectChild(m_pTaskNum = new cTaskConsNum(2u));
	int array[2] = {2, 1};
	CustomKingdomUtil::Assert(2 == ::Constants::ARRAY_SIZE(array), "error");
	for (uint num = 0u; num < 2; num++)
	{
		m_pTaskNum->SetPosition(num, 4, array[num]);
	}
}

void cTaskConsStock::Term()
{
}

void cTaskConsStock::Start(uint now)
{
	m_nStock = now;
	if (nGetState() != 1 && nGetState() != 2)
	{
		SetState(1u);
		SetAnim(0u, 48);
		Visible(0u);
	}
}

void cTaskConsStock::End()
{
	if (nGetState() != 3 && nGetState() != 0)
	{
		m_pTaskNum->End();
		SetAnim(1u, 35);
		SetState(3u);
	}
}

void cTaskConsStock::Update(uint now)
{
	m_nStock = now;
}

bool cTaskConsStock::Execute(cJetTask* pParam)
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
				SetAnim(0u, 49);
				SetAnim(1u, 33);
				Visible(1u);
				m_pTaskNum->Start(m_nStock);
				SetStep(1u);
			}
			break;
		case 1u:
			if (IsAnimEnd(1u))
			{
				SetAnim(1u, 34);
				SetState(2u);
			}
			break;
		}
		break;
	case 2u:
		m_pTaskNum->Set(m_nStock);
		break;
	case 3u:
		switch (nGetStep())
		{
		case 0u:
			if (IsAnimEnd(1u))
			{
				Invisible(1u);
				SetAnim(0u, 50);
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
