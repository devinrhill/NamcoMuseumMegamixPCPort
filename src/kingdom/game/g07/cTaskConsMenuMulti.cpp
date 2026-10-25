#include "kingdom/game/g07/cTaskConsMenuMulti.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuStart.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/game/g07/cTaskConsMenuMan.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

int& cTaskConsMenuMulti::nHitPat()
{
	return m_nHitPat;
}

cTaskConsMenuMulti::cTaskConsMenuMulti(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

cTaskConsMenuMulti::~cTaskConsMenuMulti()
{
}

void cTaskConsMenuMulti::SetData()
{
	nDrawPriority(2147483656u);
	nHitPat() = 24;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
	pAnita(0u)->SetAnimNo(nGetAnmNo(1));
	pAnita(0u)->SetState(1u, false);
	UnityUI = (static_cast<cTaskConsMenuMan*>(pTaskConsMenuMan()))->UnityUI;
}

int cTaskConsMenuMulti::nGetAnmNo(int nAnmState)
{
	int obj[4] = {338, 339, 340, 341};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 4, "State Over");
	return obj[nAnmState];
}

// DOL 0x801c9400: hit test of anita 0, pattern +0x1d8 (nHitPat); shared by the multiplayer menu's Single / Start buttons (the C# returned false: Unity used colliders)
bool cTaskConsMenuMulti::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, nHitPat());
}

void cTaskConsMenuMulti::WaitExec()
{
	if (nullptr != UnityUI || !((dynamic_cast<cTaskConsMultiMenuStart*>(this) != nullptr)))
	{
		UnityUI->ResetAnim(m_buttonType);
	}
	else
	{
		pAnita(0u)->SetAnimNo(nGetAnmNo(1), false);
	}
}

bool cTaskConsMenuMulti::AppearExec()
{
	if (nullptr != UnityUI && !((dynamic_cast<cTaskConsMultiMenuStart*>(this) != nullptr)))
	{
		return true;
	}
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetAnmNo(0));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		pAnita(0u)->SetAnimNo(nGetAnmNo(1));
		return true;
	}
	return false;
}

void cTaskConsMenuMulti::PointExec()
{
	if (nullptr != UnityUI && !((dynamic_cast<cTaskConsMultiMenuStart*>(this) != nullptr)))
	{
		if (!UnityUI->IsPlayAnim(m_buttonType, MotosUIMotosHome::EAnim::Highlight))
		{
			if (UnityUI != nullptr)
			{
				UnityUI->PlayAnim(m_buttonType, MotosUIMotosHome::EAnim::Highlight);
			}
		}
	}
	else
	{
		pAnita(0u)->SetAnimNo(nGetAnmNo(2), false);
	}
}

bool cTaskConsMenuMulti::DecideExec()
{
	if (nStep() == 0)
	{
		if (nullptr != UnityUI && !((dynamic_cast<cTaskConsMultiMenuStart*>(this) != nullptr)))
		{
			if (UnityUI != nullptr)
			{
				UnityUI->PlayAnim(m_buttonType, MotosUIMotosHome::EAnim::Push);
			}
		}
		else
		{
			pAnita(0u)->SetAnimNo(nGetAnmNo(3));
		}
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool flag = false;
		if ((!(nullptr != UnityUI) || (dynamic_cast<cTaskConsMultiMenuStart*>(this) != nullptr)) ? pAnita(0u)->IsState(65536u) : (!UnityUI->IsPlayAnim(m_buttonType, MotosUIMotosHome::EAnim::Push)))
		{
			if (UnityUI != nullptr)
			{
				UnityUI->ResetAnim(m_buttonType);
			}
			return true;
		}
	}
	return false;
}

}
