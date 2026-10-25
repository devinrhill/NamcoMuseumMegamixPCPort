#pragma once

#include "common.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"

// Rally-X Remix (g02) stage select = job 194 (0x0c200000): kingdom::game::g02::cAppG02RallyXMenu (DOL vtable 0x80440e48, 0x2b0 bytes),
// created by the job init 0x800c6504 (term 0x800c6638 = ReleaseMainTask). Ported from main.dol (no C# exists for Rally-X).
// Overrides (vtable words): slot 16 FadeInEndCallBack 0x80121f28, slot 20 MainExec 0x80121fe0, slot 24 TutorialOutCallBack 0x80121f84,
// slot 28 (a new virtual) SelectTutorial 0x80121f8c; FadeOutStart / End 0x800af4e8 / 0x800af4dc and PlaySndSe 0x8005cfe8 are shared with
// Galaga's menu; slot 19 0x800af4f8 is the shared empty callback.
namespace kingdom::game::g02
{
class cTaskConsMenuMan;
class cTaskConsMenuMotionTutorial;
class cTaskConsMenuCtlTutorial;

class cAppG02RallyXMenu : public kingdom::game::cAppGameBaseMenu
{
public:
	cAppG02RallyXMenu();             // DOL 0x80121bbc
	// DOL 0x8012219c (dtor: DetachMenuArc): the native apps tear down in Free (as cAppG01GalagaMenu)
	void Free() override;

	static void JobInit();           // DOL 0x800c6504 (job 194): new cAppG02RallyXMenu -> RegistMainTask

	void FadeInEndCallBack() override;     // DOL 0x80121f28
	void MainExec() override;              // DOL 0x80121fe0
	void TutorialOutCallBack() override;   // DOL 0x80121f84
	virtual void SelectTutorial();         // DOL 0x80121f8c: the HOW TO PLAY pages of the chosen control type
	void FadeOutStartCallBack() override { bTutorial() = true; }   // DOL 0x800af4e8
	void FadeOutEndCallBack() override { bTutorial() = false; }    // DOL 0x800af4dc
	// DOL 0x8005cfe8 (vtable +0x64): plays the cue by name. UNVERIFIED simplification (as cAppG01GalagaMenu): the DOL routes a few cues
	// through cKingdomSndHnd to the remotes; here every cue goes to the mixer.
	void PlaySndSe(const string& pSndName, int ctrl = 0) override;

private:
	// +0x1b8 cTaskScnRoot (0x80, 0x80, 0x80, 0x10; FUN_8009ac1c): natively cTaskNativeScene
	kingdom::cTaskCursor* m_pCursor{};                 // +0x2a0 (kingdom::cTaskMenuCursor in the DOL)
	cTaskConsMenuMan* m_pConsMan{};                    // +0x2a4
	cTaskConsMenuMotionTutorial* m_pMotionTutorial{};  // +0x2a8 control type 1
	cTaskConsMenuCtlTutorial* m_pCtlTutorial{};        // +0x2ac control type 2
};
}
