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

namespace kingdom::game::g03
{

cTaskBgGate* cTaskGate::pCurrentGate()
{
	return m_pCurrentGate;
}

cTaskGate::cTaskGate()
{
	SetName("cTaskGate");
	Init();
}

void cTaskGate::Init()
{
	m_pCurrentGate = nullptr;
	m_vecGate.reserve(8);
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
			item->Term();
		}
	}
	if (m_pCurrentGate != nullptr)
	{
		m_pCurrentGate->Term();
	}
}

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

void cTaskGate::Draw(int nStatus, cJetTask* pParam)
{
}

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
