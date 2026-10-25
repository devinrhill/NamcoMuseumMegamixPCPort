#pragma once

#include "common.hpp"
#include "kingdom/game/cAppGameBaseStaffRoll.hpp"

// Rally-X Remix (g02) staff roll = job 206 (0x0ce00000): kingdom::game::g02::cAppG02RallyXStaffRoll (DOL vtable 0x80438688, 0x2b0 bytes), job
// init 0x800c6b80 (term 0x800c6cc4 = ReleaseMainTask), ctor 0x80112b44 (staffroll.lzs, BGM m02bgm_Ending, +0x288 = 0x0c600000 the game over,
// RegistCamera), dtor 0x800c6eec (0x80112b94: detaches staffroll.lzs). Entered once after every stage was cleared. Ported from main.dol.
namespace kingdom::game::g02
{
class cAppG02RallyXStaffRoll : public kingdom::game::cAppGameBaseStaffRoll
{
public:
	static void JobInit();       // DOL 0x800c6b80
	cAppG02RallyXStaffRoll();    // DOL 0x80112b44
};
}
