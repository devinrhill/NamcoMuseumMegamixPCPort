#include "kingdom/game/g07/cParamDamage.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::g07
{

void cParamDamage::Init()
{
	m_bValid = false;
}

void cParamDamage::Term()
{
}

cParamDamage::cParamDamage()
{
	Init();
}

cParamDamage::cParamDamage(string key)
{
	Init();
	RegistData(key);
}

cParamDamage::~cParamDamage()
{
	Term();
}

bool cParamDamage::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	return result;
}

float cParamDamage::GetValueF32(int row, int col)
{
	return F32(row, col, 0.0f);
}

int cParamDamage::GetValueS32(int row, int col)
{
	return S32(row, col, 0);
}

}
