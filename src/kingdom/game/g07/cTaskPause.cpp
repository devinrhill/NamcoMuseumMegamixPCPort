#include "kingdom/game/g07/cTaskPause.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g07
{

cTaskPause::cTaskPause()
{
	Init();
}

cTaskPause::~cTaskPause()
{
	Free();
}

void cTaskPause::Free()
{
	Term();
	cTaskGameBasePause::Free();
}

void cTaskPause::Init()
{
	if (!KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		RegistData(ResourceDatabase::GetPath("ANT_PAUSE"), ResourceDatabase::GetPath("ANT_CURSOR"));
	}
	else
	{
		RegistData(ResourceDatabase::GetPath("ANT_PAUSE"), ResourceDatabase::GetPath("ANT_CURSOR"), KingdomGlobal::MotosSys->pGetMatchingData());
	}
	if (KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		nMenuJob() = 292552704u;
	}
	else
	{
		nMenuJob() = 287309824u;
	}
}

void cTaskPause::Term()
{
}

}
