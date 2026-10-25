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

// DOL 0x800fc2b8 (ctor): AttachArcData(g03menu.lzs, 1); tasks at +0x1b8 (prio 0x10000) and +0x2a4 (prio 0x80000008, pad mask 0x860,
// probably the HBM home-menu task: not ported); tutorial (+0x29c, prio 0x80000007); cTaskConsMenuMan (+0x2a8, 0x8005bfdc); then plays
// the cue m03bgm_Select (handle detached).
void cAppG03PacnRollMenu::Init()
{
	wii::ArchiveStore::Attach(MENU_LZS);
	// DOL 0x800fc3f4..0x800fc4b4: the pointer sprite (pad 0, BIN_COMMON_ANT_CURSOR, idle anim 0, press anim 1 on trig & 0x860, priority 0x80000008)
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

// DOL 0x800fc7b0 (dtor): BGM fade-out 500 ms (KingSys+0xa54), DetachArcData(g03menu.lzs)
void cAppG03PacnRollMenu::Term()
{
	SoundManager::get_BGM()->Stop(0.5f);
	wii::ArchiveStore::Detach(MENU_LZS);
}

void cAppG03PacnRollMenu::FadeInEndCallBack()
{
	pTaskConsMenuMan()->AppearStart();
	if (KingdomGlobal::PacnRollSys->IsOpenStage())
	{
		SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE09_NEWSTAGE);
	}
}

void cAppG03PacnRollMenu::FadeInStartCallBack()
{
	if (pTaskConsMenuMan()->UnityUI != nullptr)
	{
		pTaskConsMenuMan()->UnityUI->ResetBaseAnim(true);
	}
	if (pTaskConsMenuMan()->UnityUI != nullptr)
	{
		pTaskConsMenuMan()->UnityUI->SetEnable(false);
	}
	if (pTaskTutorial()->UnityUIButton != nullptr)
	{
		pTaskTutorial()->UnityUIButton->SetEnable(false);
	}
	if (pTaskTutorial()->UnityUIImage != nullptr)
	{
		pTaskTutorial()->UnityUIImage->SetEnable(false);
	}
	if (pTaskTutorial()->UnityUIText != nullptr)
	{
		pTaskTutorial()->UnityUIText->SetEnable(false);
	}
}

void cAppG03PacnRollMenu::TutorialOutCallBack()
{
	pTaskConsMenuMan()->Reset();
}

void cAppG03PacnRollMenu::MainExec()
{
	uint n = 0u;
	if (GamePad::Get(0).hold == 0xffffffffu) // DOL 0x800fc634: every bit held (disconnected pad word) -> job 209
	{
		kingdom::Kingdom_SetJobNo(0x0d100000u, false, false);
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
				nSetNextJob(0x0df00000u);
			}
			if (pTaskConsMenuMan()->IsStageDecide())
			{
				pTaskConsMenuMan()->Uplode();
				nSetNextJob(0x0d800000u); // DOL 0x800fc730/75c: 0x0df00000 back, 0x0d800000 = cAppG03PacnRollNunchuk (C# went to 0x0d300000)
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

void cAppG03PacnRollMenu::FadeOutEndCallBack()
{
	if (pTaskConsMenuMan()->UnityUI != nullptr)
	{
		pTaskConsMenuMan()->UnityUI->SetEnable(false);
	}
}

}
