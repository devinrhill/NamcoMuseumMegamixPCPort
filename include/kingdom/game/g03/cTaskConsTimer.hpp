#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskConsNum.hpp"

namespace kingdom::game::g03
{

class cTaskConsTimer : public cTaskConsoleBase
{
private:
	enum EAnita
	{
		eTIME = 0,
		eCOLOGNE = 1,
		eDOT = 2
	};

	enum EState
	{
		eSTATE_INVISIBLE = 0,
		eSTATE_IN = 1,
		eSTATE_IN_NUM = 2,
		eSTATE_STAY = 3,
		eSTATE_OUT_NUM = 4,
		eSTATE_OUT = 5,
		eSTATE_MAX = 6
	};

	enum EFigure
	{
		eFIGURE_MIN = 1,
		eFIGURE_SEC = 2,
		eFIGURE_MSEC = eFIGURE_SEC
	};

	cTaskConsNum* m_pTaskMin{};

	cTaskConsNum* m_pTaskSec{};

	cTaskConsNum* m_pTaskMSec{};

	uint m_nTime{};

public:
	cTaskConsTimer();

	virtual ~cTaskConsTimer();

private:
	void Init();

	void Term();

public:
	void Start(uint now);

	void End();

	void Update(uint now);

	bool Execute(cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskConsTimer::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
