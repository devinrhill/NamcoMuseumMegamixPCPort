#pragma once

// The dev window's Galaga stage editor (src/platform/devtools_g01edit.cpp; tab "Galaga editor", only in a single-player GALAGA_GAME course stage,
// not the boss stage 10). Kept out of devtools.hpp so a change here does not rebuild the game. NOT part of the 1:1 game.
// G01EditOverride swaps the stage's scn file for the applied edit (cTaskScene's constructor), G01EditOrbits replaces the orbit table read from
// EnemyOrbit_s%x.csv (cOrbitDataMan::Load), G01EditFrozen holds the stage still while the tab is shown (cAppG01GalagaGame::PauseFlag),
// G01EditDraw3D draws the selected spawn's orbit and handles (inside the 3D pass, devtools::DrawOverlay3D).

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace kingdom::game::g01 { struct cOrbit; }

namespace devtools
{
void RegisterG01EditPanel();
void G01EditOverride(const std::string& key, const std::uint8_t*& data, std::size_t& size);
void G01EditOrbits(int stage, std::vector<kingdom::game::g01::cOrbit>& orbits);
bool G01EditFrozen();
void G01EditDraw3D();
}
