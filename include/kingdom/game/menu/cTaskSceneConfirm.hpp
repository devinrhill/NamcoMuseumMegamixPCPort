#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskSceneBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/menu/cTaskConsAddWnd.hpp"
#include "kingdom/game/menu/cTaskConsBack.hpp"
#include "kingdom/game/menu/cTaskConsStart.hpp"
#include "kingdom/game/menu/cTaskConsString.hpp"

namespace kingdom::game::menu
{

class cTaskSceneConfirm : public cTaskSceneBase
{
private:
	enum EID
	{
		eID_NONE = 0,
		eID_WND = 1,
		eID_BACK = 2,
		eID_START_01 = 3,
		eID_START_02 = 4,
		eID_START_03 = 5,
		eID_START_04 = 6,
		eID_STRING = 7,
		eID_MAX = 8
	};

	enum EState
	{
		eSTATE_WAIT = 0,
		eSTATE_BACK = 1,
		eSTATE_START = 2,
		eSTATE_START_CANCEL = 3,
		eSTATE_MAX = 4
	};

public:
	enum EPri : uint
	{
		ePRI_WND = 65540u,
		ePRI_BACK = 65541u,
		ePRI_START_01 = 65542u,
		ePRI_START_02 = 65543u,
		ePRI_START_03 = 65544u,
		ePRI_START_04 = 65545u,
		ePRI_MII_NAME = 65546u,
		ePRI_STRING = 65547u,
		ePRI_MAX = 65548u
	};

private:
	int m_nState[4]{};

	int m_nStep[4]{};

	int m_nSelectNowArray[4]{};

	int m_nSelectOldArray[4]{};

	int m_nPerson{};

	cTaskConsAddWnd* m_pTaskConsConfirmWnd{};

	cTaskConsBack* m_pTaskConsBack{};

	cTaskConsString* m_pTaskConsString{};

	cTaskConsStart* m_pTaskConsStart[4]{};

	int& nState(int nNo);

	int& nStep(int nNo);

	int& nSelectNow(int nNo);

	int& nSelectOld(int nNo);

	int& nPerson();

	cTaskConsAddWnd*& pTaskConsConfirmWnd();

	cTaskConsBack*& pTaskConsBack();

	cTaskConsString*& pTaskConsString();

	cTaskConsStart*& pTaskConsStart(int nNo);

public:
	virtual ~cTaskSceneConfirm();

	cTaskSceneConfirm(int nId, string AnitaKey, string SelectKey);

protected:
	virtual void RegistAnita(string AnitaKey, string SelectKey);

	virtual void AddHitArray();

	void AllWait() override;

public:
	bool DpdSelect(const int& padNum) override;

protected:
	void KeySelect(const int& padNum) override;

	void SelectProc(const int& padNum) override;

	void DecideCheck(const int& padNum) override;

public:
	void Appear();

	bool IsAppearEnd();

	void Wait();

	void Delete();

	bool IsDeleteEnd();

	bool ErrorCheck();

	void Reset() override;

	void Select() override;

	bool Execute(cJetTask* pParam) override;

	bool IsAllDecide();

	bool IsBackDecide();
};

}
