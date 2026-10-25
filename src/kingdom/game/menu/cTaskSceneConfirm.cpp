#include "kingdom/game/menu/cTaskSceneConfirm.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/menu/cTaskConsAddWnd.hpp"
#include "kingdom/game/menu/cTaskConsBack.hpp"
#include "kingdom/game/menu/cTaskConsStart.hpp"
#include "kingdom/game/menu/cTaskConsString.hpp"
#include <ESEKind.hpp>
#include <InGamePadManager.hpp>

namespace kingdom::game::menu
{

int& cTaskSceneConfirm::nState(int nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 4, "error");
	return m_nState[nNo];
}

int& cTaskSceneConfirm::nStep(int nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 4, "error");
	return m_nStep[nNo];
}

int& cTaskSceneConfirm::nSelectNow(int nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 4, "error");
	return m_nSelectNowArray[nNo];
}

int& cTaskSceneConfirm::nSelectOld(int nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 4, "error");
	return m_nSelectOldArray[nNo];
}

int& cTaskSceneConfirm::nPerson()
{
	return m_nPerson;
}

cTaskConsAddWnd*& cTaskSceneConfirm::pTaskConsConfirmWnd()
{
	return m_pTaskConsConfirmWnd;
}

cTaskConsBack*& cTaskSceneConfirm::pTaskConsBack()
{
	return m_pTaskConsBack;
}

cTaskConsString*& cTaskSceneConfirm::pTaskConsString()
{
	return m_pTaskConsString;
}

cTaskConsStart*& cTaskSceneConfirm::pTaskConsStart(int nNo)
{
	CustomKingdomUtil::Assert(0 <= nNo && nNo < 4, "error");
	return m_pTaskConsStart[nNo];
}

cTaskSceneConfirm::~cTaskSceneConfirm()
{
}

cTaskSceneConfirm::cTaskSceneConfirm(int nId, string AnitaKey, string SelectKey)
	: cTaskSceneBase(nId)
{
	nPerson() = 0;
	RegistAnita(AnitaKey, SelectKey);
	for (int i = 0; i < 4; i++)
	{
		nState(i) = 0;
		nStep(i) = 0;
		nSelectNow(i) = (nSelectOld(i) = 3 + i);
	}
}

void cTaskSceneConfirm::RegistAnita(string AnitaKey, string SelectKey)
{
	pTaskConsBack() = new cTaskConsBack(2, AnitaKey);
	CustomKingdomUtil::Assert(pTaskConsBack() != nullptr);
	pTaskConsBack()->nDrawPriority(65541u);
	ConnectChild(pTaskConsBack());
	pTaskConsConfirmWnd() = new cTaskConsAddWnd(1, AnitaKey);
	CustomKingdomUtil::Assert(pTaskConsConfirmWnd() != nullptr);
	pTaskConsConfirmWnd()->nDrawPriority(65540u);
	ConnectChild(pTaskConsConfirmWnd());
	int num = 3;
	int num2 = 0;
	while (num <= 6)
	{
		pTaskConsStart(num2) = new cTaskConsStart(num, AnitaKey, SelectKey, (uint)num2);
		CustomKingdomUtil::Assert(pTaskConsStart(num2) != nullptr);
		pTaskConsStart(num2)->nDrawPriority((uint)(65542 + num2));
		ConnectChild(pTaskConsStart(num2));
		num++;
		num2++;
	}
	pTaskConsString() = new cTaskConsString(7, AnitaKey);
	CustomKingdomUtil::Assert(pTaskConsString() != nullptr);
	pTaskConsString()->nDrawPriority(65547u);
	pTaskConsString()->nSetStringIdx(2);
	ConnectChild(pTaskConsString());
}

void cTaskSceneConfirm::AddHitArray()
{
	vecHit().push_back(pTaskConsBack());
	for (int i = 0; (long)i < (long)pMatchingData()->nNum; i++)
	{
		vecHit().push_back(pTaskConsStart(i));
	}
}

void cTaskSceneConfirm::AllWait()
{
	pTaskConsBack()->Wait();
	pTaskConsString()->Wait();
	pTaskConsStart(nPerson())->Wait();
}

bool cTaskSceneConfirm::DpdSelect(const int& padNum)
{
	return false;
}

