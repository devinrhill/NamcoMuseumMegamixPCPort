#include "kingdom/game/g07/cParamPlayer.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"

namespace kingdom::game::g07
{

void cParamPlayer::Init()
{
	m_bValid = false;
}

void cParamPlayer::Term()
{
}

void cParamPlayer::LoadValue()
{
	int row = 1;
	CustomKingdomUtil::Assert(4 <= nRow(), "csv error");
	CustomKingdomUtil::Assert(17 <= nCol(), "csv error");
	m_sParam = new MO_CHAR_PARAMS();
	MO_CHAR_PARAMS* sParam = m_sParam;
	sParam->m_fWeight = F32(row, 1, 0.0f);
	sParam->m_nHereticVal = (uint)S32(row, 2, 0);
	sParam->m_fMaxSpeed = F32(row, 3, 0.0f);
	sParam->m_fAccelSpeed = F32(row, 4, 0.0f);
	sParam->m_fDecelSpeed = F32(row, 5, 0.0f);
	sParam->m_fHitPower = F32(row, 6, 0.0f);
	sParam->m_fChargePower = F32(row, 7, 0.0f);
	sParam->m_fSlopePower = F32(row, 8, 0.0f);
	sParam->m_fWallReflect = F32(row, 9, 0.0f);
	sParam->m_fHitSize = F32(row, 10, 0.0f);
	sParam->m_fWHitSize = F32(row, 11, 0.0f);
	sParam->m_nPoints = S32(row, 12, 0);
	sParam->m_nVSPoints = 0;
	sParam->m_fExParam[0] = F32(row, 13, 0.0f);
	sParam->m_fExParam[1] = F32(row, 14, 0.0f);
	sParam->m_fExParam[2] = F32(row, 15, 0.0f);
	m_bValid = true;
}

cParamPlayer::cParamPlayer()
{
	Init();
}

cParamPlayer::cParamPlayer(string key)
{
	Init();
	RegistData(key);
}

cParamPlayer::~cParamPlayer()
{
	Term();
}

bool cParamPlayer::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	LoadValue();
	return result;
}

MO_CHAR_PARAMS* cParamPlayer::pMotosParams()
{
	return m_sParam;
}

}
