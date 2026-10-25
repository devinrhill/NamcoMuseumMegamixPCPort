#include "kingdom/game/g07/cParamStage.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cParamSlope.hpp"
#include "kingdom/game/g07/MO_METEO_INFO.hpp"
#include "kingdom/game/g07/MO_SLOPE_INFO.hpp"
#include "kingdom/game/g07/MO_SLOPE_PAT.hpp"

namespace kingdom::game::g07
{

void cParamStage::Init()
{
	m_bValid = false;
	m_SlopeInfo = new MO_SLOPE_INFO();
	m_MeteoInfo = new MO_METEO_INFO();
}

void cParamStage::Term()
{
}

void cParamStage::LoadValue()
{
	CustomKingdomUtil::Assert(50 <= nRow(), "csv error");
	CustomKingdomUtil::Assert(10 <= nCol(), "csv error");
	int row = (int)(m_StageNum + 1);
	m_MeteoInfo->m_nLimit = S32(row, 1, 0);
	m_MeteoInfo->m_nInterval_Min = S32(row, 2, 0);
	m_MeteoInfo->m_nInterval_Max = S32(row, 3, 0);
	m_MeteoInfo->m_nStep = 0;
	int stageNum = (int)m_StageNum;
	cParamSlope* cParamSlope2 = new cParamSlope("CSV_SLOPE_PATERN");
	for (int i = 0; i < 5; i++)
	{
		int num = S32(stageNum + 1, 4 + i, 0);
		for (int j = 1; j < cParamSlope2->GetRow(); j++)
		{
			if (num == cParamSlope2->GetValueS32(j, 0))
			{
				m_SlopeInfo->m_Pat[i] = new MO_SLOPE_PAT();
				m_SlopeInfo->m_Pat[i]->m_nPatNo = num;
				m_SlopeInfo->m_Pat[i]->m_fRot = Constants::TSP_DEG2RAD(cParamSlope2->GetValueF32(j, 1));
				m_SlopeInfo->m_Pat[i]->m_nTime = cParamSlope2->GetValueS32(j, 2);
				m_SlopeInfo->m_Pat[i]->m_nStopTime = cParamSlope2->GetValueS32(j, 3);
				m_SlopeInfo->m_Pat[i]->m_nRevTime = cParamSlope2->GetValueS32(j, 4);
				m_SlopeInfo->m_Pat[i]->m_nRevStopTime = cParamSlope2->GetValueS32(j, 5);
				m_SlopeInfo->m_Pat[i]->m_nNum = cParamSlope2->GetValueS32(j, 6);
				if (m_SlopeInfo->m_Pat[i]->m_nPatNo != 0)
				{
					m_SlopeInfo->m_nPatNum++;
				}
			}
		}
	}
	for (int k = 0; k < 5; k++)
	{
		if (m_SlopeInfo->m_Pat[k]->m_nPatNo > 0)
		{
			MO_SLOPE_PAT* mO_SLOPE_PAT = m_SlopeInfo->m_Pat[k];
			mO_SLOPE_PAT->m_nStart = m_SlopeInfo->m_nTotalTime;
			mO_SLOPE_PAT->m_nTotal = mO_SLOPE_PAT->m_nTime + mO_SLOPE_PAT->m_nStopTime + mO_SLOPE_PAT->m_nRevTime + mO_SLOPE_PAT->m_nRevStopTime;
			m_SlopeInfo->m_nTotalTime += mO_SLOPE_PAT->m_nTotal * mO_SLOPE_PAT->m_nNum;
			mO_SLOPE_PAT->m_nEnd = m_SlopeInfo->m_nTotalTime;
		}
	}
	m_bValid = true;
}

cParamStage::cParamStage()
{
	Init();
}

cParamStage::cParamStage(string key, uint StageNum)
{
	m_StageNum = StageNum;
	Init();
	RegistData(key);
}

cParamStage::~cParamStage()
{
	Term();
}

bool cParamStage::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	LoadValue();
	return result;
}

MO_METEO_INFO* cParamStage::pMeteoInfo()
{
	return m_MeteoInfo;
}

MO_SLOPE_INFO* cParamStage::pSlopeInfo()
{
	return m_SlopeInfo;
}

}
