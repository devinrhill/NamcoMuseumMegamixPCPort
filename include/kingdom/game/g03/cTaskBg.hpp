#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskBgBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBg : public cTaskBgBase
{
public:
	void Initialize();

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;
};

}
