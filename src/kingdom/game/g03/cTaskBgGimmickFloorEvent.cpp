#include "kingdom/game/g03/cTaskBgGimmickFloorEvent.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "ResNode.hpp"

namespace kingdom::game::g03
{

bool cTaskBgGimmickFloorEvent::IsBoot()
{
	return m_bBoot;
}

cTaskBgGimmickFloorEvent::cTaskBgGimmickFloorEvent(ResNode* const& node0, ResNode* const& node1, const Constants::eFACE_KIND_EX& kindEx)
	: cTaskBgGimmick(node0, node1)
{
	SetName("cTaskBgGimmickFloorEvent");
	Init();
	m_nKindEx = kindEx;
}

void cTaskBgGimmickFloorEvent::Init()
{
	m_nKindEx = Constants::eFACE_KIND_EX::eFACE_KIND_EX_NONE;
	m_bBoot = false;
	IsDrawShadowDepth(true);
	this->Model->UpdateFrame();
}

void cTaskBgGimmickFloorEvent::Term()
{
	cTaskBgGimmick::Term();
}

bool cTaskBgGimmickFloorEvent::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	cTaskCharPacman* cTaskCharPacman2 = cAppG03PacnRollGame2->pTaskCharPacman();
	if (!Constants::IS_PAC_PAUSE() && !cAppG03PacnRollGame2->IsNakajimaPause(this))
	{
		if (!m_bBoot && cTaskCharPacman2 != nullptr && cTaskCharPacman2->pHitFace(1) != nullptr && cTaskCharPacman2->pHitFace(1)->nKindEx == (uint)m_nKindEx)
		{
			m_bBoot = true;
		}
		if (m_bBoot)
		{
			this->Model->UpdateFrame();
		}
	}
	UpdateMatrix();
	return BaseExecute(pParam);
}

void cTaskBgGimmickFloorEvent::Draw(int nStatus, cJetTask* pParam)
{
	cTaskBgGimmick::Draw(nStatus, pParam);
}

}
