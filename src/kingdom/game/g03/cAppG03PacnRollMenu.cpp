#include "kingdom/game/g03/cAppG03PacnRollMenu.hpp"
#include <cstdlib>

#include "kingdom/game/g03/cTaskConsMenuMan.hpp"
#include "kingdom/game/g03/cTaskConsMenuTutorial.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include <ESEKind.hpp>
#include <SoundManager.hpp>
#include "GamePad.hpp"
#include "wii/Archive.hpp"
#include "kingdom/KingdomJob.hpp"

namespace kingdom::game::g03
{

int& cAppG03PacnRollMenu::nWorldMax()
{
	return m_nWorldMax;
}

cTaskConsMenuMan*& cAppG03PacnRollMenu::pTaskConsMenuMan()
{
	return m_pTaskConsMenuMan;
}

cAppG03PacnRollMenu::cAppG03PacnRollMenu()
{
	Init();
}

void cAppG03PacnRollMenu::Free()
{
	cAppGameBaseMenu::Free();
	Term();
}

// DOL 0x800fc2b8 [audited: per AUDIT_G03_G07 row] (ctor): AttachArcData(g03menu.lzs, 1); tasks at +0x1b8 (prio 0x10000) and +0x2a4 (prio 0x80000008, pad mask 0x860,
// probably the HBM home-menu task: not ported); tutorial (+0x29c, prio 0x80000007); cTaskConsMenuMan (+0x2a8, 0x8005bfdc); then plays
// the cue m03bgm_Select (handle detached).
void cAppG03PacnRollMenu::Init()
{
	wii::ArchiveStore::Attach(MENU_LZS);
	// DOL 0x800fc3f4 [audited: described by the 43rd-session pass]..0x800fc4b4: the pointer sprite (pad 0, BIN_COMMON_ANT_CURSOR, idle anim 0, press anim 1 on trig & 0x860, priority 0x80000008)
	m_pCursor = new kingdom::cTaskCursor();
	ConnectChild(m_pCursor);
	m_pCursor->nDrawPriority(2147483656u);
	m_pCursor->Init(0, "BIN_COMMON_ANT_CURSOR", 0);
	m_pCursor->AddButton(0x860u, 1);
	ConnectChild(pTaskTutorial() = new cTaskConsMenuTutorial());
	pTaskTutorial()->SetCursor(m_pCursor);
	pTaskTutorial()->nDrawPriority(2147483655u);
	ConnectChild(pTaskConsMenuMan() = new cTaskConsMenuMan(this));
	SoundManager::get_BGM()->Play(EBGMKind::PACN_ROLL_SND_BGM01_SELECT, 0.5f, true, false, true, 0.1f);
}

IEnumerator* cAppG03PacnRollMenu::WaitPlayBGM()
{
	return nullptr; // Unity coroutine, replaced by the direct Play in Init (the DOL plays the cue in the constructor)
}

// DOL 0x800fc7b0 [audited: per AUDIT_G03_G07 row] (dtor): BGM fade-out 500 ms (KingSys+0xa54), DetachArcData(g03menu.lzs)
void cAppG03PacnRollMenu::Term()
{
	SoundManager::get_BGM()->Stop(0.5f);
	wii::ArchiveStore::Detach(MENU_LZS);
}

// DOL 0x800fc5b8 [audited: per AUDIT_G03_G07 row] (vtable +0x40, called when the fade-in ends): menu manager AppearStart 0x8005d958, then PlaySndSe("NEWSTAGE") when sys +6 (IsOpenStage) is set.
// FadeInStart / FadeOutStart / FadeOutEnd are the base's blr in the DOL (the converted C# had Unity-UI resets here, always null natively: removed)
void cAppG03PacnRollMenu::FadeInEndCallBack()
{
	pTaskConsMenuMan()->AppearStart();
	if (KingdomGlobal::PacnRollSys->IsOpenStage())
	{
		SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE09_NEWSTAGE);
	}
}

// DOL 0x800fc614 [audited: per AUDIT_G03_G07 row] (vtable +0x60): menu manager 0x8005c4e4 (= Reset)
void cAppG03PacnRollMenu::TutorialOutCallBack()
{
	pTaskConsMenuMan()->Reset();
}

// DOL 0x800fc61c [audited: per AUDIT_G03_G07 row] (vtable +0x50; the class_funcs name TutorialStart is the g01 menu's slot name)
void cAppG03PacnRollMenu::MainExec()
{
	uint n = 0u;
	if (GamePad::Get(0).hold == 0xffffffffu) // DOL 0x800fc634 [audited: described by the 43rd-session pass]: every bit held (disconnected pad word) -> job 209
	{
		kingdom::Kingdom_SetJobNo(job::PACNROLL_DEBUG_MENU, false, false);
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
				nSetNextJob(job::PACNROLL_TERM);
			}
			if (pTaskConsMenuMan()->IsStageDecide())
			{
				pTaskConsMenuMan()->Uplode();
				nSetNextJob(job::PACNROLL_NUNCHUK); // DOL 0x800fc730 [audited: described by the 43rd-session pass]/75c: 0x0df00000 back, 0x0d800000 = cAppG03PacnRollNunchuk (C# went to 0x0d300000)
			}
			static bool s_envTutorial = std::getenv("MEGAMIX_TUTORIAL") != nullptr; // debug: the pointer click on the Rule button is not ported
			if (pTaskConsMenuMan()->IsRuleDecide() || s_envTutorial)
			{
				s_envTutorial = false;
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
