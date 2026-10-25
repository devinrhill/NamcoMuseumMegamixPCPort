#include "kingdom/game/menu/cTaskScenePerson.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/menu/cTaskConsAdd.hpp"
#include "kingdom/game/menu/cTaskConsBack.hpp"
#include "kingdom/game/menu/cTaskConsCtrl.hpp"
#include "kingdom/game/menu/cTaskConsPerson.hpp"
#include "kingdom/game/menu/cTaskConsPersonWnd.hpp"
#include "kingdom/game/menu/cTaskConsString.hpp"
#include "MotosUIManager.hpp"
#include "MotosUIMotosMultiHome.hpp"
#include <ESEKind.hpp>
#include <InGamePadManager.hpp>

namespace kingdom::game::menu
{

int& cTaskScenePerson::nPerson()
{
	return m_nPerson;
}

bool& cTaskScenePerson::bErrorPerson()
{
	return m_bErrorPerson;
}

int& cTaskScenePerson::nUpdateCheckCt()
{
	return m_nUpdateCheckCt;
}

int& cTaskScenePerson::nConnectCtrlCtOld()
{
	return m_nConnectCtrlCtOld;
}

cTaskConsPersonWnd*& cTaskScenePerson::pTaskConsPersonWnd()
{
	return m_pTaskConsPersonWnd;
}

cTaskConsBack*& cTaskScenePerson::pTaskConsBack()
{
	return m_pTaskConsBack;
}

cTaskConsAdd*& cTaskScenePerson::pTaskConsAdd()
{
	return m_pTaskConsAdd;
}

cTaskConsString*& cTaskScenePerson::pTaskConsString()
{
	return m_pTaskConsString;
}

cTaskConsCtrl*& cTaskScenePerson::pTaskConsCtrl(int nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 4, "error");
	return m_pTaskConsCtrl[nNo];
}

cTaskConsPerson*& cTaskScenePerson::pTaskConsPerson(int nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 3, "error");
	return m_pTaskConsPerson[nNo];
}

cTaskScenePerson::~cTaskScenePerson()
{
}

cTaskScenePerson::cTaskScenePerson(int nId, string AnitaKey)
	: cTaskSceneBase(nId)
{
	UnityUI = MotosUIManager::Get<MotosUIMotosMultiHome>(MotosUIManager::EUIIndex::MotosMultiHome);
	nState() = 0;
	nPerson() = 0;
	bErrorPerson() = false;
	nUpdateCheckCt() = 0;
	nConnectCtrlCtOld() = 0;
	RegistAnita(AnitaKey);
	AddHitArray();
	nSelectNow() = (nSelectOld() = 3);
}

void cTaskScenePerson::RegistAnita(string AnitaKey)
{
	pTaskConsBack() = new cTaskConsBack(2, AnitaKey);
	CustomKingdomUtil::Assert(pTaskConsBack() != nullptr);
	pTaskConsBack()->nDrawPriority(65540u);
	ConnectChild(pTaskConsBack());
	pTaskConsBack()->UnityUI = UnityUI;
	pTaskConsPersonWnd() = new cTaskConsPersonWnd(1, AnitaKey);
	CustomKingdomUtil::Assert(pTaskConsPersonWnd() != nullptr);
	pTaskConsPersonWnd()->nDrawPriority(65541u);
	ConnectChild(pTaskConsPersonWnd());
	pTaskConsPersonWnd()->UnityUI = UnityUI;
	int num = 3;
	int num2 = 0;
	while (num <= 5)
	{
		pTaskConsPerson(num2) = new cTaskConsPerson(num, AnitaKey, (uint)num2);
		CustomKingdomUtil::Assert(pTaskConsPerson(num2) != nullptr);
		pTaskConsPerson(num2)->nDrawPriority((uint)(65542 + num2));
		ConnectChild(pTaskConsPerson(num2));
		pTaskConsPerson(num2)->UnityUI = UnityUI;
		num++;
		num2++;
	}
	int num3 = 6;
	int num4 = 0;
	while (num3 <= 9)
	{
		pTaskConsCtrl(num4) = new cTaskConsCtrl(num3, AnitaKey, (uint)num4);
		CustomKingdomUtil::Assert(pTaskConsCtrl(num4) != nullptr);
		pTaskConsCtrl(num4)->nDrawPriority((uint)(65545 + num4));
		ConnectChild(pTaskConsCtrl(num4));
		pTaskConsCtrl(num4)->UnityUI = UnityUI;
		num3++;
		num4++;
	}
	pTaskConsAdd() = new cTaskConsAdd(10, AnitaKey);
	CustomKingdomUtil::Assert(pTaskConsAdd() != nullptr);
	pTaskConsAdd()->nDrawPriority(65549u);
	ConnectChild(pTaskConsAdd());
	pTaskConsAdd()->UnityUI = UnityUI;
}

