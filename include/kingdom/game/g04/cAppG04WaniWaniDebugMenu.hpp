#pragma once

#include "kingdom/cAppDebugMenu.hpp"

// DOL kingdom::game::g04 WaniWani debug menu (job 225 = 0x0e100000; init 0x80199f64, term 0x8019a074 = ReleaseMainTask; vtable derived through
// cAppGameDebugMenu 0x80394410, whose Execute is the base's 0x800951f0). Title "WaniWani", 3 entries (table 0x80449340): GAME -> 227, GAME_MULTI -> 230
// (the 4 player game, reachable only from here), EXIT -> 239. No own Execute / Draw: Gator Panic's game class holds its menu, this is just the list.
// Reached natively through --job 0x0e100000 only (the top menu's G04 entry goes to job 224 as in the DOL).
namespace kingdom::game::g04
{
class cAppG04WaniWaniDebugMenu : public kingdom::cAppBaseDebugMenu
{
public:
	cAppG04WaniWaniDebugMenu();
	static void JobInit();   // DOL 0x80199f64
};
}
