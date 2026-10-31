#include "kingdom/game/g07/cAppG07MotosMenu.hpp"
#include "kingdom/KingdomJobTable.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

#include "kingdom/game/g07/cTaskConsMenuMan.hpp"
#include "kingdom/game/g07/cTaskConsMenuTutorial.hpp"
#include "KingdomGlobal.hpp"
#include "GamePad.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/cTaskCursor.hpp"
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g07
{

int& cAppG07MotosMenu::nWorldMax()
{
	return m_nWorldMax;
}

cTaskConsMenuMan*& cAppG07MotosMenu::pTaskConsMenuMan()
{
	return m_pTaskConsMenuMan;
}

cAppG07MotosMenu::cAppG07MotosMenu()
{
	Init();
}

void cAppG07MotosMenu::Free()
{
	pTaskConsMenuMan()->Free();
	Term();
	cAppGameBaseMenu::Free();
}

// DOL 0x801c490c [audited: per AUDIT_G03_G07 row] (ctor); dtor 0x801c4e64 = Term (detach g07menu.lzs only, no BGM fade)
void cAppG07MotosMenu::Init()
{
	if (KingdomGlobal::MotosSys->nStageIdx() == 0)
	{
		KingdomGlobal::MotosSys->nStageIdx() = 1u;
		KingdomGlobal::MotosSys->nStageIdxTmp() = 1u;
	}
	KingdomGlobal::MotosSys->AttachMenuData();
	// DOL 0x801c490c: the pointer sprite at +0x2a4 (pad 0, ANT_CURSOR of g07resident.lzs (crc 0x0416b4b4), idle anim 0, press anim 1 on trig & 0x860,
	// priority 0x8000000b); the tutorial gets it
	m_pCursor = new kingdom::cTaskCursor();
	ConnectChild(m_pCursor);
	m_pCursor->nDrawPriority(0x8000000bu);
	m_pCursor->Init(0, "ANT_CURSOR", 0);
	m_pCursor->AddButton(0x860u, 1);
	ConnectChild(pTaskTutorial() = new cTaskConsMenuTutorial());
	pTaskTutorial()->SetCursor(m_pCursor);
	pTaskTutorial()->nDrawPriority(2147483658u);
	ConnectChild(pTaskConsMenuMan() = new cTaskConsMenuMan(this));
	// DOL 0x801c490c end: the menu BGM m07bgm_Select unless coming back from the matching screen (0x11600000). The C# started it from a Unity
	// coroutine (WaitPlayBGM), which never ran natively: the menu was silent.
	if (KingdomGlobal::SysCore->GetLastJobNo() != job::MOTOS_MATCHING)
	{
		SoundManager::get_BGM()->Stop(0.0f);
		SoundManager::get_BGM()->Play(EBGMKind::MOTOS_SND_BGM01_SELECT, 0.5f, true, false, true, 0.1f);
	}
}

IEnumerator* cAppG07MotosMenu::WaitPlayBGM()
{
	// TODO(unity): coroutine; original C# body:
	// 	yield return new WaitWhile(() => SceneChanger.IsExec);
	// 	yield return new WaitWhile(() => InGamePadManager.Disable);
	// 	if (!SceneChanger.IsExec)
	// 	{
	// 		SoundManager.BGM.Play(EBGMKind.MOTOS_SND_BGM01_SELECT, 0.5f, bLoop: true, requestStopActive: false, checkSameAudioPlaying: true, 0.1f);
	// 	}
	return nullptr;
}

void cAppG07MotosMenu::Term()
{
	KingdomGlobal::MotosSys->DetachMenuData();
}

// DOL 0x801c4c48 [audited: per AUDIT_G03_G07 row] (vtable +0x40): menu manager all-appear 0x801c8a0c, then PlaySndSe("NEWSTAGE") when MotosSys +0x239 is set.
// FadeInStart / FadeOutEnd are the base's blr in the DOL (the C# had Unity-UI resets there: removed)
void cAppG07MotosMenu::FadeInEndCallBack()
{
	pTaskConsMenuMan()->AppearStart();
	if (KingdomGlobal::MotosSys->IsNewStageApear())
	{
		SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE09_NEWSTAGE);
	}
}


// DOL 0x801c4ca4 [audited: per AUDIT_G03_G07 row] (vtable +0x60): menu manager 0x801c8348 (= Reset)
void cAppG07MotosMenu::TutorialOutCallBack()
{
	pTaskConsMenuMan()->Reset();
}

// DOL 0x801c4cac [audited: per AUDIT_G03_G07 row] (vtable +0x50)
void cAppG07MotosMenu::MainExec()
{
	uint n = 0u;
	if (GamePad::Get(0).hold == 0xffffffffu) // DOL 0x801c4cc4 [audited: described by the 43rd-session pass]: every bit held (disconnected pad word) -> job 273 (the C# dropped it)
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
			if (pTaskConsMenuMan()->IsAppearEnd())
			{
				pTaskConsMenuMan()->AllWaitState();
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
			pTaskConsMenuMan()->Select();
			if (pTaskConsMenuMan()->IsBackDecide())
			{
				nSetNextJob(kingdom::job::MOTOS_TERM);
			}
			if (pTaskConsMenuMan()->IsStageDecide())
			{
				pTaskConsMenuMan()->Uplode();
				nSetNextJob(job::MOTOS_NUNCHUK); // DOL 0x801c4dec [audited: described by the 43rd-session pass]: the Nunchuk card (job 280), which goes on to the game; the C# went straight to 0x11300000
			}
			if (pTaskConsMenuMan()->IsMultiDecide())
			{
				nSetNextJob(kingdom::job::MOTOS_MATCHING);
				SetSndAllStop(false);
			}
			if (pTaskConsMenuMan()->IsRuleDecide())
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
