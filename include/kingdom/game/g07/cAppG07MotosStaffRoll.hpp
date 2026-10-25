#pragma once

#include "common.hpp"
#include "kingdom/game/cAppGameBaseStaffRoll.hpp"

namespace kingdom::game::g07
{

class cAppG07MotosStaffRoll : public cAppGameBaseStaffRoll
{
private:
	void Init();

	void Term();

public:
	cAppG07MotosStaffRoll();

	void Free() override;
};

}
