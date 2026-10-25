#pragma once

#include "kingdom/cAppDebugMenu.hpp"

// Rally-X debug menu (job 193 = 0x0c100000). NOT in the retail DOL: its table slot 193 is an empty stub (init 0x800c6430, term 0x800c6434), so this is a
// native addition in the style of the Pac-Motos menu (cAppMotosDebugMenu): MENU 194, MULTI MENU 200, MATCHING 199, STAGE 195, GAMEOVER 198,
// STAFFROLL 206, EXIT 207. On STAGE left / right step cG02RallyXSys::nStageIdx through 0..30 (wrapping; 25..30 are the multiplayer stages, arcs g02w11sNN).
// The name behind the STAGE label: "wxxsxx" for index 0, "w01s01".."w01s05" then "w01red" (the world's boss stage, 6th) for worlds 1..4, "vs01".."vs06" for the
// multiplayer stages (as the Motos menu).
namespace kingdom::game::g02
{
class cAppRallyXDebugMenu : public kingdom::cAppBaseDebugMenu
{
public:
	cAppRallyXDebugMenu();
	bool Execute(cJetTask* pParam) override;
	void Draw(int nStatus, cJetTask* pParam) override;

private:
	int m_stageRow{0};
};
}
