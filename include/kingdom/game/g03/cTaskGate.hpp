#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskBgGate.hpp"

namespace kingdom::game::g03
{

class cTaskGate : public cTaskKingdom
{
private:
	std::vector<cTaskBgGate*> m_vecGate;

	cTaskBgGate* m_pCurrentGate{};

public:
	cTaskBgGate* pCurrentGate();

	cTaskGate();

private:
	void Init();

public:
	void Term();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	void SearchCurrent();

public:
	void SetRetryData();
};

}
