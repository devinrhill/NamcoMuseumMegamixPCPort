#include "kingdom/game/g03/cTaskCollision.hpp"

#include "AABB.hpp"
#include "cJetTask.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cFaceList.hpp"
#include "kingdom/game/g03/cHitChar.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "LINE3.hpp"
#include "PLANE.hpp"
#include "SEGMENT3.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

Vector4 cTaskCollision::vTmp[3] = {};
Vector4 cTaskCollision::vrt[3] = {};

std::vector<cTaskChar*>& cTaskCollision::pVecChar()
{
	return m_vecChar;
}

std::vector<cTaskBgHit*>& cTaskCollision::pVecObj()
{
	return m_vecObj;
}

cTaskCollision::cTaskCollision()
{
	SetName("cTaskCollision");
	Init();
}

cTaskCollision::~cTaskCollision()
{
	Term();
}

void cTaskCollision::Init()
{
	m_vecChar.reserve(8);
	m_vecObj.reserve(32);
}

void cTaskCollision::Term()
{
}

bool cTaskCollision::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (Constants::IS_PAC_PAUSE() || cAppG03PacnRollGame2->IsNakajimaPause(this))
	{
		return true;
	}
	CollisionPacman2BG();
	CollisionChars2Chars();
	return true;
}

void cTaskCollision::Draw(int nStatus, cJetTask* pParam)
{
}

void cTaskCollision::CollisionChars2Chars()
{
	pl_atk.clear();
	pl_def.clear();
	ene_atk.clear();
	ene_def.clear();
	obj_push.clear();
	if (!Constants::IS_PAC_PAUSE() && !Constants::PACN_APP()->IsNakajimaPause(this))
	{
		for (cTaskChar* item : m_vecChar)
		{
			item->HitUpdate();
			item->HitListup(pl_atk, 0, 2u);
			item->HitListup(pl_def, 0, 4u);
			item->HitListup(ene_atk, 1, 2u);
			item->HitListup(ene_def, 1, 4u);
			item->HitListup(obj_push, -1, 8u);
		}
	}
	for (cHitChar* item2 : pl_atk)
	{
		for (cHitChar* item3 : ene_def)
		{
			if (!(::Constants::JET_ABS(item2->vPos()[1] - item3->vPos()[1]) > item2->fOfsRange() + item3->fOfsRange()))
			{
				float num = ::Constants::bizGeoVecDistance2XZ(item2->vPos(), item3->vPos());
				if (num <= item2->fRange2() + item3->fRange2())
				{
					item3->pParent()->Attacked(item3, item2);
				}
			}
		}
	}
	for (cHitChar* item4 : ene_atk)
	{
		for (cHitChar* item5 : pl_def)
		{
			if (!(::Constants::JET_ABS(item4->vPos()[1] - item5->vPos()[1]) > item4->fOfsRange() + item5->fOfsRange()))
			{
				float num = ::Constants::bizGeoVecDistance2XZ(item4->vPos(), item5->vPos());
				if (num <= item4->fRange2() + item5->fRange2())
				{
					item5->pParent()->Attacked(item5, item4);
				}
			}
		}
	}
	int count = (int)obj_push.size();
	int num2 = 0;
	while (count != 0 && num2 < count - 1)
	{
		cHitChar* cHitChar2 = obj_push[num2];
		for (int i = num2 + 1; i < count; i++)
		{
			cHitChar* cHitChar3 = obj_push[i];
			if (::Constants::JET_ABS(cHitChar2->vPos()[1] - cHitChar3->vPos()[1]) > cHitChar2->fOfsRange() + cHitChar3->fOfsRange())
			{
				continue;
			}
			float num = ::Constants::bizGeoVecDistance2XZ(cHitChar2->vPos(), cHitChar3->vPos());
			if (!(num <= cHitChar2->fRange2() + cHitChar3->fRange2()))
			{
				continue;
			}
			float num3 = Mathf::Sqrt(num);
			Vector4 v;
			if (cHitChar2->pParent()->nPushOutLevel() == cHitChar3->pParent()->nPushOutLevel())
			{
				num3 = cHitChar2->fOfsRange() + cHitChar3->fOfsRange() - num3;
				num3 /= 2.0f;
				::Constants::jetGeoVecSub(v, cHitChar2->pParent()->vTrans(), cHitChar3->pParent()->vTrans());
				v[1] = 0.0f;
				::Constants::jetGeoVecNormalize(v, v);
				::Constants::jetGeoVecScale(v, v, num3);
				cHitChar2->pParent()->HitPushOut(v, cHitChar3->pParent());
				::Constants::jetGeoVecScale(v, v, -1.0f);
				cHitChar3->pParent()->HitPushOut(v, cHitChar2->pParent());
				continue;
			}
			if (cHitChar2->pParent()->nPushOutLevel() >= cHitChar3->pParent()->nPushOutLevel())
			{
				cHitChar* obj = cHitChar3;
				cHitChar3 = cHitChar2;
				cHitChar2 = obj;
			}
			num3 = cHitChar2->fOfsRange() + cHitChar3->fOfsRange() - num3;
			::Constants::jetGeoVecSub(v, cHitChar2->pParent()->vTrans(), cHitChar3->pParent()->vTrans());
			v[1] = 0.0f;
			::Constants::jetGeoVecNormalize(v, v);
			::Constants::jetGeoVecScale(v, v, num3);
			cHitChar2->pParent()->HitPushOut(v, cHitChar3->pParent());
		}
		num2++;
	}
}

