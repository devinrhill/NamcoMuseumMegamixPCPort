#include "kingdom/game/cAppGameBaseGameOver.hpp"

#include <limits>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cKingdomAnitaVector.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/EFadeKind.hpp"
#include <ESEKind.hpp>
#include <InGamePadManager.hpp>
#include <SoundManager.hpp>

namespace kingdom::game
{

cKingdomAnita* cAppGameBaseGameOver::pAnita(uint n)
{
	return AnitaVector->pAnita(n);
}

cAppGameBaseGameOver::cAppGameBaseGameOver()
{
	Init();
}

void cAppGameBaseGameOver::Free()
{
	cAppKingdom::Free();
	Term();
}

uint& cAppGameBaseGameOver::nNextJob()
{
	return m_nNextJob;
}

uint& cAppGameBaseGameOver::nAnita()
{
	return m_nAnita;
}

void cAppGameBaseGameOver::Init()
{
	AnitaVector = new cKingdomAnitaVector();
	m_nAnita = 0u;
	m_nNextJob = std::numeric_limits<uint>::max();
	ConnectChild(m_pTaskFade = new cTaskFade());
}

void cAppGameBaseGameOver::Term()
{
}

bool cAppGameBaseGameOver::Execute(cJetTask* pParam)
{
	uint n = 0u;
	if (nModeState(n) == 0)
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch1_0u:
			nModeStep(n) = 1u;
			pAnita(m_nAnita)->SetState(2u, false);
			m_pTaskFade->Fade(EFadeKind::eFADE_TYPE_IN_BLACK);
			goto _switch1_1u;
		case 1u:
		_switch1_1u:
			if (m_pTaskFade->IsEnd())
			{
				NextMode(n, 1u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch1_2u;
		case 2u:
		_switch1_2u:
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
		_switch2_0u:
			nModeStep(n) = 1u;
			pAnita(m_nAnita)->SetState(2u);
			SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE04_GAMEOVER);
			goto _switch2_1u;
		case 1u:
		_switch2_1u:
			if (InGamePadManager::GetEnterKeyDown(InGamePadManager::GetPad1No()))
			{
				if (!SoundManager::get_SE()->IsPlayinSEKind(ESEKind::RESIDENT_SND_SE04_GAMEOVER))
				{
					SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE033_ITEM_);
					NextMode(n, 2u);
				}
			}
			else if (pAnita(m_nAnita)->IsState(327680u))
			{
				NextMode(n, 2u);
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
	else if (2 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch3_0u:
			nModeStep(n) = 1u;
			m_pTaskFade->Fade(EFadeKind::eFADE_TYPE_OUT_BLACK);
			SoundManager::get_SE()->Stop(0.2f);
			goto _switch3_1u;
		case 1u:
		_switch3_1u:
			if (m_pTaskFade->IsEnd())
			{
				CustomKingdomUtil::Assert(nNextJob() != std::numeric_limits<uint>::max(), "error");
				Constants::Kingdom_SetJobNo(nNextJob());
			}
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
	AnitaVector->UpdateFrame();
	return true;
}

void cAppGameBaseGameOver::Draw(int nStatus, cJetTask* pParam)
{
	cKingdomAnita::UpdateMatrix();
	AnitaVector->Draw();
}

}
