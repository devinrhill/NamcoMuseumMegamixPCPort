#include "kingdom/game/g03/cTaskGoal.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/sSaveDataGoal.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ReadFileUtil.hpp>

namespace kingdom::game::g03
{

bool cTaskGoal::IsGoal()
{
	return m_bGoal;
}

cTaskGoal::cTaskGoal()
{
	SetName("cTaskGoal");
	Init();
}

void cTaskGoal::Init()
{
	m_bGoal = false;
	const std::vector<byte>& array = KingdomGlobal::KingSys->SearchData("BIN_GOAL_SET_DAT");
	if (!array.empty())
	{
		sSaveDataGoal* outParam;
		int _discard1;
		ReadFileUtil::Convert<sSaveDataGoal>(array, outParam, _discard1);
		this->Model->RegistModel("BIN_GOAL_MDL");
		this->Model->RegistAnm(0u, "BIN_GOAL_CHA");
		this->Model->SetLightSetIdx(3);
		this->Model->SetFogIdx(0);
		::Constants::jetGeoVecCopy(this->Model->vTrans(), outParam->trans);
		this->Model->vRotate()[1] = ::Constants::JET_DEG2RAD(outParam->rotY);
		this->Model->AttachShadow();
		this->Model->UpdateMatrix();
		Matrix4x4 m;
		::Constants::jetGeoMtxIdentity(m);
		::Constants::jetGeoMtxTrans(m, m, this->Model->vTrans());
		::Constants::jetGeoMtxRotZYX(m, m, this->Model->vRotate());
		DummyNW4R::SetMtx(this->Model->pScnMdl(), m);
	}
}

void cTaskGoal::Term()
{
	cTaskPBModel::Term();
}

bool cTaskGoal::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	if (nullptr != this->Model->pScnMdl())
	{
		if (!m_bGoal && HitCheck())
		{
			m_bGoal = true;
		}
		if (!Constants::IS_PAC_PAUSE())
		{
			this->Model->UpdateFrame();
		}
	}
	return true;
}

void cTaskGoal::Draw(int nStatus, cJetTask* pParam)
{
	if (nullptr != this->Model->pScnMdl())
	{
		PushBack();
	}
}

bool cTaskGoal::HitCheck()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	Vector4 v;
	::Constants::jetGeoVecCopy(v, this->Model->vTrans());
	v[1] += cTaskCharPacman2->fRadius();
	Constants::sFace* sFace = cTaskCharPacman2->pHitFace(1);
	if (sFace != nullptr && !cTaskCharPacman2->IsState(16u) && (sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_NORM || sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_GRAS))
	{
		::Constants::jetGeoVecSub(v, cTaskCharPacman2->vTrans(), v);
		if (::Constants::jetGeoVecInnerProduct(v, v) <= 6.7599993f)
		{
			return true;
		}
	}
	return false;
}

}
