#pragma once

#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cAppDebugMenu.hpp"

// kingdom::game::rso: main.dol's side of the arcade collection (the classic games live in nmwii.rso / nmwii2.rso, ported under src/gotch/).
namespace kingdom
{
namespace game
{
namespace rso
{
// DOL kingdom::game::rso::cAppRSOOriginal (job 142 RSO_EXEC, init 0x800c2108, 0xb70 bytes, ctor 0x800c1700, Execute 0x800c1b90): loads the module
// of the game index (cKingdomSys +0xa70), fills the sRSOBridge (+0x280) and calls the module's MainA, which runs the game until the player
// leaves; then the bridge's result flags go to cKingdomSys and job 143 follows.
// Natively the module is the gotch port: MainA's loop runs one frame per Execute (gotch::console). Not ported: the bridge's heap blocks / settings / language, the result flags (+0xa9c / +0xaa0 / +0xacc / +0xad0 stay 0: the module's reset / power requests).
class cAppRSOOriginal : public cAppKingdom
{
public:
	cAppRSOOriginal();
	bool Execute(cJetTask* pParam) override;
	void Draw(int nStatus, cJetTask* pParam) override;
	void Free() override;   // DOL 0x800c1f00 (dtor) -> 0x800c1ab0: the save back into block 6

private:
	int m_nGame = 0;          // the bridge's +4 (game index)
	bool m_bRunning = false;  // MainA has not returned yet
	bool m_bDone = false;     // the job change is asked for
};

// DOL kingdom::game::rso::cAppRSODebugMenu (job 174, init 0x800c1f98, vtable 0x80438370): "RSO Release", rows RSO 00..17 (job 141 with the row as
// the game index), the three save-data rows, EXIT (131).
class cAppRSODebugMenu : public cAppBaseDebugMenu
{
public:
	cAppRSODebugMenu();
	bool Execute(cJetTask* pParam) override;

private:
	bool m_bInit = false;     // +0x4c
};
}
}
}
