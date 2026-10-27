#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmickMerryGoRound : public cTaskBgGimmick
{
private:
	float m_fLength{};

public:
	cTaskBgGimmickMerryGoRound(ResNode* const& node0, ResNode* const& node1);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nState, cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskBgGimmickMerryGoRound::Term(); cTaskPushBack::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
