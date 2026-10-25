#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsMatching.hpp"

namespace kingdom::game::menu
{

class cTaskConsInfoWnd : public cTaskConsMatching
{
private:
	enum EAnt
	{
		eANT_WND = 0
	};

protected:
	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_DELETE = 2,
		eANM_MAX = 3
	};

public:
	virtual ~cTaskConsInfoWnd();

	cTaskConsInfoWnd(int nId, string Key);

protected:
	virtual int nGetWndAnmNo(uint nState);

	bool AppearProc() override;

	void WaitProc() override;

	bool DeleteProc() override;
};

}
