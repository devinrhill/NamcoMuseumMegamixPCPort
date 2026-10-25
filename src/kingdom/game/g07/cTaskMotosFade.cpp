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

bool cTaskMotosFade::Execute(cJetTask* pParam)
{
	if (!IsEnd() && !KingdomGlobal::g_pMotosGame->IsPause())
	{
		UpdateFrame();
	}
	return true;
}

}
