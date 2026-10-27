#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cAppGameBaseMenuMatching.hpp"

// Rally-X Remix (g02) player matching = job 199 (0x0c700000): kingdom::game::g02::cAppG02RallyXMatching (DOL vtable 0x80438618, 0x2f8 bytes),
// job init 0x800c6a44, on the shared kingdom::game::menu::cAppGameBaseMenuMatching (0x801cf828). Rally-X's parts: ctor 0x8019a7cc (layouts
// COMMON_ANITA_POINTER 0x1a3f6a03, COMMON_ANITA_MATCHING 0xa6529323, camera COMMON_SCA_MATCHING_01 0x33494442, set COMMON_SET_MATCHING_01
// 0x62a31a3d; menu job 0x0c200000 = 194, multi job 0x0c800000 = 200) and term 0x8019a828 (from the dtor 0x800c6e38: the 0x154 byte matching
// data into the sys +0x64, confirmed or not).
namespace kingdom::game::g02
{
class cAppG02RallyXMatching : public kingdom::game::menu::cAppGameBaseMenuMatching
{
public:
	static void JobInit();       // DOL 0x800c6a44
	cAppG02RallyXMatching();     // DOL 0x8019a7cc
	void Free() override;        // DOL 0x8019a828 (from the dtor 0x800c6e38)
};
}
