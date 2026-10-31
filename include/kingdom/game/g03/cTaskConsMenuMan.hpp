#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/g03/cTaskConsMenuArrow.hpp"
#include "kingdom/game/g03/cTaskConsMenuBack.hpp"
#include "kingdom/game/g03/cTaskConsMenuBar.hpp"
#include "kingdom/game/g03/cTaskConsMenuBg.hpp"
#include "kingdom/game/g03/cTaskConsMenuCurtain.hpp"
#include "kingdom/game/g03/cTaskConsMenuMap.hpp"
#include "kingdom/game/g03/cTaskConsMenuRule.hpp"
#include "kingdom/game/g03/cTaskConsMenuStage.hpp"
#include "kingdom/game/g03/cTaskConsMenuWorld.hpp"
#include "kingdom/game/g03/cTaskMenuPacman.hpp"

namespace kingdom::game::g03
{

class cTaskConsMenuMan : public cTaskConsMenuManBase
{
private:
	enum EState
	{
		eSTATE_WAIT = 0,
		eSTATE_APPEAR = 1,
		eSTATE_BACK = 2,
		eSTATE_RULE = 3,
		eSTATE_MAP = 4,
		eSTATE_WORLD_PREV = 5,
		eSTATE_WORLD_NEXT = 6,
		eSTATE_STAGE = 7,
		eSTATE_MAX = 8
	};

	enum EPoint
	{
		ePOINT_NONE = 0,
		ePOINT_BACK = 1,
		ePOINT_RULE = 2,
		ePOINT_MAP = 3,
		ePOINT_STAGE_00 = 4,
		ePOINT_STAGE_01 = 5,
		ePOINT_STAGE_02 = 6,
		ePOINT_STAGE_03 = 7,
		ePOINT_STAGE_04 = 8,
		ePOINT_STAGE_05 = 9,
		ePOINT_ARROW_LEFT = 10,
		ePOINT_ARROW_RIGHT = 11,
		ePOINT_ARROW_ALL = 12,
		ePOINT_MAX = 13
	};

public:
	enum ELight
	{
		eLIGHT_CHAR = 0,
		eLIGHT_MAX = 1
	};

	enum EPri : uint
	{
		ePRI_BG = 65537u,
		ePRI_MAP = 65538u,
		ePRI_CURTAIN = 65539u,
		ePRI_BAR = 65540u,
		ePRI_3D_PACMAN = 65541u,
		ePRI_WORLD = 2147483649u,
		ePRI_ARROW = 2147483650u,
		ePRI_STAGE = 2147483651u,
		ePRI_RULE = 2147483652u,
		ePRI_STAGE_SELECT = 2147483653u,
		ePRI_BACK = 2147483654u,
		ePRI_TUTORIAL = 2147483655u,
		ePRI_CURSOR = 2147483656u,
		ePRI_MAX = 2147483657u
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

	cTaskConsMenuRule* m_pTaskConsMenuRule{};

	cTaskConsMenuBar* m_pTaskConsMenuBar{};

	cTaskConsMenuCurtain* m_pTaskConsMenuCurtain{};

	cTaskConsMenuWorld* m_pTaskConsMenuWorld{};

	cTaskConsMenuArrow* m_pTaskConsMenuArrow[2]{};

	cTaskConsMenuStage* m_pTaskConsMenuStage[6]{};

	cTaskConsMenuMap* m_pTaskConsMenuMap{};

	cTaskMenuPacman* m_pTaskMenuPacman{};

	cTaskConsMenuBase* m_pHitArray[13]{};

public:

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

	cTaskConsMenuRule*& pTaskConsMenuRule();

	cTaskConsMenuBar*& pTaskConsMenuBar();

	cTaskConsMenuCurtain*& pTaskConsMenuCurtain();

	cTaskConsMenuWorld*& pTaskConsMenuWorld();

	cTaskConsMenuMap*& pTaskConsMenuMap();

	cTaskMenuPacman*& pTaskMenuPacman();

	cTaskConsMenuStage*& pTaskConsMenuStage(uint nNo);

	cTaskConsMenuArrow*& pTaskConsMenuArrow(uint nType);

	cTaskConsMenuBase*& pHitArray(uint nNo);

public:
	cTaskConsMenuMan(cAppGameBaseMenu* pApp);

	void Free();

	void Reset();

protected:
	virtual void FreeStageCheck();

private:
	void StageSetting();

protected:
	virtual void AddHitArray();

public:
	void AllWaitState();

protected:
	virtual void _ObjProc();

	bool DpdSelect(int padNum);

	bool IsPointActive();

	void KeySelect(int padNum);

	void SelectProc(int padNum);

	void DecideCheck(int padNum);

public:
	void Select();

	bool Execute(cJetTask* pParam) override;

	void AppearStart();

	bool IsAppearEnd();

	bool IsBackDecide();

	bool IsStageDecide();

	bool IsRuleDecide();

	void Uplode();
};

}
