#include "kingdom/game/menu/cAppGameBaseNunchuk.hpp"

#include "GamePad.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::menu
{

// DOL 0x80143aec [audited: scene root (+0x1b8) unused by the card, not ported]: next job unset, a camera work task (+0x1b8, unused by the card: not ported) and the fade (+0x298, priority 0x8fffffff)
cAppGameBaseNunchuk::cAppGameBaseNunchuk()
{
	m_pAnita = new cTaskAnita();
	m_pAnita->nDrawPriority(0x80000000u);   // 2D; the DOL draws its Anita vector in the app's Draw (0x80144138) with an ortho of the EFB size
	ConnectChild(m_pAnita);
	m_pFade = new cTaskFade();
	m_pFade->nDrawPriority(0x8fffffffu);
	ConnectChild(m_pFade);
}

// DOL 0x80144130 [audited]
bool cAppGameBaseNunchuk::IsSkip() { return true; }

// DOL 0x8014420c [audited]: load the layout into Anita 0 and clear its visible bit
void cAppGameBaseNunchuk::RegistLayout(const string& name)
{
	m_pAnita->pAnita(0u)->RegistData(ResourceDatabase::GetPath(name));
	m_pAnita->pAnita(0u)->SetState(1u, false);
}

// DOL 0x80143dac
bool cAppGameBaseNunchuk::Execute(cJetTask* pParam)
{
	const uint n = 0u;
	switch (nModeState(n))
	{
	case 0: // skip check
		nOldState(n) = 0;
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			NextMode(n, IsSkip() ? 4u : 1u);
		}
		if (nOldState(n) != nModeState(n)) nModeStep(n) = 0;
		break;
	case 1: // show the card (anim 1), fade in 500 ms
		nOldState(n) = 1;
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			m_pAnita->pAnita(0u)->SetState(1u);
			m_pAnita->pAnita(0u)->SetAnimNo(1);
			m_pFade->Fade(EFadeKind::eFADE_TYPE_IN_BLACK, 500);
		}
		if (m_pFade->IsEnd()) NextMode(n, 2u);
		if (nOldState(n) != nModeState(n)) nModeStep(n) = 0;
		break;
	case 2: // 3 seconds; HOME held on pad 0 stops the clock
		nOldState(n) = 2;
		if (nModeStep(n) == 0) nModeStep(n) = 1;
		if (GamePad::Get(0).hold & 0x400u) StopWatchStop(0u);
		else if (!StopWatchState(0u)) StopWatchStart(0u);
		if (StopWatchCheck(0u) > 3000u) NextMode(n, 3u);
		if (nOldState(n) != nModeState(n)) nModeStep(n) = 0;
		break;
	case 3: // fade out 500 ms
		nOldState(n) = 3;
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			m_pFade->Fade(EFadeKind::eFADE_TYPE_OUT_BLACK, 500);
		}
		if (m_pFade->IsEnd()) NextMode(n, 4u);
		if (nOldState(n) != nModeState(n)) nModeStep(n) = 0;
		break;
	case 4: // next job
		nOldState(n) = 4;
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			Constants::Kingdom_SetJobNo(m_nNextJob, false);
		}
		break;
	}
	return true;
}

}