void cTaskScenePerson::AddHitArray()
{
	vecHit().push_back(pTaskConsBack());
	vecHit().push_back(pTaskConsAdd());
	for (int i = 0; i < 3; i++)
	{
		vecHit().push_back(pTaskConsPerson(i));
	}
}

void cTaskScenePerson::AllWait()
{
	pTaskConsBack()->Wait();
	pTaskConsAdd()->Wait();
	for (int i = 0; i < 4; i++)
	{
		pTaskConsCtrl(i)->Wait();
	}
	for (int j = 0; j < 3; j++)
	{
		pTaskConsPerson(j)->Wait();
	}
}

void cTaskScenePerson::KeySelect(const int& padNum)
{
	int num = 10;
	int num2 = 10;
	num = 5;
	num2 = 4;
	int array[6][5] = {{2, 5, 3, -1, -1}, {3, 2, 5, -1, -1}, {4, 2, 2, num, 5}, {5, 3, 2, 4, num2}, {10, 2, 2, 5, 4}, {-1, -1, -1, -1, -1}};
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

void cTaskScenePerson::SelectProc(const int& padNum)
{
	if (InGamePadManager::GetCancelKeyDown(padNum))
	{
		nSelectNow() = 2;
	}
	if (nSelectNow() != nSelectOld())
	{
		Wait();
		cTaskSceneBase::PlaySndSe(ESEKind::RESIDENT_SND_SE01_SELECT);
	}
	if (3 <= nSelectNow() && nSelectNow() <= 5)
	{
		nPerson() = (int)(static_cast<cTaskConsPerson*>(pGetHitObj(nSelectNow())))->nGetType();
		pMatchingData()->nNum = (uint)(nPerson() + 2);
	}
	CustomKingdomUtil::Assert(pGetHitObj(nSelectNow()) != nullptr, "nullpo");
	pGetHitObj(nSelectNow())->Point();
}

void cTaskScenePerson::DecideCheck(const int& padNum)
{
	int array[6][2] = {{2, 1}, {3, 3}, {4, 3}, {5, 3}, {10, 2}, {-1, -1}};
	if (InGamePadManager::GetCancelKeyDown(padNum) && nSelectNow() == 2 && nSelectOld() == 2)
	{
		nState() = 1;
		cTaskSceneBase::PlaySndSe(ESEKind::RESIDENT_SND_SE03_CANCEL);
	}
	else
	{
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
}

void cTaskScenePerson::Appear()
{
	pTaskConsBack()->Appear();
	pTaskConsAdd()->Appear();
	pTaskConsPersonWnd()->Appear();
	for (int i = 0; i < 4; i++)
	{
		pTaskConsCtrl(i)->Appear();
	}
	for (int j = 0; j < 3; j++)
	{
		pTaskConsPerson(j)->Appear();
	}
	if (UnityUI != nullptr)
	{
		UnityUI->SetEnable(true);
	}
	if (UnityUI != nullptr)
	{
		UnityUI->PlayBaseAnim(MotosUIMotosMultiHome::EBaseAnim::In);
	}
}

bool cTaskScenePerson::IsAppearEnd()
{
	bool flag = true;
	if (flag)
	{
		flag = pTaskConsBack()->IsAppearEnd();
	}
	if (flag)
	{
		flag = pTaskConsAdd()->IsAppearEnd();
	}
	if (flag)
	{
		flag = pTaskConsPersonWnd()->IsAppearEnd();
	}
	for (int i = 0; i < 4; i++)
	{
		if (flag)
		{
			flag = pTaskConsCtrl(i)->IsAppearEnd();
		}
	}
	for (int j = 0; j < 3; j++)
	{
		if (flag)
		{
			flag = pTaskConsPerson(j)->IsAppearEnd();
		}
	}
	if (nullptr != UnityUI && flag)
	{
		flag = !UnityUI->IsPlayBaseAnim(MotosUIMotosMultiHome::EBaseAnim::In);
	}
	if (flag)
	{
		return true;
	}
	return false;
}

void cTaskScenePerson::Delete()
{
	pTaskConsBack()->Delete();
	pTaskConsAdd()->Delete();
	pTaskConsPersonWnd()->Delete();
	for (int i = 0; i < 4; i++)
	{
		pTaskConsCtrl(i)->Delete();
	}
	for (int j = 0; j < 3; j++)
	{
		pTaskConsPerson(j)->Delete();
	}
	if (UnityUI != nullptr)
	{
		UnityUI->PlayBaseAnim(MotosUIMotosMultiHome::EBaseAnim::Out);
	}
}

bool cTaskScenePerson::IsDeleteEnd()
{
	bool flag = true;
	if (flag)
	{
		flag = pTaskConsBack()->IsDeleteEnd();
	}
	if (flag)
	{
		flag = pTaskConsAdd()->IsDeleteEnd();
	}
	if (flag)
	{
		flag = pTaskConsPersonWnd()->IsDeleteEnd();
	}
	for (int i = 0; i < 4; i++)
	{
		if (flag)
		{
			flag = pTaskConsCtrl(i)->IsDeleteEnd();
		}
	}
	for (int j = 0; j < 3; j++)
	{
		if (flag)
		{
			flag = pTaskConsPerson(j)->IsDeleteEnd();
		}
	}
	if (nullptr != UnityUI)
	{
		if (flag)
		{
			flag = !UnityUI->IsPlayBaseAnim(MotosUIMotosMultiHome::EBaseAnim::Out);
		}
		if (flag)
		{
			UnityUI->SetEnable(false);
		}
	}
	if (flag)
	{
		return true;
	}
	return false;
}

bool cTaskScenePerson::ErrorCheck()
{
	if (bErrorPerson())
	{
		nSetErrorCode(4u);
		bErrorPerson() = false;
		return true;
	}
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

void cTaskScenePerson::Wait()
{
	AllWait();
}

void cTaskScenePerson::Reset()
{
	nState() = 0;
	nStep() = 0;
	nSelectNow() = (nSelectOld() = 3);
	nUpdateCheckCt() = 0;
	nConnectCtrlCtOld() = 0;
}

void cTaskScenePerson::Select()
{
	if (nState() == 0)
	{
		cTaskSceneBase::Select();
	}
}

void cTaskScenePerson::ActiveCtrlCheck()
{
	for (int i = 0; i < 4; i++)
	{
		if (nGetConnectCtrl(i) != -1)
		{
			pTaskConsCtrl(i)->nSetCtrlNo((uint)nGetConnectCtrl(i));
			pTaskConsCtrl(i)->bActive(true);
		}
		else
		{
			pTaskConsCtrl(i)->bActive(false);
		}
	}
}

bool cTaskScenePerson::Execute(cJetTask* pParam)
{
	ActiveCtrlCheck();
	switch (nState())
	{
	case 3:
		if (nStep() != 0)
		{
			break;
		}
		if ((long)pMatchingData()->nNum <= (long)nGetConnectCtrlCt())
		{
			pTaskConsPerson(nPerson())->Decide();
			for (int i = 0; (long)i < (long)pMatchingData()->nNum; i++)
			{
				CustomKingdomUtil::Assert(nGetConnectCtrl(i) != -1, "error");
				pMatchingData()->Info[i]->nCtrl = (uint)nGetConnectCtrl(i);
			}
			nStep()++;
		}
		else
		{
			nState() = 0;
			nStep() = 0;
			bErrorPerson() = true;
		}
		break;
	case 2:
		if (nStep() == 0)
		{
			pTaskConsAdd()->Decide();
			nStep()++;
		}
		break;
	case 1:
		if (nStep() == 0)
		{
			pTaskConsBack()->Decide();
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

bool cTaskScenePerson::IsAddDecide()
{
	if (pTaskConsAdd()->IsDecideEnd())
	{
		return true;
	}
	return false;
}

bool cTaskScenePerson::IsBackDecide()
{
	if (pTaskConsBack()->IsDecideEnd())
	{
		return true;
	}
	return false;
}

bool cTaskScenePerson::IsPersonDecide()
{
	for (int i = 0; i < 3; i++)
	{
		if (pTaskConsPerson(i)->IsDecideEnd())
		{
			return true;
		}
	}
	return false;
}

}
