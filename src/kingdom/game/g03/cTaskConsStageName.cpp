#include "kingdom/game/g03/cTaskConsStageName.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "KingdomGlobal.hpp"
#include "MotosUIRollStageName.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskConsStageName::cTaskConsStageName()
{
	SetName("cTaskConsStageName");
	Init();
}

void cTaskConsStageName::Init()
{
	Regist(0u, "BIN_ANT_GAME");
	Regist(1u, "BIN_ANT_GAME", 12);
	m_nTimeCnt = 0u;
	SetState(0u);
}

void cTaskConsStageName::Term()
{
	if (UnityUI != nullptr)
	{
		UnityUI->SetEnable(false);
	}
}

void cTaskConsStageName::Start()
{
	SetState(1u);
	SetAnim(0u, nGetAnimNoIn(KingdomGlobal::PacnRollSys->nWorld()));
	if (nullptr == UnityUI)
	{
		Visible(0u);
	}
	SetAnim(1u, nGetAnimNoStageIn(KingdomGlobal::PacnRollSys->nStage()));
	Visible(1u);
	if (UnityUI != nullptr)
	{
		UnityUI->Init((int)(KingdomGlobal::PacnRollSys->nWorld() + 1));
	}
	if (UnityUI != nullptr)
	{
		UnityUI->SetEnable(true);
	}
	if (UnityUI != nullptr)
	{
		UnityUI->PlayBaseAnim(MotosUIRollStageName::EBaseAnim::In);
	}
}

void cTaskConsStageName::End()
{
	SetAnim(0u, nGetAnimNoOut(KingdomGlobal::PacnRollSys->nWorld()));
	SetAnim(1u, nGetAnimNoStageOut(KingdomGlobal::PacnRollSys->nStage()));
	SetState(3u);
	if (UnityUI != nullptr)
	{
		UnityUI->PlayBaseAnim(MotosUIRollStageName::EBaseAnim::Out);
	}
}

bool cTaskConsStageName::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	switch (nGetState())
	{
	case 1u:
		if (IsAnimEnd(0u) && (nullptr == UnityUI || !UnityUI->IsPlayBaseAnim()) && IsAnimEnd(1u))
		{
			SetAnim(0u, nGetAnimNoStay(KingdomGlobal::PacnRollSys->nWorld()));
			SetAnim(1u, nGetAnimNoStageStay(KingdomGlobal::PacnRollSys->nStage()));
			SetState(2u);
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
				m_nTimeCnt = 0u;
				SetStep(1u);
			}
			if (m_nTimeCnt < 120)
			{
				m_nTimeCnt++;
			}
			if (m_nTimeCnt == 120 && IsAnimEnd(0u) && IsAnimEnd(1u))
			{
				End();
			}
			break;
		}
	case 3u:
		if (IsAnimEnd(0u) && (nullptr == UnityUI || !UnityUI->IsPlayBaseAnim()) && IsAnimEnd(1u))
		{
			Invisible(0u);
			Invisible(1u);
			SetState(4u);
			if (UnityUI != nullptr)
			{
				UnityUI->SetEnable(false);
			}
			if (UnityUI != nullptr)
			{
				UnityUI->ResetBaseAnim();
			}
		}
		break;
	}
	return cTaskConsoleBase::Execute(pParam);
}

int cTaskConsStageName::nGetAnimNoIn(uint w)
{
	int array[5] = {51, 54, 57, 60, 63};
	CustomKingdomUtil::Assert((long)w < (long)::Constants::ARRAY_SIZE(array), "world over");
	return array[w];
}

int cTaskConsStageName::nGetAnimNoStay(uint w)
{
	int array[5] = {52, 55, 58, 61, 64};
	CustomKingdomUtil::Assert((long)w < (long)::Constants::ARRAY_SIZE(array), "world over");
	return array[w];
}

int cTaskConsStageName::nGetAnimNoOut(uint w)
{
	int array[5] = {53, 56, 59, 62, 65};
	CustomKingdomUtil::Assert((long)w < (long)::Constants::ARRAY_SIZE(array), "world over");
	return array[w];
}

int cTaskConsStageName::nGetAnimNoStageIn(uint s)
{
	int array[6] = {170, 173, 176, 179, 182, 185};
	CustomKingdomUtil::Assert((long)s < (long)::Constants::ARRAY_SIZE(array), "world over");
	return array[s];
}

int cTaskConsStageName::nGetAnimNoStageStay(uint s)
{
	int array[6] = {171, 174, 177, 180, 183, 186};
	CustomKingdomUtil::Assert((long)s < (long)::Constants::ARRAY_SIZE(array), "world over");
	return array[s];
}

int cTaskConsStageName::nGetAnimNoStageOut(uint s)
{
	int array[6] = {172, 175, 178, 181, 184, 187};
	CustomKingdomUtil::Assert((long)s < (long)::Constants::ARRAY_SIZE(array), "world over");
	return array[s];
}

bool cTaskConsStageName::IsPlayEnd()
{
	bool result = false;
	if (nGetState() == 0)
	{
		result = true;
	}
	if (nGetState() == 4)
	{
		result = true;
	}
	return result;
}

}
