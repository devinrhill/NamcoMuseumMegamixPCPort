#include "kingdom/game/g07/cTaskMotosFade.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "cJetTask.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g07
{

cTaskMotosFade::cTaskMotosFade()
{
	nDrawPriority(2415919103u);
}

// DOL 0x800a92f4 [audited: per AUDIT_G03_G07 row] (compared 2026-10-30; when ended the DOL also sets a flag byte in the app's scene object (+0x1b8), not read anywhere checked)
bool cTaskMotosFade::Execute(cJetTask* pParam)
{
	if (!IsEnd() && !KingdomGlobal::g_pMotosGame->IsPause())
	{
		UpdateFrame();
	}
	return true;
}

}
