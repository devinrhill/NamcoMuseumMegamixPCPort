#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

class cTaskConsMenuCurtain : public cTaskConsMenuBase
{
private:
	enum EAnita
	{
		eANITA_CURTAIN = 0
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1
	};

	static const int ANM_LIST[2];

public:
	cTaskConsMenuCurtain(cTaskConsMenuManBase* pConsMan);

	virtual ~cTaskConsMenuCurtain();

protected:
	void WaitExec() override;

	bool AppearExec() override;
};

}
