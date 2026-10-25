#pragma once

#include "kingdom/game/menu/cAppGameBaseNunchuk.hpp"

// Gator Panic Remix (g04) "connect the Nunchuk" card = job 231 (0x0e700000): kingdom::game::g04::cAppG04WaniWaniNunchuk (0x2a0 bytes, vtable
// 0x80442038; job init 0x8019a5ac, ctor 0x80132d8c: COMMON_ANITA_NUNCHUK, next job the game 0x0e300000; IsSkip 0x80132dd4 (secondary vtable
// 0x80442074): every player's remote (alone: pad 0, else the matching data's pads) has a Nunchuk (KPAD extension +0xb0 -> +0x5c == 1).
namespace kingdom::game::g04
{
class cAppG04WaniWaniNunchuk : public kingdom::game::menu::cAppGameBaseNunchuk
{
public:
	static void JobInit();             // DOL 0x8019a5ac (job 231)
	cAppG04WaniWaniNunchuk();          // DOL 0x80132d8c
	bool IsSkip() override;            // DOL 0x80132dd4
};
}
