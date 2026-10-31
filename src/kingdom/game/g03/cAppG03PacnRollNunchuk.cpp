#include "kingdom/game/g03/cAppG03PacnRollNunchuk.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "WiiPad.hpp"

namespace kingdom::game::g03
{

// DOL 0x800fc87c [audited: per AUDIT_G03_G07 row]: layout crc 0xbddf0075 = BIN_COMMON_ANT_NUNCHUK (g03resident.lzs), next job 0x0d300000 (the game). The DOL then starts the
// asynchronous load of the stage archive (RegistArcDataAS of the round's file); the native game job loads its stage itself.
cAppG03PacnRollNunchuk::cAppG03PacnRollNunchuk()
{
	RegistLayout("BIN_COMMON_ANT_NUNCHUK");
	nNextJob() = job::PACNROLL_GAME;
	SetName("cAppG03PacnRollNunchuk");
}

// DOL 0x800fc8e8 [audited: per AUDIT_G03_G07 row]: skip the card when pad 0 has a Nunchuk (extension type byte == 1). PC: WiiPad::NunchukConnected (--no-nunchuk unplugs it)
bool cAppG03PacnRollNunchuk::IsSkip()
{
	return WiiPad::NunchukConnected(0);
}

}
