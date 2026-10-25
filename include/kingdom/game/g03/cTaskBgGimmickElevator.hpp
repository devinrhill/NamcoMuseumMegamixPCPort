#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmickElevator : public cTaskBgGimmick
{
private:
	static bool m_bBoot;

public:
	cTaskBgGimmickElevator(ResNode* const& node0, ResNode* const& node1);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void HitCallbackFloor(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW) override;
};

}
