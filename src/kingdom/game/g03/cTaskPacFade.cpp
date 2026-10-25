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

bool cTaskPacFade::Execute(cJetTask* pParam)
{
	if (!Constants::IS_PAC_PAUSE())
	{
		UpdateFrame();
	}
	return true;
}

}
