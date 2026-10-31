#include "kingdom/game/g07/cTaskPause.hpp"
#include "kingdom/KingdomJobTable.hpp"
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

// DOL 0x80202e2c [audited: RegistData names, matching data only in multiplayer, menu job vs decomp] (ctor 0x800a59cc)
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
		nMenuJob() = kingdom::job::MOTOS_MULTI_MENU;
	}
	else
	{
		nMenuJob() = kingdom::job::MOTOS_MENU;
	}
}

void cTaskPause::Term()
{
}

}
