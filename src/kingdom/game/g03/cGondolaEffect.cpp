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

// DOL 0x801513a4 [audited: per AUDIT_G03_G07 row] (both vectors (0, 0, 0, 1) = jetGeoVecUnit, type 0)
void cGondolaEffect::Init()
{
	::Constants::jetGeoVecUnit(m_vTransL);
	::Constants::jetGeoVecUnit(m_vRotateL);
	nType() = 0u;
}

// DOL 0x801513d8 [audited: per AUDIT_G03_G07 row] (blr); destructor 0x80151310
void cGondolaEffect::Term()
{
}

}
