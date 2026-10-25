#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskConsCheckPoint : public cTaskConsoleBase
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
		eSTATE_MAX = 4
	};

	int m_nWaitCnt{};

public:
	cTaskConsCheckPoint();

	virtual ~cTaskConsCheckPoint();

private:
	void Init();

	void Term();

public:
	void Start();

	void End();

	bool Execute(cJetTask* pParam) override;
};

}
