#include "kingdom/game/g03/cCameraParam.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g03
{

const string cCameraParam::s_pParamName[22] = {"eCAMERA_TYPE", "eCAMERA_HEIGHT", "eCAMERA_DISTANCE", "eCAMERA_RECOANGLE", "eCAMERA_INITANGLE", "eCAMERA_PHIANGLE", "eCAMERA_CAMDISTANCE", "eCAMERA_TARGET_DELAY", "eCAMERA_CAMERA_DELAY", "eCAMERA_PHI_ACCEL", "eCAMERA_DIST_ACCEL", "eCAMERA_RECO_POWER", "eCAMERA_ANG_LOCK", "eCAMERA_EXRANGE00", "eCAMERA_EXRANGE01", "eCAMERA_GOAL_PHI", "eCAMERA_GOAL_DIST", "eCAMERA_GOAL_SPEED", "eCAMERA_START_SPEED", "eCAMERA_START_ANGLE", "eCAMERA_CLIP_FAR", "eCAMERA_CLIP_ANGLE"};
Constants::CAM_PARAM* cCameraParam::s_StanDard = new Constants::CAM_PARAM();

cCameraParam::cCameraParam(const string& key)
{
	Init();
	RegistData(key);
}

cCameraParam::~cCameraParam()
{
	Term();
}

void cCameraParam::Init()
{
	m_nValueFlag = 0u;
}

void cCameraParam::Term()
{
}

bool cCameraParam::RegistData(const string& key)
{
	bool result = cKingdomCSV::RegistData(key);
	LoadValue();
	return result;
}

uint cCameraParam::CheckValue(int nKind)
{
	return m_nValueFlag & (uint)(1 << nKind);
}

Constants::CAM_PARAM* cCameraParam::GetParam()
{
	return m_Param;
}

int cCameraParam::GetCameraType()
{
	return m_Param->m_nType;
}

float cCameraParam::GetParamHeight()
{
	return m_Param->m_fParamHeight;
}

float cCameraParam::GetParamDistance()
{
	return m_Param->m_fParamDistanceXZ;
}

float cCameraParam::GetInitAngle()
{
	return m_Param->m_fInitAngle;
}

void cCameraParam::LoadValue()
{
	m_Param = new Constants::CAM_PARAM();
	LoadValue(0, m_Param->m_nType, 1);
	LoadValue(1, m_Param->m_fParamHeight, 0);
	LoadValue(2, m_Param->m_fParamDistanceXZ, 0);
	LoadValue(4, m_Param->m_fInitAngle, 0);
	LoadValue(5, m_Param->m_fPhiAngle, 0);
	LoadValue(6, m_Param->m_fCamDistance, 0);
	LoadValue(7, m_Param->m_fTargetDelay, 0);
	LoadValue(8, m_Param->m_fCameraDelay, 0);
	LoadValue(3, m_Param->m_fRecoAngle, 0);
	LoadValue(11, m_Param->m_fRecoPower, 0);
	LoadValue(13, m_Param->m_fExRange[0], 0, 2);
	LoadValue(14, m_Param->m_fExRange[1], 0, 2);
	LoadValue(9, m_Param->m_fPhiAccel, 0);
	LoadValue(10, m_Param->m_fDistAccel, 0);
	LoadValue(15, m_Param->m_fGoalPhi, 0);
	LoadValue(16, m_Param->m_fGoalDist, 0);
	LoadValue(17, m_Param->m_fGoalSpeed, 0);
	LoadValue(18, m_Param->m_fStartSpeed, 0);
	LoadValue(19, m_Param->m_fStartAngle, 0);
	LoadValue(20, m_Param->m_fClipFar, 0);
	LoadValue(21, m_Param->m_fClipAngle, 0);
	m_Param->m_nFlag = m_nValueFlag;
}

void cCameraParam::GetParam(Constants::CAM_PARAM*& pParam)
{
	pParam = m_Param;
}

bool cCameraParam::LoadValue(int nID, float* pData, int nType, int nNum)
{
	std::fill(pData, pData + 2, 0.0f);
	string searchStr = s_pParamName[nID];
	int num = SearchRow(0, searchStr);
	if (num >= 0)
	{
		for (int i = 0; i < nNum; i++)
		{
			if (nType == 0)
			{
				CustomKingdomUtil::Assert(F32(num, 1 + i, pData[i]), "error");
			}
		}
		m_nValueFlag |= (uint)(1 << nID);
		return true;
	}
	return false;
}

bool cCameraParam::LoadValue(int nID, float& pData, int nType)
{
	pData = 0.0f;
	string searchStr = s_pParamName[nID];
	int num = SearchRow(0, searchStr);
	if (num >= 0)
	{
		if (nType == 0)
		{
			CustomKingdomUtil::Assert(F32(num, 1, pData), "error");
		}
		m_nValueFlag |= (uint)(1 << nID);
		return true;
	}
	return false;
}

bool cCameraParam::LoadValue(int nID, int& pData, int nType)
{
	pData = 0;
	string searchStr = s_pParamName[nID];
	int num = SearchRow(0, searchStr);
	if (num >= 0)
	{
		if (nType == 1)
		{
			CustomKingdomUtil::Assert(S32(num, 1, pData), "error");
		}
		m_nValueFlag |= (uint)(1 << nID);
		return true;
	}
	return false;
}

Constants::CAM_PARAM* cCameraParam::GetStandardParam()
{
	return s_StanDard;
}

void cCameraParam::MERGE_VAL(Constants::CAM_PARAM* p1, Constants::CAM_PARAM* p2, int& val1, int val2, int def)
{
	if (!CustomKingdomUtil::IF(p1->m_nFlag & (uint)(1 << def)) && CustomKingdomUtil::IF(p2->m_nFlag & (uint)(1 << def)))
	{
		val1 = val2;
	}
}

void cCameraParam::MERGE_VAL(Constants::CAM_PARAM* p1, Constants::CAM_PARAM* p2, float& val1, float val2, int def)
{
	if (!CustomKingdomUtil::IF(p1->m_nFlag & (uint)(1 << def)) && CustomKingdomUtil::IF(p2->m_nFlag & (uint)(1 << def)))
	{
		val1 = val2;
	}
}

void cCameraParam::MergeParam(Constants::CAM_PARAM*& pDst, Constants::CAM_PARAM* const& pDefault)
{
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_nType, pDefault->m_nType, 0);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fParamHeight, pDefault->m_fParamHeight, 1);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fInitAngle, pDefault->m_fInitAngle, 4);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fPhiAngle, pDefault->m_fPhiAngle, 5);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fCamDistance, pDefault->m_fCamDistance, 6);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fTargetDelay, pDefault->m_fTargetDelay, 7);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fCameraDelay, pDefault->m_fCameraDelay, 8);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fRecoPower, pDefault->m_fRecoPower, 11);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fPhiAccel, pDefault->m_fPhiAccel, 9);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fDistAccel, pDefault->m_fDistAccel, 10);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fGoalPhi, pDefault->m_fGoalPhi, 15);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fGoalDist, pDefault->m_fGoalDist, 16);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fGoalSpeed, pDefault->m_fGoalSpeed, 17);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fStartSpeed, pDefault->m_fStartSpeed, 18);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fStartAngle, pDefault->m_fStartAngle, 19);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fClipFar, pDefault->m_fClipFar, 20);
	cCameraParam::MERGE_VAL(pDst, pDefault, pDst->m_fClipAngle, pDefault->m_fClipAngle, 21);
	if (!CustomKingdomUtil::IF(pDst->m_nFlag & 0x2000) && CustomKingdomUtil::IF(pDefault->m_nFlag & 0x2000))
	{
		pDst->m_fExRange[0][0] = pDefault->m_fExRange[0][0];
		pDst->m_fExRange[0][1] = pDefault->m_fExRange[0][1];
	}
	if (!CustomKingdomUtil::IF(pDst->m_nFlag & 0x4000) && CustomKingdomUtil::IF(pDefault->m_nFlag & 0x4000))
	{
		pDst->m_fExRange[1][0] = pDefault->m_fExRange[1][0];
		pDst->m_fExRange[1][1] = pDefault->m_fExRange[1][1];
	}
	pDst->m_nFlag |= pDefault->m_nFlag;
}

void cCameraParam::InitStandardParam()
{
	string key = "w0_st0_area00.csv";
	if (nullptr != kingdom::ResourceDatabase::Get<CSVParam>(key))
	{
		(new cCameraParam(key))->GetParam(s_StanDard);
	}
	else
	{
		CustomKingdomUtil::Assert("not found w0_st0_area00.csv");
	}
}

}
