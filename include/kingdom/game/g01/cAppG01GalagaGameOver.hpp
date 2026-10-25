#pragma once

#include "common.hpp"
#include "kingdom/game/cAppGameBaseGameOver.hpp"

// Galaga Remix (g01) game over = job 181 (0x0b500000): kingdom::game::g01::cAppG01GalagaGameOver (DOL vtable 0x8043be58, 0x2b0 bytes).
// Only the ctor is Galaga's (0x800fbfd4): ANT_GAMEOVER (0x08e54dcf) anim 0, anita 0, then the stage select (0x0b200000). Execute 0x800fc074
// is the base one (FUN_8000d844, always 0 in this build, would go to 0x0bf00000).
namespace kingdom::game::g01
{
class cAppG01GalagaGameOver : public kingdom::game::cAppGameBaseGameOver
{
public:
	static void JobInit();     // DOL 0x800b1250
	cAppG01GalagaGameOver();   // DOL 0x800fbfd4
};
}
