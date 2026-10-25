#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmickFloorEvent : public cTaskBgGimmick
{
private:
	Constants::eFACE_KIND_EX m_nKindEx{};

	bool m_bBoot{};

public:
	bool IsBoot();

	cTaskBgGimmickFloorEvent(ResNode* const& node0, ResNode* const& node1, const Constants::eFACE_KIND_EX& kindEx);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;
};

}
