#include "kingdom/game/g11/cAppG11GrobdaMenu.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "KingdomGlobal.hpp"
#include "WiiSnd.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomModelBase.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"
#include "kingdom/game/g11/cTaskGrobdaConsMenu.hpp"

namespace kingdom::game::g11
{

namespace
{
// m11bgm_select (PTR 0x80379d78), played through a temporary cKingdomSndHnd that lets go at once: the BGM keeps playing into the next job.
// NATIVE: the voice is kept here so Free can fade it (as g02's menu)
WiiSnd::Handle s_Bgm;
}

// DOL 0x8000f164 [audited] (job 338)
void cAppG11GrobdaMenu::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG11GrobdaMenu());
}

// DOL 0x8014a518 [audited]
cAppG11GrobdaMenu::cAppG11GrobdaMenu()
{
	SetName("cAppG11GrobdaMenu");
	KingdomGlobal::KingSys->AttachArcData("g11_grobda/g11menu.lzs", true);   // DAT_80477fd8
	cG11GrobdaSys* sys = cG11GrobdaSys::Instance();
	if (!sys->IsMulti()) sys->ResetScores();   // FUN_8000ee28
	kingdom::cKingdomModelBase::ClearAssetCache();   // NATIVE: the archives changed
	// +0x1b8 cTaskScnRoot (0x80, 0x80, 0x80, 0x10): natively the native scene pass
	kingdom::NativeScene::Reset();
	ConnectChild(new kingdom::cTaskNativeScene());
	// one cTaskMenuCursor per pad (BIN_COMMON_ANT_CURSOR, crc 0x06ca4270; idle anim 2 i, press anim 2 i + 1 with A B Z)
	for (int i = 0; i < 4; i++)
	{
		if (i >= sys->nPlayer()) break;
		m_pCursor[i] = new kingdom::cTaskCursor();
		m_pCursor[i]->nDrawPriority(0x80000000u);
		m_pCursor[i]->Init(i, "BIN_COMMON_ANT_CURSOR", 2 * i);
		m_pCursor[i]->AddButton(0x860u, 2 * i + 1);
		ConnectChild(m_pCursor[i]);
		m_pCursor[i]->nDrawPriority(0x80000008u);
		m_pCursor[i]->SetPadNo(sys->nPad(i));   // vtable +0x58: the matching block's player +0x4c
		m_pCursor[i]->SetEnable(true);          // +0x1e1
	}
	// +0x29c the HOW TO PLAY pages (priority 0x80000007) on pad 0's cursor
	pTaskTutorial() = new cTaskConsMenuTutorial(m_pCursor[0]);
	pTaskTutorial()->nDrawPriority(0x80000007u);
	ConnectChild(pTaskTutorial());
	m_pConsMan = new cTaskConsMenuMan(this);   // +0x2b4
	ConnectChild(m_pConsMan);
	WiiSnd::Play(s_Bgm, "m11bgm_select");
}

// DOL 0x8014abc0 [audited]
void cAppG11GrobdaMenu::Free()
{
	// FUN_80025560(KingSys +0xa54, 500): every sound fades out, also on the way to the matching (not gated by IsSndAllStop)
	WiiSnd::FadeOut(s_Bgm, 500);
	WiiSnd::Detach(s_Bgm);
	KingdomGlobal::KingSys->DetachArcData("g11_grobda/g11menu.lzs");
	kingdom::NativeScene::Reset();
	cAppGameBaseMenu::Free();
}

// DOL 0x8014a9c4 [audited]: every console appears
void cAppG11GrobdaMenu::FadeInEndCallBack()
{
	m_pConsMan->AppearStart();
}

// DOL 0x8014a9cc [audited]: back from the HOW TO PLAY pages: every console waits, the focused one points
void cAppG11GrobdaMenu::TutorialOutCallBack()
{
	m_pConsMan->Reset();
	m_pConsMan->PointCurrent();
}

void cAppG11GrobdaMenu::PlaySndSe(const string& pSndName, int ctrl)
{
	(void)ctrl;
	snd::Play(pSndName.c_str());
}

// DOL 0x8014aa04 [audited]. Mode 0: wait until the man chooses (every console has appeared); mode 1: the man's selection decides the next job.
void cAppG11GrobdaMenu::MainExec()
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
			if (m_pConsMan->IsIdle()) NextMode(n, 1u);
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
			if (m_pConsMan->IsBackDecide()) nSetNextJob(job::GROBDA_TERM);   // job 351: back to the hub
			if (m_pConsMan->IsStartDecide()) nSetNextJob(job::GROBDA_NUNCHUK);  // job 344: the Nunchuk card, then the stage
			if (m_pConsMan->IsRuleDecide()) bTutorial() = true;
			if (m_pConsMan->IsMultiDecide())
			{
				if (!cG11GrobdaSys::Instance()->IsMulti())
				{
					SetSndAllStop(false);
					nSetNextJob(job::GROBDA_MATCHING);   // job 342: the matching (NOT PORTED)
				}
				else
				{
					cG11GrobdaSys::Instance()->nPlayer(1);  // FUN_8000ee50(sys, 1): leaving multiplayer
					cG11GrobdaSys::Instance()->nStage(0);   // sys +0x20 = 0
					nSetNextJob(job::GROBDA_MENU);               // back to this menu
				}
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
