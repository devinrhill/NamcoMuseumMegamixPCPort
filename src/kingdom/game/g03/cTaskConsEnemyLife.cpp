#include "kingdom/game/g03/cTaskConsEnemyLife.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskConsEnemyLife::cTaskConsEnemyLife()
{
	SetName("cTaskConsEnemyLife");
	Init();
}

cTaskConsEnemyLife::~cTaskConsEnemyLife()
{
	Term();
}

void cTaskConsEnemyLife::Init()
{
	m_nHP = 0u;
	m_nHPOld = 0u;
	m_nLevel = 0u;
	Regist(0u, "BIN_ANT_GAME");
	Invisible(0u);
	Regist(1u, "BIN_ANT_GAME");
	Invisible(1u);
	SetState(0u);
}

void cTaskConsEnemyLife::Term()
{
}

void cTaskConsEnemyLife::Start(uint HP)
{
	m_nHP = HP;
	m_nHPOld = HP;
	SetState(1u);
	SetAnim(0u, 151);
	Visible(0u);
	Invisible(1u);
}

void cTaskConsEnemyLife::End()
{
	SetState(5u);
}

bool cTaskConsEnemyLife::Execute(cJetTask* pParam)
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
			ChangeAnim(0u, nGetAnimHPStay(m_nHPOld));
			SetAnim(1u, 144);
			Visible(1u);
			SetPosIcon();
			SetState(2u);
		}
		break;
	case 2u:
		if (IsAnimEnd(1u))
		{
			if (pAnita(1u)->GetAnimNo() == 147)
			{
				ChangeAnim(1u, 148);
			}
			else if (pAnita(1u)->GetAnimNo() == 150)
			{
				End();
			}
			else
			{
				ChangeAnim(1u, 145);
			}
		}
		_Update();
		break;
	case 5u:
		switch (nGetStep())
		{
		case 0u:
			SetAnim(0u, 157);
			SetStep(1u);
			break;
		case 1u:
			if (IsAnimEnd(0u) && IsAnimEnd(1u))
			{
				Invisible(0u);
				Invisible(1u);
				SetState(0u);
			}
			break;
		}
		break;
	}
	return cTaskConsoleBase::Execute(pParam);
}

void cTaskConsEnemyLife::_Update()
{
	if (m_nHP < m_nHPOld)
	{
		DownHP();
	}
}

void cTaskConsEnemyLife::DownHP()
{
	if (0 < m_nHPOld)
	{
		m_nHPOld--;
	}
	ChangeAnim(0u, nGetAnimHPDown(m_nHPOld));
	if (m_nHPOld == 0)
	{
		Dead();
	}
	SetPosIcon();
}

void cTaskConsEnemyLife::Update(uint HP, uint level)
{
	m_nHP = HP;
	if (m_nHP != 0 && nGetState() != 0 && nGetState() != 1)
	{
		if (level == 3)
		{
			Cry();
		}
		else
		{
			Stay();
		}
	}
}

void cTaskConsEnemyLife::SetPosIcon()
{
	int num = (int)(m_nHPOld - 1);
	if (num < 0)
	{
		num = 0;
	}
	SetPosition(1u, 8, num);
}

int cTaskConsEnemyLife::nGetAnimHPStay(uint HP)
{
	int array[4] = {156, 155, 153, 152};
	CustomKingdomUtil::Assert((long)HP < (long)::Constants::ARRAY_SIZE(array), "over");
	return array[HP];
}

int cTaskConsEnemyLife::nGetAnimHPDown(uint HP)
{
	int array[4] = {156, 154, 153, 152};
	CustomKingdomUtil::Assert((long)HP < (long)::Constants::ARRAY_SIZE(array), "over");
	return array[HP];
}

void cTaskConsEnemyLife::Cry()
{
	if (pAnita(1u)->GetAnimNo() != 147 && pAnita(1u)->GetAnimNo() != 148)
	{
		SetAnim(1u, 147);
	}
}

void cTaskConsEnemyLife::Stay()
{
	if (pAnita(1u)->GetAnimNo() != 149 && pAnita(1u)->GetAnimNo() != 145 && pAnita(1u)->GetAnimNo() != 144)
	{
		SetAnim(1u, 149);
	}
}

void cTaskConsEnemyLife::Dead()
{
	ChangeAnim(1u, 150);
}

}
