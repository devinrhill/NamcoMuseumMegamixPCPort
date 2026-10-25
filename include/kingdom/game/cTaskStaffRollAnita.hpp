#pragma once

#include "common.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "cJetTask.hpp"

namespace kingdom::game
{

class cTaskStaffRollAnita : public cTaskAnita
{
public:
	virtual ~cTaskStaffRollAnita();

	bool Execute(cJetTask* pParam) override;
};

}
