#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskStage : public cTaskKingdom
{
public:
	cTaskStage();

	virtual ~cTaskStage();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskStage::Term(); cTaskKingdom::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
