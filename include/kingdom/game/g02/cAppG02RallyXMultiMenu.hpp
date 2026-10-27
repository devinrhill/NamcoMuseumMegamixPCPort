#pragma once

#include "common.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"

// Rally-X Remix (g02) multiplayer menu = job 200 (0x0c800000): kingdom::game::g02::cAppG02RallyXMultiMenu (DOL vtable 0x80440ef0, 0x2c4 bytes),
// created by the job init 0x800c6640 (term = ReleaseMainTask). Ported from main.dol (no C# exists for Rally-X).
// Overrides (vtable words): slot 16 FadeInEndCallBack 0x801227c0, slot 20 MainExec 0x801227d0, slot 24 TutorialOutCallBack 0x801227c8,
// slot 28 (a new virtual) SelectTutorial 0x8012295c; PlaySndSe 0x8005cfe8 and FadeOutStart / End 0x800af4e8 / 0x800af4dc are shared with the menus.
namespace kingdom::game::g02
{
class cTaskConsMultiMenuMan;
class cTaskConsMultiMenuMotionTutorial;
class cTaskConsMultiMenuCtlTutorial;

class cAppG02RallyXMultiMenu : public kingdom::game::cAppGameBaseMenu
{
public:
	cAppG02RallyXMultiMenu();        // DOL 0x80122254
	// DOL 0x801229b0 (dtor: the light manager, the scene animation, DetachMenuArc): the native apps tear down in Free
	void Free() override;

	static void JobInit();           // DOL 0x800c6640 (job 200): new cAppG02RallyXMultiMenu -> RegistMainTask

	void FadeInEndCallBack() override;     // DOL 0x801227c0
	void MainExec() override;              // DOL 0x801227d0
	void TutorialOutCallBack() override;   // DOL 0x801227c8
	virtual void SelectTutorial();         // DOL 0x8012295c: the HOW TO PLAY pages of player 0's control type
	void FadeOutStartCallBack() override { bTutorial() = true; }   // DOL 0x800af4e8
	void FadeOutEndCallBack() override { bTutorial() = false; }    // DOL 0x800af4dc
	void PlaySndSe(const string& pSndName, int ctrl = 0) override;   // DOL 0x8005cfe8

private:
	// +0x1b8 cTaskScnRoot (0x80, 0x80, 0x80, 0x10): natively cTaskNativeScene; +0x2a0 the scene animation, +0x2b8 the light manager: SetupMenuScene
	kingdom::cTaskCursor* m_pCursor[4]{};              // +0x2a4 (one per pad; only the players' are shown)
	cTaskConsMultiMenuMan* m_pConsMan{};               // +0x2b4
	cTaskConsMultiMenuMotionTutorial* m_pMotionTutorial{};   // +0x2bc control type 1
	cTaskConsMultiMenuCtlTutorial* m_pCtlTutorial{};         // +0x2c0 control type 2
};
}