void cTaskSceneConfirm::KeySelect(const int& padNum)
{
	int array[4][3][3] = {{{2, 3, 3}, {3, 2, 2}, {-1, -1, -1}}, {{2, 4, 4}, {4, 2, 2}, {-1, -1, -1}}, {{2, 5, 5}, {5, 2, 2}, {-1, -1, -1}}, {{2, 6, 6}, {6, 2, 2}, {-1, -1, -1}}};
	int num = nPerson();
	for (int i = 0; array[num][i][0] != -1; i++)
	{
		if (nSelectNow(num) != array[num][i][0])
		{
			continue;
		}
		if (GetPadTrg(padNum, cTaskSceneBase::EKey::eUP))
		{
			if (array[num][i][1] != -1)
			{
				nSelectNow(num) = array[num][i][1];
			}
		}
		else if (GetPadTrg(padNum, cTaskSceneBase::EKey::eDOWN))
		{
			if (array[num][i][2] != -1)
			{
				nSelectNow(num) = array[num][i][2];
			}
		}
		else if (GetPadTrg(padNum, cTaskSceneBase::EKey::eLEFT))
		{
			if (array[num][i][3] != -1)
			{
				nSelectNow(num) = array[num][i][3];
			}
		}
		else if (GetPadTrg(padNum, cTaskSceneBase::EKey::eRIGHT) && array[num][i][4] != -1)
		{
			nSelectNow(num) = array[num][i][4];
		}
		break;
	}
}

void cTaskSceneConfirm::SelectProc(const int& padNum)
{
	if (InGamePadManager::GetCancelKeyDown(padNum))
	{
		nSelectNow(nPerson()) = 2;
	}
	if (nSelectNow(nPerson()) != nSelectOld(nPerson()))
	{
		AllWait();
		cTaskSceneBase::PlaySndSe(ESEKind::RESIDENT_SND_SE01_SELECT);
	}
	CustomKingdomUtil::Assert(pGetHitObj(nSelectNow(nPerson())) != nullptr);
	pGetHitObj(nSelectNow(nPerson()))->Point();
}

void cTaskSceneConfirm::DecideCheck(const int& padNum)
{
	int array[6][2] = {{2, 1}, {3, 2}, {4, 2}, {5, 2}, {6, 2}, {-1, -1}};
	if (InGamePadManager::GetCancelKeyDown(padNum) && nSelectNow(nPerson()) == 2 && nSelectOld(nPerson()) == 2)
	{
		nState(nPerson()) = 1;
		cTaskSceneBase::PlaySndSe(ESEKind::RESIDENT_SND_SE03_CANCEL, (int)pMatchingData()->Info[nPerson()]->nCtrl);
	}
	else
	{
		if (!InGamePadManager::GetEnterKeyDown(padNum))
		{
			return;
		}
		for (int i = 0; array[i][0] != -1; i++)
		{
			if (nSelectNow(nPerson()) == array[i][0])
			{
				nState(nPerson()) = array[i][1];
				cTaskSceneBase::PlaySndSe(ESEKind::RESIDENT_SND_SE02_DECIDE, (int)pMatchingData()->Info[nPerson()]->nCtrl);
				break;
			}
		}
	}
}

void cTaskSceneConfirm::Appear()
{
	pTaskConsBack()->Appear();
	pTaskConsConfirmWnd()->Appear();
	pTaskConsString()->Appear();
	for (int i = 0; (long)i < (long)pMatchingData()->nNum; i++)
	{
		pTaskConsStart(i)->Appear();
		pTaskConsStart(i)->RegistMii(pMatchingData()->Info[i]);
	}
}

bool cTaskSceneConfirm::IsAppearEnd()
{
	bool flag = true;
	if (flag)
	{
		flag = pTaskConsBack()->IsAppearEnd();
	}
	if (flag)
	{
		flag = pTaskConsConfirmWnd()->IsAppearEnd();
	}
	if (flag)
	{
		flag = pTaskConsString()->IsAppearEnd();
	}
	for (int i = 0; (long)i < (long)pMatchingData()->nNum; i++)
	{
		if (flag)
		{
			flag = pTaskConsStart(i)->IsAppearEnd();
		}
	}
	if (flag)
	{
		return true;
	}
	return false;
}

void cTaskSceneConfirm::Wait()
{
	pTaskConsBack()->Wait();
	pTaskConsString()->Wait();
	for (int i = 0; (long)i < (long)pMatchingData()->nNum; i++)
	{
		pTaskConsStart(i)->Wait();
	}
}

