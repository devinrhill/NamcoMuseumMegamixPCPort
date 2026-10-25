#include "kingdom/game/g03/cGhostCommonParam.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::g03
{

void cGhostCommonParam::Init()
{
	m_bValid = false;
}

void cGhostCommonParam::Term()
{
}

cGhostCommonParam::cGhostCommonParam()
{
	Init();
}

cGhostCommonParam::cGhostCommonParam(string key)
{
	Init();
	RegistData(key);
}

cGhostCommonParam::~cGhostCommonParam()
{
	Term();
}

bool cGhostCommonParam::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);   // the converted code called itself (overload lost)
	CustomKingdomUtil::Assert(result, "csv error");
	m_bValid = true;
	return result;
}

bool cGhostCommonParam::IsValid()
{
	return m_bValid;
}

float cGhostCommonParam::GetGhostCommonIzikeSpeed()
{
	return F32(1, 1, 0.0f);
}

int cGhostCommonParam::GetGhostCommonIzikeFlash()
{
	return S32(2, 1, 0);
}

float cGhostCommonParam::GetGhostBombDire()
{
	return F32(3, 1, 0.0f);
}

float cGhostCommonParam::GetGhostBombAttackArea()
{
	return F32(4, 1, 0.0f);
}

}