void cTaskCollision::CollisionPacman2BG()
{
	Vector4 v = Vector4::zero;
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	if (cTaskCharPacman2 == nullptr || !cTaskCharPacman2->IsState(64u))
	{
		return;
	}
	cTaskCharPacman2->CollisionFoceMove();
	HitProcessWall();
	cTaskCharPacman2->PostCollisionWall();
	::Constants::jetGeoVecAdd(cTaskCharPacman2->vTrans(), cTaskCharPacman2->vTrans(), cTaskCharPacman2->vForceMove());
	HitProcessFloor();
	cTaskCharPacman2->PostCollisionFloor();
	if (cTaskCharPacman2->IsState(32768u))
	{
		Vector4 v2 = Vector4::zero;
		::Constants::jetGeoVecUnit(v2);
		v2[1] = cTaskCharPacman2->vMove()[1];
		::Constants::jetGeoVecAdd(cTaskCharPacman2->vTrans(), cTaskCharPacman2->vTrans(), v2);
	}
	else
	{
		::Constants::jetGeoVecAdd(cTaskCharPacman2->vTrans(), cTaskCharPacman2->vTrans(), cTaskCharPacman2->vMove());
	}
	cTaskCharPacman2->ClearHitParam();
	cTaskChar* taskChar = cTaskCharPacman2;
	SPHERE charSphere = cTaskCollision::GetCharSphere(taskChar);
	Vector3 sphereMin = charSphere.Bound.min();
	Vector3 sphereMax = charSphere.Bound.max();
	Vector4 v3;
	Vector4 vMoveL;
	Vector4 vMoveLN;
	Vector4 v4;
	Vector4 vTransL;
	for (cTaskBgHit* item : m_vecObj)
	{
		if (!cTaskCollision::CollisionSphereAABB(sphereMin, sphereMax, item))
		{
			continue;
		}
		::Constants::jetGeoVecTrans(v3, item->pMtxInv(), cTaskCharPacman2->vTrans());
		::Constants::jetGeoVecTrans(vMoveL, item->pMtxRotateInv(), cTaskCharPacman2->vMove());
		if (cTaskCharPacman2->pHitObj(1) != nullptr || cTaskCharPacman2->IsState(1024u))
		{
			::Constants::jetGeoVecSet(v, 0.0f, cTaskCharPacman2->fRadius(), 0.0f, 1.0f);
			::Constants::jetGeoVecTrans(v, item->pMtxRotateInv(), v);
		}
		else
		{
			::Constants::jetGeoVecUnit(v);
		}
		uint num = item->pFaceList()->nFaceNum(0);
		for (int i = 0; (long)i < (long)num; i++)
		{
			Constants::sFace* sFace = item->pFaceList()->pFace(0, i);
			if (cTaskCollision::CollisionSphereFaceAABB(sphereMin, sphereMax, item, sFace))
			{
				::Constants::jetGeoVecScale(vMoveLN, sFace->vNorm, -1.0f);
				::Constants::jetGeoVecScale(vMoveL, vMoveLN, cTaskCharPacman2->fRadius() * 2.0f);
				::Constants::jetGeoVecScale(v4, sFace->vNorm, cTaskCharPacman2->fRadius());
				::Constants::jetGeoVecAdd(vTransL, v3, v4);
				Constants::sHitParam* hitParam;
				if (cTaskCollision::CollisionFace(hitParam, cTaskCharPacman2, item, sFace, vTransL, vMoveL, vMoveLN, v))
				{
					hitParam->fDist = ::Constants::jetGeoVecDistance(v3, hitParam->vPos);
					cTaskCharPacman2->RegistHitParam(0u, hitParam);
				}
			}
		}
	}
	cTaskCharPacman2->PostCollisionWallFinish();
	for (cTaskBgHit* item2 : m_vecObj)
	{
		if (!cTaskCollision::CollisionSphereAABB(sphereMin, sphereMax, item2))
		{
			continue;
		}
		::Constants::jetGeoVecTrans(v3, item2->pMtxInv(), cTaskCharPacman2->vTrans());
		uint num2 = item2->pFaceList()->nFaceNum(1);
		for (int j = 0; (long)j < (long)num2; j++)
		{
			Constants::sFace* sFace = item2->pFaceList()->pFace(1, j);
			if ((sFace->nKind != Constants::eFACE_KIND::eFACE_KIND_WATE || cTaskCharPacman2->nKind() != cTaskChar::eKIND::eKIND_PACMAN || cTaskCharPacman2->nType() != 2) && cTaskCollision::CollisionSphereFaceAABB(sphereMin, sphereMax, item2, sFace))
			{
				::Constants::jetGeoVecScale(vMoveLN, sFace->vNorm, -1.0f);
				::Constants::jetGeoVecScale(vMoveL, vMoveLN, cTaskCharPacman2->fRadius() * 2.0f);
				::Constants::jetGeoVecScale(v4, sFace->vNorm, cTaskCharPacman2->fRadius());
				::Constants::jetGeoVecAdd(vTransL, v3, v4);
				Constants::sHitParam* hitParam2;
				if (cTaskCollision::CollisionFace(hitParam2, cTaskCharPacman2, item2, sFace, vTransL, vMoveL, vMoveLN))
				{
					hitParam2->fDist = ::Constants::jetGeoVecDistance(v3, hitParam2->vPos);
					cTaskCharPacman2->RegistHitParam(1u, hitParam2);
				}
			}
		}
	}
	cTaskCharPacman2->PostCollisionFloorFinish();
}

