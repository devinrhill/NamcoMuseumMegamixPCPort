#include "kingdom/game/g03/cTaskPacFade.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"

namespace kingdom::game::g03
{

cTaskPacFade::cTaskPacFade()
{
	SetName("cTaskPacFade");
	nDrawPriority(2415919103u);
}

// DOL 0x80187488 [audited: per AUDIT_G03_G07 row]: if the fade ended (anita state) the task stores 1 in the global byte 0x80478f88 (cTaskFade::Fade 0x800093a0 stores 0; nothing reads it that I could find: UNVERIFIED) and does not update; otherwise UpdateFrame unless paused / HOME held
bool cTaskPacFade::Execute(cJetTask* pParam)
{
	if (!Constants::IS_PAC_PAUSE())
	{
		UpdateFrame();
	}
	return true;
}

}
