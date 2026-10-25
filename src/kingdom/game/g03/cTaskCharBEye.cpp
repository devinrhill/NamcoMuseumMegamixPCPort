#include "kingdom/game/g03/cTaskCharBEye.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"

#include "cJetTask.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

cTaskCharBEye::cTaskCharBEye(cTaskModel* pParent)
{
	if (pParent != nullptr)
	{
		pParent->ConnectChild(this);
	}
	SetName("cTaskCharBEye");
	CreateModel(0, true);
	cTaskModelEx::cPBModel* cPBModel = pPBModel();
	if (cPBModel != nullptr)
	{
		cPBModel->RegistModel("BIN_BOS_EYE_MDL");
		cPBModel->IsDrawShadowDepth(true);
		cPBModel->SetLightSetIdx(1);
		cPBModel->SetFogIdx(0);
		cPBModel->RegistShadowModel("BIN_ITEMS_SHADOW_MDL");
		cPBModel->Invisible();
		::Constants::jetGeoVecSet(cPBModel->vTrans(), 0.0f, 0.0f, 0.0f, 1.0f);
		cPBModel->UpdateMatrix();
		cPBModel->RegistAnm(0u, "BIN_BOS_ANM_EYE_MOVE");
		cPBModel->get_ShadowModel()->Invisible();
	}
}

void cTaskCharBEye::Term()
{
	cTaskModelEx::Term();
}

void cTaskCharBEye::StartEye(const Vector4& pos)
{
	pModel()->Visible();
	pModel()->SetFrameAnmChr(0u, 0.0f);
	::Constants::jetGeoVecCopy(pModel()->vTrans(), pos);
	// TODO(unity): pModel().pScnMdlTransform().position = pModel().vTrans().GetUnityPos();
}

bool cTaskCharBEye::IsEndEye()
{
	if (pModel()->GetFrameMaxAnmChr(0u) <= pModel()->GetFrameAnmChr(0u))
	{
		return true;
	}
	return false;
}

cTaskCharBEye* cTaskCharBEye::Create(cTaskModel* pParent)
{
	return new cTaskCharBEye(pParent);
}

bool cTaskCharBEye::Execute(cJetTask* pParam)
{
	if (!Constants::IS_PAC_PAUSE() && pPBModel()->IsVisible() && !Constants::PACMAN_OBJ()->IsMiss())
	{
		UpdateFrame();
		if ((uint)pModel()->GetFrameAnmChr(0u) == 30)   // native frames (the C# converted Unity seconds back with * 60)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE087_BOSS_IJIKE_4);
		}
		if (pModel()->GetFrameMaxAnmChr(0u) <= pModel()->GetFrameAnmChr(0u))
		{
			pPBModel()->Invisible();
			pPBModel()->get_ShadowModel()->Invisible();
		}
	}
	return cTaskModelEx::Execute(pParam);
}

void cTaskCharBEye::Draw(int nStatus, cJetTask* pParam)
{
	cTaskModelEx::Draw(nStatus, pParam);
}

}
