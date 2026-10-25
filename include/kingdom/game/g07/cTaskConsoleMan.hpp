#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g07/cTaskConsBossLife.hpp"
#include "kingdom/game/g07/cTaskConsFade.hpp"
#include "kingdom/game/g07/cTaskConsItemMultiPlay.hpp"
#include "kingdom/game/g07/cTaskConsItemSinglePlay.hpp"
#include "kingdom/game/g07/cTaskConsMultiSetup.hpp"
#include "kingdom/game/g07/cTaskConsReady.hpp"
#include "kingdom/game/g07/cTaskConsSingleSetup.hpp"
#include "kingdom/game/g07/cTaskConsSlopeEffect.hpp"

namespace kingdom::game::g07
{

class cTaskConsoleMan : public cTaskKingdom
{
private:
	using CONSOLE_LIST_KIND_TBL_EL = std::function<cTaskAnita*()>;

	enum EConsoleKind
	{
		eCONSOLE_KIND_TBL_null = 0,
		eCONSOLE_KIND_TBL_FADE = 1,
		eCONSOLE_KIND_TBL_SINGLE_SLOPE = 2,
		eCONSOLE_KIND_TBL_BOSS_LIFE = 3,
		eCONSOLE_KIND_TBL_ITEM_POW = 4,
		eCONSOLE_KIND_TBL_SINGLE_SETUP = 5,
		eCONSOLE_KIND_TBL_SINGLE_READY = 6,
		eCONSOLE_KIND_TBL_MULTI_PLAY = 7,
		eCONSOLE_KIND_TBL_MULTI_SETUP = 8,
		eCONSOLE_KIND_TBL_MAX = 9
	};

public:
	enum ETaskPriority : uint
	{
		eTaskPriority_Begin = 2147483649u
	};

private:
	// DOL table 0x8044f2a8 (used by Create 0x801f6e30); the conversion dropped the initialiser
	cTaskConsoleMan::CONSOLE_LIST_KIND_TBL_EL s_console_task_tbl[9]{nullptr, cTaskConsFade::Construct, cTaskConsSlopeEffect::Construct, cTaskConsBossLife::Construct,
		cTaskConsItemSinglePlay::Construct, cTaskConsSingleSetup::Construct, cTaskConsReady::Construct, cTaskConsItemMultiPlay::Construct, cTaskConsMultiSetup::Construct};

	cTaskAnita* m_pAnita[9]{};

	uint m_nPriority{};

public:
	cTaskConsoleMan();

	virtual void Free();

	cTaskAnita* pGetConsole(uint set);

	uint& nPriority();

private:
	void Init();

public:
	void Create();

private:
	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void ItemSetup();
};

}
