#pragma once

#include "common.hpp"
#include "kingdom/game/cAppGameBaseGameOver.hpp"

// Grobda Remix (g11) game over = job 341 (0x15500000): kingdom::game::g11::cAppG11GrobdaGameOver (DOL 0x2b0 bytes), job init 0x8000f380 (term 0x8000f610
// = ReleaseMainTask), ctor FUN_80122d10: BIN_ANT_GAMEOVER (crc 0x2e043f4b, g11resident.lzs) anim 0 on anita 0, +0x294 = 0, next job the menu
// (0x15200000). Only the Grobda debug menu and an unreachable code site load it (NOTES "Unused / unreachable code").
namespace kingdom::game::g11
{
class cAppG11GrobdaGameOver : public kingdom::game::cAppGameBaseGameOver
{
public:
	static void JobInit();       // DOL 0x8000f380
	cAppG11GrobdaGameOver();     // DOL 0x80122d10
};
}
