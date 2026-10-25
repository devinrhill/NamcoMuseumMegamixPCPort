#include "kingdom/game/g02/cAppG02RallyXMenu.hpp"

#include "KingdomGlobal.hpp"
#include "WiiSnd.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomModelBase.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cTaskConsMenuConsoles.hpp"
#include "kingdom/game/g02/cTaskConsMenuMan.hpp"

// Rally-X Remix (g02) stage select app, ported 1:1 from main.dol (no C# exists for Rally-X).
namespace kingdom::game::g02
{

namespace
{
// DOL 0x80378820: the cursor's {idle anim, press anim} (only the first pair is used: one cursor)
const int s_CursorAnm[2] = {0, 1};

// m02bgm_Select (PTR 0x8047bf8c), played through a temporary cKingdomSndHnd (FUN_80025988) that lets go at once: the BGM keeps playing
// into the matching job. NATIVE: the voice is kept here so Free can fade it like the global BGM fade of the base menu (0x8014390c
// FUN_80025560(KingSys +0xa54, 500), which natively only reaches SoundManager's BGM)
WiiSnd::Handle s_Bgm;
}

// DOL 0x800c6504 (job 194, 0x0c200000)
void cAppG02RallyXMenu::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG02RallyXMenu());
}

// DOL 0x80121bbc
cAppG02RallyXMenu::cAppG02RallyXMenu()
{
	SetName("cAppG02RallyXMenu");
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	// dead in the DOL: nWorld (0x800c5788) never returns >= 0x19; it would reset the stage index (0x800c5858(sys, 0, 0) -> 0)
	if (0x19 <= sys->nWorld()) sys->nStageIdx(cG02RallyXSysExt::StageIdxOf(0, 0));
	cG02RallyXSysExt::AttachMenuArc();
	kingdom::cKingdomModelBase::ClearAssetCache();   // NATIVE (as cAppG02RallyXGame): the archives changed
	// +0x1b8 cTaskScnRoot (fog 0x80, 0x80, 0x80, 0x10): natively the native scene pass
	kingdom::NativeScene::Reset();
	ConnectChild(new kingdom::cTaskNativeScene());
	// +0x2a0 kingdom::cTaskMenuCursor (0x1e4 bytes, ctor FUN_800951f4): pad 0, COMMON_ANITA_POINTER (crc 0x1a3f6a03), A / B / Z press anim
	m_pCursor = new kingdom::cTaskCursor();
	m_pCursor->nDrawPriority(0x80000000u);
	m_pCursor->Init(0, "COMMON_ANITA_POINTER", s_CursorAnm[0]);
	m_pCursor->AddButton(0x860u, s_CursorAnm[1]);
	ConnectChild(m_pCursor);
	m_pCursor->nDrawPriority(0x8000000bu);
	m_pConsMan = new cTaskConsMenuMan(this);   // +0x2a4 (0x264 bytes)
	ConnectChild(m_pConsMan);
	m_pMotionTutorial = new cTaskConsMenuMotionTutorial(m_pCursor);   // +0x2a8 (0x2f0 bytes)
	m_pMotionTutorial->nDrawPriority(0x8000000au);
	ConnectChild(m_pMotionTutorial);
	m_pCtlTutorial = new cTaskConsMenuCtlTutorial(m_pCursor);         // +0x2ac
	m_pCtlTutorial->nDrawPriority(0x8000000au);
	ConnectChild(m_pCtlTutorial);
	// the BGM, unless coming back from the matching (0x0c700000)
	if (KingdomGlobal::SysCore->GetLastJobNo() != 0x0c700000u) WiiSnd::Play(s_Bgm, "m02bgm_Select");
}

void cAppG02RallyXMenu::Free()
{
	cG02RallyXSysExt::DetachMenuArc();
	if (IsSndAllStop())   // NATIVE stand-in for the base menu's global BGM fade (see s_Bgm)
	{
		WiiSnd::FadeOut(s_Bgm, 500);
		WiiSnd::Detach(s_Bgm);
	}
	kingdom::NativeScene::Reset();
	cAppGameBaseMenu::Free();
}

// DOL 0x80121f28: every console appears; the "new stage" sound when a stage was just opened (sys 0x800c5c78)
void cAppG02RallyXMenu::FadeInEndCallBack()
{
	m_pConsMan->AppearStart();
	if (cG02RallyXSysExt::bNewStageApear) PlaySndSe("m00se_09Newstage", 0);   // PTR 0x8047bf88
}

// DOL 0x80121f84
void cAppG02RallyXMenu::TutorialOutCallBack()
{
	m_pConsMan->Reset();
}

// DOL 0x80121f8c: type 1 (remote only) -> the motion pages, 2 (Nunchuk) -> the control pages (into the base menu's tutorial +0x29c)
void cAppG02RallyXMenu::SelectTutorial()
{
	const int type = m_pConsMan->nCtrlType();
	if (type == 1)
		pTaskTutorial() = m_pMotionTutorial;
	else if (type == 2)
		pTaskTutorial() = m_pCtlTutorial;
}

void cAppG02RallyXMenu::PlaySndSe(const string& pSndName, int ctrl)
{
	(void)ctrl;
	snd::Play(pSndName.c_str());
}

// DOL 0x80121fe0. FUN_8000d844 (KingSys, -> Kingdom_SetJobNo(0x0cf00000)) always returns 0 in this build: left out (as in Galaga's menu).
// Mode 0: wait until every console has appeared, then all wait; mode 1: the selection.
void cAppG02RallyXMenu::MainExec()
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
			if (m_pConsMan->IsBackDecide()) nSetNextJob(0x0cf00000u);   // job 207: back to the hub
			if (m_pConsMan->IsStageDecide())
			{
				m_pConsMan->SetStage();
				nSetNextJob(0x0c900000u);   // job 201: the Nunchuk card, then the stage
			}
			if (m_pConsMan->IsRuleDecide())
			{
				SelectTutorial();
				bTutorial() = true;
			}
			if (m_pConsMan->IsMultiDecide())
			{
				SetSndAllStop(false);
				nSetNextJob(0x0c700000u);   // job 199: the matching (NOT PORTED)
			}
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
