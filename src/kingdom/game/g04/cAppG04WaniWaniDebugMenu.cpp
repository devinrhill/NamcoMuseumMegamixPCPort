#include "kingdom/game/g04/cAppG04WaniWaniDebugMenu.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/game/g04/cG04WaniWaniSys.hpp"

namespace kingdom::game::g04
{

// DOL 0x80199f64 [audited]: FUN_800c2404(menu, "WaniWani", 3, table 0x80449340)
cAppG04WaniWaniDebugMenu::cAppG04WaniWaniDebugMenu()
{
	SetName("WaniWani Debug Menu");
	SetMenu("WaniWani", {{"GAME", job::WANIWANI_GAME}, {"GAME_MULTI", job::WANIWANI_GAME_MULTI}, {"EXIT", job::WANIWANI_TERM}});
}

// the sys is made by job 224 in the DOL; --job 0x0e100000 makes it here
void cAppG04WaniWaniDebugMenu::JobInit()
{
	if (KingdomGlobal::WaniWaniSys == nullptr) KingdomGlobal::WaniWaniSys = new cG04WaniWaniSys();
	KingdomGlobal::SysCore->RegistMainTask(new cAppG04WaniWaniDebugMenu());
}

}
