#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgGimmickFloorEvent.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmickIronBall : public cTaskBgGimmickFloorEvent
{
private:
	static constexpr float BALL_RADIUS = 2.0f;

public:
	cTaskBgGimmickIronBall(ResNode* const& node0, ResNode* const& node1, const Constants::eFACE_KIND_EX& kindEx);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	void HitCheck();

protected:
	void ReleaseTerm() override { cTaskBgGimmickIronBall::Term(); cTaskPushBack::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
