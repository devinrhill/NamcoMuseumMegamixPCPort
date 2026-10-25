#include "kingdom/game/g07/cParamVS.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"

namespace kingdom::game::g07
{

void cParamVS::Init()
{
	m_bValid = false;
}

void cParamVS::Term()
{
}

void cParamVS::LoadValue()
{
	CustomKingdomUtil::Assert(11 <= nRow(), "csv error");
	CustomKingdomUtil::Assert(4 <= nCol(), "csv error");
	m_VSInfo = new MOTOS_VS_PARAM();
	m_VSInfo->m_nEnemyTouch = S32(1, 1, 0);
	m_VSInfo->m_nMotosTouch = S32(2, 1, 0);
	m_VSInfo->m_nLPFrame = S32(3, 1, 0);
	m_VSInfo->m_nLPPoint = S32(3, 2, 0);
	m_VSInfo->m_fRobPoint = F32(4, 1, 0.0f);
	m_VSInfo->m_fFallPoint = F32(5, 1, 0.0f);
	m_VSInfo->m_nPDPoint = S32(6, 1, 0);
	m_VSInfo->m_fPDDecel = F32(6, 2, 0.0f);
	m_VSInfo->m_fPanelRebirth = F32(7, 1, 0.0f);
	m_VSInfo->m_fPlayerMuteki = F32(8, 1, 0.0f);
	m_VSInfo->m_nSPCharge = S32(9, 1, 0);
	m_VSInfo->m_nSPPiyoEnemy = S32(10, 1, 0);
	m_VSInfo->m_nSPPiyoMySelf = S32(10, 2, 0);
	m_bValid = true;
}

cParamVS::cParamVS()
{
	Init();
}

cParamVS::cParamVS(string key)
{
	Init();
	RegistData(key);
}

cParamVS::~cParamVS()
{
	Term();
}

bool cParamVS::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	LoadValue();
	return result;
}

MOTOS_VS_PARAM* cParamVS::pVSInfo()
{
	return m_VSInfo;
}

}
