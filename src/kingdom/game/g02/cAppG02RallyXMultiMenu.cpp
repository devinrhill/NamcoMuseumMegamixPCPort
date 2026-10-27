#include "kingdom/game/g02/cAppG02RallyXMultiMenu.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "KingdomGlobal.hpp"
#include "WiiSnd.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomModelBase.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g02/cAppG02RallyXMenu.hpp"
#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cTaskConsMenuMan.hpp"
#include "kingdom/game/g02/cTaskConsMultiMenuConsoles.hpp"
#include "kingdom/game/g02/cTaskConsMultiMenuMan.hpp"

// Rally-X Remix (g02) multiplayer menu app, ported 1:1 from main.dol (no C# exists for Rally-X).
namespace kingdom::game::g02
{

// DOL 0x800c6640 (job 200, 0x0c800000)
void cAppG02RallyXMultiMenu::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG02RallyXMultiMenu());
}

// DOL 0x80122254. The player count and pads come from the matching block (0x800c5e3c), or from the connected pads when entered from the
// debug menu (0x0c100000; 0x800c5e90).
cAppG02RallyXMultiMenu::cAppG02RallyXMultiMenu()
{
	SetName("cAppG02RallyXMultiMenu");
	cG02RallyXSysExt::AttachMenuArc();
	kingdom::cKingdomModelBase::ClearAssetCache();   // NATIVE (as cAppG02RallyXMenu): the archives changed
	if (KingdomGlobal::SysCore->GetLastJobNo() == job::RALLYX_DEBUG_MENU)
		cG02RallyXSysExt::DebugMatching();
	else
		cG02RallyXSysExt::ApplyMatching();
	// +0x1b8 cTaskScnRoot (fog 0x80, 0x80, 0x80, 0x10): natively the native scene pass
	kingdom::NativeScene::Reset();
	ConnectChild(new kingdom::cTaskNativeScene());
	m_pConsMan = new cTaskConsMultiMenuMan(this);   // +0x2b4 (0x2ac bytes)
	ConnectChild(m_pConsMan);
	// +0x2a4.. four kingdom::cTaskMenuCursor (0x1e4 bytes): pad i, COMMON_ANITA_POINTER, A / B / Z press anim; the pairs {0, 1} {2, 3} {4, 5} {6, 7}
	// (table 0x80378868) are the pointer colours of the four players. Only the players' cursors show, on their pad.
	for (int i = 0; i < 4; i++)
	{
		m_pCursor[i] = new kingdom::cTaskCursor();
		m_pCursor[i]->nDrawPriority(0x80000000u);
		m_pCursor[i]->Init(i, "COMMON_ANITA_POINTER", i * 2);
		m_pCursor[i]->AddButton(0x860u, i * 2 + 1);
		ConnectChild(m_pCursor[i]);
		m_pCursor[i]->nDrawPriority(0x8000000du);
		if (i < cG02RallyXSysExt::nPlayer)
		{
			m_pCursor[i]->SetPadNo(cG02RallyXSysExt::nPadNo[i]);   // vtable +0x58, sys 0x800c5b98
			m_pCursor[i]->SetEnable(true);
		}
		else
		{
			m_pCursor[i]->SetEnable(false);
		}
	}
	m_pMotionTutorial = new cTaskConsMultiMenuMotionTutorial(m_pCursor[0]);   // +0x2bc
	m_pMotionTutorial->nDrawPriority(0x8000000cu);
	ConnectChild(m_pMotionTutorial);
	m_pCtlTutorial = new cTaskConsMultiMenuCtlTutorial(m_pCursor[0]);         // +0x2c0
	m_pCtlTutorial->nDrawPriority(0x8000000cu);
	ConnectChild(m_pCtlTutorial);
	// the BGM, unless coming back from the matching (0x0c700000)
	if (KingdomGlobal::SysCore->GetLastJobNo() != job::RALLYX_MATCHING) WiiSnd::Play(g_RallyXMenuBgm, "m02bgm_Select");
	SetupMenuScene();   // the light manager (+0x2b8: MENU_LIGHT_BG set 0, MENU_LIGHT_MODEL set 1) and the scene animation (+0x2a0)
}

void cAppG02RallyXMultiMenu::Free()
{
	cG02RallyXSysExt::DetachMenuArc();
	if (IsSndAllStop())   // NATIVE stand-in for the base menu's global BGM fade (see g_RallyXMenuBgm)
	{
		WiiSnd::FadeOut(g_RallyXMenuBgm, 500);
		WiiSnd::Detach(g_RallyXMenuBgm);
	}
	kingdom::NativeScene::Reset();
	cAppGameBaseMenu::Free();
}

// DOL 0x801227c0: every console appears
void cAppG02RallyXMultiMenu::FadeInEndCallBack()
{
	m_pConsMan->AppearStart();
}

// DOL 0x801227c8
void cAppG02RallyXMultiMenu::TutorialOutCallBack()
{
	m_pConsMan->Reset();
}

// DOL 0x8012295c: type 1 (remote only) -> the motion pages, 2 (Nunchuk) -> the control pages (into the base menu's tutorial +0x29c)
void cAppG02RallyXMultiMenu::SelectTutorial()
{
	const int type = m_pConsMan->nCtrlType();
	if (type == 1)
		pTaskTutorial() = m_pMotionTutorial;
	else if (type == 2)
		pTaskTutorial() = m_pCtlTutorial;
}

void cAppG02RallyXMultiMenu::PlaySndSe(const string& pSndName, int ctrl)
{
	(void)ctrl;   // UNVERIFIED: the DOL routes the cue to the remote of player ctrl (as in cAppG02RallyXMenu)
	snd::Play(pSndName.c_str());
}

// DOL 0x801227d0. Mode 0: wait until every console has appeared, then all wait; mode 1: the selection (Back -> the hub 0x0cf00000, START ->
// the settings into the sys and the Nunchuk card 0x0c900000, Single -> the stage select 0x0c200000, Rule -> the HOW TO PLAY pages).
void cAppG02RallyXMultiMenu::MainExec()
{
	const uint n = 0u;
	if (nModeState(n) == 0)
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
			nModeStep(n) = 1u;
			[[fallthrough]];
		case 1u:
			if (m_pConsMan->IsAppearEnd())
			{
				m_pConsMan->AllWaitState();
				NextMode(n, 1u);   // DOL: mode 1 + memset(+0x44, 0, 0x20)
			}
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2u;
			[[fallthrough]];
		case 2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (nModeState(n) == 1)
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
			nModeStep(n) = 1u;
			[[fallthrough]];
		case 1u:
			m_pConsMan->Select();
			if (m_pConsMan->IsBackDecide()) nSetNextJob(job::RALLYX_TERM);   // job 207: back to the hub
			if (m_pConsMan->IsStartDecide())
			{
				m_pConsMan->ApplySettings();
				nSetNextJob(job::RALLYX_NUNCHUK);   // job 201: the Nunchuk card, then the stage
			}
			if (m_pConsMan->IsRuleDecide())
			{
				SelectTutorial();
				bTutorial() = true;
			}
			if (m_pConsMan->IsSingleDecide()) nSetNextJob(job::RALLYX_MENU);   // job 194
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2u;
			[[fallthrough]];
		case 2u:
			nModeStep(n) = 0u;
			break;
		}
	}
}

}
