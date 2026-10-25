#include "kingdom/game/menu/cTaskConsPerson.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::menu
{

uint& cTaskConsPerson::nType()
{
	return m_nType;
}

int& cTaskConsPerson::nHitPat()
{
	return m_nHitPat;
}

cTaskConsPerson::~cTaskConsPerson()
{
}

uint cTaskConsPerson::nGetType()
{
	return nType();
}

cTaskConsPerson::cTaskConsPerson(int nId, string Key, uint nType)
	: cTaskConsMatching(nId)
{
	CustomKingdomUtil::Assert(0 <= nType && nType < 3, "error");
	this->nType() = nType;
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetState(1u, false);
	nHitPat() = HIT_PAT[nType];
	switch (nType)
	{
	case 0u:
		m_buttonType = MotosUIMotosMultiHome::EButtonType::Player2;
		break;
	case 1u:
		m_buttonType = MotosUIMotosMultiHome::EButtonType::Player3;
		break;
	case 2u:
		m_buttonType = MotosUIMotosMultiHome::EButtonType::Player4;
		break;
	}
}

int cTaskConsPerson::nGetPersonAnmNo(uint nState)
{
	int obj[3][5] = {{16, 17, 18, 19, 20}, {21, 22, 23, 24, 25}, {26, 27, 28, 29, 30}};
	CustomKingdomUtil::Assert(0 <= nType() && nType() < 3, "error");
	CustomKingdomUtil::Assert(0 <= nState && nState < 5, "error");
	return obj[nType()][nState];
}

bool cTaskConsPerson::IsDpdHit(float x, float y)
{
	return false;
}

bool cTaskConsPerson::AppearProc()
{
	bool bit = nullptr == UnityUI;
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetPersonAnmNo(0u));
		pAnita(0u)->SetState(1u, bit);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsPerson::WaitProc()
{
	pAnita(0u)->SetAnimNo(nGetPersonAnmNo(1u), false);
	if (UnityUI != nullptr)
	{
		UnityUI->ResetAnim(m_buttonType);
	}
}

void cTaskConsPerson::PointProc()
{
	pAnita(0u)->SetAnimNo(nGetPersonAnmNo(2u), false);
	if (nullptr != UnityUI && !UnityUI->IsPlayAnim(m_buttonType, MotosUIMotosMultiHome::EAnim::Highlight))
	{
		if (UnityUI != nullptr)
		{
			UnityUI->PlayAnim(m_buttonType, MotosUIMotosMultiHome::EAnim::Highlight);
		}
	}
}

bool cTaskConsPerson::DecideProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetPersonAnmNo(3u), false);
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

bool cTaskConsPerson::DeleteProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetPersonAnmNo(4u), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