bool cTaskCollision::HitProcessWall()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	Vector4 v = Vector4::zero;
	Vector4 v2 = Vector4::zero;
	cTaskChar* taskChar = cTaskCharPacman2;
	SPHERE charSphere = cTaskCollision::GetCharSphere(taskChar);
	Vector3 sphereMin = charSphere.Bound.min();
	Vector3 sphereMax = charSphere.Bound.max();
	bool result = false;
	for (cTaskBgHit* item : m_vecObj)
	{
		if (!cTaskCollision::CollisionSphereAABB(sphereMin, sphereMax, item))
		{
			continue;
		}
		::Constants::jetGeoVecUnit(v);
		::Constants::jetGeoVecUnit(v2);
		Vector4 v3;
		::Constants::jetGeoVecTrans(v3, item->pMtxInv(), cTaskCharPacman2->vTrans());
		Vector4 v4;
		if (cTaskCharPacman2->pHitObj(1) == nullptr || cTaskCharPacman2->pHitObj(1) != item)
		{
			::Constants::jetGeoVecAdd(v4, cTaskCharPacman2->vMove(), cTaskCharPacman2->vForceMove());
			::Constants::jetGeoVecTrans(v4, item->pMtxRotateInv(), v4);
		}
		else
		{
			::Constants::jetGeoVecTrans(v4, item->pMtxRotateInv(), cTaskCharPacman2->vMove());
		}
		Vector4 v5;
		::Constants::jetGeoVecTrans(v5, item->pMtxRotateInv(), cTaskCharPacman2->vForceMove());
		if (item == cTaskCharPacman2->pHitObj(0) || item == cTaskCharPacman2->pHitObj(1))
		{
			::Constants::jetGeoVecAdd(v3, v3, v5);
		}
		else
		{
			::Constants::jetGeoVecAdd(v4, v4, v5);
		}
		if (cTaskCharPacman2->pHitObj(1) != nullptr || cTaskCharPacman2->IsState(1024u))
		{
			::Constants::jetGeoVecSet(v, 0.0f, cTaskCharPacman2->fRadius(), 0.0f, 1.0f);
			::Constants::jetGeoVecTrans(v, item->pMtxRotateInv(), v);
		}
		if (item->IsMove() && cTaskCharPacman2->pHitObj(1) != item)
		{
			::Constants::jetGeoVecTrans(v5, item->pMtxInvOld(), cTaskCharPacman2->vTrans());
			::Constants::jetGeoVecSub(v2, v3, v5);
			v2[1] = 0.0f;
			::Constants::jetGeoVecAdd(v4, v4, v2);
		}
		if (!Constants::IsVecNormalize(v4))
		{
			continue;
		}
		Vector4 v6;
		::Constants::jetGeoVecNormalize(v6, v4);
		uint num = item->pFaceList()->nFaceNum(0);
		for (int i = 0; (long)i < (long)num; i++)
		{
			Constants::sFace* sFace = item->pFaceList()->pFace(0, i);
			if (cTaskCollision::CollisionSphereFaceAABB(sphereMin, sphereMax, item, sFace))
			{
				::Constants::jetGeoVecScale(v5, sFace->vNorm, 0.0f - cTaskCharPacman2->fRadius());
				Vector4 v7;
				::Constants::jetGeoVecAdd(v7, v3, v5);
				::Constants::jetGeoVecScale(v5, v6, 0.0f - cTaskCharPacman2->fRadius());
				::Constants::jetGeoVecAdd(v7, v7, v5);
				Vector4 v8;
				::Constants::jetGeoVecSub(v8, v4, v5);
				Constants::sHitParam* hitParam;
				if (cTaskCollision::CollisionFace(hitParam, cTaskCharPacman2, item, sFace, v7, v8, v6, v))
				{
					::Constants::jetGeoVecCopy(hitParam->vObjMove, v2);
					hitParam->fDist = ::Constants::jetGeoVecDistance(v3, hitParam->vPos);
					cTaskCharPacman2->RegistHitParam(0u, hitParam);
					result = true;
				}
			}
		}
	}
	return result;
}

