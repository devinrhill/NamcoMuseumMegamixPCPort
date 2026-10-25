#pragma once

#include "kingdom/cAppDebugMenu.hpp"

namespace kingdom::game::g03
{

// DOL kingdom::game::g03::cAppPacnRollDebugMenu (job 209 = 0x0d100000; init 0x8012b008, vtable 0x80441528, derived through
// cAppGameDebugMenu 0x80394410). Title "Pacn Roll" (0x80378f8c), 8 entries (table 0x804414e8). Reached from the Kingdom debug menu's
// jobs only through --job 0x0d100000 in this port (the top menu's G03 entry is job 208 like in the DOL).
// Execute 0x8012bb78: while the cursor is on STAGE / EDIT / EDIT2 / EDIT_ENEMY, pad repeat bit 0x8 / 0x4 step the cG03PacnRollSys round (+0) up / down
// through 0..0x1b (wrapping); then cAppGameDebugMenu::Execute. Draw 0x8012bad0: "w%02ds%02d" (world + 1, stage + 1 of that round) right behind the
// label of the STAGE row (+0x4c, not the cursor row: x = 0x20 + strlen("STAGE") * 0x14, y = 0x30 + row * 0x14), then cAppBaseDebugMenu::Draw.
// DELIBERATE DEVIATIONS: left / right arrows (not the DOL's bit assignment under the Sideways orientation) change the round, like the cursor swap of the
// base menu; STAGE (job 212, an empty job in retail) starts the chosen stage (job 0x0d300000).
class cAppPacnRollDebugMenu : public kingdom::cAppBaseDebugMenu
{
public:
	cAppPacnRollDebugMenu();
	bool Execute(cJetTask* pParam) override;
	void Draw(int nStatus, cJetTask* pParam) override;

private:
	int m_stageRow{0};            // +0x4c: index of the STAGE entry (job 0x0d400000), found by the constructor loop 0x8012b108..0x8012b140
};

}
