#include "kingdom/game/g02/cTaskRallyXFade.hpp"

#include "KingdomGlobal.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom::game::g02
{

// DOL 0x801242c4: cTaskFade (priority 0x8fffffff, name "cTaskFade"), then priority 0x8fffffff again
cTaskRallyXFade::cTaskRallyXFade()
{
	SetName("g02::cTaskRallyXFade");
	nDrawPriority(0x8fffffffu);
}

// DOL 0x801272c8: fade ended (+0x458) -> the global "fade done" byte r13 - 0x5798 (FUN_8009c6f4) = 1 (NOT PORTED: no reader in the port,
// the generic cTaskFade port does not keep it either); else unless the app (KingSys +0xa40) is paused: step the fade (0x800166f4)
bool cTaskRallyXFade::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (IsEnd()) return true;
	kingdom::cAppKingdom* app = KingdomGlobal::KingSys != nullptr ? KingdomGlobal::KingSys->pAppMain() : nullptr;
	if (app == nullptr || !app->IsPause()) UpdateFrame();
	return true;
}

}
