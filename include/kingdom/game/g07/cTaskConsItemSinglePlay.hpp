#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g07/cTaskConsItemNumber.hpp"
#include "kingdom/game/g07/cTaskConsPacIcon.hpp"
#include "kingdom/game/g07/cTaskConsPlayTime.hpp"
#include "kingdom/game/g07/cTaskConsSingleClear.hpp"

namespace kingdom::game::g07
{

class cTaskConsItemSinglePlay : public cTaskConsoleBase
{
private:
	enum EAnm
	{
		eANM_POWER = 0,
		eANM_JUMP = 1,
		eANM_CHARGE = 2,
		eANM_STAGE_FILTER = 3,
		eANM_SCORE = 4,
		eANM_PLAYER = 5,
		eANM_MAX = 6
	};

public:
	static constexpr int MAX_TASK_SINGLE_PLAY_PAC_ICON = 3;

private:
	static const int nTikiAnm[3];

	static const int nAnm[3];

	cTaskConsItemNumber* m_pPlayerNum{};

	cTaskConsItemNumber* m_pPlayerScoreNum{};

	cTaskConsItemNumber* m_pStageNum{};

	cTaskConsItemNumber* m_pWorldNum{};

	cTaskConsSingleClear* m_pSingleClear{};

	cTaskConsPlayTime* m_pPlayTime{};

	cTaskConsPacIcon* m_pPacIcon[3]{};

	Ref<int>* m_nExecNumber{};

	Ref<int>* m_nExecScoreNumber{};

	Ref<int>* m_nItemUseNumber[3]{};

	Ref<int>* m_nWorldNumber{};

	Ref<int>* m_nStageNumber{};

public:
	static float PLAYER_SYMBOL_POS_X;

	static float PLAYER_SYMBOL_POS_Y;

	static float PLAYER_SYMBOL_NUMBER_POS_X;

	static float PLAYER_SYMBOL_NUMBER_POS_Y;

	static float SCORE_NUMBER_POS_X;

	static float SCORE_NUMBER_POS_Y;

	static float ITEM_POWER_POS_X;

	static float ITEM_POWER_POS_Y;

	static float ITEM_JUMP_POS_X;

	static float ITEM_JUMP_POS_Y;

	static float ITEM_CHARGE_POS_X;

	static float ITEM_CHARGE_POS_Y;

	static float WORLD_NUMBER_POS_X;

	static float WORLD_NUMBER_POS_Y;

	static float STAGE_NUMBER_POS_X;

	static float STAGE_NUMBER_POS_Y;

	cTaskConsItemSinglePlay();

	virtual ~cTaskConsItemSinglePlay();

	void Free() override;

	static cTaskAnita* Construct();

	void Setup();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;
};

}
