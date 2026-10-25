#include "kingdom/game/g03/cTaskConsPacLife.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskConsPacLife::cTaskConsPacLife()
{
	SetName("cTaskConsPacLife");
	Init();
}

cTaskConsPacLife::~cTaskConsPacLife()
{
	Term();
}

void cTaskConsPacLife::Init()
{
	m_nMaxHP = 0u;
	m_nMaxHPOld = 0u;
	m_nHP = 0u;
	m_nHPOld = 0u;
	Regist(0u, "BIN_ANT_GAME");
	Regist(1u, "BIN_ANT_GAME");
	SetState(0u);
}

void cTaskConsPacLife::Term()
{
}

void cTaskConsPacLife::Start(uint max)
{
	m_nMaxHP = max;
	m_nMaxHPOld = max;
	m_nHP = max;
	m_nHPOld = max;
	SetState(1u);
	SetAnim(0u, nGetAnimFlameIn(m_nMaxHPOld));
	Visible(0u);
}

void cTaskConsPacLife::End()
{
	if (m_nMaxHP >= 3)
	{
		SetState(5u);
	}
}

bool cTaskConsPacLife::Execute(cJetTask* pParam)
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
			ChangeAnim(0u, nGetAnimHPStay(m_nMaxHPOld, m_nHPOld));
			SetAnim(1u, 96);
			Visible(1u);
			SetPosIcon();
			SetState(2u);
		}
		break;
	case 2u:
		if (IsAnimEnd(1u))
		{
			ChangeAnim(1u, 97);
		}
		UpdateOld();
		break;
	case 3u:
		if (IsAnimEnd(0u))
		{
			SetState(2u);
		}
		break;
	case 4u:
		if (IsAnimEnd(0u))
		{
			ChangeAnim(0u, nGetAnimHPStay(m_nMaxHPOld, m_nHPOld));
			SetState(2u);
		}
		break;
	case 5u:
		switch (nGetStep())
		{
		case 0u:
			SetAnim(1u, 98);
			SetAnim(0u, nGetAnimFlameOut(m_nMaxHPOld, m_nHPOld));
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

void cTaskConsPacLife::UpdateOld()
{
	if (m_nMaxHPOld < m_nMaxHP)
	{
		UpMaxHP();
	}
	else
	{
		(void)(m_nMaxHP);
		(void)(m_nMaxHPOld);
	}
	if (m_nHPOld < m_nHP)
	{
		UpHP();
	}
	else if (m_nHP < m_nHPOld)
	{
		DownHP();
	}
}

void cTaskConsPacLife::UpHP()
{
	if (m_nHPOld < m_nMaxHPOld)
	{
		m_nHPOld++;
	}
	ChangeAnim(0u, nGetAnimHPUp(m_nMaxHPOld, m_nHPOld));
	SetPosIcon();
	SetState(3u);
}

void cTaskConsPacLife::DownHP()
{
	if (0 < m_nHPOld)
	{
		m_nHPOld--;
	}
	ChangeAnim(0u, nGetAnimHPDown(m_nMaxHPOld, m_nHPOld));
	SetPosIcon();
	SetState(4u);
}

void cTaskConsPacLife::UpMaxHP()
{
	m_nMaxHPOld = m_nMaxHP;
	m_nHP = m_nMaxHP;
	m_nHPOld = m_nMaxHP;
	ChangeAnim(0u, nGetAnimFlameUP(m_nMaxHPOld));
	SetPosIcon();
	SetState(3u);
	m_bMaxAnime = true;
}

void cTaskConsPacLife::_UpMaxHP()
{
	if (m_nMaxHPOld < m_nMaxHP)
	{
		m_nMaxHPOld++;
	}
	else
	{
		m_bMaxAnime = false;
	}
	ChangeAnim(0u, nGetAnimFlameUP(m_nMaxHPOld));
	SetPosIcon();
	SetState(3u);
}

void cTaskConsPacLife::Update(uint max, uint HP)
{
	m_nMaxHP = max;
	m_nHP = HP;
}

void cTaskConsPacLife::SetPosIcon()
{
	int num = (int)(m_nHPOld - 1);
	if (num < 0)
	{
		num = 0;
	}
	SetPosition(1u, 10, num);
}

int cTaskConsPacLife::nGetAnimFlameIn(uint max)
{
	int array[8] = {-1, -1, -1, 99, 106, 114, 123, 133};
	CustomKingdomUtil::Assert((long)max < (long)::Constants::ARRAY_SIZE(array), "over");
	return array[max];
}

int cTaskConsPacLife::nGetAnimFlameOut(uint max, uint HP)
{
	int array[8] = {-1, -1, -1, 105, 113, 122, 132, 143};
	CustomKingdomUtil::Assert((long)max < (long)::Constants::ARRAY_SIZE(array), "over");
	return array[max];
}

int cTaskConsPacLife::nGetAnimHPUp(uint max, uint HP)
{
	int obj[8][8] = {{-1, -1, -1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1, -1, -1}, {-1, 195, 195, 196, -1, -1, -1, -1}, {-1, 197, 197, 198, 199, -1, -1, -1}, {-1, 200, 200, 201, 202, 203, -1, -1}, {-1, 204, 204, 205, 206, 207, 208, -1}, {-1, 209, 209, 210, 211, 212, 213, 214}};
	CustomKingdomUtil::Assert(max < 8, "over");
	CustomKingdomUtil::Assert(HP < 8, "over");
	return obj[max][HP];
}

int cTaskConsPacLife::nGetAnimHPStay(uint max, uint HP)
{
	int obj[8][8] = {{-1, -1, -1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1, -1, -1}, {104, 103, 101, 100, -1, -1, -1, -1}, {112, 111, 109, 108, 107, -1, -1, -1}, {121, 120, 118, 117, 116, 114, -1}, {131, 130, 128, 127, 126, 125, 124, -1}, {142, 141, 139, 138, 137, 136, 135, 134}};
	CustomKingdomUtil::Assert(max < 8, "over");
	CustomKingdomUtil::Assert(HP < 8, "over");
	return obj[max][HP];
}

int cTaskConsPacLife::nGetAnimHPDown(uint max, uint HP)
{
	int obj[8][8] = {{-1, -1, -1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1, -1, -1}, {104, 102, 101, 100, -1, -1, -1, -1}, {112, 110, 109, 108, 107, -1, -1, -1}, {121, 119, 118, 117, 116, 114, -1}, {131, 129, 128, 127, 126, 125, 124, -1}, {142, 140, 139, 138, 137, 136, 135, 134}};
	CustomKingdomUtil::Assert(max < 8, "over");
	CustomKingdomUtil::Assert(HP < 8, "over");
	return obj[max][HP];
}

int cTaskConsPacLife::nGetAnimFlameUP(uint max)
{
	int array[8] = {-1, -1, -1, -1, 191, 192, 193, 194};
	CustomKingdomUtil::Assert((long)max < (long)::Constants::ARRAY_SIZE(array), "over");
	return array[max];
}

}
