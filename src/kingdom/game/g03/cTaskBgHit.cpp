#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "platform/devtools.hpp"
#include "wii/Anim.hpp"
#include "kingdom/cKingdomModelBase.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cFaceList.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "ResMdl.hpp"
#include "ResNode.hpp"
#include <Constants.hpp>
#include <ResourceCache.hpp>

namespace kingdom::game::g03
{

cFaceList*& cTaskBgHit::pFaceList()
{
	return m_pHitFace;
}

bool& cTaskBgHit::IsMove()
{
	return m_bMove;
}

bool& cTaskBgHit::IsMoment()
{
	return m_bMoment;
}

float& cTaskBgHit::fMoment()
{
	return m_fMoment;
}

Matrix4x4& cTaskBgHit::pMtx()
{
	return m_Mtx;
}

Matrix4x4& cTaskBgHit::pMtxInv()
{
	return m_MtxInv;
}

Matrix4x4& cTaskBgHit::pMtxOld()
{
	return m_MtxOld;
}

Matrix4x4& cTaskBgHit::pMtxInvOld()
{
	return m_MtxInvOld;
}

Matrix4x4& cTaskBgHit::pMtxRotate()
{
	return m_MtxRotate;
}

Matrix4x4& cTaskBgHit::pMtxRotTrans()
{
	return m_MtxRotTrans;
}

Matrix4x4& cTaskBgHit::pMtxRotateInv()
{
	return m_MtxRotateInv;
}

Matrix4x4& cTaskBgHit::pMtxRotateOld()
{
	return m_MtxRltateOld;
}

Matrix4x4& cTaskBgHit::pMtxRotateInvOld()
{
	return m_MtxRltateInvOld;
}

bool& cTaskBgHit::IsCollision()
{
	return m_bCollision;
}

uint& cTaskBgHit::nState()
{
	return m_nState;
}

void cTaskBgHit::SetState(uint n)
{
	m_nState = n;
}

bool cTaskBgHit::IsState(uint n)
{
	if (!CustomKingdomUtil::IF(m_nState & n))
	{
		return false;
	}
	return true;
}

void cTaskBgHit::OnState(uint n)
{
	m_nState |= n;
}

void cTaskBgHit::OffState(uint n)
{
	m_nState &= ~n;
}

// DOL 0x80154bb0 [audited: per AUDIT_G03_G07 row]
cTaskBgHit::cTaskBgHit()
{
	SetName("cTaskBgHit");
	Init();
}

cTaskBgHit::cTaskBgHit(ResNode* const& node0)
	: cTaskBgBase(node0, node0)
{
	Init();
}

cTaskBgHit::cTaskBgHit(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgBase(node0, node1)
{
	Init();
}

// DOL 0x80154d18 [audited: per AUDIT_G03_G07 row]
void cTaskBgHit::Init()
{
	m_nState = 0u;
	m_pHitFace = nullptr;
	m_bMove = false;
	m_bMoment = false;
	m_fMoment = 0.0f;
	m_bCollision = true;
	::Constants::jetGeoMtxIdentity(m_Mtx);
	::Constants::jetGeoMtxIdentity(m_MtxInv);
	::Constants::jetGeoMtxIdentity(m_MtxOld);
	::Constants::jetGeoMtxIdentity(m_MtxInvOld);
	::Constants::jetGeoMtxIdentity(m_MtxRotate);
	::Constants::jetGeoMtxIdentity(m_MtxRotateInv);
	::Constants::jetGeoMtxIdentity(m_MtxRotTrans);
	::Constants::jetGeoMtxIdentity(m_MtxRltateOld);
	::Constants::jetGeoMtxIdentity(m_MtxRltateInvOld);
	this->Model->Invisible();
}

void cTaskBgHit::Term()
{
	cTaskBgBase::Term();
}

// DOL 0x80154db8 [audited: per AUDIT_G03_G07 row]
bool cTaskBgHit::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (pFaceList() != nullptr && IsCollision())
	{
		cAppG03PacnRollGame2->pTaskCollision()->PushBackCollisionObj(this);
	}
	return true;
}

// DOL 0x80154e40 [audited: per AUDIT_G03_G07 row]
void cTaskBgHit::Draw(int nStatus, cJetTask* pParam)
{
	if (this->Model->IsVisible())
	{
		PushBack();
	}
}

// DOL 0x80154e64 [audited: per AUDIT_G03_G07 row] (vtable +0x18): recomputed on every call, moving or not (the C#'s "calculated once unless IsMove" gate is gone)
void cTaskBgHit::UpdateMatrix()
{
	::Constants::jetGeoMtxCopy(pMtxOld(), pMtx());
	::Constants::jetGeoMtxCopy(pMtxInvOld(), pMtxInv());
	::Constants::jetGeoMtxCopy(pMtxRotateOld(), pMtxRotate());
	::Constants::jetGeoMtxCopy(pMtxRotateInvOld(), pMtxRotateInv());
	::Constants::jetGeoMtxIdentity(m_MtxRotate);
	::Constants::jetGeoMtxRotZYX(m_MtxRotate, m_MtxRotate, this->Model->vRotate());
	::Constants::jetGeoMtxIdentity(m_MtxRotTrans);
	::Constants::jetGeoMtxTrans(m_MtxRotTrans, m_MtxRotTrans, this->Model->vTrans());
	::Constants::jetGeoMtxMult(m_MtxRotTrans, m_MtxRotTrans, m_MtxRotate);
	::Constants::jetGeoMtxScale(m_Mtx, m_MtxRotTrans, this->Model->vScale());
	// the scene model's current CHR result for its "root" node (ChrAnmResult via the ScnMdl, vtable +0x38) moves the collision:
	// m_Mtx *= S(root.scale) * rt(root) (rotation + translation), m_MtxRotate *= rt(root) with the translation zeroed (moving platforms, rotating
	// bridges). Natively: the first CHR slot that poses the model (weight > 0) and animates "root"; RegistAnmNoIdx puts a gimmick's CHR clip after
	// its SRT / PAT clips, so it is often not slot 0 (the w4 rotating / swinging / rising bounce pads stayed static)
	ApplyAnmRoot();
	if (devtools::G03BgCollision()) devtools::G03BgCollisionRefresh(this); // devtools: --g03-bg-collision (not 1:1)
	::Constants::jetGeoMtxInverse(m_MtxRotateInv, m_MtxRotate);
	::Constants::jetGeoMtxInverse(m_MtxInv, m_Mtx);
}

// natively: the first CHR slot that poses the model (weight > 0) and animates "root"; RegistAnmNoIdx puts a gimmick's CHR clip after its SRT / PAT clips,
// so it is often not slot 0 (the DOL reads the ScnMdl's blended CHR result, there is no slot)
int cTaskBgHit::AnmRootSlot()
{
	for (uint s = 0; s < (uint)cKingdomModelBase::BLEND_MAX; s++)
	{
		std::shared_ptr<wii::ChrAnim> anm = this->Model->AnmChrOf(s);
		if (!anm || this->Model->GetWeightAnmChr(s) <= 0.0f || anm->numFrame <= 0) continue;
		wii::BoneKey root;
		if (anm->Sample("root", 0.0f, root)) return (int)s;
	}
	return -1;
}

void cTaskBgHit::ApplyAnmRoot()
{
	const int s = AnmRootSlot();
	if (s < 0) return;
	std::shared_ptr<wii::ChrAnim> anm = this->Model->AnmChrOf((uint)s);
	wii::BoneKey root;
	if (!anm->Sample("root", wii::ClipFrame(this->Model->GetFrameAnmChr((uint)s), anm->numFrame, anm->loops), root)) return;
	::Constants::jetGeoMtxScale(m_Mtx, m_Mtx, Vector4(root.scale[0], root.scale[1], root.scale[2], 1.0f));
	::Constants::jetGeoMtxMult(m_Mtx, m_Mtx, cKingdomModelBase::BoneRt(root, true));
	::Constants::jetGeoMtxMult(m_MtxRotate, m_MtxRotate, cKingdomModelBase::BoneRt(root, false));
}

// DOL 0x80155204 [audited: per AUDIT_G03_G07 row]
void cTaskBgHit::RegistHitData(string key)
{
	CustomKingdomUtil::Assert(!key.empty(), "nullpo");
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	cFaceList* cFaceList2 = cAppG03PacnRollGame2->pHitHash(key);
	if (cFaceList2 == nullptr)
	{
		GameObject* gameObject = kingdom::LoadModelObject(key);
		if (nullptr != gameObject)
		{
			cFaceList2 = new cFaceList(new ResMdl(gameObject));
			cAppG03PacnRollGame2->pHitHash().emplace(key, cFaceList2);
		}
	}
	pFaceList() = cFaceList2;
	if (devtools::G03BgCollision()) devtools::G03BgCollisionReplace(this); // devtools: --g03-bg-collision (not 1:1)
	if (pFaceList() != nullptr)
	{
		cAppG03PacnRollGame2->pTaskCollision()->PushBackCollisionObj(this);
	}
}

void cTaskBgHit::HitCallbackWall(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW)
{
}

void cTaskBgHit::HitCallbackFloor(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW)
{
}

}
