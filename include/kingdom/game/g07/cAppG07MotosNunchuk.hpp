#pragma once

#include "kingdom/game/menu/cAppGameBaseNunchuk.hpp"

// Job 280 (0x11800000): kingdom::game::g07::cAppG07MotosNunchuk, vtable 0x8044b390, job init 0x800a33a4 (term 0x800a3508), ctor part 0x801c5574,
// IsSkip 0x801c55d4. No Unity counterpart.
namespace kingdom::game::g07
{

class cAppG07MotosNunchuk : public menu::cAppGameBaseNunchuk
{
public:
	cAppG07MotosNunchuk();

protected:
	bool IsSkip() override;
};

}
