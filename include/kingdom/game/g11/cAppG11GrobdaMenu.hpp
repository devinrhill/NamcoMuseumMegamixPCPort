#pragma once

#include "common.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"

// Grobda Remix (g11) menu = job 338 (0x15200000): kingdom::game::g11::cAppG11GrobdaMenu (DOL vtable 0x80445248, 0x2b8 bytes, job init 0x8000f164,
// ctor 0x8014a518, dtor 0x8014abc0). Ported from main.dol (no C# exists for Grobda). Overrides (vtable words): slot 16 FadeInEndCallBack 0x8014a9c4,
// slot 20 MainExec 0x8014aa04, slot 24 TutorialOutCallBack 0x8014a9cc (as g02's menu). Single player only: the multiplayer branches are NOT PORTED.
namespace kingdom::game::g11
{
class cTaskConsMenuMan;
class cTaskConsMenuTutorial;

class cAppG11GrobdaMenu : public kingdom::game::cAppGameBaseMenu
{
public:
	cAppG11GrobdaMenu();             // DOL 0x8014a518
	void Free() override;            // DOL 0x8014abc0 (the dtor: BGM fade 500, DetachArcData g11menu.lzs)
	static void JobInit();           // DOL 0x8000f164

	void FadeInEndCallBack() override;     // DOL 0x8014a9c4
	void MainExec() override;              // DOL 0x8014aa04
	void TutorialOutCallBack() override;   // DOL 0x8014a9cc
	void FadeOutStartCallBack() override { bTutorial() = true; }   // DOL 0x800af4e8
	void FadeOutEndCallBack() override { bTutorial() = false; }    // DOL 0x800af4dc
	void PlaySndSe(const string& pSndName, int ctrl = 0) override;   // DOL 0x8005cfe8

private:
	kingdom::cTaskCursor* m_pCursor[4]{};      // +0x2a4 per pad (kingdom::cTaskMenuCursor)
	cTaskConsMenuMan* m_pConsMan{};            // +0x2b4
};
}
