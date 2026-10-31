#include "kingdom/game/g03/cHitChar.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

void cHitChar::SetOfsPos(Vector4 vPos)
{
	::Constants::jetGeoVecCopy(m_vOfsPos, vPos);
}

// DOL 0x8014ae4c [audited: per AUDIT_G03_G07 row] (+0x10 = x y z 1.0)
void cHitChar::SetOfsPos(float x, float y, float z)
{
	::Constants::jetGeoVecSet(m_vOfsPos, x, y, z, 1.0f);
}

void cHitChar::SetBoundCenter(Vector4 vPos)
{
	::Constants::jetGeoVecCopy(m_vBoundCenter, vPos);
}

// DOL 0x8014ae34 [audited: per AUDIT_G03_G07 row] (+0x30 = x y z 1.0)
void cHitChar::SetBoundCenter(float x, float y, float z)
{
	::Constants::jetGeoVecSet(m_vBoundCenter, x, y, z, 1.0f);
}

void cHitChar::SetRange(float fRange)
{
	m_fOfsRange = fRange;
}

uint cHitChar::FlagTest(uint nFlag)
{
	return m_nFlag & nFlag;
}

bool cHitChar::bFlagTest(uint nFlag)
{
	return CustomKingdomUtil::IF(m_nFlag & nFlag);
}

bool cHitChar::FlagAccod(uint nFlag)
{
	if ((m_nFlag & nFlag) != nFlag)
	{
		return false;
	}
	return true;
}

void cHitChar::FlagOn(uint nFlag)
{
	m_nFlag |= nFlag;
}

void cHitChar::FlagOff(uint nFlag)
{
	m_nFlag &= ~nFlag;
}

uint cHitChar::nSide()
{
	return m_nSide;
}

cTaskChar* cHitChar::pParent()
{
	return m_pParent;
}

Vector4 cHitChar::vOfsPos()
{
	return m_vOfsPos;
}

float cHitChar::fOfsRange()
{
	return m_fOfsRange;
}

Vector4 cHitChar::vPos()
{
	return m_vPos;
}

Vector4 cHitChar::vBoundCenter()
{
	return m_vBoundCenter;
}

float cHitChar::fRange2()
{
	return m_fRange;
}

cHitChar::cHitChar(cTaskChar* pParent, int nSide, int nFlag)
{
	m_pParent = pParent;
	m_nSide = (uint)nSide;
	m_nFlag = (uint)nFlag;
	::Constants::jetGeoVecSet(m_vOfsPos, 0.0f, 0.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecSet(m_vPos, 0.0f, 0.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecSet(m_vBoundCenter, 0.0f, 0.0f, 0.0f, 1.0f);
	m_fOfsRange = 0.38f;
	m_fRange = 0.1444f;
}

// DOL 0x8014acf0 [audited: per AUDIT_G03_G07 row]
cHitChar::~cHitChar()
{
}

// DOL 0x8014ad30 [audited: per AUDIT_G03_G07 row] (pos = 4x4 matrix * offset position incl. w, range^2 = offsetRange^2)
void cHitChar::UpdateMatrix(const Matrix4x4& m44)
{
	::Constants::jetGeoVecTrans(m_vPos, m44, m_vOfsPos);
	m_fRange = m_fOfsRange * m_fOfsRange;
}

}
