#include "kingdom/game/g03/cTaskConsNum.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"

namespace kingdom::game::g03
{

const int cTaskConsNum::scInAnimNo[10] = {0, 3, 6, 9, 12, 15, 18, 21, 24, 27};
const int cTaskConsNum::scWaitAnimNo[10] = {1, 4, 7, 10, 13, 16, 19, 22, 25, 28};
const int cTaskConsNum::scOutAnimNo[10] = {2, 5, 8, 11, 14, 17, 20, 23, 26, 29};

cTaskConsNum::cTaskConsNum()
{
	SetName("cTaskConsNum");
	Init();
}

cTaskConsNum::cTaskConsNum(uint figure)
{
	SetName("cTaskConsNum");
	Init(figure);
}

cTaskConsNum::~cTaskConsNum()
{
	Term();
}

void cTaskConsNum::Init(uint figure)
{
	m_nNum = 0u;
	m_nFigure = figure;
	for (uint num = 0u; num < m_nFigure; num++)
	{
		Regist(num, "BIN_ANT_GAME");
	}
	SetState(0u);
}

void cTaskConsNum::Term()
{
}

bool cTaskConsNum::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	switch (nGetState())
	{
	case 1u:
		if (IsAnimEnd())
		{
			uint num2 = m_nNum;
			for (uint num3 = 0u; num3 < m_nFigure; num3++)
			{
				SetAnim(num3, nGetAnimNoStay(num2 % 10));
				num2 /= 10;
			}
			SetState(2u);
		}
		break;
	case 3u:
		if (IsAnimEnd())
		{
			for (uint num = 0u; num < m_nFigure; num++)
			{
				Invisible(num);
			}
			SetState(0u);
		}
		break;
	}
	return cTaskConsoleBase::Execute(pParam);
}

void cTaskConsNum::Start(uint num)
{
	m_nNum = num;
	uint num2 = m_nNum;
	for (uint num3 = 0u; num3 < m_nFigure; num3++)
	{
		SetAnim(num3, nGetAnimNoIn(num2 % 10));
		Visible(num3);
		num2 /= 10;
	}
	SetState(1u);
}

bool cTaskConsNum::End()
{
	uint num = m_nNum;
	for (uint num2 = 0u; num2 < m_nFigure; num2++)
	{
		SetAnim(num2, nGetAnimNoOut(num % 10));
		num /= 10;
	}
	SetState(3u);
	return true;
}

bool cTaskConsNum::IsSet()
{
	if (nGetState() == 0 || nGetState() == 2)
	{
		return true;
	}
	return false;
}

bool cTaskConsNum::Set(uint num)
{
	if (nGetState() != 2)
	{
		return false;
	}
	if (m_nNum == num)
	{
		return false;
	}
	m_nNum = num;
	uint num2 = m_nNum;
	for (uint num3 = 0u; num3 < m_nFigure; num3++)
	{
		SetAnim(num3, nGetAnimNoStay(num2 % 10));
		Visible(num3);
		num2 /= 10;
	}
	return true;
}

bool cTaskConsNum::IsAnimEnd()
{
	return cTaskConsoleBase::IsAnimEnd(0u); // the converted overload resolved to itself (infinite recursion)
}

int cTaskConsNum::nGetAnimNoIn(uint num)
{
	return scInAnimNo[num];
}

int cTaskConsNum::nGetAnimNoStay(uint num)
{
	return scWaitAnimNo[num];
}

int cTaskConsNum::nGetAnimNoOut(uint num)
{
	return scOutAnimNo[num];
}

}
