#pragma once

#include "kingdom/game/menu/cAppGameBaseNunchuk.hpp"

// Grobda Remix (g11) "connect the Nunchuk" card = job 344 (0x15800000): kingdom::game::g11::cAppG11GrobdaNunchuk (DOL vtable 0x80441020, 0x2a0 bytes),
// job init 0x8000f804 (term 0x8000fa5c = ReleaseMainTask), ctor FUN_80122db4: layout BIN_COMMON_ANT_NUNCHUK (crc 0xbddf0075), next job the stage
// (0x15300000). IsSkip (vtable sub +0x0c 0x80122e20) is `return 1`: the card is always skipped.
namespace kingdom::game::g11
{
class cAppG11GrobdaNunchuk : public kingdom::game::menu::cAppGameBaseNunchuk
{
public:
	static void JobInit();       // DOL 0x8000f804
	cAppG11GrobdaNunchuk();      // DOL 0x80122db4
protected:
	bool IsSkip() override { return true; }   // DOL 0x80122e20
};
}
