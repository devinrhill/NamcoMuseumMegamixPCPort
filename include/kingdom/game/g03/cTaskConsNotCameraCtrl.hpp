#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskConsNotCameraCtrl : public cTaskConsoleBase
{
private:
	enum EAnita
	{
		eNOTCAMERACTRL = 0
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

public:
	cTaskConsNotCameraCtrl();

	virtual ~cTaskConsNotCameraCtrl();

private:
	void Init();

	void Term();

public:
	void Start();

	void End();

	void Update(bool go);

	bool Execute(cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskConsNotCameraCtrl::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
