#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskGimmickBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"

namespace kingdom::game::g07
{

class cTaskGimmickReaction : public cTaskGimmickBase
{
private:
	float m_fHatchComa{};

public:
	cTaskGimmickReaction();

	static cTaskGimmickBase* Construct();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;

	bool Execute(cJetTask* pParam) override;

	bool IsAction(cTaskChar* pChar) override;
};

}
