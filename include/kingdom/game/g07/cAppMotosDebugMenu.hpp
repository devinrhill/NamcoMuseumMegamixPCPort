#pragma once

#include "kingdom/cAppDebugMenu.hpp"

namespace kingdom::game::g07
{

// DOL kingdom::game::g07::cAppMotosDebugMenu (job 273 = 0x11100000; init 0x800a29bc, term 0x800a2b20, vtable 0x80394d40). Title "Motos"
// (0x8036ebe4), 7 entries (table 0x80394d08). Execute 0x800a36c4: on STAGE (job 0x11400000) pad repeat 0x8 / 0x4 step cG07MotosSys::nStageIdx
// up / down through 0..0x2d (wrapping; 0x24.. are the 10 multiplayer stages). Draw 0x800a35a4 behind the STAGE label: "wxxsxx" for index 0,
// "w%02ds%02d" (nWorld, nStage) below 0x24, else "vs%02d" (nStage). Same deliberate left / right swap as the Pac-N-Roll menu.
class cAppMotosDebugMenu : public kingdom::cAppBaseDebugMenu
{
public:
	cAppMotosDebugMenu();
	bool Execute(cJetTask* pParam) override;
	void Draw(int nStatus, cJetTask* pParam) override;

private:
	int m_stageRow{0};            // +0x4c
};

}
