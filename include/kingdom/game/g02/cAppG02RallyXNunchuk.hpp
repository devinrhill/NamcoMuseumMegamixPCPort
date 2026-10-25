#pragma once

#include "kingdom/game/menu/cAppGameBaseNunchuk.hpp"

// Rally-X Remix (g02) "connect the Nunchuk" card = job 201 (0x0c900000): kingdom::game::g02::cAppG02RallyXNunchuk (DOL vtable 0x80440f98),
// job init 0x800c6ccc (term 0x800c6e30 = ReleaseMainTask), ctor 0x80122ab0, IsSkip 0x80122b10, dtor 0x80122bec. Then the stage
// (0x0c300000). Ported from main.dol (no C# exists for Rally-X).
namespace kingdom::game::g02
{
class cAppG02RallyXNunchuk : public kingdom::game::menu::cAppGameBaseNunchuk
{
public:
	static void JobInit();       // DOL 0x800c6ccc
	cAppG02RallyXNunchuk();      // DOL 0x80122ab0
	~cAppG02RallyXNunchuk() override;   // DOL 0x80122bec (DetachMenuArc)

protected:
	bool IsSkip() override;      // DOL 0x80122b10
};
}
