#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskConsCookieWarning : public cTaskConsoleBase
{
private:
	enum EAnita
	{
		eTEXT = 0
	};

	enum EState
	{
		eSTATE_INVISIBLE = 0,
		eSTATE_WARNING_IN = 1,
		eSTATE_WARNING_PLAY = 2,
		eSTATE_WARNING_OUT = 3,
		eSTATE_GATE_IN = 4,
		eSTATE_GATE_PLAY = 5,
		eSTATE_GATE_OUT = 6,
		eSTATE_MAX = 7
	};

	uint m_nWaitCnt{};

public:

	cTaskConsCookieWarning();

private:
	void Init();

public:
	void Term();

	void Start();

	void StartWarning();

	void StartGate();

	void End();

	void EndWarning();

	void EndGate();

	bool Execute(cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskConsCookieWarning::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
