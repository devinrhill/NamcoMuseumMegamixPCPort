#pragma once

#include "common.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"

// Galaga Remix (g01) stage select = job 178 (0x0b200000): kingdom::game::g01::cAppG01GalagaMenu (DOL vtable 0x80396100, 0x2c0 bytes),
// created by JobInit 0x800b0d98. Overrides only the ctor / dtor, FadeInEndCallBack, MainExec and TutorialOutCallBack; slot 19 (0x800af4f8,
// named "PlaySndSe" in the maps) is an empty callback, the real PlaySndSe is slot 25 (+0x64) = the shared 0x8005cfe8.
namespace kingdom::game::g01
{
class cTaskConsMenuMan;

class cAppG01GalagaMenu : public kingdom::game::cAppGameBaseMenu
{
public:
	cAppG01GalagaMenu();             // DOL 0x800aea18
	// DOL 0x800af4fc (dtor: DetachMenuArc): the native apps tear down in Free (as cAppG07MotosMenu)
	void Free() override;

	static void JobInit();           // DOL 0x800b0d98 (job 178): new cAppG01GalagaMenu -> RegistMainTask

	void FadeInEndCallBack() override;     // DOL 0x800af290
	void MainExec() override;              // DOL 0x800af2f4
	void TutorialOutCallBack() override;   // DOL 0x800af2ec
	// +0x299 (the base menu's bTutorial) is set for the fade out and cleared at its end (the tutorial slots are the base menu's)
	void FadeOutStartCallBack() override { bTutorial() = true; }   // DOL 0x800af4e8
	void FadeOutEndCallBack() override { bTutorial() = false; }    // DOL 0x800af4dc
	// DOL 0x8005cfe8 (vtable +0x64): plays the cue by name. UNVERIFIED simplification: the DOL compares the name with a few cues first
	// (r2-0x7414 / -0x7410 ...) and routes those through cKingdomSndHnd to the remotes; here every cue goes to the mixer.
	void PlaySndSe(const string& pSndName, int ctrl = 0) override;

private:
	// +0x1b8 cTaskScnRoot (4 cameras) and +0x1bc g01::cTaskEffect: NOT PORTED (no 3D / particles on this screen are drawn natively yet)
	bool m_bMulti{};                         // +0x2a0 GalagaSys->IsMulti()
	kingdom::cTaskCursor* m_pCursor[4]{};    // +0x2ac (kingdom::cTaskMenuCursor in the DOL)
	cTaskConsMenuMan* m_pConsMan{};          // +0x2bc
};
}