bool cTaskCollision::HitProcessFloor()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	cTaskChar* taskChar = cTaskCharPacman2;
	SPHERE charSphere = cTaskCollision::GetCharSphere(taskChar);
	Vector3 sphereMin = charSphere.Bound.min();
	Vector3 sphereMax = charSphere.Bound.max();
	for (cTaskBgHit* item : m_vecObj)
	{
		if (!cTaskCollision::CollisionSphereAABB(sphereMin, sphereMax, item))
		{
			continue;
		}
		Vector4 v;
		::Constants::jetGeoVecTrans(v, item->pMtxInv(), cTaskCharPacman2->vTrans());
		Vector4 v2;
		::Constants::jetGeoVecTrans(v2, item->pMtxRotateInv(), cTaskCharPacman2->vMove());
		Vector4 v3;
		if (item->IsState(514u) && item->IsMove() && cTaskCharPacman2->pHitObj(1) != item)
		{
			::Constants::jetGeoVecTrans(v3, item->pMtxInvOld(), cTaskCharPacman2->vTrans());
			::Constants::jetGeoVecSub(v3, v, v3);
			::Constants::jetGeoVecAdd(v2, v2, v3);
		}
		if (!Constants::IsVecNormalize(v2))
		{
			continue;
		}
		Vector4 v4;
		::Constants::jetGeoVecNormalize(v4, v2);
		uint num = item->pFaceList()->nFaceNum(1);
		for (int i = 0; (long)i < (long)num; i++)
		{
			Constants::sFace* sFace = item->pFaceList()->pFace(1, i);
			if ((sFace->nKind != Constants::eFACE_KIND::eFACE_KIND_WATE || cTaskCharPacman2->nKind() != cTaskChar::eKIND::eKIND_PACMAN || cTaskCharPacman2->nType() != 2) && cTaskCollision::CollisionSphereFaceAABB(sphereMin, sphereMax, item, sFace))
			{
				::Constants::jetGeoVecScale(v3, sFace->vNorm, 0.0f - cTaskCharPacman2->fRadius());
				Vector4 v5;
				::Constants::jetGeoVecAdd(v5, v, v3);
				::Constants::jetGeoVecScale(v3, v4, 0.0f - cTaskCharPacman2->fRadius());
				::Constants::jetGeoVecAdd(v5, v5, v3);
				Vector4 v6;
				::Constants::jetGeoVecSub(v6, v2, v3);
				Constants::sHitParam* hitParam;
				if (cTaskCollision::CollisionFace(hitParam, cTaskCharPacman2, item, sFace, v5, v6, v4))
				{
					hitParam->fDist = ::Constants::jetGeoVecDistance(v, hitParam->vPos);
					cTaskCharPacman2->RegistHitParam(1u, hitParam);
				}
			}
		}
	}
	return true;
}

bool cTaskCollision::CollisionFace(Constants::sHitParam*& hitParam, cTaskChar* chr, cTaskBgHit* obj, Constants::sFace* face, const Vector4& vTransL, const Vector4& vMoveL, const Vector4& vMoveLN, Nullable<Vector4> vOffsetL)
{
	hitParam = nullptr;
	Vector4 v;
	::Constants::jetGeoVecCopy(v, vTransL);
	if (vOffsetL.has_value())
	{
		::Constants::jetGeoVecAdd(v, v, vOffsetL.value());
	}
	if (0.0f < ::Constants::jetGeoVecInnerProduct(vMoveLN, face->vNorm))
	{
		return false;
	}
	Vector4 v2;
	::Constants::jetGeoVecAdd(v2, v, vMoveL);
	Vector4 vOutCross;
	if (!cTaskCollision::CollisionSegmentPlane(v, v2, face->vVrt[0], face->vNorm, vOutCross))
	{
		return false;
	}
	uint num = 0u;
	int nEdgeVrtIdx = -1;
	Vector4 v3 = Vector4::zero;
	Vector4 v4 = Vector4::zero;
	::Constants::jetGeoVecUnit(v3);
	::Constants::jetGeoVecUnit(v4);
	if (cTaskCollision::CollisionInnerTriangle(vOutCross, face->vVrt[0], face->vVrt[1], face->vVrt[2], face->vNorm))
	{
		num = 1u;
	}
	else
	{
		if (0 > (nEdgeVrtIdx = cTaskCollision::CollisionTriangleEdge(vOutCross, face->vVrt[0], face->vVrt[1], face->vVrt[2], face->vNorm, chr->fRadius() * 0.9f, v3, v4)))
		{
			return false;
		}
		num = 2u;
	}
	if (vOffsetL.has_value())
	{
		::Constants::jetGeoVecSub(vOutCross, vOutCross, vOffsetL.value());
	}
	hitParam = new Constants::sHitParam();
	::Constants::jetGeoVecCopy(hitParam->vPos, vOutCross);
	::Constants::jetGeoVecCopy(hitParam->vNorm, face->vNorm);
	::Constants::jetGeoVecCopy(hitParam->vRay, vMoveLN);
	::Constants::jetGeoVecCopy(hitParam->vVrt[0], face->vVrt[0]);
	::Constants::jetGeoVecCopy(hitParam->vVrt[1], face->vVrt[1]);
	::Constants::jetGeoVecCopy(hitParam->vVrt[2], face->vVrt[2]);
	hitParam->nFlag = num;
	hitParam->pObj = obj;
	hitParam->pFace = face;
	hitParam->nEdgeVrtIdx = nEdgeVrtIdx;
	::Constants::jetGeoVecCopy(hitParam->vEdgePos, v3);
	::Constants::jetGeoVecCopy(hitParam->vEdgeVec, v4);
	return true;
}

void cTaskCollision::ClearEntry()
{
	m_vecChar.clear();
	m_vecObj.clear();
}

void cTaskCollision::PushBackChar(cTaskChar* chr)
{
	m_vecChar.push_back(chr);
}

void cTaskCollision::PushBackCollisionObj(cTaskBgHit* obj)
{
	m_vecObj.push_back(obj);
}

