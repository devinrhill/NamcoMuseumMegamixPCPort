#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskConsNum : public cTaskConsoleBase
{
private:
	enum EAnita
	{
		eNUM = 0
	};

	enum EState
	{
		eSTATE_INVISIBLE = 0,
		eSTATE_IN = 1,
		eSTATE_STAY = 2,
		eSTATE_OUT = 3,
		eSTATE_OUT_ADD = 4,
		eSTATE_OUT_SUB = 5,
		eSTATE_MAX = 6
	};

	enum EFigure
	{
		DEFAULT_FIGURE = 3
	};

	uint m_nNum{};

	uint m_nFigure{};

	static const int scInAnimNo[10];

	static const int scWaitAnimNo[10];

	static const int scOutAnimNo[10];

public:
	cTaskConsNum();

	cTaskConsNum(uint figure);

	virtual ~cTaskConsNum();

private:
	void Init(uint figure = 3u);

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Start(uint num);

	bool End();

	bool IsSet();

	bool Set(uint num);

	bool IsAnimEnd();

private:
	int nGetAnimNoIn(uint num);

	int nGetAnimNoStay(uint num);

	int nGetAnimNoOut(uint num);
};

}
