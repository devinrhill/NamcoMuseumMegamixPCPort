#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g07/cTaskConsItemNumber.hpp"

namespace kingdom::game::g07
{

class cTaskConsItemMultiPlayIcon : public cTaskConsoleBase
{
private:
	enum EAnm
	{
		eANM_PACMAN = 0,
		eANM_PLAYER_NUMBER = 1,
		eANM_WIN_ICON1 = 2,
		eANM_WIN_ICON2 = 3,
		eANM_WIN_ICON3 = 4,
		eANM_TOP_PLAYER = 5,
		eANM_MAX = 6
	};

	enum EFace
	{
		eFACE_NORMAL = 0,
		eFACE_SMILE = 1,
		eFACE_ANGER = 2,
		eFACE_SARROW = 3,
		eFACE_SURPRISE = 4,
		eFACE_BLINK = 5,
		eFACE_OPEN_MOUTH = 6,
		eFACE_MAX = 7
	};

	cTaskConsItemNumber* m_pScoreNum{};

	uint m_nPlayerNum{};

	Ref<int>* m_nExecPlayerScore = new Ref<int>();

public:
	cTaskConsItemMultiPlayIcon(uint PlayerNum);

	virtual ~cTaskConsItemMultiPlayIcon();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;
};

}
