#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cAppGameBaseMenuMatching.hpp"

// Grobda Remix (g11) player matching = job 342 (0x15600000): kingdom::game::g11::cAppG11GrobdaMatching (DOL vtable 0x80389a34, 0x2f8 bytes),
// job init 0x8000fa64, on the shared kingdom::game::menu::cAppGameBaseMenuMatching (0x801cf828). Grobda's parts: ctor 0x8014a450 (layouts
// BIN_COMMON_ANT_CURSOR 0x06ca4270, BIN_COMMON_ANT_MATCHING 0xcd70519f, BIN_COMMON_CAMERA_MATCHING 0x08ffab97, BIN_COMMON_MDL_MATCHING 0x1eac8586
// from g11resident; menu and multi job both 0x15200000 = job 338) and term 0x8014a4a8 (confirmed: the 0x154 byte matching data into the sys and
// sys +0x20 = 1; cancelled: 1 player, sys +0x20 = 0).
namespace kingdom::game::g11
{
class cAppG11GrobdaMatching : public kingdom::game::menu::cAppGameBaseMenuMatching
{
public:
	static void JobInit();       // DOL 0x8000fa64
	cAppG11GrobdaMatching();     // DOL 0x8014a450
	void Free() override;        // DOL 0x8014a4a8 (from the dtor 0x8000fe88)
};
}
