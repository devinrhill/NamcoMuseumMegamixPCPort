#pragma once

#include "common.hpp"
#include "kingdom/game/cAppGameBaseStaffRoll.hpp"

// The collection's own staff roll = job 139 (0x08b00000): kingdom::menu::cAppG10StaffRoll (DOL 0x2b0 bytes), job init 0x8019b250 (term 0x8019b394
// = ReleaseMainTask), ctor tail 0x80202ed0 (staffroll.lzs, BGM "m03bgm_Ending", +0x288 = 0x08900000 the title, RegistCamera), dtor 0x8019b39c
// (0x80202f20 detaches staffroll.lzs). Nothing in retail's job flow asks for 139 (reachable with --job).
namespace kingdom::menu
{
class cAppG10StaffRoll : public kingdom::game::cAppGameBaseStaffRoll
{
public:
	static void JobInit();     // DOL 0x8019b250
	cAppG10StaffRoll();        // DOL 0x80202ed0
};
}
