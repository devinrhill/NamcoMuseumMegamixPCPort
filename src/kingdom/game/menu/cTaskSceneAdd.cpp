#include "kingdom/game/menu/cTaskSceneAdd.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/menu/cTaskConsAddWnd.hpp"
#include "kingdom/game/menu/cTaskConsBigCtrl.hpp"
#include "kingdom/game/menu/cTaskConsOk.hpp"
#include "kingdom/game/menu/cTaskConsString.hpp"
#include <ESEKind.hpp>
#include <InGamePadManager.hpp>

namespace kingdom::game::menu
{

int& cTaskSceneAdd::nUpdateCheckCt()
{
	return m_nUpdateCheckCt;
}

int& cTaskSceneAdd::nConnectCtrlCtOld()
{
	return m_nConnectCtrlCtOld;
}

cTaskConsAddWnd*& cTaskSceneAdd::pTaskConsAddWnd()
{
	return m_pTaskConsAddWnd;
}

cTaskConsOk*& cTaskSceneAdd::pTaskConsOk()
{
	return m_pTaskConsOk;
}

cTaskConsString*& cTaskSceneAdd::pTaskConsString()
{
	return m_pTaskConsString;
}

cTaskConsBigCtrl*& cTaskSceneAdd::pTaskConsBigCtrl(int nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 4, "error");
	return m_pTaskConsBigCtrl[nNo];
}

cTaskSceneAdd::~cTaskSceneAdd()
{
}

cTaskSceneAdd::cTaskSceneAdd(int nId, string AnitaKey)
	: cTaskSceneBase(nId)
{
	nState() = 0;
	RegistAnita(AnitaKey);
	nConnectCtrlCtOld() = 0;
	AddHitArray();
	nSelectNow() = (nSelectOld() = 2);
}

void cTaskSceneAdd::RegistAnita(string AnitaKey)
{
	pTaskConsAddWnd() = new cTaskConsAddWnd(1, AnitaKey);
	CustomKingdomUtil::Assert(pTaskConsAddWnd() != nullptr);
	pTaskConsAddWnd()->nDrawPriority(65540u);
	ConnectChild(pTaskConsAddWnd());
	int num = 3;
	int num2 = 0;
	while (num <= 6)
	{
		pTaskConsBigCtrl(num2) = new cTaskConsBigCtrl(num, AnitaKey, (uint)num2);
		CustomKingdomUtil::Assert(pTaskConsBigCtrl(num2) != nullptr);
		pTaskConsBigCtrl(num2)->nDrawPriority((uint)(65542 + num2));
		ConnectChild(pTaskConsBigCtrl(num2));
		num++;
		num2++;
	}
	pTaskConsOk() = new cTaskConsOk(2, AnitaKey);
	CustomKingdomUtil::Assert(pTaskConsOk() != nullptr);
	pTaskConsOk()->nDrawPriority(65541u);
	ConnectChild(pTaskConsOk());
	pTaskConsString() = new cTaskConsString(7, AnitaKey);
	CustomKingdomUtil::Assert(pTaskConsString() != nullptr);
	pTaskConsString()->nSetStringIdx(0);
	pTaskConsString()->nDrawPriority(65546u);
	ConnectChild(pTaskConsString());
}

void cTaskSceneAdd::AddHitArray()
{
	vecHit().push_back(pTaskConsOk());
}

void cTaskSceneAdd::AllWait()
{
	pTaskConsOk()->Wait();
	for (int i = 0; i < 4; i++)
	{
		pTaskConsBigCtrl(i)->Wait();
	}
}

void cTaskSceneAdd::KeySelect(const int& padNum)
{
	int array[2][5] = {{2, -1, -1, -1, -1}, {-1, -1, -1, -1, -1}};
	for (int i = 0; array[i][0] != -1; i++)
	{
		if (nSelectNow() != array[i][0])
		{
			continue;
		}
		if (GetPadTrg(padNum, cTaskSceneBase::EKey::eUP))
		{
			if (array[i][1] != -1)
			{
				nSelectNow() = array[i][1];
			}
		}
		else if (GetPadTrg(padNum, cTaskSceneBase::EKey::eDOWN))
		{
			if (array[i][2] != -1)
			{
				nSelectNow() = array[i][2];
			}
		}
		else if (GetPadTrg(padNum, cTaskSceneBase::EKey::eLEFT))
		{
			if (array[i][3] != -1)
			{
				nSelectNow() = array[i][3];
			}
		}
		else if (GetPadTrg(padNum, cTaskSceneBase::EKey::eRIGHT) && array[i][4] != -1)
		{
			nSelectNow() = array[i][4];
		}
		break;
	}
}

