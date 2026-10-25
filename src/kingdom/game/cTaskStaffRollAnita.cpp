#include "kingdom/game/cTaskStaffRollAnita.hpp"

#include "cJetTask.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game
{

cTaskStaffRollAnita::~cTaskStaffRollAnita()
{
}

bool cTaskStaffRollAnita::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::KingSys->pAppMain()->IsPause())
	{
		return true;
	}
	cTaskAnita::Execute(pParam);
	return true;
}

}
