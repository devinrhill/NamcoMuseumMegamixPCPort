#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskBgBox.hpp"

namespace kingdom::game::g03
{

class cTaskBox : public cTaskKingdom
{
private:
	std::vector<cTaskBgBox*> m_vecBox;

public:
	cTaskBox();

private:
	void Init();

public:
	void Term();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void SetRetryData();
};

}
