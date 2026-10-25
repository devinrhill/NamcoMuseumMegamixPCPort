#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "MotosUIMotosHome.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/g07/cTaskConsMenuArrow.hpp"
#include "kingdom/game/g07/cTaskConsMenuBack.hpp"
#include "kingdom/game/g07/cTaskConsMenuBar.hpp"
#include "kingdom/game/g07/cTaskConsMenuBg.hpp"
#include "kingdom/game/g07/cTaskConsMenuCurtain.hpp"
#include "kingdom/game/g07/cTaskConsMenuMap.hpp"
#include "kingdom/game/g07/cTaskConsMenuMulti.hpp"
#include "kingdom/game/g07/cTaskConsMenuRule.hpp"
#include "kingdom/game/g07/cTaskConsMenuStage.hpp"
#include "kingdom/game/g07/cTaskConsMenuWorld.hpp"

namespace kingdom::game::g07
{

class cTaskConsMenuMan : public cTaskConsMenuManBase
{
private:
	enum EState
	{
		eSTATE_WAIT = 0,
		eSTATE_APPEAR = 1,
		eSTATE_BACK = 2,
		eSTATE_MULTI = 3,
		eSTATE_RULE = 4,
		eSTATE_MAP = 5,
		eSTATE_WORLD_PREV = 6,
		eSTATE_WORLD_NEXT = 7,
		eSTATE_STAGE = 8,
		eSTATE_MAX = 9
	};

	enum EPoint
	{
		ePOINT_NONE = 0,
		ePOINT_BACK = 1,
		ePOINT_MULTI = 2,
		ePOINT_RULE = 3,
		ePOINT_MAP = 4,
		ePOINT_STAGE_00 = 5,
		ePOINT_STAGE_01 = 6,
		ePOINT_STAGE_02 = 7,
		ePOINT_STAGE_03 = 8,
		ePOINT_STAGE_04 = 9,
		ePOINT_STAGE_05 = 10,
		ePOINT_STAGE_06 = 11,
		ePOINT_ARROW_LEFT = 12,
		ePOINT_ARROW_RIGHT = 13,
		ePOINT_ARROW_ALL = 14,
		ePOINT_MAX = 15
	};

public:
	enum EPri : uint
	{
		ePRI_BG = 65537u,
		ePRI_MAP = 65538u,
		ePRI_CURTAIN = 2147483649u,
		ePRI_BAR = 2147483650u,
		ePRI_WORLD = 2147483651u,
		ePRI_ARROW = 2147483652u,
		ePRI_STAGE = 2147483653u,
		ePRI_RULE = 2147483654u,
		ePRI_STAGE_SELECT = 2147483655u,
		ePRI_MULTI = 2147483656u,
		ePRI_BACK = 2147483657u,
		ePRI_TUTORIAL = 2147483658u,
		ePRI_CURSOR = 2147483659u,
		ePRI_MAX = 2147483660u
	};

private:
	int m_nState{};

	int m_nStep{};

	uint m_nWorldNo{};

	uint m_nStageNo{};

	uint m_nFreeWorldNo{};

	uint m_nFreeStageNo{};

	int m_nSelectObjNow{};

	int m_nSelectObjOld{};

	cTaskConsMenuBg* m_pTaskConsMenuBg{};

	cTaskConsMenuBack* m_pTaskConsMenuBack{};

	cTaskConsMenuMulti* m_pTaskConsMenuMulti{};

	cTaskConsMenuRule* m_pTaskConsMenuRule{};

	cTaskConsMenuBar* m_pTaskConsMenuBar{};

	cTaskConsMenuCurtain* m_pTaskConsMenuCurtain{};

	cTaskConsMenuWorld* m_pTaskConsMenuWorld{};

	cTaskConsMenuArrow* m_pTaskConsMenuArrow[2]{};

	cTaskConsMenuStage* m_pTaskConsMenuStage[7]{};

	cTaskConsMenuMap* m_pTaskConsMenuMap{};

	cTaskConsMenuBase* m_pHitArray[15]{};

public:
	MotosUIMotosHome* UnityUI{};

protected:
	int& nState();

	int& nStep();

	uint& nWorldNo();

	uint& nStageNo();

	uint& nFreeWorldNo();

	uint& nFreeStageNo();

	int& nSelectObjNow();

	int& nSelectObjOld();

	cTaskConsMenuBg*& pTaskConsMenuBg();

	cTaskConsMenuBack*& pTaskConsMenuBack();

	cTaskConsMenuMulti*& pTaskConsMenuMulti();

	cTaskConsMenuRule*& pTaskConsMenuRule();

	cTaskConsMenuBar*& pTaskConsMenuBar();

	cTaskConsMenuCurtain*& pTaskConsMenuCurtain();

	cTaskConsMenuWorld*& pTaskConsMenuWorld();

	cTaskConsMenuMap*& pTaskConsMenuMap();

	cTaskConsMenuStage*& pTaskConsMenuStage(uint nNo);

	cTaskConsMenuArrow*& pTaskConsMenuArrow(uint nType);

	cTaskConsMenuBase*& pHitArray(uint nNo);

public:
	cTaskConsMenuMan(cAppGameBaseMenu* pApp);

	void Free();

private:
	void StageSetting();

	void FreeStageCheck();

	void AddHitArray();

public:
	void AllWaitState();

private:
	void _ObjProc();

protected:
	bool DpdSelect(int padNum);

	bool IsPointActive();

	void KeySelect(int padNum);

	void SelectProc(int padNum);

	void DecideCheck(int padNum);

public:
	void Select();

	void Reset();

	bool Execute(cJetTask* pParam) override;

	void AppearStart();

	bool IsAppearEnd();

	bool IsBackDecide();

	bool IsStageDecide();

	bool IsMultiDecide();

	bool IsRuleDecide();

	void Uplode();
};

}
