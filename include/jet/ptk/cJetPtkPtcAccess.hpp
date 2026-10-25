#pragma once

#include "common.hpp"
#include "jet/ptk/datGen.hpp"
#include "jet/ptk/datScn.hpp"
#include "jet/ptk/PtkTex.hpp"
#include "jet/ptk/cJetPtkEmi.hpp"
#include "jet/ptk/cJetPtkGen.hpp"
#include "jet/ptk/cJetPtkPtc.hpp"
#include "jet/ptk/cJetPtkWork.hpp"
#include "jet/ptk/wrkGen.hpp"
#include "jet/ptk/wrkPtc.hpp"

namespace jet::ptk
{

class cJetPtkPtcAccess
{
private:
	cJetPtkGen* m_pGen{};

	cJetPtkWork* m_pPtkWork{};

	std::vector<cJetPtkPtc*>* m_pListMan{}; // non-owning, the list lives in cJetPtkMan

public:
	bool IsVecNormalize(const Vector4& v);

private:
	void Init();

public:
	void Init(cJetPtkWork* pPtkWork, std::vector<cJetPtkPtc*>& pListMan);

	cJetPtkPtc* Generate(datScn pDatScn, cJetPtkEmi* pEmi, cJetPtkGen* pGen, PtkTex* pTex);

private:
	bool InitWork(const datGen& pDatGen, wrkPtc* pWrkPtc);

	void InitPos(const datGen& pDatGen, wrkGen* pWrkGen, wrkPtc* pWrkPtc);

public:
	cJetPtkPtcAccess();

	virtual ~cJetPtkPtcAccess();
};

}
