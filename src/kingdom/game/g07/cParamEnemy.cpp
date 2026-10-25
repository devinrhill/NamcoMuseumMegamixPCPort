#include "kingdom/game/g07/cParamEnemy.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"

namespace kingdom::game::g07
{

void cParamEnemy::Init()
{
	m_bValid = false;
	m_nEnemyLevel = -1;
}

void cParamEnemy::Term()
{
}

void cParamEnemy::LoadValue()
{
	CustomKingdomUtil::Assert(m_nEnemyLevel != -1, "error");
	CustomKingdomUtil::Assert(m_nEnemyLevel < 28, "error");
	CustomKingdomUtil::Assert(28 <= nRow(), "csv error");
	CustomKingdomUtil::Assert(18 <= nCol(), "csv error");
	m_sParam = new MO_CHAR_PARAMS();
	MO_CHAR_PARAMS* sParam = m_sParam;
	int nEnemyLevel = m_nEnemyLevel;
	sParam->m_fWeight = F32(nEnemyLevel, 1, 0.0f);
	sParam->m_nHereticVal = (uint)S32(nEnemyLevel, 2, 0);
	sParam->m_fMaxSpeed = F32(nEnemyLevel, 3, 0.0f);
	sParam->m_fAccelSpeed = F32(nEnemyLevel, 4, 0.0f);
	sParam->m_fDecelSpeed = F32(nEnemyLevel, 5, 0.0f);
	sParam->m_fHitPower = F32(nEnemyLevel, 6, 0.0f);
	sParam->m_fChargePower = F32(nEnemyLevel, 7, 0.0f);
	sParam->m_fSlopePower = F32(nEnemyLevel, 8, 0.0f);
	sParam->m_fWallReflect = F32(nEnemyLevel, 9, 0.0f);
	sParam->m_fHitSize = F32(nEnemyLevel, 10, 0.0f);
	sParam->m_fWHitSize = F32(nEnemyLevel, 11, 0.0f);
	sParam->m_nPoints = S32(nEnemyLevel, 12, 0);
	sParam->m_nVSPoints = S32(nEnemyLevel, 13, 0);
	sParam->m_fExParam[0] = F32(nEnemyLevel, 14, 0.0f);
	sParam->m_fExParam[1] = F32(nEnemyLevel, 15, 0.0f);
	sParam->m_fExParam[2] = F32(nEnemyLevel, 16, 0.0f);
	m_bValid = true;
}

cParamEnemy::cParamEnemy()
{
	Init();
}

cParamEnemy::cParamEnemy(string key, uint level)
{
	Init();
	RegistData(key, level);
}

cParamEnemy::~cParamEnemy()
{
	Term();
}

bool cParamEnemy::RegistData(string key, uint level)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	m_nEnemyLevel = (int)(level + 1);
	LoadValue();
	return result;
}

MO_CHAR_PARAMS* cParamEnemy::pMotosParams()
{
	return m_sParam;
}

}
