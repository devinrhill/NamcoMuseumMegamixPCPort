#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::menu
{

class cTaskConsMatching : public cTaskConsBase
{
protected:
	enum EState
	{
		eSTATE_FADE_IN = 0,
		eSTATE_APPEAR = 1,
		eSTATE_APPEAR_END = 2,
		eSTATE_WAIT = 3,
		eSTATE_POINT = 4,
		eSTATE_DECIDE = 5,
		eSTATE_DECIDE_END = 6,
		eSTATE_DELETE = 7,
		eSTATE_DELETE_END = 8
	};

	virtual void WaitProc();

	virtual void PointProc();

	virtual bool AppearProc();

	virtual bool DecideProc();

	virtual bool DeleteProc();

public:
	virtual ~cTaskConsMatching();

	virtual void Appear();

	virtual bool IsAppear();

	virtual bool IsAppearEnd();

	virtual void Wait();

	virtual bool IsWait();

	virtual void Point();

	virtual bool IsPoint();

	virtual void Decide();

	virtual bool IsDecide();

	virtual bool IsDecideEnd();

	virtual void Delete();

	virtual bool IsDelete();

	virtual bool IsDeleteEnd();

	cTaskConsMatching(int nId);

	bool Execute(cJetTask* pParam) override;
};

}
