#include "kingdom/game/g03/cTaskPoint.hpp"
#include "DummyNW4R.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/sCheckPoint.hpp"
#include "kingdom/game/g03/sSaveDataPoint.hpp"
#include "kingdom/sKingdomData.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <ReadFileUtil.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

int& cTaskPoint::nRetryIdx()
{
	return m_nRetryIdx;
}

Vector4& cTaskPoint::vRetryPos()
{
	CustomKingdomUtil::Assert(0 <= m_nRetryIdx, "error");
	return pModel((uint)m_nRetryIdx)->vTrans();
}

float cTaskPoint::vRetryPos(uint n)
{
	CustomKingdomUtil::Assert(0 <= m_nRetryIdx, "error");
	return pModel((uint)m_nRetryIdx)->vTrans()[(int)n];
}

Vector4& cTaskPoint::vRetryRot()
{
	CustomKingdomUtil::Assert(0 <= m_nRetryIdx, "error");
	return pModel((uint)m_nRetryIdx)->vRotate();
}

float cTaskPoint::vRetryRot(uint n)
{
	CustomKingdomUtil::Assert(0 <= m_nRetryIdx, "error");
	return pModel((uint)m_nRetryIdx)->vRotate()[(int)n];
}

bool cTaskPoint::IsFontDraw()
{
	return m_bFont;
}

cTaskPoint::cTaskPoint()
{
	SetName("cTaskConsoleMan");
	Init();
}

// DOL 0x8018772c
void cTaskPoint::Init()
{
	m_nRetryIdx = -1;
	m_nNum = 0u;
	m_bFont = false;
	sKingdomData* sKingdomData2 = KingdomGlobal::KingSys->SearchDataEx("BIN_POINT_SET_DAT");
	if (sKingdomData2 != nullptr && 0 < sKingdomData2->nSize)
	{
		std::vector<sSaveDataPoint*> outParams;
		int outParamNum;
		int _discard1;
		ReadFileUtil::Convert<sSaveDataPoint>(sKingdomData2->pBin, outParams, outParamNum, _discard1);
		m_nNum = (uint)outParamNum;
		for (uint num = 0u; num < m_nNum; num++)
		{
			CreateModel(num);
			cTaskModel::cPBModel* cPBModel = pPBModel(num);
			cPBModel->RegistModel("BIN_POINT_MDL");
			// DOL 0x8018772c: two clips, slot 0 = BIN_POINT_1_CHA (0x64260380, the old medallion turning away, weight 1) and slot 1 = BIN_POINT_2_CHA
			// (0x23867950, the pass animation of a later visit, weight 0); both stopped. The converted code only had slot 1 (first pass showed nothing)
			cPBModel->RegistAnm(0u, "BIN_POINT_1_CHA");
			cPBModel->RegistAnm(1u, "BIN_POINT_2_CHA");
			cPBModel->SetWeightAnmChr(0u, 1.0f);
			cPBModel->SetWeightAnmChr(1u, 0.0f);
			cPBModel->SetUpdateRateAnmChr(0u, 0.0f);
			cPBModel->SetUpdateRateAnmChr(1u, 0.0f);
			cPBModel->IsDrawShadowDepth(true);
			cPBModel->SetLightSetIdx(3);
			cPBModel->SetFogIdx(0);
			cPBModel->vRotate()[1] = ::Constants::JET_DEG2RAD(outParams[num]->rotY);
			::Constants::jetGeoVecCopy(cPBModel->vTrans(), outParams[num]->trans);
			cPBModel->Visible();
			cPBModel->UpdateMatrix();
			Matrix4x4 m;
			::Constants::jetGeoMtxIdentity(m);
			::Constants::jetGeoMtxTrans(m, m, cPBModel->vTrans());
			::Constants::jetGeoMtxRotZYX(m, m, cPBModel->vRotate());
			DummyNW4R::SetMtx(cPBModel->pScnMdl(), m);
		}
		for (uint num2 = 0u; num2 < m_nNum; num2++)
		{
			CreateModel(num2 + m_nNum);
			cTaskModel::cPBModel* cPBModel = pPBModel(num2 + m_nNum);
			cPBModel->RegistModel("BIN_POINT_FLAG_MDL");
			cPBModel->RegistAnm(0u, "BIN_POINT_FLAG_CHA");
			cPBModel->IsDrawShadowDepth(false);
			cPBModel->SetLightSetIdx(3);
			cPBModel->SetFogIdx(0);
			cPBModel->vRotate()[1] = ::Constants::JET_DEG2RAD(outParams[num2]->rotY);
			::Constants::jetGeoVecCopy(cPBModel->vTrans(), outParams[num2]->trans);
			cPBModel->Visible();
			cPBModel->UpdateMatrix();
			Matrix4x4 m2;
			::Constants::jetGeoMtxIdentity(m2);
			::Constants::jetGeoMtxTrans(m2, m2, cPBModel->vTrans());
			::Constants::jetGeoMtxRotZYX(m2, m2, cPBModel->vRotate());
			DummyNW4R::SetMtx(cPBModel->pScnMdl(), m2);
		}
	}
	m_pCheckPoint.assign(m_nNum, {});
	for (int i = 0; i < (int)m_pCheckPoint.size(); i++)
	{
		m_pCheckPoint[i] = new sCheckPoint();
	}
	if (!KingdomGlobal::PacnRollSys->IsRetry())
	{
		return;
	}
	m_nRetryIdx = KingdomGlobal::PacnRollSys->pRetryData()->nRetryPointIdx;
	for (cG03PacnRollSys::sRetryPoint* item : KingdomGlobal::PacnRollSys->pRetryData()->vecPoint)
	{
		uint nPassIdx = item->nPassIdx;
		cTaskModel::cPBModel* cPBModel2 = pPBModel(nPassIdx);
		// DOL 0x8018772c (retry): both clips stopped at their last frame
		cPBModel2->SetUpdateRateAnmChr(0u, 0.0f);
		cPBModel2->SetUpdateRateAnmChr(1u, 0.0f);
		cPBModel2->SetFrameAnmChr(1u, cPBModel2->GetFrameMaxAnmChr(1u));
		cPBModel2->SetFrameAnmChr(0u, cPBModel2->GetFrameMaxAnmChr(0u));
		m_pCheckPoint[nPassIdx]->isPass = true;
		m_pCheckPoint[nPassIdx]->isHitOld = true;
	}
}

