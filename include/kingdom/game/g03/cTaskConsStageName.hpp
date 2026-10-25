#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "MotosUIRollStageName.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskConsStageName : public cTaskConsoleBase
{
private:
	enum EAnita
	{
		eTITLE = 0,
		eSTAGE = 1
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

	uint m_nTimeCnt{};

public:
	MotosUIRollStageName* UnityUI{};

	cTaskConsStageName();

private:
	void Init();

public:
	void Term();

	void Start();

	void End();

	bool Execute(cJetTask* pParam) override;

private:
	int nGetAnimNoIn(uint w);

	int nGetAnimNoStay(uint w);

	int nGetAnimNoOut(uint w);

	int nGetAnimNoStageIn(uint s);

	int nGetAnimNoStageStay(uint s);

	int nGetAnimNoStageOut(uint s);

public:
	bool IsPlayEnd();
};

}
