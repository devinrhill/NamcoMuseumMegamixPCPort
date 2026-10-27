#include "kingdom/game/g01/cAppG01GalagaNunchuk.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "InGamePadManager.hpp"
#include "KingdomGlobal.hpp"
#include "WiiPad.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"

namespace kingdom::game::g01
{

// DOL 0x800b1658 (job 184)
void cAppG01GalagaNunchuk::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG01GalagaNunchuk());
}

// DOL 0x800af620: the menu archive (ANT_NUNCHUK 0x21d4da2a is in g01menu.lzs), next job the stage; RegistStageArcAS (the asynchronous load of
// the stage archives) is left out: the native stage job attaches its archives itself
cAppG01GalagaNunchuk::cAppG01GalagaNunchuk()
{
	cG01GalagaSys::AttachMenuArc();
	RegistLayout("ANT_NUNCHUK");
	nNextJob() = job::GALAGA_GAME;
	SetName("cAppG01GalagaNunchuk");
}

// DOL 0x800af724
cAppG01GalagaNunchuk::~cAppG01GalagaNunchuk()
{
	cG01GalagaSys::DetachMenuArc();
}

// DOL 0x800af6a8: every player's remote (pad nWork(player)) has a Nunchuk (KPAD extension +0xb0 -> +0x5c == 1)
bool cAppG01GalagaNunchuk::IsSkip()
{
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	for (int i = 0; i < sys->nPlayer(); i++)
		if (!WiiPad::NunchukConnected(sys->nWork(i))) return false;
	return true;
}

}
