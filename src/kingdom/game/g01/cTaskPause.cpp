#include "kingdom/game/g01/cTaskPause.hpp"
#include "kingdom/KingdomJobTable.hpp"

namespace kingdom::game::g01
{

// DOL 0x801b92cc + Init 0x800f6060 (the matching data is the multiplayer one; alone none is passed natively)
cTaskPause::cTaskPause()
{
	RegistData("ANT_PAUSE", "ANT_MENU_POINTER", nullptr);   // 0x80147a64(this, 0xdf5903f3, 0xf63671cb, matching)
	nMenuJob() = job::GALAGA_MENU;
}

cTaskPause::~cTaskPause()
{
	Free();
}

}
