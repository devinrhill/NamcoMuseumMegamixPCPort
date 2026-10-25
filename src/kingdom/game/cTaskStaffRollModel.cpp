#include "kingdom/game/cTaskStaffRollModel.hpp"

#include "cJetTask.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game
{

cTaskStaffRollModel::~cTaskStaffRollModel()
{
}

bool cTaskStaffRollModel::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::KingSys->pAppMain()->IsPause())
	{
		return true;
	}
	cTaskModel::UpdateFrame();
	cTaskModel::Execute(pParam);
	return true;
}

}
