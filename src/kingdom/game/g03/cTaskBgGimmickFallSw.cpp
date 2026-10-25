#include "kingdom/game/g03/cTaskBgGimmickFallSw.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "ResNode.hpp"

namespace kingdom::game::g03
{

cTaskBgGimmickFallSw::cTaskBgGimmickFallSw(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgGimmickFall(node0, node1)
{
	SetName("cTaskBgGimmickFallSw");
	Init();
}

void cTaskBgGimmickFallSw::Init()
{
	IsReturn() = false;
}

void cTaskBgGimmickFallSw::Term()
{
	cTaskBgGimmickFall::Term();
}

bool cTaskBgGimmickFallSw::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (cAppG03PacnRollGame2->pTaskBgGimmickSwitch() != nullptr && cAppG03PacnRollGame2->pTaskBgGimmickSwitch()->IsBoot())
	{
		if (!IsBoot())
		{
			cAppG03PacnRollGame2->SetEffect(30u, cAppG03PacnRollGame2->pTaskBgGimmickSwitch()->Model->vTrans(), Vector4());
		}
		IsBoot() = true;
	}
	return cTaskBgGimmickFall::Execute(pParam);
}

void cTaskBgGimmickFallSw::Draw(int nStatus, cJetTask* pParam)
{
	cTaskBgGimmickFall::Draw(nStatus, pParam);
}

void cTaskBgGimmickFallSw::HitCallbackFloor(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW)
{
}

}
