#pragma once

#include "common.hpp"
#include "jet/ptk/datGen.hpp"
#include "jet/ptk/PtkTex.hpp"
#include "jet/ptk/cJetPtkEmi.hpp"
#include "jet/ptk/wrkGen.hpp"

namespace jet::ptk
{
class cJetPtkPtcAccess;
}

namespace jet::ptk
{

class cJetPtkGen
{
private:
	enum EFlag
	{
		eGEN_USE = 1,
		eGEN_ACTIVE = 2
	};

	datGen m_pDatGen{};

	cJetPtkEmi* m_pEmi{};

	cJetPtkPtcAccess* m_pPtcAces{};

	PtkTex* m_pTex{};

	wrkGen* m_WrkGen;

	uint m_nFlags{};

	cJetPtkGen* m_pNext{};

	cJetPtkGen* m_pPrev{};

public:
	float OverrideLoadFactor = -1.0f;

	cJetPtkGen();

	virtual ~cJetPtkGen();

	void Init();

	void AnimUpdate(float fStep, float fLoadFactor);

	void bUse(bool b);

	bool IsUse();

	bool IsParentUse();

	bool IsActive();

	bool IsDelay();

	bool IsClippingVisible();

	bool IsBarth();

	void pDatGen(const datGen& pDat);

	datGen& pDatGen();

	void pEmi(cJetPtkEmi* pEmi);

	cJetPtkEmi* pEmi();

	void pPtcAces(cJetPtkPtcAccess* pPtcAces);

	cJetPtkPtcAccess* pPtcAces();

	void pTex(PtkTex* pTex);

	PtkTex* pTex();

	wrkGen* pWrkGen();

	void pNext(cJetPtkGen* pNext);

	cJetPtkGen* pNext();

	void pPrev(cJetPtkGen* pPrev);

	cJetPtkGen* pPrev();
};

}
