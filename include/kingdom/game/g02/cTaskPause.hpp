#pragma once

#include "kingdom/game/cTaskGameBasePause.hpp"

// Rally-X Remix (g02) the pause menu: kingdom::game::g02::cTaskPause (0x1f0 bytes, vtable 0x804411a8, parent cTaskGameBasePause; ctor
// 0x80123e84 + Init 0x8007a244, dtor 0x8012734c). Everything else is the shared cTaskGameBasePause (Execute 0x80147cb0 / Draw 0x80148b88):
// COMMON_ANITA_PAUSE (0x6e1c482e) + COMMON_ANITA_POINTER (0x1a3f6a03); menu job 0x0c200000 (stage select) single player, 0x0c800000 multiplayer.
// The app (+0x2ac) starts it (0x80147c8c) on HOME / MINUS (pad & 0x500) in states 1..6 while the retry is idle and copies its pause flag
// (+0x1d4) to app +0x1b4 every frame (see NOTES "Rally-X game flow").
namespace kingdom::game::g02
{
class cTaskPause : public kingdom::game::cTaskGameBasePause
{
public:
	cTaskPause();                // DOL 0x80123e84 + 0x8007a244
	~cTaskPause() override;      // DOL 0x8012734c
};
}
