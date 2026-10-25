#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskConsNum.hpp"

namespace kingdom::game::g03
{

class cTaskConsCombo : public cTaskConsoleBase
{
private:
	enum EAnita
	{
		eCOMBO = 0
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

	enum EComboMax
	{
		eComboMax = 99
	};

	cTaskConsNum* m_pTaskNum{};

	uint m_nCombo{};

	uint m_nWaitCnt{};

public:
	cTaskConsCombo();

	virtual ~cTaskConsCombo();

private:
	void Init();

	void Term();

public:
	void Start(uint now);

	void End();

	void Update(uint now);

	bool Execute(cJetTask* pParam) override;

	bool IsInvisible();
};

}
