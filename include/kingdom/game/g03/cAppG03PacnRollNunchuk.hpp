#pragma once

#include "kingdom/game/menu/cAppGameBaseNunchuk.hpp"

// Job 216 (0x0d800000): kingdom::game::g03::cAppG03PacnRollNunchuk, vtable 0x8043bf88, job init 0x8012b964, ctor part 0x800fc87c, IsSkip 0x800fc8e8.
namespace kingdom::game::g03
{

class cAppG03PacnRollNunchuk : public menu::cAppGameBaseNunchuk
{
public:
	cAppG03PacnRollNunchuk();

protected:
	bool IsSkip() override;
};

}
