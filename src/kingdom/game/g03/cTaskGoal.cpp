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

// DOL 0x801815b4 [audited: described by the 43rd-session pass] (the task is its own push-back model: Execute / Draw / dtor are reached through the +0xd8 sub-vtable 0x80447f04)
cTaskGoal::cTaskGoal()
{
	SetName("cTaskGoal");
	Init();
}

// DOL 0x801816c4 [audited: init, hit test (6.76, kinds 3/6, not hanging), update checked vs decomp]
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

// DOL 0x801817c4 [audited: init, hit test (6.76, kinds 3/6, not hanging), update checked vs decomp]
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

// DOL 0x8018194c [audited: init, hit test (6.76, kinds 3/6, not hanging), update checked vs decomp]
void cTaskGoal::Draw(int nStatus, cJetTask* pParam)
{
	if (nullptr != this->Model->pScnMdl())
	{
		PushBack();
	}
}

// inlined in Execute 0x801817c4: goal position + Pac-Man's radius, a floor face of kind 3 (NORM) or 6 (GRAS), not hanging (state 0x10), squared distance <= 6.76
bool cTaskGoal::HitCheck()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	Vector4 v;
	::Constants::jetGeoVecCopy(v, this->Model->vTrans());
	v[1] += cTaskCharPacman2->fRadius();
	Constants::sFace* sFace = cTaskCharPacman2->pHitFace(1);
	if (sFace != nullptr && !cTaskCharPacman2->IsState(cTaskCharPacman::STATE_HANG) && (sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_NORM || sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_GRAS))
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