Constants::sFace* cTaskCollision::CheckDashFloor(cTaskChar* chr, cTaskBgHit*& outDashObj)
{
	outDashObj = nullptr;
	Constants::sFace* sFace = nullptr;
	float num = 0.0f;
	SPHERE charSphere = cTaskCollision::GetCharSphere(chr);
	Vector3 sphereMin = charSphere.Bound.min();
	Vector3 sphereMax = charSphere.Bound.max();
	for (cTaskBgHit* item : m_vecObj)
	{
		if (!cTaskCollision::CollisionSphereAABB(sphereMin, sphereMax, item))
		{
			continue;
		}
		Vector4 v;
		::Constants::jetGeoVecTrans(v, item->pMtxInv(), chr->vTrans());
		uint num2 = item->pFaceList()->nFaceNum(2);
		for (int i = 0; (long)i < (long)num2; i++)
		{
			Constants::sFace* sFace2 = item->pFaceList()->pFace(2, i);
			Vector4 v2;
			::Constants::jetGeoVecScale(v2, sFace2->vNorm, -1.0f);
			Vector4 v3;
			::Constants::jetGeoVecScale(v3, sFace2->vNorm, (0.0f - chr->fRadius()) * 2.0f);
			Constants::sHitParam* hitParam;
			if (cTaskCollision::CollisionFace(hitParam, chr, item, sFace2, v, v3, v2))
			{
				Vector4 v4;
				::Constants::jetGeoVecSub(v4, hitParam->vPos, v);
				float num3 = ::Constants::jetGeoVecInnerProduct(v4, v4);
				if (sFace == nullptr || num3 < num)
				{
					sFace = hitParam->pFace;
					num = num3;
					outDashObj = item;
				}
			}
		}
	}
	return sFace;
}

bool cTaskCollision::CollisionSphereAABB(cTaskChar* taskChar, cTaskBgHit* obj)
{
	if (obj->pFaceList()->IsAABB() && obj->pFaceList()->ReadOnlyAABB().IsSet)
	{
		AABB aabb = obj->pFaceList()->CalcedAABB();
		if (obj->IsMove() || obj->pFaceList()->IsReuse() || !aabb.IsSet)
		{
			aabb = obj->pFaceList()->pAABB();
			aabb.Set(aabb, obj->pMtx());
			obj->pFaceList()->CalcedAABB() = aabb;
		}
		float num = ::Constants::jetGeoVecLength(taskChar->vMove());
		Vector4 v;
		::Constants::jetGeoVecScale(v, taskChar->vMove(), 0.5f);
		::Constants::jetGeoVecAdd(v, taskChar->vTrans(), v);
		return DummyNW4R::IntersectionSphereAABB(SPHERE((Vector3)v, num + taskChar->fRadius() * 2.0f), aabb);
	}
	return false;
}

bool cTaskCollision::CollisionSphereAABB(const Vector3& sphereMin, const Vector3& sphereMax, cTaskBgHit* obj)
{
	if (obj->pFaceList()->IsAABB() && obj->pFaceList()->ReadOnlyAABB().IsSet)
	{
		AABB target = obj->pFaceList()->CalcedAABB();
		if (obj->IsMove() || obj->pFaceList()->IsReuse() || !target.IsSet)
		{
			target = obj->pFaceList()->pAABB();
			target.Set(target, obj->pMtx());
			obj->pFaceList()->CalcedAABB() = target;
		}
		return DummyNW4R::Intersection(sphereMin, sphereMax, target.This.max(), target.This);
	}
	return false;
}

bool cTaskCollision::CollisionSphereFaceAABB(const Vector3& sphereMin, const Vector3& sphereMax, cTaskBgHit* obj, Constants::sFace* face)
{
	if (face->aabb.IsSet)
	{
		AABB target = face->calcedAABB;
		if (obj->IsMove() || obj->pFaceList()->IsReuse() || !target.IsSet)
		{
			target = face->aabb;
			target.Set(target, obj->pMtx());
			face->calcedAABB = target;
		}
		return DummyNW4R::Intersection(sphereMin, sphereMax, target.This.max(), target.This);
	}
	return false;
}

SPHERE cTaskCollision::GetCharSphere(cTaskChar* const& taskChar)
{
	float num = ::Constants::jetGeoVecLength(taskChar->vMove());
	Vector4 v;
	::Constants::jetGeoVecScale(v, taskChar->vMove(), 0.5f);
	::Constants::jetGeoVecAdd(v, taskChar->vTrans(), v);
	return SPHERE((Vector3)v, num + taskChar->fRadius() * 2.0f);
}

bool cTaskCollision::CollisionRayAABB(const Vector4& pos, const Vector4& move, cTaskBgHit* obj, float maxDist)
{
	if (obj->pFaceList()->IsAABB() && obj->pFaceList()->ReadOnlyAABB().IsSet)
	{
		AABB aabb = obj->pFaceList()->CalcedAABB();
		if (obj->IsMove() || obj->pFaceList()->IsReuse() || !aabb.IsSet)
		{
			aabb = obj->pFaceList()->pAABB();
			aabb.Set(aabb, obj->pMtx());
			obj->pFaceList()->CalcedAABB() = aabb;
		}
		float outLength;
		bool result = DummyNW4R::IntersectionRay3AABB(RAY3(pos, move), aabb, outLength);
		if (maxDist >= 0.0f && outLength > maxDist)
		{
			result = false;
		}
		return result;
	}
	return false;
}

bool cTaskCollision::CollisionRayFaceAABB(const RAY3& ray, cTaskBgHit* obj, Constants::sFace* face, float maxDist)
{
	if (face->aabb.IsSet)
	{
		AABB aabb = face->calcedAABB;
		if (obj->IsMove() || obj->pFaceList()->IsReuse() || !aabb.IsSet)
		{
			aabb = face->aabb;
			aabb.Set(aabb, obj->pMtx());
			face->calcedAABB = aabb;
		}
		float outLength;
		bool result = DummyNW4R::IntersectionRay3AABB(ray, aabb, outLength);
		if (maxDist >= 0.0f && outLength > maxDist)
		{
			result = false;
		}
		return result;
	}
	return false;
}

