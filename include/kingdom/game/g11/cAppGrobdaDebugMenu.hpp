#pragma once

#include "kingdom/cAppDebugMenu.hpp"

// DOL kingdom::game::g11::cAppGrobdaDebugMenu (job 337 = 0x15100000; init 0x8000ef44, term 0x8000f0a8, vtable derived through cAppGameDebugMenu). Title "Grobda",
// 6 entries (table 0x803899a0): MENU 338, MODE 340, STAGE 340, GAMEOVER 341, STAFFROLL 350, EXIT 351. Execute 0x8000fcf8: on MODE (row 1) pad repeat
// right / left step the player count 1..4 (sys **(+4)); on STAGE (row 2) they toggle the stage index (sys +0x20, 0 / 1); then cAppGameDebugMenu::Execute.
// Draw 0x8000fc38: "           Player Num %d" at y 0x44 and "           Stage%02d" at y 0x58, x = 0x20 + strlen("MODE") * 0x14.
// DELIBERATE DEVIATIONS (as the Pac-N-Roll menu): left / right arrows (not the DOL's bit assignment under the Sideways orientation); STAGE (job 340, empty in
// retail) starts the chosen stage (job 0x15300000).
namespace kingdom::game::g11
{
class cAppGrobdaDebugMenu : public kingdom::cAppBaseDebugMenu
{
public:
	cAppGrobdaDebugMenu();
	bool Execute(cJetTask* pParam) override;             // DOL 0x8000fcf8
	void Draw(int nStatus, cJetTask* pParam) override;   // DOL 0x8000fc38
	static void JobInit();                               // DOL 0x8000ef44
};
}
