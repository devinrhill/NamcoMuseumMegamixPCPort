#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskConsClear : public cTaskConsoleBase
{
private:
	enum EAnita
	{
		eSTART = 0
	};

	enum EState
	{
		eSTATE_INVISIBLE = 0,
		eSTATE_IN = 1,
		eSTATE_STAY = 2,
		eSTATE_OUT = 3,
		eSTATE_END = 4,
		eSTATE_MAX = 5
	};

	bool m_bPerfect{};

	uint m_nWait{};

public:
	cTaskConsClear();

	virtual ~cTaskConsClear();

private:
	void Init();

	void Term();

public:
	void Start();

	void PStart();

	void End();

	bool Execute(cJetTask* pParam) override;

	bool IsPerfect();

	bool IsPlayEnd();

protected:
	void ReleaseTerm() override { cTaskConsClear::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
