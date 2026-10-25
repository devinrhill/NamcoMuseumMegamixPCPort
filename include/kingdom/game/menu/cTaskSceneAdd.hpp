#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskSceneBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/menu/cTaskConsAddWnd.hpp"
#include "kingdom/game/menu/cTaskConsBigCtrl.hpp"
#include "kingdom/game/menu/cTaskConsOk.hpp"
#include "kingdom/game/menu/cTaskConsString.hpp"

namespace kingdom::game::menu
{

class cTaskSceneAdd : public cTaskSceneBase
{
private:
	enum EID
	{
		eID_NONE = 0,
		eID_ADD_WND = 1,
		eID_OK = 2,
		eID_BIG_CTRL_01 = 3,
		eID_BIG_CTRL_02 = 4,
		eID_BIG_CTRL_03 = 5,
		eID_BIG_CTRL_04 = 6,
		eID_STRING = 7,
		eID_MAX = 8
	};

	enum EState
	{
		eSTATE_WAIT = 0,
		eSTATE_OK = 1,
		eSTATE_MAX = 2
	};

public:
	enum EPri : uint
	{
		ePRI_ADD_WND = 65540u,
		ePRI_OK = 65541u,
		ePRI_BIG_CTRL_01 = 65542u,
		ePRI_BIG_CTRL_02 = 65543u,
		ePRI_BIG_CTRL_03 = 65544u,
		ePRI_BIG_CTRL_04 = 65545u,
		ePRI_STRING = 65546u,
		ePRI_MAX = 65547u
	};

private:
	int m_nUpdateCheckCt{};

	int m_nConnectCtrlCtOld{};

	cTaskConsAddWnd* m_pTaskConsAddWnd{};

	cTaskConsOk* m_pTaskConsOk{};

	cTaskConsString* m_pTaskConsString{};

	cTaskConsBigCtrl* m_pTaskConsBigCtrl[4]{};

	int& nUpdateCheckCt();

	int& nConnectCtrlCtOld();

	cTaskConsAddWnd*& pTaskConsAddWnd();

	cTaskConsOk*& pTaskConsOk();

	cTaskConsString*& pTaskConsString();

	cTaskConsBigCtrl*& pTaskConsBigCtrl(int nNo);

public:
	virtual ~cTaskSceneAdd();

	cTaskSceneAdd(int nId, string AnitaKey);

protected:
	virtual void RegistAnita(string AnitaKey);

	virtual void AddHitArray();

	void AllWait() override;

	void KeySelect(const int& padNum) override;

	void SelectProc(const int& padNum) override;

	void DecideCheck(const int& padNum) override;

public:
	void Appear();

	bool IsAppearEnd();

	void Delete();

	bool IsDeleteEnd();

	void Wait();

	bool ErrorCheck();

	void Reset() override;

	void Select() override;

private:
	void ActiveCtrlCheck();

public:
	bool Execute(cJetTask* pParam) override;

	bool IsOkDecide();
};

}
