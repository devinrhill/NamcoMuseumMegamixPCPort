#pragma once

#include "common.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/game/menu/cParamMessage.hpp"
#include "kingdom/game/menu/cTaskConsBg.hpp"
#include "kingdom/game/menu/cTaskConsError.hpp"
#include "kingdom/game/menu/cTaskConsInfoWnd.hpp"
#include "kingdom/game/menu/cTaskSceneAdd.hpp"
#include "kingdom/game/menu/cTaskSceneBase.hpp"
#include "kingdom/game/menu/cTaskSceneConfirm.hpp"
#include "kingdom/game/menu/cTaskScenePerson.hpp"
#include "kingdom/sMatchingData.hpp"

namespace kingdom { class cTaskCursor; }

namespace kingdom::game::menu
{

class cAppGameBaseMenuMatching : public cAppKingdom
{
private:
	enum EState
	{
		eSTATE_FADE_IN = 0,
		eSTATE_PERSON = 1,
		eSTATE_ADD = 2,
		eSTATE_MII = 3,
		eSTATE_CONFIRM = 4,
		eSTATE_ERROR = 5,
		eSTATE_ERROR_FADE = 6,
		eSTATE_FADE_OUT = 7,
		eSTATE_DB_BROKEN = 8
	};

	enum EID
	{
		eID_BG = 0,
		eID_INFO_WND = 1,
		eID_ERROR = 2,
		eID_SCENE_PERSON = 3,
		eID_SCENE_ADD = 4,
		eID_SCENE_MII = 5,
		eID_SCENE_CONFIRM = 6,
		eID_MAX = 7
	};

public:
	enum EPri : uint
	{
		ePRI_BG = 65537u,
		ePRI_INFO_WND = 65538u,
		ePRI_MESSAGE_INFO = 65539u,
		ePRI_MAX = 65540u,
		ePRI_ERROR = 2147483680u,
		ePRI_ERROR_MESSAGE = 2147483681u,
		ePRI_CURSOR = 2147483682u
	};

private:
	uint m_nMenuJob{};

	uint m_nMultiJob{};

	int m_nState{};

	int m_nStep{};

	bool m_bDataBaseBroken{};

	uint m_nRecoveryMode{};

	bool m_bRecoveryReset{};

	bool m_bErrorFlag{};

	bool m_bConfirmDecide{};

	int m_nConnectCtrl[4]{};

	int m_nConnectCtrlCt{};

	kingdom::cTaskCursor* m_pCursor[4]{};   // +0x2c4: one pointer per pad (the C# dropped them: Unity drew its own pointer)

	sMatchingData* m_pMatchingData{};

	cParamMessage* m_pParamMessage{};

	cTaskFade* m_pTaskFade{};

	cTaskConsBg* m_pTaskConsBg{};

	cTaskConsInfoWnd* m_pTaskConsInfoWnd{};

	cTaskConsError* m_pTaskConsError{};

	cTaskSceneBase* m_pTaskSceneError{};

	cTaskScenePerson* m_pTaskScenePerson{};

	cTaskSceneAdd* m_pTaskSceneAdd{};

	cTaskSceneConfirm* m_pTaskSceneConfirm{};

protected:
	uint& nMenuJob();

	uint& nMultiJob();

	int& nState();

	int& nStep();

	bool& bDataBaseBroken();

	uint& nRecoveryState();

	bool& bRecoveryReset();

	bool& bErrorFlag();

	bool& bConfirmDecide();

	int& nConnectCtrl(int nPlayerNo);

	int& nConnectCtrlCt();

	sMatchingData*& pMatchingData();

	cParamMessage*& pParamMessage();

	cTaskFade*& pTaskFade();

	cTaskConsBg*& pTaskConsBg();

	cTaskConsInfoWnd*& pTaskConsInfoWnd();

	cTaskConsError*& pTaskConsError();

	cTaskSceneBase*& pTaskSceneError();

	cTaskScenePerson*& pTaskScenePerson();

	cTaskSceneAdd*& pTaskSceneAdd();

	cTaskSceneConfirm*& pTaskSceneConfirm();

public:
	cAppGameBaseMenuMatching();

	void Free() override;

	sMatchingData* pGetMatchingData();

	int nGetConnectCtrl(int nPlayerNo);

	int nGetConnectCtrlCt();

	bool IsConfirmDecide();
	void GuestNames();   // stand-in for the Mii / guest scene (not ported): player i = guest i

private:
	void Init();

public:
	void RegistData(string PointKey, string AnitaKey, string CameraKey, string SetKey);

private:
	void Term();

	bool _IsConnectError(int nCtrl);

protected:
	bool ErrorProc();

public:
	void ConnectCtrlCheck();

protected:
	void ConnectCursor();

public:
	bool Execute(cJetTask* pParam) override;

	void nSetErrorCode(uint nIdx);

	uint nGetErrorCode();

	void nSetMessageIdx(uint nIdx);

	void Draw(int nStatus, cJetTask* pParam) override;
};

}
