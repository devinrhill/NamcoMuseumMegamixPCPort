#pragma once

#include "kingdom/game/menu/cAppGameBaseNunchuk.hpp"

// Galaga Remix (g01) "connect the Nunchuk" card = job 184 (0x0b800000): kingdom::game::g01::cAppG01GalagaNunchuk (DOL vtable 0x80396298),
// job init 0x800b1658, ctor 0x800af620, IsSkip 0x800af6a8, dtor 0x800af724. Then the stage (0x0b300000).
namespace kingdom::game::g01
{
class cAppG01GalagaNunchuk : public kingdom::game::menu::cAppGameBaseNunchuk
{
public:
	static void JobInit();       // DOL 0x800b1658
	cAppG01GalagaNunchuk();      // DOL 0x800af620
	~cAppG01GalagaNunchuk() override;   // DOL 0x800af724 (DetachMenuArc)

protected:
	bool IsSkip() override;      // DOL 0x800af6a8
};
}
