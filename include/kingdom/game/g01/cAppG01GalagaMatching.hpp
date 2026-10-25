#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cAppGameBaseMenuMatching.hpp"

// Galaga Remix (g01) player matching = job 182 (0x0b600000): kingdom::game::g01::cAppG01GalagaMatching (DOL vtable 0x803964c4, 0x2f8 bytes),
// job init 0x800b13d0, on the shared kingdom::game::menu::cAppGameBaseMenuMatching (0x801cf828). Galaga's parts: init 0x801059f4 (layouts
// ANT_MENU_POINTER 0xf63671cb, ANT_MATCHING 0x36381620, camera MATCHING_CAMERA_SCA_01 0xccf7a78c, set MATCHING_SELECT_SET_MDL 0x2a07a0b8;
// menu and multi job both 0x0b200000) and term 0x80105a4c (the matching data into cG01GalagaSys, or 1 player when cancelled).
namespace kingdom::game::g01
{
class cAppG01GalagaMatching : public kingdom::game::menu::cAppGameBaseMenuMatching
{
public:
	static void JobInit();       // DOL 0x800b13d0
	cAppG01GalagaMatching();     // DOL 0x801059f4
	void Free() override;        // DOL 0x80105a4c (from the dtor 0x800b1888)
};
}
