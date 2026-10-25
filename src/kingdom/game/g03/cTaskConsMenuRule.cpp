#include "kingdom/game/g03/cTaskConsMenuRule.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

const int cTaskConsMenuRule::ANM_LIST[4] = {279, 280, 281, 282};

cTaskConsMenuRule::cTaskConsMenuRule(cTaskConsMenuManBase* pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(2147483652u);
	string fileName = "BIN_ANT_MENU_SELECT01";
	pAnita(0u)->RegistData(fileName);
	pAnita(0u)->SetAnimNo(ANM_LIST[1]);
	pAnita(0u)->SetState(1u, false);
}

cTaskConsMenuRule::~cTaskConsMenuRule()
{
}

// DOL 0x8005e344: pattern 0x11 of anita 0
bool cTaskConsMenuRule::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, 17);
}

void cTaskConsMenuRule::WaitExec()
{
	pAnita(0u)->SetAnimNo(ANM_LIST[1], false);
	if (UnityUI != nullptr)
	{
		UnityUI->ResetAnim(m_buttonType);
	}
}

bool cTaskConsMenuRule::AppearExec()
{
	if (nStep() == 0)
	{
		bool bit = nullptr == UnityUI;
		pAnita(0u)->SetAnimNo(ANM_LIST[0]);
		pAnita(0u)->SetState(1u, bit);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		pAnita(0u)->SetAnimNo(ANM_LIST[1]);
		return true;
	}
	if (nullptr != UnityUI)
	{
		return true;
	}
	return false;
}

void cTaskConsMenuRule::PointExec()
{
	pAnita(0u)->SetAnimNo(ANM_LIST[2], false);
	if (nullptr != UnityUI && !UnityUI->IsPlayAnim(m_buttonType, MotosUIRollHome::EAnim::Highlight))
	{
		if (UnityUI != nullptr)
		{
			UnityUI->PlayAnim(m_buttonType, MotosUIRollHome::EAnim::Highlight);
		}
	}
}

bool cTaskConsMenuRule::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(ANM_LIST[3]);
		if (UnityUI != nullptr)
		{
			UnityUI->PlayAnim(m_buttonType, MotosUIRollHome::EAnim::Push);
		}
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u) && (nullptr == UnityUI || !UnityUI->IsPlayAnim(m_buttonType, MotosUIRollHome::EAnim::Push)))
	{
		if (UnityUI != nullptr)
		{
			UnityUI->ResetAnim(m_buttonType);
		}
		return true;
	}
	return false;
}

}
