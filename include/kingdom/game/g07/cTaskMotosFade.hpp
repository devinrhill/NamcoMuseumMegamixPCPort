#pragma once

#include "common.hpp"
#include "kingdom/cTaskFade.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g07
{

class cTaskMotosFade : public cTaskFade
{
public:
	cTaskMotosFade();

	bool Execute(cJetTask* pParam) override;
};

}