RAY3 cTaskCollision::GetRay(const Vector4& pos, const Vector4& move)
{
	return RAY3(pos, move);
}

bool cTaskCollision::CollisionLinePlane(const Vector4& vPos, const Vector4& vLine, const Vector4& vrt, const Vector4& vNorm, Vector4& vOutCross)
{
	vOutCross = Vector4();
	Vector3 inNormal = vNorm;
	float distance = 0.0f - ::Constants::jetGeoVecInnerProduct(vNorm, vrt);
	PLANE plane = PLANE(inNormal, distance);
	LINE3 line = LINE3((Vector3)vPos, vLine);
	Vector3 outCross = Vector3(0.0f, 0.0f, 0.0f);
	if (!DummyNW4R::IntersectionLine3Plane(line, plane, distance, outCross))
	{
		return false;
	}
	::Constants::jetGeoVecSet(vOutCross, outCross.x, outCross.y, outCross.z, 1.0f);
	return true;
}

bool cTaskCollision::CollisionRayPlane(const Vector4& vPos, const Vector4& vRay, const Vector4& vrt, const Vector4& vNorm, Vector4& vOutCross)
{
	vOutCross = Vector4();
	Vector3 inNormal = vNorm;
	float distance = 0.0f - ::Constants::jetGeoVecInnerProduct(vNorm, vrt);
	PLANE plane = PLANE(inNormal, distance);
	RAY3 ray = RAY3(vPos, vRay);
	Vector3 outCross = Vector3(0.0f, 0.0f, 0.0f);
	if (!DummyNW4R::IntersectionRay3Plane(ray, plane, distance, outCross))
	{
		return false;
	}
	::Constants::jetGeoVecSet(vOutCross, outCross.x, outCross.y, outCross.z, 1.0f);
	return true;
}

bool cTaskCollision::CollisionSegmentPlane(const Vector4& vPos0, const Vector4& vPos1, const Vector4& vrt, const Vector4& vNorm, Vector4& vOutCross)
{
	vOutCross = Vector4();
	Vector3 inNormal = vNorm;
	float distance = 0.0f - ::Constants::jetGeoVecInnerProduct(vrt, vNorm);
	PLANE plane = PLANE(inNormal, distance);
	SEGMENT3 Segment = SEGMENT3(vPos0, (Vector3)vPos1);
	Vector3 outCross = Vector3(0.0f, 0.0f, 0.0f);
	if (!DummyNW4R::IntersectionSegment3Plane(Segment, plane, distance, outCross))
	{
		return false;
	}
	::Constants::jetGeoVecSet(vOutCross, outCross.x, outCross.y, outCross.z, 1.0f);
	return true;
}

bool cTaskCollision::CollisionInnerTriangle(const Vector4& vPos, const Vector4& vrt0, const Vector4& vrt1, const Vector4& vrt2, const Vector4& vNorm)
{
	::Constants::jetGeoVecCopy(vrt[0], vrt0);
	::Constants::jetGeoVecCopy(vrt[1], vrt1);
	::Constants::jetGeoVecCopy(vrt[2], vrt2);
	for (uint num = 0u; num < 3; num++)
	{
		uint num2 = (num + 1) % 3;
		::Constants::jetGeoVecSub(vTmp[0], vrt[num], vPos);
		::Constants::jetGeoVecSub(vTmp[1], vrt[num2], vPos);
		::Constants::jetGeoVecOuterProduct(vTmp[2], vTmp[1], vTmp[0]);
		if (Constants::IsVecNormalize(vTmp[2]))
		{
			::Constants::jetGeoVecNormalize(vTmp[2], vTmp[2]);
			if (0.0f >= ::Constants::jetGeoVecInnerProduct(vTmp[2], vNorm))
			{
				return false;
			}
		}
	}
	return true;
}

