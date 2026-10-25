#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cAppGameBaseMenuMatching.hpp"

// Gator Panic (g04) player matching = job 229 (0x0e500000): cAppG04WaniWaniMatching (DOL vtable 0x80449358, init 0x8019a470) on the shared
// kingdom::game::menu::cAppGameBaseMenuMatching (0x801cf828). Ctor 0x80095044: COMMON_ANITA_POINTER 0x1a3f6a03, COMMON_ANITA_MATCHING 0xa6529323,
// COMMON_CAM_MATCHING_SCA 0x78f4875c, COMMON_SET_MATCHING_MDL 0xfb037b8d (g04resident3); menu job and multi job 0x0e700000 (job 231). Term 0x8009509c.
namespace kingdom::game::g04
{
class cAppG04WaniWaniMatching : public kingdom::game::menu::cAppGameBaseMenuMatching
{
public:
	static void JobInit();             // DOL 0x8019a470
	cAppG04WaniWaniMatching();         // DOL 0x80095044
	void Free() override;              // DOL 0x8009509c (from the dtor 0x8019a718)
};
}
