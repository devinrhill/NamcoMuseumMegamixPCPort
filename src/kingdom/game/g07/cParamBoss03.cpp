#include "kingdom/game/g07/cParamBoss03.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"

namespace kingdom::game::g07
{

void cParamBoss03::Init()
{
	m_bValid = false;
}

void cParamBoss03::Term()
{
}

cParamBoss03::cParamBoss03()
{
	Init();
}

cParamBoss03::cParamBoss03(string key)
{
	Init();
	RegistData(key);
}

cParamBoss03::~cParamBoss03()
{
	Term();
}

bool cParamBoss03::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	return result;
}

float cParamBoss03::GetValueF32(int row, int col)
{
	return F32(row, col, 0.0f);
}

int cParamBoss03::GetValueS32(int row, int col)
{
	return S32(row, col, 0);
}

}
