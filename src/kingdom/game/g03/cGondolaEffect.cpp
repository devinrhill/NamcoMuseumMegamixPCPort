#include "kingdom/game/g03/cGondolaEffect.hpp"

#include "CustomKingdomUtil.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

Vector4& cGondolaEffect::vTransL()
{
	return m_vTransL;
}

float cGondolaEffect::vTransL(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vTransL[n];
}

Vector4& cGondolaEffect::vRotateL()
{
	return m_vRotateL;
}

float cGondolaEffect::vRotateL(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vRotateL[n];
}

uint& cGondolaEffect::nType()
{
	return m_nType;
}

cGondolaEffect::cGondolaEffect()
{
	Init();
}

cGondolaEffect::~cGondolaEffect()
{
	Term();
}

void cGondolaEffect::Init()
{
	::Constants::jetGeoVecUnit(m_vTransL);
	::Constants::jetGeoVecUnit(m_vRotateL);
	nType() = 0u;
}

void cGondolaEffect::Term()
{
}

}
