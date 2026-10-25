#include "kingdom/game/menu/cTaskConsCtrl.hpp"

#include <limits>

#include "CustomKingdomUtil.hpp"
#include "MotosUIMotosMultiHome.hpp"

namespace kingdom::game::menu
{

uint& cTaskConsCtrl::nCtrlNo()
{
	return m_nCtrlNo;
}

uint& cTaskConsCtrl::nPlayerNo()
{
	return m_nPlayerNo;
}

bool& cTaskConsCtrl::bActive()
{
	return m_bActive;
}

cTaskConsCtrl::~cTaskConsCtrl()
{
}

void cTaskConsCtrl::nSetCtrlNo(uint nNo)
{
	nCtrlNo() = nNo;
}

uint cTaskConsCtrl::nGetCtrlNo()
{
	return nCtrlNo();
}

void cTaskConsCtrl::bActive(bool b)
{
	bActive() = b;
}

bool cTaskConsCtrl::IsActive()
{
	return bActive();
}

cTaskConsCtrl::cTaskConsCtrl(int nId, string Key, uint nPlayerNo)
	: cTaskConsMatching(nId)
{
	this->nPlayerNo() = nPlayerNo;
	nCtrlNo() = std::numeric_limits<uint>::max();
	bActive() = false;
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetState(1u, false);
}

int cTaskConsCtrl::nGetCtrlAnmNo(uint nState)
{
	int obj[4][3] = {{31, 32, 147}, {37, 38, 148}, {43, 44, 149}, {49, 50, 150}};
	CustomKingdomUtil::Assert(0 <= nState && nState < 3, "error");
	int num = obj[nPlayerNo()][nState];
	if (nState == 1 && bActive())
	{
		num += (int)(1 + nCtrlNo());
	}
	return num;
}

bool cTaskConsCtrl::AppearProc()
{
	bool bit = nullptr == UnityUI;
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetCtrlAnmNo(0u));
		pAnita(0u)->SetState(1u, bit);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsCtrl::WaitProc()
{
	pAnita(0u)->SetAnimNo(nGetCtrlAnmNo(1u), false);
}

bool cTaskConsCtrl::DeleteProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetCtrlAnmNo(2u));
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
