#include "kingdom/game/g07/cParamBoss02.hpp"
#include "platform/devtools_g07edit.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"

namespace kingdom::game::g07
{

void cParamBoss02::Init()
{
	m_bValid = false;
}

void cParamBoss02::Term()
{
}

cParamBoss02::cParamBoss02()
{
	Init();
}

cParamBoss02::cParamBoss02(string key)
{
	Init();
	RegistData(key);
}

cParamBoss02::~cParamBoss02()
{
	Term();
}

bool cParamBoss02::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	return result;
}

float cParamBoss02::GetValueF32(int row, int col)
{
	if (row == 1) row += devtools::G07Level(); // devtools: dev window difficulty, rows NORMAL / EASY / HARD (not 1:1)
	return F32(row, col, 0.0f);
}

int cParamBoss02::GetValueS32(int row, int col)
{
	if (row == 1) row += devtools::G07Level(); // devtools: dev window difficulty, rows NORMAL / EASY / HARD (not 1:1)
	return S32(row, col, 0);
}

}