int cTaskCollision::CollisionTriangleEdge(const Vector4& vPos, const Vector4& vrt0, const Vector4& vrt1, const Vector4& vrt2, const Vector4& vNorm, const float& length, Vector4& vOutEdgePos, Vector4& vOutEdgeVec)
{
	vOutEdgePos = Vector4();
	vOutEdgeVec = Vector4();
	Vector4 v = Vector4::zero;
	Vector4 v2 = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	::Constants::jetGeoVecUnit(v2);
	::Constants::jetGeoVecCopy(vrt[0], vrt0);
	::Constants::jetGeoVecCopy(vrt[1], vrt1);
	::Constants::jetGeoVecCopy(vrt[2], vrt2);
	for (uint num = 0u; num < 3; num++)
	{
		uint num2 = (num + 1) % 3;
		::Constants::jetGeoVecSub(vTmp[0], vrt[num2], vrt[num]);
		float num3 = ::Constants::jetGeoVecLength(vTmp[0]);
		::Constants::jetGeoVecNormalize(vTmp[0], vTmp[0]);
		::Constants::jetGeoVecOuterProduct(vTmp[1], vNorm, vTmp[0]);
		if (!Constants::IsVecNormalize(vTmp[1]))
		{
			continue;
		}
		::Constants::jetGeoVecNormalize(vTmp[1], vTmp[1]);
		::Constants::jetGeoVecScale(v2, vTmp[1], -1.0f);
		PLANE plane = PLANE((Vector3)vTmp[1], 0.0f - ::Constants::jetGeoVecInnerProduct(vTmp[1], vrt[num]));
		RAY3 ray = RAY3(vPos, v2);
		Vector3 outCross = Vector3(0.0f, 0.0f, 0.0f);
		float outDist;
		bool num4 = DummyNW4R::IntersectionRay3Plane(ray, plane, outDist, outCross);
		::Constants::jetGeoVecSet(v, outCross.x, outCross.y, outCross.z, 1.0f);
		if (num4 && outDist <= length)
		{
			::Constants::jetGeoVecSub(vTmp[0], vrt[num], v);
			::Constants::jetGeoVecSub(vTmp[1], vrt[num2], v);
			if (::Constants::jetGeoVecLength(vTmp[0]) <= num3 && ::Constants::jetGeoVecLength(vTmp[1]) <= num3)
			{
				::Constants::jetGeoVecCopy(vOutEdgePos, v);
				::Constants::jetGeoVecCopy(vOutEdgeVec, v2);
				return (int)num;
			}
		}
	}
	return -1;
}

bool cTaskCollision::CollisionCircleSegment(const Vector4& segSt, const Vector4& segEn, const Vector4& cirPos, const float& cirR)
{
	Vector4 v = Vector4::zero;
	Vector4 v2 = Vector4::zero;
	::Constants::jetGeoVecSet(v, segEn.x - segSt.x, 0.0f, segEn.z - segSt.z, 1.0f);
	::Constants::jetGeoVecSet(v2, cirPos.x - segSt.x, 0.0f, cirPos.z - segSt.z, 1.0f);
	float num = ::Constants::jetGeoVecInnerProduct(v, v2);
	if (num < 0.0f)
	{
		if (!(::Constants::jetGeoVecInnerProduct(v2, v2) < cirR * cirR))
		{
			return false;
		}
		return true;
	}
	float num2 = ::Constants::jetGeoVecInnerProduct(v, v);
	if (num > num2)
	{
		::Constants::jetGeoVecSet(v2, cirPos.x - segEn.x, 0.0f, cirPos.z - segEn.z, 1.0f);
		if (!(::Constants::jetGeoVecInnerProduct(v2, v2) < cirR * cirR))
		{
			return false;
		}
		return true;
	}
	if (!(::Constants::jetGeoVecInnerProduct(v2, v2) - num / num2 * num < cirR * cirR))
	{
		return false;
	}
	return true;
}

bool cTaskCollision::CheckBottom(const Vector4& vPos, float& outDist, Vector4& outCross, Constants::sFace*& outFace, cTaskBgHit*& outObj)
{
	outDist = 0.0f;
	outCross = vPos;
	outFace = nullptr;
	outObj = nullptr;
	float num = 0.0f;
	Vector4 v = Vector4::zero;
	Constants::sFace* sFace = nullptr;
	cTaskBgHit* cTaskBgHit2 = nullptr;
	Vector4 v2 = Vector4();
	::Constants::jetGeoVecSet(v, 0.0f, -1.0f, 0.0f, 1.0f);
	for (cTaskBgHit* item : m_vecObj)
	{
		if (!cTaskCollision::CollisionRayAABB(vPos, v, item))
		{
			continue;
		}
		Vector4 v3;
		::Constants::jetGeoVecTrans(v3, item->pMtxInv(), vPos);
		Vector4 v4;
		::Constants::jetGeoVecTrans(v4, item->pMtxRotateInv(), v);
		RAY3 ray = RAY3(v3, v4);
		uint num2 = item->pFaceList()->nFaceNum(1);
		for (int i = 0; (long)i < (long)num2; i++)
		{
			Constants::sFace* sFace2 = item->pFaceList()->pFace(1, i);
			Vector4 vOutCross;
			if (cTaskCollision::CollisionRayFaceAABB(ray, item, sFace2) && 0.0f > ::Constants::jetGeoVecInnerProduct(v4, sFace2->vNorm) && cTaskCollision::CollisionRayPlane(v3, v4, sFace2->vVrt[0], sFace2->vNorm, vOutCross) && cTaskCollision::CollisionInnerTriangle(vOutCross, sFace2->vVrt[0], sFace2->vVrt[1], sFace2->vVrt[2], sFace2->vNorm))
			{
				Vector4 v5;
				::Constants::jetGeoVecSub(v5, vOutCross, v3);
				float num3 = ::Constants::jetGeoVecInnerProduct(v5, v5);
				if (sFace == nullptr || num > num3)
				{
					sFace = sFace2;
					cTaskBgHit2 = item;
					num = num3;
					::Constants::jetGeoVecCopy(v2, vOutCross);
				}
			}
		}
	}
	if (sFace != nullptr)
	{
		outDist = num;
		::Constants::jetGeoVecTrans(outCross, cTaskBgHit2->pMtx(), v2);
		outFace = sFace;
		outObj = cTaskBgHit2;
		return true;
	}
	return false;
}

