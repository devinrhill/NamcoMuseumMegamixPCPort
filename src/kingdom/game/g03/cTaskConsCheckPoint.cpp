#include "kingdom/game/g03/cTaskConsCheckPoint.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

cTaskConsCheckPoint::cTaskConsCheckPoint()
{
	SetName("cTaskConsCheckPoint");
	Init();
}

cTaskConsCheckPoint::~cTaskConsCheckPoint()
{
	Term();
}

void cTaskConsCheckPoint::Init()
{
	Regist(0u, "BIN_ANT_GAME");
	SetState(0u);
}

void cTaskConsCheckPoint::Term()
{
}

void cTaskConsCheckPoint::Start()
{
	m_nWaitCnt = 0;
	if (nGetState() != 1 || nGetState() != 2)
	{
		SetState(1u);
		SetAnim(0u, 75);
		Visible(0u);
	}
}

void cTaskConsCheckPoint::End()
{
	SetState(3u);
	SetAnim(0u, 77);
}

bool cTaskConsCheckPoint::Execute(cJetTask* pParam)
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
			SetAnim(0u, 76);
			SetState(2u);
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE028_POINT_TUUKA);
		}
		break;
	case 2u:
		switch (nGetStep())
		{
		case 0u:
			if (IsAnimEnd(0u))
			{
				SetStep(1u);
				m_nWaitCnt = 0;
			}
			break;
		case 1u:
			if (60 < ++m_nWaitCnt)
			{
				End();
			}
			break;
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
