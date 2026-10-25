#include "kingdom/game/g02/cAppG02RallyXNunchuk.hpp"

#include "KingdomGlobal.hpp"
#include "WiiPad.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"

namespace kingdom::game::g02
{

// DOL 0x800c6ccc (job 201)
void cAppG02RallyXNunchuk::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG02RallyXNunchuk());
}

// DOL 0x80122ab0: the menu archive (COMMON_ANITA_NUNCHUK 0xf44c4825), next job the stage. The asynchronous load of the stage archive
// (sys 0x800c5acc(0) -> cKingdomSys::RegistArcDataAS of table 0x803710e0) is left out: the native stage job attaches its archives itself
// (as the Galaga Nunchuk card).
cAppG02RallyXNunchuk::cAppG02RallyXNunchuk()
{
	cG02RallyXSysExt::AttachMenuArc();
	RegistLayout("COMMON_ANITA_NUNCHUK");
	nNextJob() = 0x0c300000u;
	SetName("cAppG02RallyXNunchuk");
}

// DOL 0x80122bec
cAppG02RallyXNunchuk::~cAppG02RallyXNunchuk()
{
	cG02RallyXSysExt::DetachMenuArc();
}

// DOL 0x80122b10. Single player (sys 0x800c5ca0 = not the multiplayer world): skip when player 0's control type (0x800c5b88) is 1 (remote
// only) or pad 0 has a Nunchuk (KPAD extension +0xb0 -> +0x5c == 1). Multiplayer: skip when every player (pad = sys 0x800c5b98(i)) has
// type 1 or a Nunchuk.
bool cAppG02RallyXNunchuk::IsSkip()
{
	if (!KingdomGlobal::RallyXSys->IsMulti())
	{
		if (cG02RallyXSysExt::nCtrlType[0] == 1) return true;
		return WiiPad::NunchukConnected(0);
	}
	for (int i = 0; i < cG02RallyXSysExt::nPlayer && i < 4; i++)
	{
		const int pad = cG02RallyXSysExt::nPadNo[i];
		if (cG02RallyXSysExt::nCtrlType[pad] != 1 && !WiiPad::NunchukConnected(pad)) return false;   // 0x800c5b88(sys, pad), as in the DOL
	}
	return true;
}

}