void cTaskPoint::Term()
{
	cTaskModel::Term();
}

// DOL 0x80187b70
bool cTaskPoint::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (Constants::IS_PAC_PAUSE() || !cAppG03PacnRollGame2->IsGame())
	{
		return true;
	}
	m_bFont = false;
	for (uint num = 0u; num < m_nNum; num++)
	{
		bool isHitOld = m_pCheckPoint[num]->isHitOld;
		m_pCheckPoint[num]->isHitOld = false;
		cTaskModel::cPBModel* cPBModel = pPBModel(num);
		if (HitCheck(cPBModel->vTrans()))
		{
			if (!m_pCheckPoint[num]->isPass)
			{
				m_pCheckPoint[num]->isPass = true;
				m_nRetryIdx = (int)num;
				// DOL 0x80187b70: the first pass plays slot 0 and parks slot 1 at its end (so a later visit can start it)
				cPBModel->SetUpdateRateAnmChr(0u, 1.0f);
				cPBModel->SetFrameAnmChr(1u, cPBModel->GetFrameMaxAnmChr(1u));
				cAppG03PacnRollGame2->SetEffect(18u, cPBModel->vTrans(), Vector4());
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE028_POINT_TUUKA);
				m_bFont = true;
			}
			else if (!isHitOld && cPBModel->GetFrameMaxAnmChr(0u) <= cPBModel->GetFrameAnmChr(0u) && cPBModel->GetFrameMaxAnmChr(1u) <= cPBModel->GetFrameAnmChr(1u))
			{
				// DOL 0x80187b70: a later visit once both clips are done: slot 1 takes over (weight 0 / 1), restarts; slot 0 stays at its end
				m_nRetryIdx = (int)num;
				cPBModel->SetWeightAnmChr(0u, 0.0f);
				cPBModel->SetWeightAnmChr(1u, 1.0f);
				cPBModel->SetUpdateRateAnmChr(0u, 0.0f);
				cPBModel->SetUpdateRateAnmChr(1u, 1.0f);
				cPBModel->SetFrameAnmChr(1u, 0.0f);
				cPBModel->SetFrameAnmChr(0u, cPBModel->GetFrameMaxAnmChr(0u));
				cAppG03PacnRollGame2->SetEffect(18u, cPBModel->vTrans(), Vector4());
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE028_POINT_TUUKA);
				m_bFont = true;
			}
			m_pCheckPoint[num]->isHitOld = true;
		}
	}
	UpdateFrame();
	return cTaskModel::Execute(pParam);
}

void cTaskPoint::Draw(int nStatus, cJetTask* pParam)
{
	cTaskModel::Draw(nStatus, pParam);
}

bool cTaskPoint::HitCheck(const Vector4& trans)
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	Vector4 v;
	::Constants::jetGeoVecCopy(v, trans);
	v[1] += cTaskCharPacman2->fRadius();
	Constants::sFace* sFace = cTaskCharPacman2->pHitFace(1);
	if (sFace != nullptr && (sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_NORM || sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_GRAS))
	{
		::Constants::jetGeoVecSub(v, cTaskCharPacman2->vTrans(), v);
		if (::Constants::jetGeoVecInnerProduct(v, v) <= 2.25f)
		{
			return true;
		}
	}
	return false;
}

void cTaskPoint::SetRetryData()
{
	KingdomGlobal::PacnRollSys->pRetryData()->nRetryPointIdx = m_nRetryIdx;
	for (uint num = 0u; num < m_nNum; num++)
	{
		if (m_pCheckPoint[num]->isPass)
		{
			cG03PacnRollSys::sRetryPoint* sRetryPoint = new cG03PacnRollSys::sRetryPoint();
			sRetryPoint->nPassIdx = num;
			KingdomGlobal::PacnRollSys->pRetryData()->vecPoint.push_back(sRetryPoint);
		}
	}
}

}
