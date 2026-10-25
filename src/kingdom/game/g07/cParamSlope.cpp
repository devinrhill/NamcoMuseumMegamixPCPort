#include "kingdom/game/g07/cParamSlope.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::g07
{

void cParamSlope::Init()
{
	m_bValid = false;
}

void cParamSlope::Term()
{
}

cParamSlope::cParamSlope()
{
	Init();
}

cParamSlope::cParamSlope(string key)
{
	Init();
	RegistData(key);
}

cParamSlope::~cParamSlope()
{
	Term();
}

bool cParamSlope::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	return result;
}

float cParamSlope::GetValueF32(int row, int col)
{
	return F32(row, col, 0.0f);
}

int cParamSlope::GetValueS32(int row, int col)
{
	return S32(row, col, 0);
}

int cParamSlope::GetRow()
{
	return nRow();
}

}
