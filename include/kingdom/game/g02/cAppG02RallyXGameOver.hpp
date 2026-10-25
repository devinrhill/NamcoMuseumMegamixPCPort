#pragma once

#include "common.hpp"
#include "kingdom/game/cAppGameBaseGameOver.hpp"

// Rally-X Remix (g02) game over = job 198 (0x0c600000): kingdom::game::g02::cAppG02RallyXGameOver (DOL vtable 0x80396b70, 0x2b0 bytes), job
// init 0x800c68c4 (term 0x800c6a3c = ReleaseMainTask). Ported from main.dol (no C# exists for Rally-X). The ctor 0x800b329c attaches the stage
// archives (sys 0x800c5a1c: g02game.lzs holds ANITA_GAMEOVER), anim 0 of ANITA_GAMEOVER (crc 0xb284d9e6) on anita 0, then the stage select
// (0x0c200000). Execute 0x800b3344 is a tail call of the base one (cAppGameBaseGameOver::Execute 0x80143110); the dtor 0x800b3348 detaches the
// stage archives (0x800c5a78).
namespace kingdom::game::g02
{
class cAppG02RallyXGameOver : public kingdom::game::cAppGameBaseGameOver
{
public:
	static void JobInit();       // DOL 0x800c68c4
	cAppG02RallyXGameOver();     // DOL 0x800b329c
	// DOL 0x800b3348 (dtor: DetachStageArc): natively in Free
	void Free() override;
};
}
