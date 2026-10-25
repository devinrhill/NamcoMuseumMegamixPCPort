#include "kingdom/game/g03/cTaskBgGimmickSeesaw.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/cTaskCollision.hpp"
#include "ResMdl.hpp"
#include "ResNode.hpp"
#include "ResUserData.hpp"
#include "ResUserDataItem.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskBgGimmickSeesaw::cTaskBgGimmickSeesaw(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgGimmick(node0, node1)
{
	SetName("cTaskBgGimmickSeesaw");
	Init();
	ResUserData* resUserData = node1->GetResUserData();
	if (resUserData->IsValid())
	{
		ResUserDataItem* item = resUserData->GetItem("def_rot");
		if (item->IsValid())
		{
			m_fRotInit = ::Constants::JET_DEG2RAD(item->GetF32());
		}
		else
		{
			m_bLevel = true;
			m_fRotInit = 0.0f;
		}
		item = resUserData->GetItem("max_rot");
		if (item->IsValid())
		{
			m_fRotMax = ::Constants::JET_DEG2RAD(item->GetF32());
		}
	}
	this->Model->vRotate()[0] = m_fRotInit;
	m_fRot = m_fRotInit;
}

void cTaskBgGimmickSeesaw::Init()
{
	::Constants::jetGeoVecUnit(m_vFulcrumL);
	m_bLevel = false;
	m_fRotInit = 0.0f;
	m_fRotMax = 0.0f;
	m_fRot = 0.0f;
	m_fRotAcc = 0.0f;
	m_fRotAccMax = 0.015f;
	m_fLength = 4.0f;
	m_fMoment = 0.001f;
	m_fNoRiddenRot = 0.015f;
	::Constants::jetGeoVecUnit(m_vFulcrumL);
	::Constants::jetGeoVecUnit(m_vApplication);
	OnState(4u);
	Constants::PACN_APP();
	ResMdl* resMdl = new ResMdl(this->Model->pScnMdl());
	ResNode* resNode = resMdl->GetResNode("root");
	CustomKingdomUtil::Assert(resNode->IsValid(), "error");
	if (resNode->IsValid())
	{
		Vector3 vector = resNode->GetTranslate();
		::Constants::jetGeoVecSet(m_vFulcrumL, vector.x, vector.y, vector.z, 1.0f);
	}
	resMdl = pFaceList()->pResMdl();
	if (!resMdl->IsValid())
	{
		return;
	}
	resNode = resMdl->GetResNode("length");
	if (resNode->IsValid())
	{
		Vector3 vector = resNode->GetTranslate();
		if (::Constants::fabs(vector.z) > 0.0f)
		{
			m_fLength = ::Constants::fabs(vector.z);
		}
	}
}

void cTaskBgGimmickSeesaw::Term()
{
	cTaskBgGimmick::Term();
}

bool cTaskBgGimmickSeesaw::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	cTaskCharPacman* cTaskCharPacman2 = cAppG03PacnRollGame2->pTaskCharPacman();
	if (!Constants::IS_PAC_PAUSE() && cTaskCharPacman2 != nullptr && !cAppG03PacnRollGame2->IsNakajimaPause(this))
	{
		float fRot = m_fRot;
		bool flag = false;
		if (cTaskCharPacman2->pHitObj(1) == this || (cTaskCharPacman2->IsState(3072u) && CheckBottom()))
		{
			if (0.0f < m_fLength)
			{
				float fSeesawAccMax = cTaskCharPacman2->GetParam()->fSeesawAccMax;
				Vector4 v;
				::Constants::jetGeoVecTrans(v, pMtxInv(), cTaskCharPacman2->vTrans());
				float num = cTaskCharPacman2->GetParam()->fSeesawMoment * (v[2] / m_fLength);
				m_fRotAcc += num;
				if (m_fRotAcc > fSeesawAccMax)
				{
					m_fRotAcc = fSeesawAccMax;
				}
				else if (m_fRotAcc < 0.0f - fSeesawAccMax)
				{
					m_fRotAcc = 0.0f - fSeesawAccMax;
				}
				m_fRot += m_fRotAcc;
				flag = true;
			}
		}
		else
		{
			m_fRotAcc = 0.0f;
			if (m_bLevel)
			{
				if (m_fRot > 0.0f)
				{
					m_fRot -= m_fNoRiddenRot;
				}
				else if (m_fRot < 0.0f)
				{
					m_fRot += m_fNoRiddenRot;
				}
				if (0.0f - m_fNoRiddenRot < m_fRot && m_fRot < m_fNoRiddenRot)
				{
					m_fRot = 0.0f;
				}
			}
			else if (m_fRot >= 0.0f)
			{
				m_fRot += m_fNoRiddenRot;
			}
			else
			{
				m_fRot -= m_fNoRiddenRot;
			}
		}
		if (m_fRot > m_fRotMax)
		{
			m_fRot = m_fRotMax;
		}
		else if (m_fRot < 0.0f - m_fRotMax)
		{
			m_fRot = 0.0f - m_fRotMax;
		}
		this->Model->vRotate()[0] = m_fRot;
		if (!flag && cTaskCharPacman2->IsState(3072u))
		{
			UpdateMatrix();
			if (CheckBottom())
			{
				this->Model->vRotate()[0] = (m_fRot = fRot);
			}
		}
	}
	return cTaskBgGimmick::Execute(pParam);
}

void cTaskBgGimmickSeesaw::Draw(int nStatus, cJetTask* pParam)
{
	cTaskBgGimmick::Draw(nStatus, pParam);
}

bool cTaskBgGimmickSeesaw::CheckBottom()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	if (cTaskCharPacman2 == nullptr)
	{
		return false;
	}
	if (!cTaskCollision::CollisionRayAABB(cTaskCharPacman2->vTrans(), Vector4(), this))
	{
		return false;
	}
	Vector4 v;
	::Constants::jetGeoVecTrans(v, pMtxInv(), cTaskCharPacman2->vTrans());
	uint num = pFaceList()->nFaceNum(1);
	for (int i = 0; (long)i < (long)num; i++)
	{
		Constants::sFace* sFace = pFaceList()->pFace(1, i);
		Vector4 v2;
		::Constants::jetGeoVecScale(v2, sFace->vNorm, -1.0f);
		Vector4 vOutCross;
		Vector4 _discard1;
		Vector4 _discard2;
		if (cTaskCollision::CollisionRayPlane(v, v2, sFace->vVrt[0], sFace->vNorm, vOutCross) && (cTaskCollision::CollisionInnerTriangle(vOutCross, sFace->vVrt[0], sFace->vVrt[1], sFace->vVrt[2], sFace->vNorm) || cTaskCollision::CollisionTriangleEdge(vOutCross, sFace->vVrt[0], sFace->vVrt[1], sFace->vVrt[2], sFace->vNorm, cTaskCharPacman2->fRadius(), _discard1, _discard2) >= 0))
		{
			Vector4 v3;
			::Constants::jetGeoVecSub(v3, vOutCross, v);
			float num2 = ::Constants::jetGeoVecInnerProduct(v3, v3);
			if (cTaskCharPacman2->fRadius() * cTaskCharPacman2->fRadius() * 2.0f >= num2)
			{
				return true;
			}
		}
	}
	return false;
}

}
