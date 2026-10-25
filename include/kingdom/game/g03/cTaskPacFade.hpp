#pragma once

#include "common.hpp"
#include "kingdom/cTaskFade.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskPacFade : public cTaskFade
{
public:
	cTaskPacFade();

	bool Execute(cJetTask* pParam) override;
};

}
