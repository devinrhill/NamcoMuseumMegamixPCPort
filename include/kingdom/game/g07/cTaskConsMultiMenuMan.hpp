#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "MotosUIMotosHome.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuArrow.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuBack.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuBg.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuBoard.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuCurtain.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuItem.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuMap.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuMiniArrow.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuPlayer.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuRound.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuRule.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuSingle.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuStart.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuTime.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuWorld.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuMan : public cTaskConsMenuManBase
{
private:
	enum EState
	{
		eSTATE_WAIT = 0,
		eSTATE_APPEAR = 1,
		eSTATE_BACK = 2,
		eSTATE_SINGLE = 3,
		eSTATE_RULE = 4,
		eSTATE_MAP = 5,
		eSTATE_WORLD_PREV = 6,
		eSTATE_WORLD_NEXT = 7,
		eSTATE_ROUND_PREV = 8,
		eSTATE_ROUND_NEXT = 9,
		eSTATE_TIME_PREV = 10,
		eSTATE_TIME_NEXT = 11,
		eSTATE_ITEM_PREV = 12,
		eSTATE_ITEM_NEXT = 13,
		eSTATE_START = 14,
		eSTATE_MAX = 15
	};

	enum EPoint
	{
		ePOINT_NONE = 0,
		ePOINT_BACK = 1,
		ePOINT_SINGLE = 2,
		ePOINT_RULE = 3,
		ePOINT_MAP = 4,
		ePOINT_START = 5,
		ePOINT_ARROW_LEFT = 6,
		ePOINT_ARROW_RIGHT = 7,
		ePOINT_ARROW_ALL = 8,
		ePOINT_ROUND_ARROW_LEFT = 9,
		ePOINT_ROUND_ARROW_RIGHT = 10,
		ePOINT_ROUND_ARROW_ALL = 11,
		ePOINT_TIME_ARROW_LEFT = 12,
		ePOINT_TIME_ARROW_RIGHT = 13,
		ePOINT_TIME_ARROW_ALL = 14,
		ePOINT_ITEM_ARROW_LEFT = 15,
		ePOINT_ITEM_ARROW_RIGHT = 16,
		ePOINT_ITEM_ARROW_ALL = 17,
		ePOINT_MAX = 18
	};

public:
	enum EPri : uint
	{
		ePRI_BG = 65537u,
		ePRI_MAP = 65538u,
		ePRI_PACMAN = 65539u,
		ePRI_CURTAIN = 2147483649u,
		ePRI_BOARD = 2147483650u,
		ePRI_ROUND = 2147483651u,
		ePRI_TIME = 2147483652u,
		ePRI_ITEM = 2147483653u,
		ePRI_START = 2147483654u,
		ePRI_MINI_ARROW = 2147483655u,
		ePRI_WORLD = 2147483656u,
		ePRI_PLAYER1 = 2147483657u,
		ePRI_PLAYER2 = 2147483658u,
		ePRI_PLAYER3 = 2147483659u,
		ePRI_PLAYER4 = 2147483660u,
		ePRI_MII_NAME1 = 2147483661u,
		ePRI_MII_NAME2 = 2147483662u,
		ePRI_MII_NAME3 = 2147483663u,
		ePRI_MII_NAME4 = 2147483664u,
		ePRI_MII_FACE1 = 2147483665u,
		ePRI_MII_FACE2 = 2147483666u,
		ePRI_MII_FACE3 = 2147483667u,
		ePRI_MII_FACE4 = 2147483668u,
		ePRI_ARROW = 2147483669u,
		ePRI_RULE = 2147483670u,
		ePRI_SINGLE = 2147483671u,
		ePRI_BACK = 2147483672u,
		ePRI_TUTORIAL = 2147483673u,
		ePRI_CURSOR = 2147483674u,
		ePRI_MAX = 2147483675u
	};

private:
	int m_nState{};

	int m_nStep{};

	int m_nSelectObjNow{};

	int m_nSelectObjOld{};

	uint m_nWorldNo{};

	uint m_nFreeWorldNo{};

	uint m_nPlayerMax{};

	cTaskConsMultiMenuBg* m_pTaskConsMultiMenuBg{};

	cTaskConsMultiMenuBack* m_pTaskConsMultiMenuBack{};

	cTaskConsMultiMenuSingle* m_pTaskConsMultiMenuSingle{};

	cTaskConsMultiMenuRule* m_pTaskConsMultiMenuRule{};

	cTaskConsMultiMenuCurtain* m_pTaskConsMultiMenuCurtain{};

	cTaskConsMultiMenuWorld* m_pTaskConsMultiMenuWorld{};

	cTaskConsMultiMenuArrow* m_pTaskConsMultiMenuArrow[2]{};

	cTaskConsMultiMenuMiniArrow* m_pTaskConsMultiMenuMiniArrow[6]{};

	cTaskConsMultiMenuBoard* m_pTaskConsMultiMenuBoard{};

	cTaskConsMultiMenuRound* m_pTaskConsMultiMenuRound{};

	cTaskConsMultiMenuItem* m_pTaskConsMultiMenuItem{};

	cTaskConsMultiMenuTime* m_pTaskConsMultiMenuTime{};

	cTaskConsMultiMenuStart* m_pTaskConsMultiMenuStart{};

	cTaskConsMultiMenuPlayer* m_pTaskConsMultiMenuPlayer[4]{};

	cTaskConsMultiMenuMap* m_pTaskConsMultiMenuMap{};

	cTaskConsMenuBase* m_pHitArray[18]{};

public:
	MotosUIMotosHome* UnityUI{};

protected:
	int& nState();

	int& nStep();

	int& nSelectObjNow();

	int& nSelectObjOld();

	uint& nWorldNo();

	uint& nFreeWorldNo();

	uint& nPlayerMax();

	cTaskConsMultiMenuBg*& pTaskConsMultiMenuBg();

	cTaskConsMultiMenuBack*& pTaskConsMultiMenuBack();

	cTaskConsMultiMenuSingle*& pTaskConsMultiMenuSingle();

	cTaskConsMultiMenuRule*& pTaskConsMultiMenuRule();

	cTaskConsMultiMenuCurtain*& pTaskConsMultiMenuCurtain();

	cTaskConsMultiMenuWorld*& pTaskConsMultiMenuWorld();

	cTaskConsMultiMenuBoard*& pTaskConsMultiMenuBoard();

	cTaskConsMultiMenuRound*& pTaskConsMultiMenuRound();

	cTaskConsMultiMenuItem*& pTaskConsMultiMenuItem();

	cTaskConsMultiMenuTime*& pTaskConsMultiMenuTime();

	cTaskConsMultiMenuStart*& pTaskConsMultiMenuStart();

	cTaskConsMultiMenuMap*& pTaskConsMultiMenuMap();

	cTaskConsMultiMenuPlayer*& pTaskConsMultiMenuPlayer(uint nNo);

	cTaskConsMultiMenuArrow*& pTaskConsMultiMenuArrow(uint nType);

	cTaskConsMultiMenuMiniArrow*& pTaskConsMultiMenuMiniArrow(uint nType);

	cTaskConsMenuBase*& pHitArray(uint nNo);

public:
	cTaskConsMultiMenuMan(cAppGameBaseMenu* pApp);

private:
	void AddHitArray();

public:
	virtual ~cTaskConsMultiMenuMan();

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

	void Reset();

	bool Execute(cJetTask* pParam) override;

	void AppearStart();

	bool IsAppearEnd();

	bool IsBackDecide();

	bool IsStartDecide();

	bool IsSingleDecide();

	bool IsRuleDecide();

	void Upload();
};

}
