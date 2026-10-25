#include "kingdom/game/g07/cParamBG.hpp"

#include "CustomKingdomUtil.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

void cParamBG::Init()
{
	for (uint num = 0u; num < 4; num++)
	{
		::Constants::jetGeoVecUnit(m_Param[num].vTrans);
		::Constants::jetGeoVecUnit(m_Param[num].vRotate);
		::Constants::jetGeoVecOne(m_Param[num].vScale);
		m_Param[num].fFrameRate = 0.0f;
	}
	m_bValid = false;
}

void cParamBG::Term()
{
}

void cParamBG::LoadValue()
{
	CustomKingdomUtil::Assert(5 <= nRow(), "csv error");
	CustomKingdomUtil::Assert(10 <= nCol(), "csv error");
	for (int i = 0; i < 4; i++)
	{
		m_Param[i].vTrans[0] = F32(1 + i, 0, 0.0f);
		m_Param[i].vTrans[1] = F32(1 + i, 1, 0.0f);
		m_Param[i].vTrans[2] = F32(1 + i, 2, 0.0f);
		m_Param[i].vRotate[0] = ::Constants::JET_DEG2RAD(F32(1 + i, 3, 0.0f));
		m_Param[i].vRotate[1] = ::Constants::JET_DEG2RAD(F32(1 + i, 4, 0.0f));
		m_Param[i].vRotate[2] = ::Constants::JET_DEG2RAD(F32(1 + i, 5, 0.0f));
		m_Param[i].vScale[0] = F32(1 + i, 6, 0.0f);
		m_Param[i].vScale[1] = F32(1 + i, 7, 0.0f);
		m_Param[i].vScale[2] = F32(1 + i, 8, 0.0f);
		m_Param[i].fFrameRate = F32(1 + i, 9, 1.0f);
	}
	m_bValid = true;
}

cParamBG::cParamBG()
{
	Init();
}

cParamBG::cParamBG(string key)
{
	Init();
	RegistData(key);
}

cParamBG::~cParamBG()
{
	Term();
}

bool cParamBG::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	LoadValue();
	return result;
}

Vector4 cParamBG::vGetTrans(uint n)
{
	CustomKingdomUtil::Assert(n < 4, "idx error");
	return m_Param[n].vTrans;
}

Vector4 cParamBG::vGetRotate(uint n)
{
	CustomKingdomUtil::Assert(n < 4, "idx error");
	return m_Param[n].vRotate;
}

Vector4 cParamBG::vGetScale(uint n)
{
	CustomKingdomUtil::Assert(n < 4, "idx error");
	return m_Param[n].vScale;
}

float cParamBG::fGetFrameRate(uint n)
{
	CustomKingdomUtil::Assert(n < 4, "idx error");
	return m_Param[n].fFrameRate;
}

}
