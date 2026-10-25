#include "kingdom/game/menu/cTaskConsBack.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::menu
{

cTaskConsBack::~cTaskConsBack()
{
}

cTaskConsBack::cTaskConsBack(int nId, string Key)
	: cTaskConsMatching(nId)
{
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetState(1u, false);
}

int cTaskConsBack::nGetBackAnmNo(uint nState)
{
	int obj[5] = {3, 4, 5, 6, 160};
	CustomKingdomUtil::Assert(0 <= nState && nState < 5, "error");
	return obj[nState];
}

bool cTaskConsBack::IsDpdHit(float x, float y)
{
	return false;
}

bool cTaskConsBack::AppearProc()
{
	bool bit = nullptr == UnityUI;
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetBackAnmNo(0u));
		pAnita(0u)->SetState(1u, bit);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsBack::WaitProc()
{
	pAnita(0u)->SetAnimNo(nGetBackAnmNo(1u), false);
	if (UnityUI != nullptr)
	{
		UnityUI->ResetAnim(m_buttonType);
	}
}

void cTaskConsBack::PointProc()
{
	pAnita(0u)->SetAnimNo(nGetBackAnmNo(2u), false);
	if (nullptr != UnityUI && !UnityUI->IsPlayAnim(m_buttonType, MotosUIMotosMultiHome::EAnim::Highlight))
	{
		if (UnityUI != nullptr)
		{
			UnityUI->PlayAnim(m_buttonType, MotosUIMotosMultiHome::EAnim::Highlight);
		}
	}
}

bool cTaskConsBack::DecideProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetBackAnmNo(3u), false);
		if (UnityUI != nullptr)
		{
			UnityUI->PlayAnim(m_buttonType, MotosUIMotosMultiHome::EAnim::Push);
		}
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u) && (nullptr == UnityUI || !UnityUI->IsPlayAnim(m_buttonType, MotosUIMotosMultiHome::EAnim::Push)))
	{
		return true;
	}
	return false;
}

bool cTaskConsBack::DeleteProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetBackAnmNo(4u), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
