#include "kingdom/game/g01/cAppG01GalagaMenu.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "KingdomGlobal.hpp"
#include "WiiSnd.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskConsMenuMan.hpp"
#include "kingdom/game/g01/cTaskConsMenuTutorial.hpp"
#include <EBGMKind.hpp>
#include <SoundManager.hpp>

// Galaga Remix (g01) stage select app, ported 1:1 from main.dol (no C# exists for Galaga).
namespace kingdom::game::g01
{

namespace
{
// DOL 0x8036f398: per cursor {idle anim, press anim} of ANT_MENU_POINTER
const int s_CursorAnm[4][2] = {{0, 1}, {2, 3}, {4, 5}, {6, 7}};
}

// DOL 0x800b0d98 (job 178, 0x0b200000)
void cAppG01GalagaMenu::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG01GalagaMenu());
}

// DOL 0x800aea18
cAppG01GalagaMenu::cAppG01GalagaMenu()
{
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	m_bMulti = sys->IsMulti();
	// BGM m01bgm_Select unless coming back from the multiplayer menu (0x0b600000)
	if (KingdomGlobal::SysCore->GetLastJobNo() != job::GALAGA_MATCHING)
	{
		SoundManager::get_BGM()->Stop(0.0f);
		SoundManager::get_BGM()->Play(EBGMKind::GALAGA_SND_BGM01_SELECT, 1.0f, true);
	}
	cG01GalagaSys::AttachMenuArc();
	// +0x1b8 cTaskScnRoot (fog 0x80,0x80,0x80,0x10; 4 cameras: eye (0,0,100.3), fovy 30, aspect 4:3, near 0.1, far 32766, 640x480) and
	// +0x1bc g01::cTaskEffect (FUN_801c1ec0): not created natively. Nothing on this screen uses them: no menu code (0x800ae000..0x800b2000,
	// 0x800bb000..0x800c4000) calls the particle Generate 0x80021a9c, and the consoles are 2D layouts.
	for (int i = 0; i < 4; i++)
	{
		m_pCursor[i] = new kingdom::cTaskCursor();
		m_pCursor[i]->Init(i, "ANT_MENU_POINTER", s_CursorAnm[i][0]);
		m_pCursor[i]->AddButton(0x860u, s_CursorAnm[i][1]);
		m_pCursor[i]->nDrawPriority(0x80000006u);
		ConnectChild(m_pCursor[i]);
		if ((uint)i < (uint)sys->nPlayer())
		{
			m_pCursor[i]->SetPadNo(sys->nWork(i));
			m_pCursor[i]->SetEnable(true);
		}
		else
		{
			m_pCursor[i]->SetEnable(false);
		}
	}
	pTaskTutorial() = new cTaskConsMenuTutorial(m_bMulti, m_pCursor[0]);
	pTaskTutorial()->nDrawPriority(0x80000005u);
	ConnectChild(pTaskTutorial());
	m_pConsMan = new cTaskConsMenuMan(this, m_bMulti);
	ConnectChild(m_pConsMan);
}

void cAppG01GalagaMenu::Free()
{
	cG01GalagaSys::DetachMenuArc();
	cAppGameBaseMenu::Free();
}

// DOL 0x800af290
void cAppG01GalagaMenu::FadeInEndCallBack()
{
	m_pConsMan->AppearStart();
	if (KingdomGlobal::GalagaSys->bNewStageApear()) PlaySndSe("m00se_09Newstage", 0);
}

// DOL 0x800af2ec
void cAppG01GalagaMenu::TutorialOutCallBack()
{
	m_pConsMan->Reset();
}

void cAppG01GalagaMenu::PlaySndSe(const string& pSndName, int ctrl)
{
	(void)ctrl;
	snd::Play(pSndName.c_str());
}

// DOL 0x800af2f4. FUN_8000d844 (KingSys, -> Kingdom_SetJobNo(0x0bf00000)) always returns 0 in this build: left out.
void cAppG01GalagaMenu::MainExec()
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
			if (m_pConsMan->nState() == 0) NextMode(n, 1u);   // DOL: mode 1 + memset(+0x44, 0, 0x20)
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
			if (m_pConsMan->IsStageDecide())
			{
				KingdomGlobal::GalagaSys->nStage(3 * m_pConsMan->nWorldNo() + m_pConsMan->nStageNo() + 1);
				nSetNextJob(job::GALAGA_NUNCHUK);
			}
			if (m_pConsMan->IsRuleDecide()) bTutorial() = true;
			if (m_pConsMan->IsMultiDecide())
			{
				if (!m_bMulti)
				{
					nSetNextJob(job::GALAGA_MATCHING);
					SetSndAllStop(false);
				}
				else
				{
					KingdomGlobal::GalagaSys->nPlayer(1);
					nSetNextJob(job::GALAGA_MENU);
				}
			}
			if (m_pConsMan->IsBackDecide()) nSetNextJob(job::GALAGA_TERM);
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
