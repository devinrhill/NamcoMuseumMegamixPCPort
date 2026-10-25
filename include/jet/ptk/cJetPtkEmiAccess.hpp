#pragma once

#include "common.hpp"
#include "cJetPtkHnd.hpp"
#include "jet/ptk/cJetPtkData.hpp"
#include "jet/ptk/cJetPtkEmi.hpp"
#include "jet/ptk/cJetPtkGenAccess.hpp"
#include "jet/ptk/cJetPtkWork.hpp"

namespace jet::ptk
{

class cJetPtkEmiAccess
{
private:
	enum EFlag
	{
		eEMI_GEN_DEAD = 4,
		eEMI_PTC_DEAD = 2
	};

	cJetPtkEmi* m_pEmi{};

	cJetPtkWork* m_pPtkWork{};

	std::vector<cJetPtkEmi*>* m_pListMan{}; // non-owning, the list lives in cJetPtkMan

	cJetPtkGenAccess* m_pGenAces{};

public:
	cJetPtkEmiAccess();

	virtual ~cJetPtkEmiAccess();

	cJetPtkEmi* Generate(cJetPtkHnd* pHnd, cJetPtkData* pDat, bool bOneTime, bool bDepend, bool bClipping, float fDelay, int nLightSetIdx, int nFogIdx = -1, int drawlayer = 0, float loadFactor = -1.0f, uint nSlotNo = 0u);

private:
	void Init();

	float fGetLifeTime(cJetPtkData* pDat);

public:
	void Init(cJetPtkWork* pDat, std::vector<cJetPtkEmi*>& pList, cJetPtkGenAccess* pGenAces);
};

}
