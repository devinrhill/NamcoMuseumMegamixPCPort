#include "kingdom/game/g07/cParamCamera.hpp"

#include "CustomKingdomUtil.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

void cParamCamera::Init()
{
	::Constants::jetGeoVecUnit(m_vPosition);
	::Constants::jetGeoVecUnit(m_vTarget);
	m_fFovy = ::Constants::JET_DEG2RAD(60.0f);
	m_fNear = 1.0f;
	m_fFar = 10000.0f;
	m_bValid = false;
}

void cParamCamera::Term()
{
}

void cParamCamera::LoadValue()
{
	CustomKingdomUtil::Assert(2 <= nRow(), "csv error");
	CustomKingdomUtil::Assert(10 <= nCol(), "csv error");
	// DOL 0x801e23fc (g07 cTaskCamera::Setup, LoadValue inlined): x is stored as read; the C# negated it for the X-mirrored Unity import
	m_vPosition[0] = F32(1, 0, 0.0f);
	m_vPosition[1] = F32(1, 1, 0.0f);
	m_vPosition[2] = F32(1, 2, 0.0f);
	m_vTarget[0] = F32(1, 3, 0.0f);
	m_vTarget[1] = F32(1, 4, 0.0f);
	m_vTarget[2] = F32(1, 5, 0.0f);
	// error values: pass copies (an lvalue picks the `out` overload of F32, which returns bool -> every value became 1.0)
	m_fFovy = ::Constants::JET_DEG2RAD(F32(1, 6, float(m_fFovy)));
	m_fNear = F32(1, 7, float(m_fNear));
	m_fFar = F32(1, 8, float(m_fFar));
	m_fAspect = F32(1, 9, float(m_fAspect));
	m_bValid = true;
}

cParamCamera::cParamCamera()
{
	Init();
}

cParamCamera::cParamCamera(string key)
{
	Init();
	RegistData(key);
}

cParamCamera::~cParamCamera()
{
	Term();
}

bool cParamCamera::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	LoadValue();
	return result;
}

Vector4 cParamCamera::vGetPosition()
{
	return m_vPosition;
}

Vector4 cParamCamera::vGetTarget()
{
	return m_vTarget;
}

float cParamCamera::fGetFovy()
{
	return m_fFovy;
}

float cParamCamera::fGetNear()
{
	return m_fNear;
}

float cParamCamera::fGetFar()
{
	return m_fFar;
}

float cParamCamera::fGetAspect()
{
	return m_fAspect;
}

}
