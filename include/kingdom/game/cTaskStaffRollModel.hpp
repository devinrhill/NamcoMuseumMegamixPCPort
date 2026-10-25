#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "cJetTask.hpp"

namespace kingdom::game
{

class cTaskStaffRollModel : public cTaskModel
{
public:
	virtual ~cTaskStaffRollModel();

	bool Execute(cJetTask* pParam) override;
};

}
