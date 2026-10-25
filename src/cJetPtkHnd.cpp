#include "cJetPtkHnd.hpp"

#include "Constants.hpp"
#include "jet/ptk/cJetPtkEmi.hpp"

using namespace jet::ptk;

void cJetPtkHnd::Init()
{
	m_pEmi = nullptr;
	m_bUpdate = false;
	m_bWillUpdate = true;
	m_bInMtx = false;
	Constants::jetGeoVecUnit(m_vTrans);
	m_vRotate = Vector4::zero;
	Constants::jetGeoVecOne(m_vScale);
}

void cJetPtkHnd::Term()
{
	if (m_bWillUpdate)
	{
		UpdateMatrix();
	}
	if (m_pEmi != nullptr)
	{
		m_pEmi->DetachHnd();
	}
}

cJetPtkHnd::cJetPtkHnd()
{
	Init();
}

cJetPtkHnd::~cJetPtkHnd()
{
	Term();
}

void cJetPtkHnd::vSetTrans(Vector4 v0)
{
	m_bUpdate = true;
	v0[0] = 0.0f - v0[0];
	Constants::jetGeoVecCopy(m_vTrans, v0);
}

void cJetPtkHnd::vSetRotate(Vector4 v0)
{
	m_bUpdate = true;
	v0.y *= -1.0f;
	Constants::jetGeoVecCopy(m_vRotate, v0);
}

void cJetPtkHnd::vSetScale(Vector4 v0)
{
	m_bUpdate = true;
	Constants::jetGeoVecCopy(m_vScale, v0);
}

Vector4 cJetPtkHnd::vGetTrans()
{
	return m_vTrans;
}

Vector4 cJetPtkHnd::vGetRotate()
{
	return m_vRotate;
}

Vector4 cJetPtkHnd::vGetScale()
{
	return m_vScale;
}

bool cJetPtkHnd::UpdateMatrix()
{
	Matrix4x4 m = Matrix4x4::identity();
	m_bInMtx = false;
	return UpdateMatrix(m);
}

bool cJetPtkHnd::UpdateMatrix(const Matrix4x4& m0)
{
	if (m_bInMtx)
	{
		m_bUpdate = true;
	}
	m_bInMtx = true;
	if (m_pEmi != nullptr && m_bUpdate)
	{
		Matrix4x4 m1 = Matrix4x4::identity();
		Constants::jetGeoMtxTrans(m1, m1, m_vTrans);
		Constants::jetGeoMtxRotZYX(m1, m1, m_vRotate);
		Constants::jetGeoMtxScale(m1, m1, m_vScale);
		Constants::jetGeoMtxMult(m_pEmi->pWorldMtx(), m0, m1);
		m_bUpdate = false;
		return true;
	}
	return false;
}

bool cJetPtkHnd::IsActive()
{
	return pEmi()->IsActive();
}

void cJetPtkHnd::Active()
{
	pEmi()->bActive(true);
}

void cJetPtkHnd::Inactive()
{
	pEmi()->bActive(false);
}

bool cJetPtkHnd::IsVisible()
{
	return pEmi()->IsVisible();
}

void cJetPtkHnd::Visible()
{
	pEmi()->bVisible(true);
}

void cJetPtkHnd::Invisible()
{
	pEmi()->bVisible(false);
}

void cJetPtkHnd::nSetLightSetIdx(int idx)
{
	pEmi()->nLightSetIdx(idx);
}

int cJetPtkHnd::nGetLightSetIdx()
{
	return pEmi()->nLightSetIdx();
}

void cJetPtkHnd::nSetFogIdx(int idx)
{
	pEmi()->nFogIdx(idx);
}

int cJetPtkHnd::nGetFogIdx()
{
	return pEmi()->nFogIdx();
}

bool cJetPtkHnd::IsValid()
{
	return pEmi() != nullptr;
}

bool cJetPtkHnd::Detach(bool b)
{
	if (pEmi() == nullptr)
	{
		return false;
	}
	if (b)
	{
		pEmi()->bDeadReserve(true);
	}
	else
	{
		pEmi()->bWait(true);
	}
	pEmi()->DetachHnd();
	return true;
}

bool cJetPtkHnd::IsWillUpdate()
{
	return m_bWillUpdate;
}

bool cJetPtkHnd::IsUpdate()
{
	return m_bUpdate;
}

void cJetPtkHnd::pEmi(cJetPtkEmi* pPtkEmi)
{
	m_pEmi = pPtkEmi;
}

cJetPtkEmi* cJetPtkHnd::pEmi()
{
	return m_pEmi;
}
