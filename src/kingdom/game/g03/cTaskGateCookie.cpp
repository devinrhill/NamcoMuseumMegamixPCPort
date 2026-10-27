#include "kingdom/game/g03/cTaskGateCookie.hpp"

#include "cJetTask.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

void cTaskGateCookie::SetCnt(uint n)
{
	m_nCnt = n;
}

cTaskGateCookie::cTaskGateCookie(const Vector4& trans)
{
	Init();
	for (uint num = 0u; num < 2; num++)
	{
		CreateModel(num);
		cTaskModel::cPBModel* cPBModel = pPBModel(num);
		cPBModel->RegistModel((num == 0) ? "BIN_GATE_NUM_1_MDL" : "BIN_GATE_NUM_2_MDL");
		cPBModel->RegistAnm(0u, "BIN_GATE_NUM_TPA");
		cPBModel->IsDrawShadowDepth(false);
		cPBModel->SetLightSetIdx(3);
		cPBModel->SetFogIdx(0);
		::Constants::jetGeoVecCopy(cPBModel->vTrans(), trans);
		cPBModel->vTrans()[1] += 4.5f;
		cPBModel->UpdateMatrix();
		cPBModel->CopyMaterial();
		// TODO(unity): cPBModel.pScnMdlTransform().position = cPBModel.vTrans().GetUnityPos();
	}
}

void cTaskGateCookie::Init()
{
	m_nCnt = 0u;
	m_bBreak = false;
}

void cTaskGateCookie::Term()
{
	cTaskModel::Term();
}

bool cTaskGateCookie::Execute(cJetTask* pParam)
{
	return true;
}

// DOL 0x8017cb74: unless broken (+0x1d0), each digit sets its pattern frame and its push-back draws itself (vtable +0x54 = 0x8009abe0)
void cTaskGateCookie::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_bBreak)
	{
		uint num = m_nCnt % 10;
		cTaskModel::cPBModel* cPBModel = pPBModel();
		cPBModel->SetFrameAnmTexPat(0u, num * 10);
		cPBModel->UpdateMatrix();
		// TODO(unity): cPBModel.m_Materials[0].SetTexture("_MainTex", ResourceCache.Get<Texture>("g03_pacnroll/gimmick/textures/ww/m03g_gaten" + num + "_c"));
		num = m_nCnt / 10;
		cTaskModel::cPBModel* cPBModel2 = pPBModel(1u);
		cPBModel2->SetFrameAnmTexPat(0u, num * 10);
		cPBModel2->UpdateMatrix();
		// TODO(unity): cPBModel2.m_Materials[0].SetTexture("_MainTex", ResourceCache.Get<Texture>("g03_pacnroll/gimmick/textures/ww/m03g_gaten" + num + "_c"));
	}
}

void cTaskGateCookie::Invisible()
{
	for (uint num = 0u; num < 2; num++)
	{
		pPBModel(num)->Invisible();
	}
	m_bBreak = true;
}

}
