#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsCtrl.hpp"

namespace kingdom::game::menu
{

class cTaskConsBigCtrl : public cTaskConsCtrl
{
public:
	cTaskConsBigCtrl(int nId, string Key, uint nPlayerNo);

	virtual ~cTaskConsBigCtrl();

protected:
	int nGetCtrlAnmNo(uint nState) override;
};

}
