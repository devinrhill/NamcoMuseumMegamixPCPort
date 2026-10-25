#pragma once

#include "kingdom/cAppDebugMenu.hpp"

// DOL kingdom::game::g01::cAppGalagaDebugMenu (job 177 = 0x0b100000; init 0x800b0c8c, term 0x800b0ce0, vtable 0x80396348, derived through
// cAppGameDebugMenu 0x80394410). Title "Galaga" (0x8036f798), 3 entries (table 0x80396428): MENU 0x0b200000, STAGE 0x0b300000, EXIT 0x0bf00000.
// Same deliberate left / right convention as the Pac-N-Roll / Pac-Motos debug menus (kingdom/cAppDebugMenu.hpp).
namespace kingdom::game::g01
{
class cAppGalagaDebugMenu : public kingdom::cAppBaseDebugMenu
{
public:
	cAppGalagaDebugMenu();                                    // DOL 0x800af848
	bool Execute(cJetTask* pParam) override;                  // DOL 0x800af96c
	void Draw(int nStatus, cJetTask* pParam) override;        // DOL 0x800afabc
	bool SaveData();                                          // DOL 0x800afba8 (vtable slot 13; no caller found in the base menus)

	static void JobInit();                                    // DOL 0x800b0c8c

private:
	int m_nStage{};                                           // +0x4c (1..10)
	int m_nPlayer{1};                                         // +0x50 (1..4)
};
}
