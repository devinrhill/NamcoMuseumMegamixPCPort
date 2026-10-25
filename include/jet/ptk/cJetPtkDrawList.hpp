#pragma once

#include "common.hpp"
#include "jet/ptk/cJetPtkPtc.hpp"

namespace jet::ptk
{

class cJetPtkDrawList
{
private:
	cJetPtkPtc* m_pIte{};

	cJetPtkPtc* m_Top;

	cJetPtkPtc* m_End;

	uint m_nSize{};

	void Init();

	void Term();

public:
	cJetPtkDrawList();

	virtual ~cJetPtkDrawList();

	float CalcCamLength(Vector4 vPtcPos, Vector4 vCamPos);

	void SetUp(cJetPtkPtc* pDat, Vector4 vPos);

	void SetUp(cJetPtkPtc* pDat, Vector4 vPos, Matrix4x4 mtx);

	void SetDown(cJetPtkPtc* pDat, Vector4 vPos);

	void SetDown(cJetPtkPtc* pDat, Vector4 vPos, Matrix4x4 mtx);

	void Clear(cJetPtkPtc* pDat);

	void IteratorReset();

	cJetPtkPtc* GetIterator();

	uint nSize();

	void Release();
};

}
