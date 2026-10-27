#include "kingdom/game/g07/cParamBoss04.hpp"
#include "platform/devtools_g07edit.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"

namespace kingdom::game::g07
{

void cParamBoss04::Init()
{
	m_bValid = false;
}

void cParamBoss04::Term()
{
}

cParamBoss04::cParamBoss04()
{
	Init();
}

cParamBoss04::cParamBoss04(string key)
{
	Init();
	RegistData(key);
}

cParamBoss04::~cParamBoss04()
{
	Term();
}

bool cParamBoss04::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	return result;
}

float cParamBoss04::GetValueF32(int row, int col)
{
	if (row == 1) row += devtools::G07Level(); // devtools: dev window difficulty, rows NORMAL / EASY / HARD (not 1:1)
	return F32(row, col, 0.0f);
}

int cParamBoss04::GetValueS32(int row, int col)
{
	if (row == 1) row += devtools::G07Level(); // devtools: dev window difficulty, rows NORMAL / EASY / HARD (not 1:1)
	return S32(row, col, 0);
}

}
