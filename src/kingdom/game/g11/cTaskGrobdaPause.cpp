#include "kingdom/game/g11/cTaskGrobdaPause.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "KingdomGlobal.hpp"

namespace kingdom::game::g11
{

// DOL 0x80037378 (FUN_80147a64 = RegistData with the multiplayer matching block sys +4: NOT PORTED, nullptr passed)
cTaskGrobdaPause::cTaskGrobdaPause()
{
	SetName("g11::cTaskGrobdaPause");
	RegistData("BIN_COMMON_ANT_PAUSE", "BIN_COMMON_ANT_CURSOR", nullptr);   // crc 0x6e3ccd30 / 0x06ca4270 (g11resident.lzs)
	nMenuJob() = job::GROBDA_MENU;   // +0x1c8
}

}
