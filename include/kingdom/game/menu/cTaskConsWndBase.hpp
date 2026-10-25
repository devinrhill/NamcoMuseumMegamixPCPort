#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsMatching.hpp"
#include "MotosUIMotosMultiHome.hpp"

namespace kingdom::game::menu
{

class cTaskConsWndBase : public cTaskConsMatching
{
protected:
	enum EAnt
	{
		eANT_WND = 0
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_DELETE = 2,
		eANM_MAX = 3
	};

public:
	MotosUIMotosMultiHome* UnityUI{};

	virtual ~cTaskConsWndBase();

	cTaskConsWndBase(int nId);

protected:
	virtual int nGetWndAnmNo(uint nState);

	bool AppearProc() override;

	void WaitProc() override;

	bool DeleteProc() override;
};

}
