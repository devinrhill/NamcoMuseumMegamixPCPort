#pragma once

#include "kingdom/game/cTaskGameBasePause.hpp"

// Grobda Remix (g11) pause menu: kingdom::game::g11::cTaskGrobdaPause (0x1f0 bytes, ctor/Init FUN_80037378). Same shared
// cTaskGameBasePause as Rally-X (BIN_COMMON_ANT_PAUSE + BIN_COMMON_ANT_CURSOR); menu job (+0x1c8) = 0x15200000 (job 338).
// The app (+0x29c) starts it with FUN_80147c8c on HOME / MINUS (pad trig & 0x500) and copies its pause flag (+0x1d4) to app +0x1b4.
namespace kingdom::game::g11
{
class cTaskGrobdaPause : public kingdom::game::cTaskGameBasePause
{
public:
	cTaskGrobdaPause();   // DOL 0x80037378
};
}
