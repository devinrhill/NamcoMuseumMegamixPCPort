#include "kingdom/game/g07/cParamAxisAttack.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::g07
{

void cParamAxisAttack::Init()
{
	m_bValid = false;
}

void cParamAxisAttack::Term()
{
}

void cParamAxisAttack::LoadValue()
{
	for (uint num = 1u; num < 4; num++)
	{
		for (uint num2 = 1u; num2 < 7; num2++)
		{
			uint num3 = num - 1;
			uint num4 = num2 - 1;
			CustomKingdomUtil::Assert(num3 < 3, "error");
			CustomKingdomUtil::Assert(num4 < 6, "error");
			float num5 = F32((int)num2, (int)num, 0.0f);
			m_aaAxisParam[num3][num4] = num5;
		}
	}
	m_bValid = true;
}

cParamAxisAttack::cParamAxisAttack()
	
{
	Init();
}

cParamAxisAttack::cParamAxisAttack(string key)
	
{
	Init();
	RegistData(key);
}

cParamAxisAttack::~cParamAxisAttack()
{
	Term();
}

bool cParamAxisAttack::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	LoadValue();
	return result;
}

float* cParamAxisAttack::pAxisAttackLevel(uint level)
{
	return m_aaAxisParam[level];
}

float cParamAxisAttack::pAxisAttackLevelFacter(uint level, uint facter)
{
	return m_aaAxisParam[level][facter];
}

}
