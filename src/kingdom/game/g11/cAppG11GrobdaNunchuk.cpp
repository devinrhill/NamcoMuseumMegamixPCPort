#include "kingdom/game/g11/cAppG11GrobdaNunchuk.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"

namespace kingdom::game::g11
{

// DOL 0x8000f804 [audited] (job 344)
void cAppG11GrobdaNunchuk::JobInit()
{
	if (KingdomGlobal::GrobdaSys == nullptr) KingdomGlobal::GrobdaSys = new cG11GrobdaSys();   // NATIVE: the sys is made by job 336 in the DOL; --job skips it
	KingdomGlobal::SysCore->RegistMainTask(new cAppG11GrobdaNunchuk());
}

// DOL 0x80122db4 [audited]. The asynchronous load of the stage archive (cKingdomSys::RegistArcDataAS(StageArc(sys +0x20))) is left out: the native stage job
// attaches its archives itself (as the other Nunchuk cards).
cAppG11GrobdaNunchuk::cAppG11GrobdaNunchuk()
{
	RegistLayout("BIN_COMMON_ANT_NUNCHUK");   // vtable +0x4c(this, 0xbddf0075)
	nNextJob() = job::GROBDA_GAME;
	SetName("cAppG11GrobdaNunchuk");
}

}
