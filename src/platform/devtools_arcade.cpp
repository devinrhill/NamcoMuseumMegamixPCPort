// Dev-only: --arcade NAME runs a classic game of the arcade collection (src/gotch/) instead of the Megamix boot. Not part of the 1:1 game: the
// original reaches these games through the museum's classic menu (job 140, not ported yet) or the debug build's RSO menu (job 174) -> job 142,
// which runs the same gotch::console host (src/kingdom/game/rso/cAppRSOOriginal.cpp).
#include "platform/devtools.hpp"

#include <cstdio>

#include "gotch/Console.hpp"

namespace devtools
{
namespace
{
std::string g_arcade;
}

const std::string& Arcade() { return g_arcade; }
void SetArcade(const std::string& name) { g_arcade = name; }

bool ArcadeInit()
{
	if (g_arcade == "pacman") return gotch::console::Start(9);    // vol 2 game 0
	if (g_arcade == "rallyx") return gotch::console::Start(10);   // vol 2 game 1
	if (g_arcade == "newrallyx") return gotch::console::Start(11);   // vol 2 game 2
	std::fprintf(stderr, "--arcade %s: unknown game (available: pacman, rallyx, newrallyx)\n", g_arcade.c_str());
	return false;
}

void ArcadeFrame()
{
	// HOME (the exit stand-in) only stops the machine here: the dev loop has no job to go back to
	gotch::console::Frame();
	gotch::console::Draw();
}
}
