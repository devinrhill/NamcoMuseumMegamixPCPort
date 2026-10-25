#include "kingdom/game/g03/cTaskBgGimmickElevator.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "ResNode.hpp"

namespace kingdom::game::g03
{

bool cTaskBgGimmickElevator::m_bBoot{};

cTaskBgGimmickElevator::cTaskBgGimmickElevator(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgGimmick(node0, node1)
{
	SetName("cTaskBgGimmick");
	Init();
}

void cTaskBgGimmickElevator::Init()
{
	OnState(2u);
}

void cTaskBgGimmickElevator::Term()
{
	m_bBoot = false;
	cTaskBgGimmick::Term();
}

bool cTaskBgGimmickElevator::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (!Constants::IS_PAC_PAUSE() && cAppG03PacnRollGame2->IsGame() && m_bBoot && !cAppG03PacnRollGame2->IsNakajimaPause(this))
	{
		this->Model->UpdateFrame();
	}
	UpdateMatrix();
	return BaseExecute(pParam);
}

void cTaskBgGimmickElevator::Draw(int nStatus, cJetTask* pParam)
{
	cTaskBgGimmick::Draw(nStatus, pParam);
}

void cTaskBgGimmickElevator::HitCallbackFloor(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW)
{
	m_bBoot = true;
}

}
