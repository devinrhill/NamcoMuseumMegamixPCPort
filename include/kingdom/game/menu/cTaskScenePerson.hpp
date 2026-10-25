#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskSceneBase.hpp"
#include "MotosUIMotosMultiHome.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/menu/cTaskConsAdd.hpp"
#include "kingdom/game/menu/cTaskConsBack.hpp"
#include "kingdom/game/menu/cTaskConsCtrl.hpp"
#include "kingdom/game/menu/cTaskConsPerson.hpp"
#include "kingdom/game/menu/cTaskConsPersonWnd.hpp"
#include "kingdom/game/menu/cTaskConsString.hpp"

namespace kingdom::game::menu
{

class cTaskScenePerson : public cTaskSceneBase
{
private:
	enum EID
	{
		eID_NONE = 0,
		eID_PERSON_WND = 1,
		eID_BACK = 2,
		eID_PERSON_02 = 3,
		eID_PERSON_03 = 4,
		eID_PERSON_04 = 5,
		eID_CTRL_01 = 6,
		eID_CTRL_02 = 7,
		eID_CTRL_03 = 8,
		eID_CTRL_04 = 9,
		eID_ADD = 10,
		eID_MAX = 11
	};

	enum EState
	{
		eSTATE_WAIT = 0,
		eSTATE_BACK = 1,
		eSTATE_ADD = 2,
		eSTATE_PERSON = 3,
		eSTATE_MAX = 4
	};

public:
	enum EPri : uint
	{
		ePRI_BACK = 65540u,
		ePRI_PERSON_WND = 65541u,
		ePRI_PERSON_02 = 65542u,
		ePRI_PERSON_03 = 65543u,
		ePRI_PERSON_04 = 65544u,
		ePRI_CTRL_01 = 65545u,
		ePRI_CTRL_02 = 65546u,
		ePRI_CTRL_03 = 65547u,
		ePRI_CTRL_04 = 65548u,
		ePRI_ADD = 65549u,
		ePRI_MAX = 65550u
	};

private:
	int m_nPerson{};

	bool m_bErrorPerson{};

	int m_nUpdateCheckCt{};

	int m_nConnectCtrlCtOld{};

	cTaskConsPersonWnd* m_pTaskConsPersonWnd{};

	cTaskConsBack* m_pTaskConsBack{};

	cTaskConsAdd* m_pTaskConsAdd{};

	cTaskConsString* m_pTaskConsString{};

	cTaskConsCtrl* m_pTaskConsCtrl[4]{};

	cTaskConsPerson* m_pTaskConsPerson[3]{};

public:
	MotosUIMotosMultiHome* UnityUI{};

private:
	int& nPerson();

	bool& bErrorPerson();

	int& nUpdateCheckCt();

	int& nConnectCtrlCtOld();

	cTaskConsPersonWnd*& pTaskConsPersonWnd();

	cTaskConsBack*& pTaskConsBack();

	cTaskConsAdd*& pTaskConsAdd();

	cTaskConsString*& pTaskConsString();

	cTaskConsCtrl*& pTaskConsCtrl(int nNo);

	cTaskConsPerson*& pTaskConsPerson(int nNo);

public:
	virtual ~cTaskScenePerson();

	cTaskScenePerson(int nId, string AnitaKey);

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

	bool ErrorCheck();

	void Wait();

	void Reset() override;

	void Select() override;

private:
	void ActiveCtrlCheck();

public:
	bool Execute(cJetTask* pParam) override;

	bool IsAddDecide();

	bool IsBackDecide();

	bool IsPersonDecide();
};

}
