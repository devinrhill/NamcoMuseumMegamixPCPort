#include "kingdom/game/g04/cAppG04WaniWaniNunchuk.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "KingdomGlobal.hpp"
#include "WiiPad.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/game/g04/cG04WaniWaniSys.hpp"

namespace kingdom::game::g04
{

// DOL 0x8019a5ac
void cAppG04WaniWaniNunchuk::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG04WaniWaniNunchuk());
}

// DOL 0x80132d8c (the layout through vtable +0x4c = RegistLayout)
cAppG04WaniWaniNunchuk::cAppG04WaniWaniNunchuk()
{
	RegistLayout("COMMON_ANITA_NUNCHUK");   // 0xf44c4825
	nNextJob() = job::WANIWANI_GAME;
	SetName("cAppG04WaniWaniNunchuk");
}

// DOL 0x80132dd4
bool cAppG04WaniWaniNunchuk::IsSkip()
{
	cG04WaniWaniSys* sys = KingdomGlobal::WaniWaniSys;
	if (sys->nPlayers() == 1) return WiiPad::NunchukConnected(0);
	for (int i = 0; i < sys->nPlayers(); i++)
		if (!WiiPad::NunchukConnected(sys->nWork(i))) return false;
	return true;
}

}
