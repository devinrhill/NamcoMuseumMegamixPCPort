#pragma once

#include "kingdom/game/g01/cTaskConsolePlayer.hpp"

// Galaga Remix (g01) multiplayer console player: kingdom::game::g01::cTaskConsoleMultiPlayer (vtable 0x80439830, 0x270 B, a cTaskConsolePlayer built by
// 0x800ebab0 for every player when more than one plays). Its only difference is the result slot (0x800d740c replaces 0x800d6444): the player's panel
// and digits go out only once the multiplayer result console (cTaskConsoleResultMulti) has finished, and it keeps the console disabled.
namespace kingdom::game::g01
{
class cTaskConsoleResultMulti;

class cTaskConsoleMultiPlayer : public cTaskConsolePlayer
{
public:
	using cTaskConsolePlayer::cTaskConsolePlayer;
	void SetResult(cTaskConsoleResultMulti* r) { m_pResult = r; }

protected:
	bool ResultExec() override;   // DOL 0x800d740c

private:
	cTaskConsoleResultMulti* m_pResult{};   // the console manager's +0x23c
};
}
