#include "kingdom/game/g07/cParamBoss05Last.hpp"
#include "platform/devtools_g07edit.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"

namespace kingdom::game::g07
{

void cParamBoss05Last::Init()
{
	m_bValid = false;
}

void cParamBoss05Last::Term()
{
}

cParamBoss05Last::cParamBoss05Last()
{
	Init();
}

cParamBoss05Last::cParamBoss05Last(string key)
{
	Init();
	RegistData(key);
}

cParamBoss05Last::~cParamBoss05Last()
{
	Term();
}

bool cParamBoss05Last::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	return result;
}

float cParamBoss05Last::GetValueF32(int row, int col)
{
	if (row == 1) row += devtools::G07Level(); // devtools: dev window difficulty, rows NORMAL / EASY / HARD (not 1:1)
	return F32(row, col, 0.0f);
}

int cParamBoss05Last::GetValueS32(int row, int col)
{
	if (row == 1) row += devtools::G07Level(); // devtools: dev window difficulty, rows NORMAL / EASY / HARD (not 1:1)
	return S32(row, col, 0);
}

}
