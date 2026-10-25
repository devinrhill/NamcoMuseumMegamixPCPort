#pragma once

#include "kingdom/game/cTaskGameBasePause.hpp"

// Galaga Remix (g01) the pause menu: kingdom::game::g01::cTaskPause (vtable 0x8044a9c0; ctor 0x801b92cc), the shared cTaskGameBasePause
// (Execute 0x80147cb0, Draw 0x80148b88) with Init 0x800f6060: ANT_PAUSE (0xdf5903f3) and ANT_MENU_POINTER (0xf63671cb) with the matching data,
// the menu job 0x0b200000 (Galaga's stage select). The apps start it (0x80147c8c) when a pad triggers HOME (0x400) or MINUS (0x100) in READY,
// GAME or MISS, and set the pause flag (DAT_80479118, cG01GalagaSys::IsPause) from it every frame.
namespace kingdom::game::g01
{
class cTaskPause : public kingdom::game::cTaskGameBasePause
{
public:
	cTaskPause();                // DOL 0x801b92cc + 0x800f6060
	~cTaskPause() override;      // DOL 0x801bafec
};
}
