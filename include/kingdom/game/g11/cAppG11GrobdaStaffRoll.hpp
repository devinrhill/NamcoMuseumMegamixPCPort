#pragma once

#include "common.hpp"
#include "kingdom/game/cAppGameBaseStaffRoll.hpp"

// Grobda Remix (g11) staff roll = job 350 (0x15e00000): kingdom::game::g11::cAppG11GrobdaStaffRoll (DOL 0x2b0 bytes), job init 0x8000f618 (term 0x8000f7fc
// = ReleaseMainTask), ctor 0x80207f04 (staffroll.lzs, +0x288 = 0x15500000 the game over, RegistCamera), dtor 0x80207f4c (detaches staffroll.lzs).
// Only the Grobda debug menu loads it.
namespace kingdom::game::g11
{
class cAppG11GrobdaStaffRoll : public kingdom::game::cAppGameBaseStaffRoll
{
public:
	static void JobInit();       // DOL 0x8000f618
	cAppG11GrobdaStaffRoll();    // DOL 0x80207f04
};
}
