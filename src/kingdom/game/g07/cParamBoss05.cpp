#include "kingdom/game/g07/cParamBoss05.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"

namespace kingdom::game::g07
{

void cParamBoss05::Init()
{
	m_bValid = false;
}

void cParamBoss05::Term()
{
}

cParamBoss05::cParamBoss05()
{
	Init();
}

cParamBoss05::cParamBoss05(string key)
{
	Init();
	RegistData(key);
}

cParamBoss05::~cParamBoss05()
{
	Term();
}

bool cParamBoss05::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	return result;
}

float cParamBoss05::GetValueF32(int row, int col)
{
	return F32(row, col, 0.0f);
}

int cParamBoss05::GetValueS32(int row, int col)
{
	return S32(row, col, 0);
}

}
