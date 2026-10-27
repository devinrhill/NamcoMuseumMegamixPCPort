#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskConsNum.hpp"

namespace kingdom::game::g03
{

class cTaskConsGate : public cTaskConsoleBase
{
private:
	enum EAnita
	{
		eGATE = 0
	};

	enum EState
	{
		eSTATE_INVISIBLE = 0,
		eSTATE_IN = 1,
		eSTATE_STAY = 2,
		eSTATE_OUT = 3,
		eSTATE_MAX = 4
	};

	enum EFigure
	{
		eFIGURE = 2
	};

	cTaskConsNum* m_pTaskNum{};

	uint m_nGate{};

public:
	cTaskConsGate();

	virtual ~cTaskConsGate();

private:
	void Init();

	void Term();

public:
	void Start(uint now);

	void End();

	void Update(uint now);

	bool Execute(cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskConsGate::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
