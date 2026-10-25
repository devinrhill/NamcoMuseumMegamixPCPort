#include "kingdom/game/g07/cParamBoss01.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"

namespace kingdom::game::g07
{

void cParamBoss01::Init()
{
	m_bValid = false;
}

void cParamBoss01::Term()
{
}

cParamBoss01::cParamBoss01()
{
	Init();
}

cParamBoss01::cParamBoss01(string key)
{
	Init();
	RegistData(key);
}

cParamBoss01::~cParamBoss01()
{
	Term();
}

bool cParamBoss01::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	return result;
}

float cParamBoss01::GetValueF32(int row, int col)
{
	return F32(row, col, 0.0f);
}

int cParamBoss01::GetValueS32(int row, int col)
{
	return S32(row, col, 0);
}

}
