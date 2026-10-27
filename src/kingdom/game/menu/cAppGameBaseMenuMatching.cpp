#include "GamePad.hpp"
#include "kingdom/game/menu/cAppGameBaseMenuMatching.hpp"

#include <limits>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/cTaskScnRoot.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/game/menu/cParamMessage.hpp"
#include "kingdom/game/menu/cTaskConsBg.hpp"
#include "kingdom/game/menu/cTaskConsError.hpp"
#include "kingdom/game/menu/cTaskConsInfoWnd.hpp"
#include "kingdom/game/menu/cTaskSceneAdd.hpp"
#include "kingdom/game/menu/cTaskSceneBase.hpp"
#include "kingdom/game/menu/cTaskSceneConfirm.hpp"
#include "kingdom/game/menu/cTaskScenePerson.hpp"
#include "kingdom/sMatchingData.hpp"
#include "kingdom/MsgTable.hpp"
#include <ESEKind.hpp>
#include <InGamePadManager.hpp>

namespace kingdom::game::menu
{

uint& cAppGameBaseMenuMatching::nMenuJob()
{
	return m_nMenuJob;
}

uint& cAppGameBaseMenuMatching::nMultiJob()
{
	return m_nMultiJob;
}

int& cAppGameBaseMenuMatching::nState()
{
	return m_nState;
}

int& cAppGameBaseMenuMatching::nStep()
{
	return m_nStep;
}

bool& cAppGameBaseMenuMatching::bDataBaseBroken()
{
	return m_bDataBaseBroken;
}

uint& cAppGameBaseMenuMatching::nRecoveryState()
{
	return m_nRecoveryMode;
}

bool& cAppGameBaseMenuMatching::bRecoveryReset()
{
	return m_bRecoveryReset;
}

bool& cAppGameBaseMenuMatching::bErrorFlag()
{
	return m_bErrorFlag;
}

bool& cAppGameBaseMenuMatching::bConfirmDecide()
{
	return m_bConfirmDecide;
}

int& cAppGameBaseMenuMatching::nConnectCtrl(int nPlayerNo)
{
	CustomKingdomUtil::Assert(0 <= nPlayerNo && nPlayerNo < 4, "error");
	return m_nConnectCtrl[nPlayerNo];
}

int& cAppGameBaseMenuMatching::nConnectCtrlCt()
{
	return m_nConnectCtrlCt;
}

sMatchingData*& cAppGameBaseMenuMatching::pMatchingData()
{
	return m_pMatchingData;
}

cParamMessage*& cAppGameBaseMenuMatching::pParamMessage()
{
	return m_pParamMessage;
}

cTaskFade*& cAppGameBaseMenuMatching::pTaskFade()
{
	return m_pTaskFade;
}

cTaskConsBg*& cAppGameBaseMenuMatching::pTaskConsBg()
{
	return m_pTaskConsBg;
}

cTaskConsInfoWnd*& cAppGameBaseMenuMatching::pTaskConsInfoWnd()
{
	return m_pTaskConsInfoWnd;
}

cTaskConsError*& cAppGameBaseMenuMatching::pTaskConsError()
{
	return m_pTaskConsError;
}

cTaskSceneBase*& cAppGameBaseMenuMatching::pTaskSceneError()
{
	return m_pTaskSceneError;
}

cTaskScenePerson*& cAppGameBaseMenuMatching::pTaskScenePerson()
{
	return m_pTaskScenePerson;
}

cTaskSceneAdd*& cAppGameBaseMenuMatching::pTaskSceneAdd()
{
	return m_pTaskSceneAdd;
}

cTaskSceneConfirm*& cAppGameBaseMenuMatching::pTaskSceneConfirm()
{
	return m_pTaskSceneConfirm;
}

cAppGameBaseMenuMatching::cAppGameBaseMenuMatching()
{
	Init();
}

void cAppGameBaseMenuMatching::Free()
{
	Term();
	cAppKingdom::Free();
}

sMatchingData* cAppGameBaseMenuMatching::pGetMatchingData()
{
	return pMatchingData();
}

int cAppGameBaseMenuMatching::nGetConnectCtrl(int nPlayerNo)
{
	CustomKingdomUtil::Assert(0 <= nPlayerNo && nPlayerNo < 4, "error");
	return nConnectCtrl(nPlayerNo);
}

int cAppGameBaseMenuMatching::nGetConnectCtrlCt()
{
	return nConnectCtrlCt();
}

bool cAppGameBaseMenuMatching::IsConfirmDecide()
{
	return bConfirmDecide();
}

void cAppGameBaseMenuMatching::Init()
{
	GamePad::SetStickDpad(true);   // 0x801cf828 starts with 0x800219d4(1)
	nState() = 0;
	nStep() = 0;
	bErrorFlag() = false;
	bConfirmDecide() = false;
	bRecoveryReset() = true;
	for (int i = 0; i < 4; i++)
	{
		nConnectCtrl(i) = -1;
	}
	nConnectCtrl(0) = 0;
	nRecoveryState() = 0u;
	nMenuJob() = std::numeric_limits<uint>::max();
	nMultiJob() = std::numeric_limits<uint>::max();
}

void cAppGameBaseMenuMatching::RegistData(string PointKey, string AnitaKey, string CameraKey, string SetKey)
{
	if (bDataBaseBroken())
	{
		pTaskScnRoot() = new cTaskScnRoot();
		CustomKingdomUtil::Assert(pTaskScnRoot() != nullptr);
		ConnectChild(pTaskScnRoot());
		pMatchingData() = new sMatchingData();
		for (uint num = 0u; num < 4; num++)
		{
			pMatchingData()->Info[num]->nCtrl = num;
		}
		pTaskFade() = new cTaskFade();
		CustomKingdomUtil::Assert(pTaskFade() != nullptr);
		ConnectChild(pTaskFade());
		pParamMessage() = new cParamMessage();
		CustomKingdomUtil::Assert(pParamMessage() != nullptr);
		pParamMessage()->RegistData("RESIDENT_MSG_MATCHING");   // DOL 0x801cf8bc: CRC 0x17855010
		pTaskConsBg() = new cTaskConsBg(0, AnitaKey);
		CustomKingdomUtil::Assert(pTaskConsBg() != nullptr);
		pTaskConsBg()->nDrawPriority(65537u);
		ConnectChild(pTaskConsBg());
		pTaskConsError() = new cTaskConsError(2, AnitaKey, pParamMessage());
		CustomKingdomUtil::Assert(pTaskConsError() != nullptr);
		pTaskConsError()->nDrawPriority(2147483680u);
		pTaskConsError()->nSetErrorCode(2u);
		ConnectChild(pTaskConsError());
		return;
	}
	pTaskScnRoot() = new cTaskScnRoot();
	CustomKingdomUtil::Assert(pTaskScnRoot() != nullptr);
	ConnectChild(pTaskScnRoot());
	pMatchingData() = new sMatchingData();
	for (uint num2 = 0u; num2 < 4; num2++)
	{
		pMatchingData()->Info[num2]->nCtrl = num2;
	}
	pTaskFade() = new cTaskFade();
	CustomKingdomUtil::Assert(pTaskFade() != nullptr);
	ConnectChild(pTaskFade());
	// DOL 0x801cf8bc (after the scene camera +0x2b4): a cTaskMenuCursor per pad on the PointKey layout, idle anim 2 i, press anim 2 i + 1 on A B Z
	// (table 0x8037fd48 = 0..7), disabled (+0x1e1 = 0) until ConnectCursor enables the connected ones, priority 0x80000022
	for (int i = 0; i < 4; i++)
	{
		m_pCursor[i] = new kingdom::cTaskCursor();
		m_pCursor[i]->nDrawPriority(0x80000000u);
		m_pCursor[i]->Init(i, PointKey.c_str(), 2 * i);
		m_pCursor[i]->AddButton(0x860u, 2 * i + 1);
		m_pCursor[i]->SetEnable(false);
		m_pCursor[i]->SetPadNo(i);
		m_pCursor[i]->nDrawPriority(0x80000022u);
		ConnectChild(m_pCursor[i]);
	}
	pParamMessage() = new cParamMessage();
	CustomKingdomUtil::Assert(pParamMessage() != nullptr);
	pParamMessage()->RegistData("RESIDENT_MSG_MATCHING");   // DOL 0x801cf8bc: CRC 0x17855010
	pTaskConsBg() = new cTaskConsBg(0, AnitaKey);
	CustomKingdomUtil::Assert(pTaskConsBg() != nullptr);
	pTaskConsBg()->nDrawPriority(65537u);
	ConnectChild(pTaskConsBg());
	pTaskConsInfoWnd() = new cTaskConsInfoWnd(1, AnitaKey);
	CustomKingdomUtil::Assert(pTaskConsInfoWnd() != nullptr);
	pTaskConsInfoWnd()->nDrawPriority(65538u);
	ConnectChild(pTaskConsInfoWnd());
	pTaskConsError() = new cTaskConsError(2, AnitaKey, pParamMessage());
	CustomKingdomUtil::Assert(pTaskConsError() != nullptr);
	pTaskConsError()->nDrawPriority(2147483680u);
	pTaskConsError()->nSetErrorCode(2u);
	ConnectChild(pTaskConsError());
	pTaskScenePerson() = new cTaskScenePerson(3, AnitaKey);
	CustomKingdomUtil::Assert(pTaskScenePerson() != nullptr);
	ConnectChild(pTaskScenePerson());
	pTaskSceneAdd() = new cTaskSceneAdd(4, AnitaKey);
	CustomKingdomUtil::Assert(pTaskSceneAdd() != nullptr);
	ConnectChild(pTaskSceneAdd());
	pTaskSceneConfirm() = new cTaskSceneConfirm(6, AnitaKey, SetKey);
	CustomKingdomUtil::Assert(pTaskSceneConfirm() != nullptr);
	ConnectChild(pTaskSceneConfirm());
	pTaskSceneError() = nullptr;
}

void cAppGameBaseMenuMatching::Term()
{
	GamePad::SetStickDpad(false);   // 0x801d0864 (the dtors) ends with 0x800219d4(0)
	pMatchingData() = nullptr;
	pParamMessage() = nullptr;
}

// Stand-in for the Mii / guest scene (cTaskSceneMii, cTaskMiiWii / cTaskMiiGuest; not ported: it shows RFL Mii faces, out of scope since the
// 39th session). In the DOL every player picks a Wii Mii or one of the six guests, and the name copied into the matching data is that Mii's
// (guest k: FUN_801d9b58 copies its name from the guest table). Here player i gets guest i: "Guest A".."Guest D" from RESIDENT_MSG_MII_NAME
// (rows eMII_DEFAULT_1..6, English column). UNVERIFIED: which guest a player would get without choosing.
void cAppGameBaseMenuMatching::GuestNames()
{
	MsgTable names;
	const bool ok = names.Load("RESIDENT_MSG_MII_NAME");
	for (uint i = 0; i < 4; i++)
	{
		if (pMatchingData()->Info[i] == nullptr) continue;
		std::string n = ok ? names.Get((int)i) : std::string();
		pMatchingData()->Info[i]->Name = n.empty() ? "PLAYER" + std::to_string(i + 1) : n;
	}
}

// inlined in DOL 0x801d0a78 (ConnectCtrlCheck): the KPAD status of the pad (get_pad1 +0xb0) reports an error (+0x5d not 0 and not -2 = busy) or
// no device (+0x5c dev_type 0xfd): natively = the channel is not connected
bool cAppGameBaseMenuMatching::_IsConnectError(int nCtrl)
{
	return !InGamePadManager::IsConnectPad(nCtrl);
}

bool cAppGameBaseMenuMatching::ErrorProc()
{
	bool result = false;
	switch (pTaskSceneError()->nGetId())
	{
	case 3:
		if (nGetErrorCode() == 1)
		{
			nRecoveryState() = 1u;
			result = true;
		}
		else if (nGetErrorCode() == 4)
		{
			nRecoveryState() = 1u;
			result = false;
		}
		break;
	case 4:
		if (nGetErrorCode() == 1)
		{
			nRecoveryState() = 2u;
			result = true;
		}
		break;
	case 5:
		if (nGetErrorCode() == 1)
		{
			nRecoveryState() = 1u;
			result = true;
		}
		else if (nGetErrorCode() == 14)
		{
			nRecoveryState() = 3u;
			result = false;
		}
		break;
	case 6:
		if (nGetErrorCode() == 1)
		{
			nRecoveryState() = 1u;
			result = true;
		}
		break;
	}
	return result;
}

void cAppGameBaseMenuMatching::ConnectCtrlCheck()
{
	for (int i = 0; i < 4; i++)
	{
		if (nConnectCtrl(i) != -1 && _IsConnectError(nConnectCtrl(i)))
		{
			nConnectCtrl(i) = -1;
		}
	}
	for (int j = 1; j < 3; j++)
	{
		if (nConnectCtrl(j) != -1)
		{
			continue;
		}
		for (int k = j + 1; k < 4; k++)
		{
			if (nConnectCtrl(k) != -1)
			{
				nConnectCtrl(j) = nConnectCtrl(k);
				nConnectCtrl(k) = -1;
				break;
			}
		}
	}
	if (_IsConnectError(0))
	{
		nConnectCtrl(0) = -1;
	}
	else
	{
		nConnectCtrl(0) = 0;
	}
	for (int l = 1; l < 4; l++)
	{
		if (_IsConnectError(l))
		{
			continue;
		}
		bool flag = false;
		for (int m = 0; m < 4; m++)
		{
			if (nConnectCtrl(m) == l)
			{
				flag = true;
				break;
			}
		}
		if (flag)
		{
			continue;
		}
		for (int n = 0; n < 4; n++)
		{
			if (nConnectCtrl(n) == -1)
			{
				nConnectCtrl(n) = l;
				break;
			}
		}
	}
	int num = 0;
	for (int num2 = 0; num2 < 4; num2++)
	{
		if (nConnectCtrl(num2) != -1)
		{
			num++;
		}
	}
	nConnectCtrlCt() = num;
}

// DOL 0x801d0d44 (inlined in Execute after ConnectCtrlCheck 0x801d0a78): cursor i follows controller nConnectCtrl(i) (vtable +0x58 SetPadNo) and is
// shown only for i < nConnectCtrlCt
void cAppGameBaseMenuMatching::ConnectCursor()
{
	for (int i = 0; i < 4; i++)
	{
		if (m_pCursor[i] == nullptr) continue;
		if (nConnectCtrl(i) != -1) m_pCursor[i]->SetPadNo(nConnectCtrl(i));
		m_pCursor[i]->SetEnable(i < nConnectCtrlCt());
	}
}

bool cAppGameBaseMenuMatching::Execute(cJetTask* pParam)
{
	ConnectCtrlCheck();
	ConnectCursor();
	uint n = 0u;
	if (nModeState(n) == 0)
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch2_0u:
			nModeStep(n) = 1u;
			pTaskFade()->Fade(EFadeKind::eFADE_TYPE_IN_BLACK);
			goto _switch2_1u;
		case 1u:
		_switch2_1u:
			if (pTaskFade()->IsEnd())
			{
				if (bDataBaseBroken())
				{
					NextMode(n, 8u);
				}
				else
				{
					NextMode(n, 1u);
				}
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch2_2u;
		case 2u:
		_switch2_2u:
			nModeStep(n) = 0u;
			nStep() = 0;
			break;
		}
	}
	else if (1 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch3_0u:
			nModeStep(n) = 1u;
			pTaskSceneError() = pTaskScenePerson();
			if (bRecoveryReset())
			{
				pTaskScenePerson()->Reset();
				pTaskScenePerson()->Appear();
				pTaskConsInfoWnd()->Appear();
			}
			else
			{
				nStep()++;
			}
			goto _switch3_1u;
		case 1u:
		_switch3_1u:
			if (nStep() == 0)
			{
				if (pTaskScenePerson()->IsAppearEnd())
				{
					pTaskScenePerson()->Wait();
					nStep()++;
				}
			}
			else if (nStep() == 1)
			{
				if (pTaskScenePerson()->ErrorCheck())
				{
					NextMode(n, 5u);
				}
				else
				{
					pTaskScenePerson()->Select();
					if (pTaskScenePerson()->IsPersonDecide())
					{
						pTaskScenePerson()->Delete();
						nStep() = 2;
					}
					if (pTaskScenePerson()->IsAddDecide())
					{
						pTaskScenePerson()->Delete();
						nStep() = 3;
					}
					if (pTaskScenePerson()->IsBackDecide())
					{
						pTaskScenePerson()->Delete();
						pTaskConsInfoWnd()->Delete();
						NextMode(n, 7u);
					}
				}
			}
			else if (nStep() == 2)
			{
				if (pTaskScenePerson()->IsDeleteEnd())
				{
					bConfirmDecide() = true;
					pTaskSceneConfirm()->Delete();
					NextMode(n, 7u);
				}
			}
			else if (nStep() == 3 && pTaskScenePerson()->IsDeleteEnd())
			{
				NextMode(n, 2u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch3_2u;
		case 2u:
		_switch3_2u:
			nModeStep(n) = 0u;
			nStep() = 0;
			bRecoveryReset() = true;
			break;
		}
	}
	else if (2 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch4_0u:
			nModeStep(n) = 1u;
			pTaskSceneError() = pTaskSceneAdd();
			if (bRecoveryReset())
			{
				pTaskSceneAdd()->Reset();
				pTaskSceneAdd()->Appear();
				pTaskConsInfoWnd()->Appear();
			}
			else
			{
				nStep()++;
			}
			goto _switch4_1u;
		case 1u:
		_switch4_1u:
			if (nStep() == 0)
			{
				if (pTaskSceneAdd()->IsAppearEnd())
				{
					pTaskSceneAdd()->Wait();
					nStep()++;
				}
			}
			else if (nStep() == 1)
			{
				if (pTaskSceneAdd()->ErrorCheck())
				{
					NextMode(n, 5u);
				}
				else
				{
					pTaskSceneAdd()->Select();
					if (pTaskSceneAdd()->IsOkDecide())
					{
						pTaskSceneAdd()->Delete();
						nStep() = 2;
					}
				}
			}
			else if (nStep() == 2 && pTaskSceneAdd()->IsDeleteEnd())
			{
				NextMode(n, 1u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch4_2u;
		case 2u:
		_switch4_2u:
			nModeStep(n) = 0u;
			nStep() = 0;
			bRecoveryReset() = true;
			break;
		}
	}
	else if (4 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch5_0u:
			nModeStep(n) = 1u;
			pTaskSceneError() = pTaskSceneConfirm();
			if (bRecoveryReset())
			{
				pTaskSceneConfirm()->Reset();
				pTaskSceneConfirm()->Appear();
				pTaskConsInfoWnd()->Appear();
			}
			else
			{
				nStep()++;
			}
			goto _switch5_1u;
		case 1u:
		_switch5_1u:
			if (nStep() == 0)
			{
				if (pTaskSceneConfirm()->IsAppearEnd())
				{
					pTaskSceneConfirm()->Wait();
					nStep()++;
				}
			}
			else if (nStep() == 1)
			{
				if (pTaskSceneConfirm()->ErrorCheck())
				{
					NextMode(n, 5u);
				}
				else
				{
					pTaskSceneConfirm()->Select();
					if (pTaskSceneConfirm()->IsBackDecide())
					{
						pTaskSceneConfirm()->Delete();
						nStep() = 2;
					}
					if (pTaskSceneConfirm()->IsAllDecide())
					{
						bConfirmDecide() = true;
						pTaskSceneConfirm()->Delete();
						NextMode(n, 7u);
					}
				}
			}
			else if (nStep() == 2 && pTaskSceneConfirm()->IsDeleteEnd())
			{
				NextMode(n, 3u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch5_2u;
		case 2u:
		_switch5_2u:
			nModeStep(n) = 0u;
			nStep() = 0;
			bRecoveryReset() = true;
			break;
		}
	}
	else if (5 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch6_0u:
			nModeStep(n) = 1u;
			pTaskConsError()->Appear();
			goto _switch6_1u;
		case 1u:
		_switch6_1u:
			if (nStep() == 0)
			{
				if (pTaskConsError()->IsAppearEnd())
				{
					nStep()++;
				}
			}
			else if (nStep() == 1)
			{
				if (InGamePadManager::GetEnterKeyDown(InGamePadManager::GetPad1No()))
				{
					cTaskSceneBase::PlaySndSe(ESEKind::RESIDENT_SND_SE02_DECIDE);
					if (ErrorProc())
					{
						bRecoveryReset() = true;
						NextMode(n, 6u);
					}
					else
					{
						bRecoveryReset() = false;
						pTaskConsError()->Delete();
						nStep()++;
					}
				}
			}
			else if (nStep() == 2 && pTaskConsError()->IsDeleteEnd())
			{
				NextMode(n, nRecoveryState());
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch6_2u;
		case 2u:
		_switch6_2u:
			nModeStep(n) = 0u;
			nStep() = 0;
			break;
		}
	}
	else if (6 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch7_0u:
			nModeStep(n) = 1u;
			pTaskConsError()->Delete();
			pTaskSceneError()->Delete();
			pTaskFade()->Fade(EFadeKind::eFADE_TYPE_OUT_BLACK);
			pTaskConsInfoWnd()->Delete();
			goto _switch7_1u;
		case 1u:
		_switch7_1u:
			if (nStep() == 0)
			{
				if (pTaskFade()->IsEnd() && pTaskConsError()->IsDeleteEnd() && pTaskSceneError()->IsDeleteEnd())
				{
					pTaskFade()->Fade(EFadeKind::eFADE_TYPE_IN_BLACK);
					nStep()++;
				}
			}
			else if (nStep() == 1 && pTaskFade()->IsEnd())
			{
				NextMode(n, nRecoveryState());
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch7_2u;
		case 2u:
		_switch7_2u:
			nModeStep(n) = 0u;
			nStep() = 0;
			break;
		}
	}
	else if (7 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch8_0u:
			nModeStep(n) = 1u;
			pTaskFade()->Fade(EFadeKind::eFADE_TYPE_OUT_BLACK);
			pTaskConsInfoWnd()->Delete();
			goto _switch8_1u;
		case 1u:
		_switch8_1u:
			if (pTaskFade()->IsEnd())
			{
				if (((pTaskScenePerson() != nullptr) ? pTaskScenePerson()->UnityUI : nullptr) != nullptr)
				{
					((pTaskScenePerson() != nullptr) ? pTaskScenePerson()->UnityUI : nullptr)->SetEnable(false);
				}
				if (IsConfirmDecide())
				{
					GuestNames();
					Constants::Kingdom_SetJobNo(nMultiJob());
				}
				else
				{
					Constants::Kingdom_SetJobNo(nMenuJob());
				}
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch8_2u;
		case 2u:
		_switch8_2u:
			nModeStep(n) = 0u;
			nStep() = 0;
			break;
		}
	}
	else if (8 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch9_0u:
			nModeStep(n) = 1u;
			pTaskConsError()->Appear();
			pTaskConsError()->nSetErrorCode(13u);
			goto _switch9_1u;
		case 1u:
		_switch9_1u:
			if (nStep() == 0)
			{
				if (pTaskConsError()->IsAppearEnd())
				{
					nStep()++;
				}
			}
			else if (nStep() == 1 && InGamePadManager::GetEnterKeyDown(InGamePadManager::GetPad1No()))
			{
				cTaskSceneBase::PlaySndSe(ESEKind::RESIDENT_SND_SE02_DECIDE);
				pTaskConsError()->Delete();
				pTaskFade()->Fade(EFadeKind::eFADE_TYPE_OUT_BLACK);
				nStep()++;
			}
			if (nStep() == 2 && pTaskFade()->IsEnd())
			{
				Constants::Kingdom_SetJobNo(nMenuJob());
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch9_2u;
		case 2u:
		_switch9_2u:
			nModeStep(n) = 0u;
			nStep() = 0;
			break;
		}
	}
	return true;
}

void cAppGameBaseMenuMatching::nSetErrorCode(uint nIdx)
{
	CustomKingdomUtil::Assert(pTaskConsError() != nullptr);
	pTaskConsError()->nSetErrorCode(nIdx);
	bErrorFlag() = true;
}

uint cAppGameBaseMenuMatching::nGetErrorCode()
{
	CustomKingdomUtil::Assert(pTaskConsError() != nullptr);
	return pTaskConsError()->nGetErrorCode();
}

void cAppGameBaseMenuMatching::nSetMessageIdx(uint nIdx)
{
}

void cAppGameBaseMenuMatching::Draw(int nStatus, cJetTask* pParam)
{
}

}
