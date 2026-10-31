#include "kingdom/game/g07/cAppG07MotosMultiMenu.hpp"
#include "kingdom/KingdomJobTable.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

#include "kingdom/game/g07/cTaskConsMultiMenuMan.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuTutorial.hpp"
#include "KingdomGlobal.hpp"
#include "GamePad.hpp"
#include "kingdom/KingdomJob.hpp"
#include <EBGMKind.hpp>
#include <SoundManager.hpp>

#include "kingdom/cTaskCursor.hpp"
#include <InGamePadManager.hpp>
namespace kingdom::game::g07
{

int& cAppG07MotosMultiMenu::nWorldMax()
{
	return m_nWorldMax;
}

cTaskConsMultiMenuMan*& cAppG07MotosMultiMenu::pTaskConsMultiMenuMan()
{
	return m_pTaskConsMultiMenuMan;
}

cAppG07MotosMultiMenu::cAppG07MotosMultiMenu()
{
	Init();
}

void cAppG07MotosMultiMenu::Free()
{
	Term();   // DOL dtor 0x801c54bc: detach g07menu.lzs, then the base menu's Term
	cAppGameBaseMenu::Free();
}

// DOL 0x801c4f1c [audited: per AUDIT_G03_G07 row] (ctor)
void cAppG07MotosMultiMenu::Init()
{
	KingdomGlobal::MotosSys->AttachMenuData();
	if (KingdomGlobal::MotosSys->nStageIdx() == 0)
	{
		KingdomGlobal::MotosSys->nStageIdx() = 1u;
		KingdomGlobal::MotosSys->nStageIdxTmp() = 1u;
	}
	ConnectChild(pTaskConsMultiMenuMan() = new cTaskConsMultiMenuMan(this));
	// DOL 0x801c4f1c: one pointer sprite per player at +0x2a4..+0x2b0 (pad i, ANT_CURSOR, idle anim 2i, press anim 2i + 1 on trig & 0x860 (table
	// 0x8037e460), priority 0x8000001a); the first nNum (matched players) follow that player's pad (slot 0x58) and are enabled (+0x1e1), the rest are off
	const sMatchingData* match = KingdomGlobal::MotosSys->pGetMatchingData();
	for (uint i = 0; i < 4; i++)
	{
		m_pCursor[i] = new kingdom::cTaskCursor();
		m_pCursor[i]->Init((int)i, "ANT_CURSOR", (int)(i * 2));
		m_pCursor[i]->AddButton(0x860u, (int)(i * 2 + 1));
		ConnectChild(m_pCursor[i]);
		m_pCursor[i]->nDrawPriority(0x8000001au);
		if (match != nullptr && i < match->nNum)
		{
			m_pCursor[i]->SetPadNo(InGamePadManager::GetPadNum((int)match->Info[i]->nCtrl));
			m_pCursor[i]->SetEnable(true);
		}
		else m_pCursor[i]->SetEnable(false);
	}
	ConnectChild(pTaskTutorial() = new cTaskConsMultiMenuTutorial());
	pTaskTutorial()->SetCursor(m_pCursor[0]);
	pTaskTutorial()->nDrawPriority(2147483673u);
	if (KingdomGlobal::SysCore->GetLastJobNo() != kingdom::job::MOTOS_MATCHING)
	{
		SoundManager::get_BGM()->Stop(0.0f);
		SoundManager::get_BGM()->Play(EBGMKind::MOTOS_SND_BGM01_SELECT);
	}
}

void cAppG07MotosMultiMenu::Term()
{
	KingdomGlobal::MotosSys->DetachMenuData();
}

// DOL 0x801c530c [audited: per AUDIT_G03_G07 row] (vtable +0x40): multi menu manager 0x801cd3c8 (all appear). FadeOutEnd = the base blr (C# Unity UI override removed)
void cAppG07MotosMultiMenu::FadeInEndCallBack()
{
	pTaskConsMultiMenuMan()->AppearStart();
}

// DOL 0x801c5314 [audited: per AUDIT_G03_G07 row] (vtable +0x60): multi menu manager 0x801cc99c (= Reset)
void cAppG07MotosMultiMenu::TutorialOutCallBack()
{
	pTaskConsMultiMenuMan()->Reset();
}

// DOL 0x801c531c [audited: per AUDIT_G03_G07 row] (vtable +0x50)
void cAppG07MotosMultiMenu::MainExec()
{
	uint n = 0u;
	if (GamePad::Get(0).hold == 0xffffffffu) // DOL 0x801c5334 [audited: described by the 43rd-session pass]: every bit held (disconnected pad word) -> job 273 (the C# dropped it)
	{
		kingdom::Kingdom_SetJobNo(job::MOTOS_DEBUG_MENU, false, false);
	}
	if (nModeState(n) == 0)
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch1_0u:
			nModeStep(n) = 1u;
			goto _switch1_1u;
		case 1u:
		_switch1_1u:
			if (pTaskConsMultiMenuMan()->IsAppearEnd())
			{
				pTaskConsMultiMenuMan()->AllWaitState();
				NextMode(n, 1u);
			}
			if (nOldState(n) != nModeState(n))
			{
				nModeStep(n) = 2u;
				goto _switch1_2u;
			}
			break;
		case 2u:
		_switch1_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else
	{
		if (1 != nModeState(n))
		{
			return;
		}
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch2_0u:
			nModeStep(n) = 1u;
			goto _switch2_1u;
		case 1u:
		_switch2_1u:
			pTaskConsMultiMenuMan()->Select();
			if (pTaskConsMultiMenuMan()->IsBackDecide())
			{
				nSetNextJob(kingdom::job::MOTOS_TERM);
			}
			if (pTaskConsMultiMenuMan()->IsStartDecide())
			{
				pTaskConsMultiMenuMan()->Upload();
				nSetNextJob(job::MOTOS_NUNCHUK); // DOL 0x801c5450 [audited: described by the 43rd-session pass]: the Nunchuk card (job 280), which goes on to the game; the C# went straight to 0x11300000
			}
			if (pTaskConsMultiMenuMan()->IsSingleDecide())
			{
				nSetNextJob(kingdom::job::MOTOS_MENU);   // DOL 0x801c5468 [audited: described by the 43rd-session pass]: no SetSndAllStop here (the C# kept the BGM)
			}
			if (pTaskConsMultiMenuMan()->IsRuleDecide())
			{
				bTutorial() = true;
			}
			if (nOldState(n) != nModeState(n))
			{
				nModeStep(n) = 2u;
				goto _switch2_2u;
			}
			break;
		case 2u:
		_switch2_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
}


}
