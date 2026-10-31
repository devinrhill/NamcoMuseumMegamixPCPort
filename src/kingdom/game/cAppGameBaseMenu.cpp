#include "GamePad.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"

#include <limits>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/game/menu/cTaskConsMenuTutorialBase.hpp"
#include "KingdomGlobal.hpp"
#include <SoundManager.hpp>

namespace kingdom::game
{

cTaskFade*& cAppGameBaseMenu::pTaskFade()
{
	return m_pTaskFade;
}

int& cAppGameBaseMenu::nStageNo()
{
	return m_nStageNo;
}

int& cAppGameBaseMenu::nWorldNo()
{
	return m_nWorldNo;
}

bool& cAppGameBaseMenu::bTutorial()
{
	return m_bTutorial;
}

kingdom::game::menu::cTaskConsMenuTutorialBase*& cAppGameBaseMenu::pTaskTutorial()
{
	return m_pTaskTutorial;
}

cAppGameBaseMenu::cAppGameBaseMenu()
{
	Init();
}

void cAppGameBaseMenu::Free()
{
	Term();
	cAppKingdom::Free();
}

void cAppGameBaseMenu::nSetNextJob(uint nNo)
{
	m_nNextJob = nNo;
}

void cAppGameBaseMenu::MainInCallBack()
{
}

void cAppGameBaseMenu::MainOutCallBack()
{
}

void cAppGameBaseMenu::TutorialOutCallBack()
{
}

void cAppGameBaseMenu::PlaySndSe(const string& pSndName, int ctrl)
{
}

void cAppGameBaseMenu::TutorialStart()
{
	m_bTutorial = true;
}

void cAppGameBaseMenu::TutorialEnd()
{
	m_bTutorial = false;
}

void cAppGameBaseMenu::Init()
{
	pTaskFade() = new cTaskFade();
	CustomKingdomUtil::Assert(pTaskFade() != nullptr, "null pointer");
	pTaskFade()->Fade(2u);
	ConnectChild(pTaskFade());
	m_nState = 0;
	m_nStep = 0;
	m_nNextJob = std::numeric_limits<uint>::max();
	m_nWorldNo = 0;
	m_nStageNo = 0;
	SetSndAllStop(true);
	m_bTutorial = false;
	pTaskTutorial() = nullptr;
	GamePad::SetStickDpad(true);   // 0x801434f8 starts with 0x800219d4(1)
}

// DOL 0x801436ec [audited] (the menus' dtors): the stick no longer doubles as the dpad
void cAppGameBaseMenu::Term()
{
	GamePad::SetStickDpad(false);
}

// DOL 0x801436f4 [audited] (the tutorial's +0x5c)
void cAppGameBaseMenu::TutorialInCallBack()
{
	pTaskTutorial()->Start();
}

// DOL 0x80143708 [audited] (the tutorial's +0x58, then +0x60 ends it: +0x299 = 0)
void cAppGameBaseMenu::TutorialExec()
{
	pTaskTutorial()->Select();
	if (pTaskTutorial()->IsDeleteEnd())
	{
		m_bTutorial = false;
	}
}

// DOL 0x80143768
bool cAppGameBaseMenu::Execute(cJetTask* pParam)
{
	HomeButtonCheck();
	if (KingdomGlobal::KingSys->pAppMain()->IsPause())
	{
		return true;
	}
	int nState = m_nState;
	switch (m_nState)
	{
	case 0:
		{
			int nStep = m_nStep;
			if ((uint)nStep <= 1u)
			{
				if (m_nStep == 0)
				{
					m_nState = 0;
					FadeInStartCallBack();
					m_nStep++;
				}
				if (pTaskFade()->IsEnd())
				{
					m_nState = 1;
					m_nStep = 0;
				}
			}
			if (m_nState != nState)
			{
				FadeInEndCallBack();
			}
			break;
		}
	case 1:
		{
			int nStep = m_nStep;
			if ((uint)nStep <= 1u)
			{
				if (m_nStep == 0)
				{
					m_nState = 1;
					MainInCallBack();
					m_nStep++;
				}
				MainExec();
				if (m_bTutorial)
				{
					m_nState = 2;
					m_nStep = 0;
				}
				if (m_nNextJob != std::numeric_limits<uint>::max())
				{
					if (IsSndAllStop())
					{
						SoundManager::get_BGM()->Stop(0.5f); // DOL 0x8014390c [audited]: FUN_80025560(KingSys+0xa54, 500)
					}
					m_nState = 3;
					m_nStep = 0;
				}
			}
			if (m_nState != nState)
			{
				MainOutCallBack();
			}
			break;
		}
	case 2:
		{
			int nStep = m_nStep;
			if ((uint)nStep <= 1u)
			{
				if (m_nStep == 0)
				{
					m_nState = 2;
					TutorialInCallBack();
					m_nStep++;
				}
				TutorialExec();
				if (!m_bTutorial)
				{
					m_nState = 1;
					m_nStep = 0;
				}
			}
			if (m_nState != nState)
			{
				TutorialOutCallBack();
			}
			break;
		}
	case 3:
		{
			int nStep = m_nStep;
			if ((uint)nStep <= 1u)
			{
				if (m_nStep == 0)
				{
					m_nState = 3;
					pTaskFade()->Fade(65538u);
					FadeOutStartCallBack();
					m_nStep++;
				}
				if (pTaskFade()->IsEnd())
				{
					m_nState = 4;
					m_nStep = 0;
				}
			}
			if (m_nState != nState)
			{
				FadeOutEndCallBack();
				if (m_nNextJob == KingdomGlobal::SysCore->GetLastJobNo())
				{
					Constants::Kingdom_SetJobNo(m_nNextJob);
				}
				else
				{
					KingdomGlobal::SysCore->ForceSetJobNo(m_nNextJob);
				}
			}
			break;
		}
	}
	return true;
}

void cAppGameBaseMenu::Draw(int nStatus, cJetTask* pParam)
{
}

void cAppGameBaseMenu::FadeInStartCallBack()
{
}

void cAppGameBaseMenu::FadeInEndCallBack()
{
}

void cAppGameBaseMenu::FadeOutStartCallBack()
{
}

void cAppGameBaseMenu::FadeOutEndCallBack()
{
}

bool cAppGameBaseMenu::IsSndAllStop()
{
	return m_bAllStop;
}

void cAppGameBaseMenu::SetSndAllStop(bool sw)
{
	m_bAllStop = sw;
}

void cAppGameBaseMenu::HomeButtonCheck()
{
}

}
