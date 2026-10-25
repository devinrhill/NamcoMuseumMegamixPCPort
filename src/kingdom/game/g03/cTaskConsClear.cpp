#include "kingdom/game/g03/cTaskConsClear.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

cTaskConsClear::cTaskConsClear()
{
	SetName("cTaskConsClear");
	Init();
}

cTaskConsClear::~cTaskConsClear()
{
	Term();
}

void cTaskConsClear::Init()
{
	Regist(0u, "BIN_ANT_GAME");
	SetState(0u);
	m_bPerfect = false;
	m_nWait = 0u;
}

void cTaskConsClear::Term()
{
}

void cTaskConsClear::Start()
{
	m_bPerfect = false;
	SetState(1u);
	int ano = 78;
	SetAnim(0u, ano);
	Visible(0u);
}

void cTaskConsClear::PStart()
{
	m_bPerfect = true;
	SetState(1u);
	int ano = 81;
	SetAnim(0u, ano);
	Visible(0u);
}

void cTaskConsClear::End()
{
	SetState(3u);
	int ano = 80;
	if (IsPerfect())
	{
		ano = 83;
	}
	SetAnim(0u, ano);
}

bool cTaskConsClear::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	switch (nGetState())
	{
	case 1u:
	_switch1_1u:
		if (IsAnimEnd(0u))
		{
			int ano = 79;
			if (IsPerfect())
			{
				ano = 82;
			}
			SetAnim(0u, ano);
			SetState(2u);
		}
		break;
	case 2u:
	_switch1_2u:
		switch (nGetStep())
		{
		case 0u:
		_switch2_0u:
			{
				m_nWait = 0u;
				SetStep(1u);
				uint num = (uint)Random::Range(0, 3);
				ESEKind array[3] = {ESEKind::PACN_ROLL_SND_SE107_GOAL_1, ESEKind::PACN_ROLL_SND_SE108_GOAL_2, ESEKind::PACN_ROLL_SND_SE109_GOAL_3};
				SoundManager::get_SE()->PlaySE(array[num]);
				goto _switch2_1u;
			}
		case 1u:
		_switch2_1u:
			if (IsAnimEnd(0u))
			{
				SetStep(2u);
			}
			break;
		case 2u:
		_switch2_2u:
			m_nWait++;
			if (120 < m_nWait)
			{
				int ano2 = 80;
				if (IsPerfect())
				{
					ano2 = 83;
				}
				SetAnim(0u, ano2);
				SetState(3u);
			}
			break;
		}
		break;
	case 3u:
	_switch1_3u:
		if (IsAnimEnd(0u))
		{
			Invisible(0u);
			SetState(0u);
		}
		break;
	}
	return cTaskConsoleBase::Execute(pParam);
}

bool cTaskConsClear::IsPerfect()
{
	return m_bPerfect;
}

bool cTaskConsClear::IsPlayEnd()
{
	return nGetState() == 4;
}

}
