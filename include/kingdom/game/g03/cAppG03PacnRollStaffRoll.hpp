#pragma once

#include "common.hpp"
#include "kingdom/game/cAppGameBaseStaffRoll.hpp"

namespace kingdom::game::g03
{

class cAppG03PacnRollStaffRoll : public cAppGameBaseStaffRoll
{
private:
	void Init();

	void Term();

public:
	cAppG03PacnRollStaffRoll();

	void Free() override;
};

}
