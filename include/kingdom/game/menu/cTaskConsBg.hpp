#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsMatching.hpp"

namespace kingdom::game::menu
{

class cTaskConsBg : public cTaskConsMatching
{
private:
	enum EAnt
	{
		eANT_BG = 0
	};

public:
	virtual ~cTaskConsBg();

	cTaskConsBg(int nId, string Key);
};

}
