#include "kingdom/game/g03/cTaskBgGimmickSwitch.hpp"
#include "DummyNW4R.hpp"

#include "cJetTask.hpp"
#include "kingdom/cTaskPBModel.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "ResNode.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

bool cTaskBgGimmickSwitch::IsBoot()
{
	return m_bBoot;
}

cTaskBgGimmickSwitch::cTaskBgGimmickSwitch(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgGimmick(node0, node1)
{
	SetName("cTaskBgGimmickSwitch");
	Init();
}

void cTaskBgGimmickSwitch::Init()
{
	m_bBoot = false;
	m_pSleepMdl = nullptr;
	m_pSleepMdl = new cTaskPBModel("BIN_SLEEP_SWITCH_MDL");
	m_pSleepMdl->Model->RegistModel("BIN_SLEEP_SWITCH_MDL");
	m_pSleepMdl->Model->Invisible();
	::Constants::jetGeoVecCopy(m_pSleepMdl->Model->vTrans(), this->Model->vTrans());
	::Constants::jetGeoVecCopy(m_pSleepMdl->Model->vRotate(), this->Model->vRotate());
	::Constants::jetGeoVecCopy(m_pSleepMdl->Model->vScale(), this->Model->vScale());
	m_pSleepMdl->Model->UpdateMatrix();
}

void cTaskBgGimmickSwitch::Term()
{
	m_pSleepMdl->Term();
	cTaskBgGimmick::Term();
}

bool cTaskBgGimmickSwitch::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (!Constants::IS_PAC_PAUSE() && cAppG03PacnRollGame2->IsGame() && !cAppG03PacnRollGame2->IsNakajimaPause(this))
	{
		if (pFaceList() != nullptr && IsCollision() && !m_bBoot)
		{
			cAppG03PacnRollGame2->pTaskCollision()->PushBackCollisionObj(this);
		}
		this->Model->UpdateFrame();
	}
	return true;
}

void cTaskBgGimmickSwitch::Draw(int nStatus, cJetTask* pParam)
{
	UpdateMatrix();
	Matrix4x4 matrix = pMtxRotTrans();
	DummyNW4R::SetMtx(this->Model->pScnMdl(), matrix);
	DummyNW4R::SetScale(this->Model->pScnMdl(), this->Model->vScale()[0], this->Model->vScale()[1], this->Model->vScale()[2]);
	DummyNW4R::SetMtx(m_pSleepMdl->Model->pScnMdl(), matrix);
	DummyNW4R::SetScale(m_pSleepMdl->Model->pScnMdl(), this->Model->vScale()[0], this->Model->vScale()[1], this->Model->vScale()[2]);
	if (!m_bBoot)
	{
		if (!this->Model->IsVisible())
		{
			this->Model->Visible();
		}
		if (m_pSleepMdl->Model->IsVisible())
		{
			m_pSleepMdl->Model->Invisible();
		}
	}
	else
	{
		if (this->Model->IsVisible())
		{
			this->Model->Invisible();
		}
		if (!m_pSleepMdl->Model->IsVisible())
		{
			m_pSleepMdl->Model->Visible();
		}
	}
}

void cTaskBgGimmickSwitch::HitCallbackFloor(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW)
{
	m_bBoot = true;
}

}