bool cTaskCollision::CheckVector(uint nKind, const Vector4& vPos, const Vector4& vVec, float& outDist, Vector4& outCross, Constants::sFace*& outFace, cTaskBgHit*& outObj)
{
	outDist = 0.0f;
	outCross = Vector4();
	outFace = nullptr;
	outObj = nullptr;
	float num = 0.0f;
	Constants::sFace* sFace = nullptr;
	cTaskBgHit* cTaskBgHit2 = nullptr;
	Vector4 v = Vector4();
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, vVec);
	for (cTaskBgHit* item : m_vecObj)
	{
		if (!cTaskCollision::CollisionRayAABB(vPos, v2, item))
		{
			continue;
		}
		Vector4 v3;
		::Constants::jetGeoVecTrans(v3, item->pMtxInv(), vPos);
		Vector4 v4;
		::Constants::jetGeoVecTrans(v4, item->pMtxRotateInv(), v2);
		uint num2 = item->pFaceList()->nFaceNum((int)nKind);
		for (int i = 0; (long)i < (long)num2; i++)
		{
			Constants::sFace* sFace2 = item->pFaceList()->pFace((int)nKind, i);
			Vector4 vOutCross;
			if (0.0f > ::Constants::jetGeoVecInnerProduct(v4, sFace2->vNorm) && cTaskCollision::CollisionRayPlane(v3, v4, sFace2->vVrt[0], sFace2->vNorm, vOutCross) && cTaskCollision::CollisionInnerTriangle(vOutCross, sFace2->vVrt[0], sFace2->vVrt[1], sFace2->vVrt[2], sFace2->vNorm))
			{
				Vector4 v5;
				::Constants::jetGeoVecSub(v5, vOutCross, v3);
				float num3 = ::Constants::jetGeoVecInnerProduct(v5, v5);
				if (sFace == nullptr || num > num3)
				{
					sFace = sFace2;
					cTaskBgHit2 = item;
					num = num3;
					::Constants::jetGeoVecCopy(v, vOutCross);
				}
			}
		}
	}
	if (sFace != nullptr)
	{
		outDist = num;
		::Constants::jetGeoVecTrans(outCross, cTaskBgHit2->pMtx(), v);
		outFace = sFace;
		outObj = cTaskBgHit2;
		return true;
	}
	return false;
}

bool cTaskCollision::CheckVectorSimple(const uint& nKind, const Vector4& vPos, const Vector4& vVec, Vector4& outCross, Constants::sFace*& outFace, cTaskBgHit*& outObj)
{
	outCross = Vector4();
	outFace = nullptr;
	outObj = nullptr;
	float num = 0.0f;
	Constants::sFace* sFace = nullptr;
	cTaskBgHit* cTaskBgHit2 = nullptr;
	Vector4 v = Vector4();
	float num2 = ::Constants::bizGeoVecLength2(vVec);
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, vVec);
	for (cTaskBgHit* item : m_vecObj)
	{
		if (!cTaskCollision::CollisionRayAABB(vPos, v2, item))
		{
			continue;
		}
		Vector4 v3;
		::Constants::jetGeoVecTrans(v3, item->pMtxInv(), vPos);
		Vector4 v4;
		::Constants::jetGeoVecTrans(v4, item->pMtxRotateInv(), v2);
		uint num3 = item->pFaceList()->nFaceNum((int)nKind);
		for (int i = 0; (long)i < (long)num3; i++)
		{
			Constants::sFace* sFace2 = item->pFaceList()->pFace((int)nKind, i);
			Vector4 vOutCross;
			if (0.0f > ::Constants::jetGeoVecInnerProduct(v4, sFace2->vNorm) && cTaskCollision::CollisionRayPlane(v3, v4, sFace2->vVrt[0], sFace2->vNorm, vOutCross) && cTaskCollision::CollisionInnerTriangle(vOutCross, sFace2->vVrt[0], sFace2->vVrt[1], sFace2->vVrt[2], sFace2->vNorm))
			{
				Vector4 v5;
				::Constants::jetGeoVecSub(v5, vOutCross, v3);
				float num4 = ::Constants::jetGeoVecInnerProduct(v5, v5);
				if (sFace == nullptr || num > num4)
				{
					sFace = sFace2;
					cTaskBgHit2 = item;
					num = num4;
					::Constants::jetGeoVecCopy(v, vOutCross);
				}
			}
		}
	}
	if (sFace != nullptr && num2 >= num)
	{
		::Constants::jetGeoVecTrans(outCross, cTaskBgHit2->pMtx(), v);
		outFace = sFace;
		outObj = cTaskBgHit2;
		return true;
	}
	return false;
}

void cTaskCollision::HitListup(std::vector<cHitChar*>& pVec, const int& nSide, const uint& nFlag)
{
	for (cTaskChar* item : m_vecChar)
	{
		item->HitListup(pVec, nSide, nFlag);
	}
}

bool cTaskCollision::HitPreCheck(cHitChar* pHit)
{
	std::vector<cHitChar*> list;
	uint num = pHit->nSide() ^ 1;
	for (cTaskChar* item : m_vecChar)
	{
		item->HitUpdate();
		int nSide = (int)num;
		item->HitListup(list, nSide, 4u);
	}
	for (cHitChar* item2 : list)
	{
		if (!(::Constants::JET_ABS(pHit->vPos().y - item2->vPos().y) > pHit->fOfsRange() + item2->fOfsRange()) && ::Constants::bizGeoVecDistance2XZ(pHit->vPos(), item2->vPos()) <= pHit->fRange2() + item2->fRange2())
		{
			return true;
		}
	}
	return false;
}

}
