#pragma once

#include "common.hpp"
#include "jet/ptk/datGen.hpp"
#include "jet/ptk/PtkTex.hpp"
#include "jet/ptk/cJetPtkEmi.hpp"
#include "jet/ptk/cJetPtkGen.hpp"
#include "jet/ptk/cJetPtkPtcAccess.hpp"
#include "jet/ptk/cJetPtkWork.hpp"
#include "jet/ptk/wrkGen.hpp"

namespace jet::ptk
{

class cJetPtkGenAccess
{
private:
	cJetPtkWork* m_pPtkWork{};

	std::vector<cJetPtkGen*>* m_pListMan{}; // non-owning, the list lives in cJetPtkMan

	cJetPtkPtcAccess* m_pPtcAces{};

public:
	cJetPtkGenAccess();

	virtual ~cJetPtkGenAccess();

private:
	void Init();

public:
	void Init(cJetPtkWork* pPtkWork, std::vector<cJetPtkGen*>& pListMan, cJetPtkPtcAccess* pPtcAces);

	cJetPtkGen* Generate(const datGen& pDatGen, cJetPtkEmi* pEmi, PtkTex* pTex, float loadFactor);

	void InitWork(wrkGen* pWrkGen, const datGen& pDatGen);
};

}
