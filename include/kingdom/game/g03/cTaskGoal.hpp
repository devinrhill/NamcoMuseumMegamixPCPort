#pragma once

#include "common.hpp"
#include "kingdom/cTaskPBModel.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskGoal : public cTaskPBModel
{
private:
	static constexpr float GOAL_RADIUS = 6.7599993f;

	bool m_bGoal{};

public:
	bool IsGoal();

	cTaskGoal();

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	bool HitCheck();
};

}
