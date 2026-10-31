#include "kingdom/game/g03/cTaskGate.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgGate.hpp"
#include "kingdom/game/g03/sSaveDataGate.hpp"
#include "kingdom/sKingdomData.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ReadFileUtil.hpp>
#include "platform/devtools.hpp"

namespace kingdom::game::g03
{

cTaskBgGate* cTaskGate::pCurrentGate()
{
	return m_pCurrentGate;
}

// DOL 0x8017a74c [audited: per AUDIT_G03_G07 row]
cTaskGate::cTaskGate()
{
	SetName("cTaskGate");
	Init();
}

// DOL 0x8017a93c [audited: per AUDIT_G03_G07 row] (reserve 8; BIN_GATE_SET_DAT 0x30-byte records or the retry gates; mode 0; current gate search inlined)
void cTaskGate::Init()
{
	m_pCurrentGate = nullptr;
	m_vecGate.reserve(8);
	if (devtools::G03NoGates()) { NextMode(0u, 0u); SearchCurrent(); return; } // devtools: --g03-no-gates (not 1:1)
	if (!KingdomGlobal::PacnRollSys->IsRetry())
	{
		sKingdomData* sKingdomData2 = KingdomGlobal::KingSys->SearchDataEx("BIN_GATE_SET_DAT");
		if (sKingdomData2 != nullptr && 0 < sKingdomData2->nSize)
		{
			std::vector<sSaveDataGate*> outParams;
			int outParamNum;
			int _discard1;
			ReadFileUtil::Convert<sSaveDataGate>(sKingdomData2->pBin, outParams, outParamNum, _discard1);
			for (uint num = 0u; (long)num < (long)outParamNum; num++)
			{
				cTaskBgGate* cTaskBgGate2 = new cTaskBgGate();
				CustomKingdomUtil::Assert(cTaskBgGate2 != nullptr, "nullpo");
				cTaskBgGate2->RegistData(*outParams[num]);
				m_vecGate.push_back(cTaskBgGate2);
				ConnectChild(cTaskBgGate2);
			}
		}
	}
	else
	{
		for (cG03PacnRollSys::sRetryGate* item : KingdomGlobal::PacnRollSys->pRetryData()->vecGate)
		{
			cTaskBgGate* cTaskBgGate3 = new cTaskBgGate();
			CustomKingdomUtil::Assert(cTaskBgGate3 != nullptr, "nullpo");
			cTaskBgGate3->RegistData(item->nKind, item->nNeedCookieNum, item->nPriorty, item->vTrans, item->vRotate);
			m_vecGate.push_back(cTaskBgGate3);
			ConnectChild(cTaskBgGate3);
		}
	}
	NextMode(0u, 0u);
	SearchCurrent();
}

void cTaskGate::Term()
{
	for (cTaskBgGate* item : m_vecGate)
	{
		if (item != nullptr)
		{
			item->TermOnce();
		}
	}
	if (m_pCurrentGate != nullptr)
	{
		m_pCurrentGate->TermOnce();
	}
}

// DOL 0x8017b420 [audited: per AUDIT_G03_G07 row] (mode 0: no current gate -> mode 1; a current gate whose need count is 0 -> search again; mode 1 idle)
bool cTaskGate::Execute(cJetTask* pParam)
{
	Constants::PACN_APP();
	uint n = 0u;
	if (nModeState(n) == 0)
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch2_0u:
			nModeStep(n) = 1u;
			goto _switch2_1u;
		case 1u:
		_switch2_1u:
			if (m_pCurrentGate != nullptr)
			{
				if (m_pCurrentGate->IsBreak())
				{
					SearchCurrent();
				}
			}
			else
			{
				NextMode(n, 1u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch2_2u;
		case 2u:
		_switch2_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (1 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch3_0u:
			nModeStep(n) = 1u;
			goto _switch3_1u;
		case 1u:
		_switch3_1u:
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch3_2u;
		case 2u:
		_switch3_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	return true;
}

// DOL 0x8017b5ac [audited: blr] (blr)
void cTaskGate::Draw(int nStatus, cJetTask* pParam)
{
}

// inlined in Init / Execute: the not-yet-opened gate (need != 0) with the smallest priority becomes current and wakes
void cTaskGate::SearchCurrent()
{
	uint num = 0u;
	m_pCurrentGate = nullptr;
	for (cTaskBgGate* item : m_vecGate)
	{
		if (!item->IsBreak() && (m_pCurrentGate == nullptr || num > item->GetPriority()))
		{
			m_pCurrentGate = item;
			num = item->GetPriority();
		}
	}
	if (m_pCurrentGate != nullptr)
	{
		m_pCurrentGate->IsWake() = true;
	}
}

// DOL 0x8017b5b0 [audited: per AUDIT_G03_G07 row]
void cTaskGate::SetRetryData()
{
	for (cTaskBgGate* item : m_vecGate)
	{
		cG03PacnRollSys::sRetryGate* sRetryGate = new cG03PacnRollSys::sRetryGate();
		sRetryGate->nKind = item->nKind();
		sRetryGate->nNeedCookieNum = item->nNeedCookie();
		sRetryGate->nPriorty = item->GetPriority();
		::Constants::jetGeoVecCopy(sRetryGate->vTrans, item->Model->vTrans());
		::Constants::jetGeoVecCopy(sRetryGate->vRotate, item->Model->vRotate());
		KingdomGlobal::PacnRollSys->pRetryData()->vecGate.push_back(sRetryGate);
	}
}

}
