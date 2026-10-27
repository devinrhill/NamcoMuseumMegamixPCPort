#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g07/cTaskConsItemMultiPlayIcon.hpp"
#include "kingdom/game/g07/cTaskConsItemNumber.hpp"

namespace kingdom::game::g07
{

class cTaskConsItemMultiPlay : public cTaskConsoleBase
{
private:
	enum EAnm
	{
		eANM_CONSOLE = 0,
		eANM_COLON = 1,
		eANM_FINISH = 2,
		eANM_DRAW = 3,
		eANM_MAX = 4
	};

	cTaskConsItemMultiPlayIcon* m_pIcon[4]{};

	cTaskConsItemNumber* m_pTimerNum[2]{};

	uint m_nTimer{};

	uint m_nDrawTimer{};

	Ref<int>* m_TimerMin = new Ref<int>();

	Ref<int>* m_TimerSecconds = new Ref<int>();

	static const float PLAYER_WINNER_POS[4][2];

	static const float PLAYER_ICON_POS[4][2];

public:
	cTaskConsItemMultiPlay();

	virtual ~cTaskConsItemMultiPlay();

	static cTaskAnita* Construct();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskConsItemMultiPlay::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