void cTaskSceneConfirm::Delete()
{
	pTaskConsBack()->Delete();
	pTaskConsConfirmWnd()->Delete();
	pTaskConsString()->Delete();
	for (int i = 0; (long)i < (long)pMatchingData()->nNum; i++)
	{
		pTaskConsStart(i)->Delete();
	}
}

bool cTaskSceneConfirm::IsDeleteEnd()
{
	bool flag = true;
	if (flag)
	{
		flag = pTaskConsBack()->IsDeleteEnd();
	}
	if (flag)
	{
		flag = pTaskConsConfirmWnd()->IsDeleteEnd();
	}
	if (flag)
	{
		flag = pTaskConsString()->IsDeleteEnd();
	}
	for (int i = 0; (long)i < (long)pMatchingData()->nNum; i++)
	{
		if (flag)
		{
			flag = pTaskConsStart(i)->IsDeleteEnd();
		}
	}
	if (flag)
	{
		return true;
	}
	return false;
}

bool cTaskSceneConfirm::ErrorCheck()
{
	for (int i = 0; (long)i < (long)pMatchingData()->nNum; i++)
	{
		if ((long)pMatchingData()->Info[i]->nCtrl != (long)nGetConnectCtrl(i))
		{
			nSetErrorCode(1u);
			return true;
		}
	}
	return false;
}

void cTaskSceneConfirm::Reset()
{
	for (int i = 0; i < 4; i++)
	{
		nSelectNow(i) = (nSelectOld(i) = 3 + i);
	}
	for (int j = 0; j < 4; j++)
	{
		nState(j) = 0;
		nStep(j) = 0;
		nSelectNow(j) = (nSelectOld(j) = 3 + j);
	}
	for (int k = 0; (long)k < (long)pMatchingData()->nNum; k++)
	{
		pTaskConsStart(k)->nSetCtrlNo(pMatchingData()->Info[k]->nCtrl);
	}
	AddHitArray();
}

void cTaskSceneConfirm::Select()
{
	if (pTaskConsBack()->IsDecide() || pTaskConsBack()->IsDecideEnd())
	{
		return;
	}
	for (int i = 0; (long)i < (long)pMatchingData()->nNum; i++)
	{
		nPerson() = i;
		int padNum = InGamePadManager::GetPadNum((int)pMatchingData()->Info[nPerson()]->nCtrl);
		if (nState(nPerson()) == 0)
		{
			nSelectNow(nPerson()) = nSelectOld(nPerson());
			if (!DpdSelect(padNum))
			{
				KeySelect(padNum);
			}
			SelectProc(padNum);
			DecideCheck(padNum);
			nSelectOld(nPerson()) = nSelectNow(nPerson());
		}
		else if (nState(nPerson()) == 2 && InGamePadManager::GetCancelKeyDown(padNum))
		{
			nState(nPerson()) = 3;
			nStep(nPerson()) = 0;
		}
	}
}

bool cTaskSceneConfirm::Execute(cJetTask* pParam)
{
	for (int i = 0; (long)i < (long)pMatchingData()->nNum; i++)
	{
		switch (nState(i))
		{
		case 2:
			if (nStep(i) == 0)
			{
				pTaskConsStart(i)->Decide();
				nStep(i)++;
			}
			break;
		case 3:
			if (nStep(i) == 0)
			{
				pTaskConsStart(i)->Cancel();
				pTaskConsStart(i)->Point();
				nState(i) = 0;
			}
			break;
		case 1:
			if (nStep(i) == 0)
			{
				pTaskConsBack()->Decide();
				nStep(i)++;
			}
			break;
		default:
			CustomKingdomUtil::Assert("error");
			break;
		case 0:
			break;
		}
	}
	return true;
}

bool cTaskSceneConfirm::IsAllDecide()
{
	bool flag = true;
	for (int i = 0; (long)i < (long)pMatchingData()->nNum; i++)
	{
		flag = pTaskConsStart(i)->IsDecideEnd();
		if (!flag)
		{
			break;
		}
	}
	return flag;
}

bool cTaskSceneConfirm::IsBackDecide()
{
	if (pTaskConsBack()->IsDecideEnd())
	{
		return true;
	}
	return false;
}

}
