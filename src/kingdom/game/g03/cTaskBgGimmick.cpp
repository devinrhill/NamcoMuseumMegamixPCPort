#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "DummyNW4R.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "ResNode.hpp"
#include "ResUserData.hpp"
#include "ResUserDataItem.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

float cTaskBgGimmick::ModelHalfDiagonal()
{
	ResMdl resMdl(this->Model->pScnMdl());
	if (resMdl.Mdl == nullptr)
	{
		return 0.0f;
	}
	Vector4 v = Vector4();
	v[0] = (resMdl.Mdl->volumeMax[0] - resMdl.Mdl->volumeMin[0]) * 0.5f;
	v[1] = (resMdl.Mdl->volumeMax[1] - resMdl.Mdl->volumeMin[1]) * 0.5f;
	v[2] = (resMdl.Mdl->volumeMax[2] - resMdl.Mdl->volumeMin[2]) * 0.5f;
	v[3] = 1.0f;
	return ::Constants::jetGeoVecLength(v);
}

uint& cTaskBgGimmick::nActFlag()
{
	return m_nActFlag;
}

uint cTaskBgGimmick::GetRotFrameMax()
{
	return m_nRotFrameMax;
}

// DOL 0x8014cc94 [audited: per AUDIT_G03_G07 row] (suffix tokens at 0x8037a0d3: _hu shadow depth; _me _ma _ou _bs _ws _wm _wl = moment 0.06; _lr _ba _je _bu _ou _ws _wm _wl = state 0x10, else _pi = 0x200; _je _yu _mj = 0x100; hit model "<name>_h.brres")
cTaskBgGimmick::cTaskBgGimmick(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgHit(node0, node1)
{
	CustomKingdomUtil::Assert(node0 != nullptr, "nullpo");
	CustomKingdomUtil::Assert(node1 != nullptr, "nullpo");
	SetName("cTaskBgGimmick");
	Init();
	GetUserData(node1);
	::Constants::jetGeoVecCopy(m_vDefaultRotate, this->Model->vRotate());
	IsMove() = true;
	if (CustomKingdomUtil::strstr(node0->GetName(), "_hu"))
	{
		IsDrawShadowDepth(true);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_me") || CustomKingdomUtil::strstr(node0->GetName(), "_ma") || CustomKingdomUtil::strstr(node0->GetName(), "_ou") || CustomKingdomUtil::strstr(node0->GetName(), "_bs") || CustomKingdomUtil::strstr(node0->GetName(), "_ws") || CustomKingdomUtil::strstr(node0->GetName(), "_wm") || CustomKingdomUtil::strstr(node0->GetName(), "_wl"))
	{
		IsMoment() = true;
		fMoment() = 0.06f;
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_lr") || CustomKingdomUtil::strstr(node0->GetName(), "_ba") || CustomKingdomUtil::strstr(node0->GetName(), "_je") || CustomKingdomUtil::strstr(node0->GetName(), "_bu") || CustomKingdomUtil::strstr(node0->GetName(), "_ou") || CustomKingdomUtil::strstr(node0->GetName(), "_ws") || CustomKingdomUtil::strstr(node0->GetName(), "_wm") || CustomKingdomUtil::strstr(node0->GetName(), "_wl"))
	{
		OnState(16u);
	}
	else if (CustomKingdomUtil::strstr(node0->GetName(), "_pi"))
	{
		OnState(512u);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_je") || CustomKingdomUtil::strstr(node0->GetName(), "_yu") || CustomKingdomUtil::strstr(node0->GetName(), "_mj"))
	{
		OnState(256u);
	}
	string outStr;
	CustomKingdomUtil::snprintf(outStr, 0, "%s_h.brres", node0->GetName());
	RegistHitData(outStr);
	m_nGimmickType = 0u;
	if ((1u & ((!CustomKingdomUtil::strstr(node0->GetName(), "_ha")) ? 1u : 0u) & ((!IsState(512u)) ? 1u : 0u)) != 0)
	{
		std::vector<SkinnedMeshRenderer*> componentsInChildren; // TODO(unity): componentsInChildren = base.Model.pScnMdl().GetComponentsInChildren<SkinnedMeshRenderer>()
		for (SkinnedMeshRenderer* obj : componentsInChildren)
		{
			Bounds localBounds = Bounds(); // TODO(unity): localBounds = obj.localBounds
			localBounds.Expand(100.0f); // the converter dropped the argument (C#: Expand(100f))
			// TODO(unity): obj.localBounds = localBounds;
		}
	}
}

void cTaskBgGimmick::Init()
{
	m_nActFlag = 1u;
	m_nRotFrameMax = 0u;
	m_nRotFrame = 0u;
	::Constants::jetGeoVecUnit(m_vDefaultRotate);
	this->Model->SetLightSetIdx(0);
	this->Model->SetFogIdx(3);
	this->Model->Visible();
}

void cTaskBgGimmick::Term()
{
	cTaskBgHit::Term();
}

// DOL 0x8014d794 [audited: per AUDIT_G03_G07 row] (rotating gimmicks (act flag 2 / 3 with rot_l / rot_r user data): rotation.y = default + 360 / frames * frame, reversed for 3; then UpdateFrame, UpdateMatrix, BgHit::Execute)
bool cTaskBgGimmick::Execute(cJetTask* pParam)
{
	bool flag = (CustomKingdomUtil::IF(nActFlag() & 0xC0000000u) ? true : false);
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (!Constants::IS_PAC_PAUSE() && !flag && !cAppG03PacnRollGame2->IsNakajimaPause(this) && cAppG03PacnRollGame2->m_initCoObj == nullptr)
	{
		uint num = nActFlag() & 0xFF;
		if (num - 2 <= 1 && 0 < m_nRotFrameMax)
		{
			m_nRotFrame = ++m_nRotFrame % m_nRotFrameMax;
			float num2 = 360.0f / (float)m_nRotFrameMax;
			num2 *= (float)m_nRotFrame;
			if (nActFlag() == 3)
			{
				num2 = 0.0f - num2;
			}
			this->Model->vRotate()[1] = m_vDefaultRotate[1] + ::Constants::JET_DEG2RAD(num2);
		}
		this->Model->UpdateFrame();
	}
	UpdateMatrix();
	return cTaskBgHit::Execute(pParam);
}

bool cTaskBgGimmick::BaseExecute(cJetTask* pParam)
{
	return cTaskBgHit::Execute(pParam);
}

// DOL 0x8014d90c [audited: per AUDIT_G03_G07 row]
void cTaskBgGimmick::Draw(int nStatus, cJetTask* pParam)
{
	Matrix4x4 matrix = pMtxRotTrans();
	DummyNW4R::SetMtx(this->Model->pScnMdl(), matrix);
	DummyNW4R::SetScale(this->Model->pScnMdl(), this->Model->vScale()[0], this->Model->vScale()[1], this->Model->vScale()[2]);
	cTaskBgHit::Draw(nStatus, pParam);
}

// DOL 0x8014d994 [audited: per AUDIT_G03_G07 row] (user data "rot_l" -> act flag 2, "rot_r" -> 3, state 8, frame count = the item value)
void cTaskBgGimmick::GetUserData(ResNode* const& node)
{
	ResUserData* resUserData = node->GetResUserData();
	if (!resUserData->IsValid())
	{
		return;
	}
	for (int i = 0; i < resUserData->GetItemNumEntries(); i++)
	{
		ResUserDataItem* item = resUserData->GetItem(i);
		if (item->IsValid())
		{
			if (CustomKingdomUtil::strstr(item->GetName(), "rot_l"))
			{
				nActFlag() = 2u;
				SetState(8u);
				m_nRotFrameMax = (uint)item->GetS32();
			}
			else if (CustomKingdomUtil::strstr(item->GetName(), "rot_r"))
			{
				nActFlag() = 3u;
				SetState(8u);
				m_nRotFrameMax = (uint)item->GetS32();
			}
		}
	}
}

}