void cTaskSceneAdd::SelectProc(const int& padNum)
{
	if (nSelectNow() != nSelectOld())
	{
		AllWait();
		cTaskSceneBase::PlaySndSe(ESEKind::RESIDENT_SND_SE01_SELECT);
	}
	pGetHitObj(nSelectNow())->Point();
}

void cTaskSceneAdd::DecideCheck(const int& padNum)
{
	int array[2][2] = {{2, 1}, {-1, -1}};
	if (!InGamePadManager::GetEnterKeyDown(padNum))
	{
		return;
	}
	for (int i = 0; array[i][0] != -1; i++)
	{
		if (nSelectNow() == array[i][0])
		{
			nState() = array[i][1];
			cTaskSceneBase::PlaySndSe(ESEKind::RESIDENT_SND_SE02_DECIDE);
			break;
		}
	}
}

void cTaskSceneAdd::Appear()
{
	pTaskConsAddWnd()->Appear();
	pTaskConsOk()->Appear();
	pTaskConsString()->Appear();
	for (int i = 0; i < 4; i++)
	{
		pTaskConsBigCtrl(i)->Appear();
	}
}

bool cTaskSceneAdd::IsAppearEnd()
{
	bool flag = true;
	if (flag)
	{
		flag = pTaskConsAddWnd()->IsAppearEnd();
	}
	if (flag)
	{
		flag = pTaskConsOk()->IsAppearEnd();
	}
	if (flag)
	{
		flag = pTaskConsString()->IsAppearEnd();
	}
	for (int i = 0; i < 4; i++)
	{
		if (flag)
		{
			flag = pTaskConsBigCtrl(i)->IsAppearEnd();
		}
	}
	if (flag)
	{
		return true;
	}
	return false;
}

void cTaskSceneAdd::Delete()
{
	pTaskConsAddWnd()->Delete();
	pTaskConsOk()->Delete();
	pTaskConsString()->Delete();
	for (int i = 0; i < 4; i++)
	{
		pTaskConsBigCtrl(i)->Delete();
	}
}

bool cTaskSceneAdd::IsDeleteEnd()
{
	bool flag = true;
	if (flag)
	{
		flag = pTaskConsAddWnd()->IsDeleteEnd();
	}
	if (flag)
	{
		flag = pTaskConsOk()->IsDeleteEnd();
	}
	if (flag)
	{
		flag = pTaskConsString()->IsDeleteEnd();
	}
	for (int i = 0; i < 4; i++)
	{
		if (flag)
		{
			flag = pTaskConsBigCtrl(i)->IsDeleteEnd();
		}
	}
	if (flag)
	{
		return true;
	}
	return false;
}

void cTaskSceneAdd::Wait()
{
	AllWait();
}

bool cTaskSceneAdd::ErrorCheck()
{
	if (nConnectCtrlCtOld() > nGetConnectCtrlCt())
	{
		nSetErrorCode(1u);
		nConnectCtrlCtOld() = 0;
		nUpdateCheckCt() = 0;
		return true;
	}
	if (nConnectCtrlCtOld() <= nGetConnectCtrlCt())
	{
		if ((long)nUpdateCheckCt() > 100L)
		{
			nConnectCtrlCtOld() = nGetConnectCtrlCt();
			nUpdateCheckCt() = 0;
		}
		else
		{
			nUpdateCheckCt()++;
		}
	}
	return false;
}

void cTaskSceneAdd::Reset()
{
	nState() = 0;
	nStep() = 0;
	nSelectNow() = (nSelectOld() = 2);
	nUpdateCheckCt() = 0;
	nConnectCtrlCtOld() = 0;
}

void cTaskSceneAdd::Select()
{
	if (nState() == 0)
	{
		cTaskSceneBase::Select();
	}
}

void cTaskSceneAdd::ActiveCtrlCheck()
{
	for (int i = 0; i < 4; i++)
	{
		if (nGetConnectCtrl(i) != -1)
		{
			pTaskConsBigCtrl(i)->nSetCtrlNo((uint)nGetConnectCtrl(i));
			pTaskConsBigCtrl(i)->bActive(true);
		}
		else
		{
			pTaskConsBigCtrl(i)->bActive(false);
		}
	}
}

bool cTaskSceneAdd::Execute(cJetTask* pParam)
{
	ActiveCtrlCheck();
	switch (nState())
	{
	case 1:
		if (nStep() == 0)
		{
			pTaskConsOk()->Decide();
			nStep()++;
		}
		break;
	default:
		CustomKingdomUtil::Assert("error");
		break;
	case 0:
		break;
	}
	return true;
}

bool cTaskSceneAdd::IsOkDecide()
{
	if (pTaskConsOk()->IsDecideEnd())
	{
		return true;
	}
	return false;
}

}
