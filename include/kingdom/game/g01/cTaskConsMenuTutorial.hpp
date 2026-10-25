#pragma once

#include "common.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/game/menu/cTaskConsMenuTutorialBase.hpp"

// Galaga Remix (g01) HOW TO PLAY pages: kingdom::game::g01::cTaskConsMenuTutorial (DOL vtable 0x80437f00, 0x2f0 bytes).
// Only the ctor / dtor are Galaga's; every other slot is kingdom::game::menu::cTaskConsMenuTutorialBase's.
namespace kingdom::game::g01
{

class cTaskConsMenuTutorial : public kingdom::game::menu::cTaskConsMenuTutorialBase
{
public:
	// DOL 0x800bfb9c. Created by cAppG01GalagaMenu's ctor (0x800af0d4) as new cTaskConsMenuTutorial(app +0x2a0, app +0x2ac):
	// bMulti = cG01GalagaSys::IsMulti (2..4 players: the "Cooperate ..." pages), pCursor = the first player's cursor.
	cTaskConsMenuTutorial(bool bMulti, kingdom::cTaskCursor* pCursor);

	virtual ~cTaskConsMenuTutorial();   // DOL 0x800bfc54
};

}
