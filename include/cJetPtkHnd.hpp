#pragma once

#include "common.hpp"

namespace jet::ptk
{
class cJetPtkEmi;
}

class cJetPtkHnd
{
private:
	jet::ptk::cJetPtkEmi* m_pEmi{};

	Vector4 m_vTrans{};

	Vector4 m_vRotate{};

	Vector4 m_vScale{};

	bool m_bUpdate{};

	bool m_bInMtx{};

	bool m_bWillUpdate{};

protected:
	virtual void Init();

	virtual void Term();

public:
	cJetPtkHnd();

	virtual ~cJetPtkHnd();

	void vSetTrans(Vector4 v0);

	void vSetRotate(Vector4 v0);

	void vSetScale(Vector4 v0);

	Vector4 vGetTrans();

	Vector4 vGetRotate();

	Vector4 vGetScale();

	bool UpdateMatrix();

	bool UpdateMatrix(const Matrix4x4& m0);

	bool IsActive();

	void Active();

	void Inactive();

	bool IsVisible();

	void Visible();

	void Invisible();

	void nSetLightSetIdx(int idx);

	int nGetLightSetIdx();

	void nSetFogIdx(int idx);

	int nGetFogIdx();

	bool IsValid();

	bool Detach(bool b);

	bool IsWillUpdate();

	bool IsUpdate();

	void pEmi(jet::ptk::cJetPtkEmi* pPtkEmi);

	jet::ptk::cJetPtkEmi* pEmi();
};
