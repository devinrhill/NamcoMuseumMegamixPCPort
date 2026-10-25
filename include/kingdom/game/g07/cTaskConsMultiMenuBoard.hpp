#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuBoard : public cTaskConsMenuBase
{
protected:
	enum EAnita
	{
		eANITA_BOARD = 0,
		eANITA_MAX = 1
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_MAX = 2
	};

public:
	cTaskConsMultiMenuBoard(cTaskConsMenuManBase* const& pConsMan);

	virtual ~cTaskConsMultiMenuBoard();

	void SetData();

protected:
	virtual int nGetBoardAnmNo(int nAnmState);

	bool AppearExec() override;

	void WaitExec() override;
};

}
