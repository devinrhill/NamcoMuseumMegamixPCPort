#pragma once

#include "common.hpp"
#include "kingdom/game/cAppGameBaseStaffRoll.hpp"

// Galaga Remix (g01) staff roll = job 183 (0x0b700000): kingdom::game::g01::cAppG01GalagaStaffRoll (DOL vtable 0x80396440, 0x2b0 bytes), job
// init 0x800b150c, ctor 0x800b3490 (staffroll.lzs, BGM m01bgm_Ending, +0x288 = 0x0b500000, RegistCamera), dtor 0x800b17c4.
namespace kingdom::game::g01
{
class cAppG01GalagaStaffRoll : public kingdom::game::cAppGameBaseStaffRoll
{
public:
	static void JobInit();       // DOL 0x800b150c
	cAppG01GalagaStaffRoll();    // DOL 0x800b3490
};
}
