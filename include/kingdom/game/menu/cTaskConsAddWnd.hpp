#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsWndBase.hpp"

namespace kingdom::game::menu
{

class cTaskConsAddWnd : public cTaskConsWndBase
{
public:
	virtual ~cTaskConsAddWnd();

	cTaskConsAddWnd(int nId, string Key);

protected:
	int nGetWndAnmNo(uint nState) override;
};

}
