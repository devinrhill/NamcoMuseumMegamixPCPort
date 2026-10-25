#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskConsButtCursor : public cTaskConsoleBase
{
private:
	enum EAnita
	{
		eBUTTCURSOR = 0
	};

	enum EState
	{
		eSTATE_INVISIBLE = 0,
		eSTATE_IN = 1,
		eSTATE_STAY = 2,
		eSTATE_OUT = 3,
		eSTATE_MAX = 4
	};

	uint m_nWaitCnt{};

	bool m_bRed{};

	Vector4 m_vTrans{};

	Vector4 m_vTargetTrans{};

public:
	cTaskConsButtCursor();

	virtual ~cTaskConsButtCursor();

private:
	void Init();

	void Term();

public:
	void Start(Vector4 trans);

	bool Red();

	void End();

	bool Execute(cJetTask* pParam) override;

private:
	bool IsInScreen();

	void UpdatePosition();

public:
	void Update(Vector4 trans);
};

}
