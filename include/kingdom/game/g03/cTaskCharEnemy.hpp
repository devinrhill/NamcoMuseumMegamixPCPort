#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskCharEnemy : public cTaskChar
{
private:
	void Init();

public:
	void Term() override;

	virtual void SetModel();

	cTaskCharEnemy();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;
};

}
