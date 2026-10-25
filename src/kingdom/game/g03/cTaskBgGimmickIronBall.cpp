#include "kingdom/game/g03/cTaskBgGimmickIronBall.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "ResNode.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskBgGimmickIronBall::cTaskBgGimmickIronBall(ResNode* const& node0, ResNode* const& node1, const Constants::eFACE_KIND_EX& kindEx)
	: cTaskBgGimmickFloorEvent(node0, node1, kindEx)
{
	SetName("cTaskBgGimmickIronBall");
	Init();
}

void cTaskBgGimmickIronBall::Init()
{
	IsDrawShadowDepth(true);
	std::vector<SkinnedMeshRenderer*> componentsInChildren; // TODO(unity): componentsInChildren = base.Model.pScnMdl().GetComponentsInChildren<SkinnedMeshRenderer>()
	for (SkinnedMeshRenderer* obj : componentsInChildren)
	{
		Bounds localBounds = Bounds(); // TODO(unity): localBounds = obj.localBounds
		localBounds.Expand(1000.0f); // the converter dropped the argument (C#: Expand(1000f))
		// TODO(unity): obj.localBounds = localBounds;
	}
	m_forceUpdateMtx = true;
}

void cTaskBgGimmickIronBall::Term()
{
	cTaskBgGimmickFloorEvent::Term();
}

bool cTaskBgGimmickIronBall::Execute(cJetTask* pParam)
{
	cTaskBgGimmickFloorEvent::Execute(pParam);
	HitCheck();
	return true;
}

void cTaskBgGimmickIronBall::Draw(int nStatus, cJetTask* pParam)
{
	cTaskBgGimmickFloorEvent::Draw(nStatus, pParam);
}

void cTaskBgGimmickIronBall::HitCheck()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	if (cTaskCharPacman2 != nullptr && !cTaskCharPacman2->IsState(8u) && cTaskCharPacman2->nHP() != 0)
	{
		Vector4 v;
		::Constants::jetGeoVecTrans(v, pMtx(), this->Model->vTrans());
		Vector4 v2;
		::Constants::jetGeoVecSub(v2, cTaskCharPacman2->vTrans(), v);
		float num = ::Constants::jetGeoVecInnerProduct(v2, v2);
		float num2 = cTaskCharPacman2->fRadius() * cTaskCharPacman2->fRadius() + 4.0f;
		if (num < num2)
		{
			cTaskCharPacman2->DamageProcess(1u, v2, 0.35f);
		}
	}
}

}
