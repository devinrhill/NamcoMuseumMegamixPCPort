#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMenuCurtain : public cTaskConsMenuBase
{
protected:
	enum EAnita
	{
		eANITA_CURTAIN = 0
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_MAX = 2
	};

public:
	cTaskConsMenuCurtain(cTaskConsMenuManBase* const& pConsMan);

	void SetData();

protected:
	virtual int nGetCurtainAnmNo(int nAnmState);

public:
	virtual ~cTaskConsMenuCurtain();

protected:
	void WaitExec() override;

	bool AppearExec() override;
};

}
