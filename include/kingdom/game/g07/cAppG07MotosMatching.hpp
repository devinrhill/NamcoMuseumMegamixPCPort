#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cAppGameBaseMenuMatching.hpp"

namespace kingdom::game::g07
{

class cAppG07MotosMatching : public kingdom::game::menu::cAppGameBaseMenuMatching
{
public:
	cAppG07MotosMatching();

	void Free() override;

private:
	void Init();

	void Term();
};

}
