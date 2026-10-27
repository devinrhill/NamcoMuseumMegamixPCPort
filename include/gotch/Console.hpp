#pragma once

// The host side of the gotch frontend (nmwii*.rso): what MainA (RSO2 0x80d53814) / GoTitle / CConsole do around a machine. On the Wii, MainA
// runs its own frame loop until the player leaves the game; natively the loop is split into one call per frame of the port's main loop.
// Ported games: vol 2 index 9 (PACMAN). Stand-ins until the Wii menu (WII_MENU) is ported: PLUS / MINUS = the menu's 1P / 2P START, HOME =
// the menu's exit (DEVIATION: the module exits through its own menu).
namespace gotch
{
namespace console
{
// the game index of the bridge (cKingdomSys +0xa70): 0..8 = vol 1 (nmwii.rso), 9..17 = vol 2 (nmwii2.rso, game = index - 9)
bool IsPorted(int gameIndex);
// MainA -> GoTitle(index - 9) -> X::Setup, the console's CWiiSound; false when the game is not ported or the module lacks its data
bool Start(int gameIndex);
// one frame of X::Execute (the pads, CConsole::OnWaitVSync, the machine, OnRender); false when the player left the game
bool Frame();
// the picture of the last frame, fitted to the 640 x 480 game rectangle
void Draw();
// the end of MainA (the console's sound stops)
void Stop();
}
}
