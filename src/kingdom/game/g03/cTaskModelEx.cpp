#include "platform/devtools.hpp"
#include "kingdom/game/g03/cTaskModelEx.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModelBase.hpp"
#include "kingdom/cTaskPushBack.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCamera.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom::game::g03
{

cKingdomModel* cTaskModelEx::cPBModel::get_ShadowModel()
{
	return m_pShadowModel;
}

bool cTaskModelEx::cPBModel::RegistShadowModel(string key)
{
	CustomKingdomUtil::Assert(m_pShadowModel == nullptr, "error");
	if (m_pShadowModel == nullptr)
	{
		m_pShadowModel = new cKingdomModel(key);
		m_pShadowModel->Invisible();
		m_pShadowModel->SetShadowOnly(true);   // depth-shadow caster: cTaskItem::Visible() shows it again, but it must never reach the colour pass (it drew a ball over every cookie)
		std::vector<Renderer*> componentsInChildren; // TODO(unity): componentsInChildren = m_pShadowModel.pScnMdl().GetComponentsInChildren<Renderer>()
		for (int i = 0; i < (int)componentsInChildren.size(); i++)
		{
			// TODO(unity): componentsInChildren[i].shadowCastingMode = ShadowCastingMode.Off;
		}
		return true;
	}
	return false;
}

cTaskModelEx::cPBModel::cPBModel()
{
	PushBack = new cTaskPushBack();
	PushBack->set_IsClipping(false);
	m_pShadowModel = nullptr;
}

void cTaskModelEx::cPBModel::Term()
{
	m_pShadowModel->Term();
	cKingdomModel::Term();
}

void cTaskModelEx::cPBModel::IsDrawShadowDepth(bool b)
{
	cKingdomModelBase::IsDrawShadowDepth(b);
}

bool cTaskModelEx::cPBModel::IsDrawShadowDepth()
{
	return cKingdomModelBase::IsDrawShadowDepth();
}

void cTaskModelEx::cPBModel::SetShadowModelMtx(Matrix4x4 m34, Vector4 scale)
{
}

void cTaskModelEx::cPBModel::UpdateShadowModelMtx()
{
	if (m_pShadowModel != nullptr)
	{
		// the blob shadow model follows this model's matrix (shadows are not drawn natively yet: the model is invisible)
		Matrix4x4 outMatrix;
		DummyNW4R::GetMtx(pScnMdl(), outMatrix);
		DummyNW4R::SetMtx(m_pShadowModel->pScnMdl(), outMatrix, false);
	}
}

void cTaskModelEx::cPBModel::SetMaterialColor(Color color)
{
}

cTaskModelEx::cTaskModelEx()
{
	Init();
}

cTaskModelEx::cTaskModelEx(string key, uint option)
{
	Init();
	CreateModel()->RegistModel(key, option);
}

cTaskModelEx::cTaskModelEx(Object* data, uint option)
{
	Init();
	CreateModel()->RegistModel(data, option);
}

void cTaskModelEx::Init()
{
	m_vecPBModel.clear();
	m_vecPBModel.reserve(32);
}

void cTaskModelEx::Term()
{
	EnableGarbageModel();
	ClearGarbageModel();
	for (int i = 0; i < (int)m_vecPBModel.size(); i++)
	{
		if (m_vecPBModel[i] != nullptr)
		{
			m_vecPBModel[i]->Term();
		}
	}
	m_vecPBModel.clear();
}

cKingdomModel* cTaskModelEx::CreateModel(int idx, bool isUpdateFrame)
{
	cTaskModelEx::cPBModel* cPBModel2 = new cTaskModelEx::cPBModel();
	cPBModel2->IsUpdateFrame = isUpdateFrame;
	CustomKingdomUtil::Assert(cPBModel2 != nullptr, "nullpo");
	if ((int)m_vecPBModel.size() <= idx)
	{
		m_vecPBModel.reserve(idx + 1);
	}
	if (idx >= (int)m_vecPBModel.size())
	{
		m_vecPBModel.push_back(cPBModel2);
	}
	else
	{
		m_vecPBModel[idx] = cPBModel2;
	}
	return m_vecPBModel[idx];
}

cKingdomModel* cTaskModelEx::CreateModelEx(int& idx, bool isUpdateFrame)
{
	CustomKingdomUtil::Assert(idx != 0, "nullpo");
	cTaskModelEx::cPBModel* cPBModel2 = new cTaskModelEx::cPBModel();
	cPBModel2->IsUpdateFrame = isUpdateFrame;
	CustomKingdomUtil::Assert(cPBModel2 != nullptr, "nullpo");
	bool flag = false;
	for (int i = 0; i < (int)m_vecPBModel.size(); i++)
	{
		if (m_vecPBModel[i] == nullptr)
		{
			idx = i;
			flag = true;
			m_vecPBModel[idx] = cPBModel2;
			break;
		}
	}
	if (!flag)
	{
		idx = (int)m_vecPBModel.size();
		m_vecPBModel.reserve(idx + 1);
		m_vecPBModel.push_back(cPBModel2);
	}
	return m_vecPBModel[idx];
}

void cTaskModelEx::DeleteModel(int idx)
{
	CustomKingdomUtil::Assert((int)m_vecPBModel.size() > idx, "error");
	if (m_vecPBModel[idx] != nullptr)
	{
		m_vecPBModel[idx] = nullptr;
	}
}

cKingdomModel* cTaskModelEx::pModel(int idx)
{
	if ((int)m_vecPBModel.size() <= idx)
	{
		return nullptr;
	}
	return m_vecPBModel[idx];
}

cTaskModelEx::cPBModel* cTaskModelEx::pPBModel(int idx)
{
	if ((int)m_vecPBModel.size() <= idx)
	{
		return nullptr;
	}
	return m_vecPBModel[idx];
}

void cTaskModelEx::UpdateFrame()
{
	for (cTaskModelEx::cPBModel* item : m_vecPBModel)
	{
		if (item != nullptr && item->IsUpdateFrame)
		{
			item->UpdateFrame();
		}
	}
}

bool cTaskModelEx::Execute(cJetTask* pParam)
{
	return cTaskModelBase::Execute(m_vecPBModel);
}

// DOL 0x80187270 [audited: per AUDIT_G03_G07 row]: every visible model inside the camera's view volume (sys app +0x294, 0x80159428 with radius 0.38) computes its matrix
// (vtable +0x18 = 0x8001df9c), its blob shadow follows it, and its push-back (+0xd8) goes into this frame's scene list (0x8009c3f4)
void cTaskModelEx::Draw(int nStatus, cJetTask* pParam)
{
	cAppG03PacnRollGame* app = dynamic_cast<cAppG03PacnRollGame*>(KingdomGlobal::KingSys->pAppMain());
	cTaskCamera* camera = app != nullptr ? app->pTaskCamera() : nullptr;
	for (cTaskModelEx::cPBModel* item : m_vecPBModel)
	{
		if (item != nullptr)
		{
			item->EnableGarbageModel();   // 0x8001e594
			// (no camera = an app the DOL never runs this in: drawn unclipped)
			if (item->IsVisible() && (camera == nullptr || devtools::G03EditNoCull() || camera->HitViewVolume(item->vTrans(), 0.38f)))   // devtools: the editor draws everything (not 1:1)
			{
				item->UpdateMatrix();
				item->UpdateShadowModelMtx();   // the shadow's ScnObj options 2 / 3 / 4 off (vtable +0x20) are not modelled
			}
		}
	}
	EnableGarbageModel();
}

void cTaskModelEx::SetMaterialColor(int n, Color color)
{
}

}
