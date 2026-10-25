#include "kingdom/game/g07/cParamPlayerLevel.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::g07
{

void cParamPlayerLevel::Init()
{
	m_bValid = false;
}

void cParamPlayerLevel::Term()
{
}

void cParamPlayerLevel::LoadValue()
{
	for (int i = 0; i < 11; i++)
	{
		int row = i + 1;
		m_PowerLevel2Power[i] = F32(row, 1, 0.0f);
		m_HoverLevel2Time[i] = (short)S32(row, 2, 0);
		m_ChargeLevel2Time[i] = (short)S32(row, 3, 0);
		m_ChargeLevel2Power[i] = F32(row, 4, 0.0f);
		m_ChargeLevel2MyRush[i] = F32(row, 5, 0.0f);
		m_ChargeLevel2Heretic[i] = F32(row, 6, 0.0f);
		m_ChargeLevel2Speed[i] = F32(row, 7, 0.0f);
		m_ChargeLevel2Weight[i] = (short)S32(row, 8, 0);
		m_ChargeLevel2PiyoTime[i] = (short)S32(row, 9, 0);
	}
	m_bValid = true;
}

cParamPlayerLevel::cParamPlayerLevel()
{
	Init();
}

cParamPlayerLevel::cParamPlayerLevel(string key)
{
	Init();
	RegistData(key);
}

cParamPlayerLevel::~cParamPlayerLevel()
{
	Term();
}

bool cParamPlayerLevel::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	LoadValue();
	return result;
}

float cParamPlayerLevel::PowerLevel2Power(int n)
{
	CustomKingdomUtil::Assert(n < 11, "array error");
	return m_PowerLevel2Power[n];
}

short cParamPlayerLevel::HoverLevel2Time(int n)
{
	CustomKingdomUtil::Assert(n < 11, "array error");
	return m_HoverLevel2Time[n];
}

short cParamPlayerLevel::ChargeLevel2Time(int n)
{
	CustomKingdomUtil::Assert(n < 11, "array error");
	return m_ChargeLevel2Time[n];
}

float cParamPlayerLevel::ChargeLevel2Power(int n)
{
	CustomKingdomUtil::Assert(n < 11, "array error");
	return m_ChargeLevel2Power[n];
}

float cParamPlayerLevel::ChargeLevel2MyRush(int n)
{
	CustomKingdomUtil::Assert(n < 11, "array error");
	return m_ChargeLevel2MyRush[n];
}

float cParamPlayerLevel::ChargeLevel2Heretic(int n)
{
	CustomKingdomUtil::Assert(n < 11, "array error");
	return m_ChargeLevel2Heretic[n];
}

float cParamPlayerLevel::ChargeLevel2Speed(int n)
{
	CustomKingdomUtil::Assert(n < 11, "array error");
	return m_ChargeLevel2Speed[n];
}

short cParamPlayerLevel::ChargeLevel2Weight(int n)
{
	CustomKingdomUtil::Assert(n < 11, "array error");
	return m_ChargeLevel2Weight[n];
}

short cParamPlayerLevel::ChargeLevel2PiyoTime(int n)
{
	CustomKingdomUtil::Assert(n < 11, "array error");
	return m_ChargeLevel2PiyoTime[n];
}

int cParamPlayerLevel::MaxLevel()
{
	return 11;
}

}
