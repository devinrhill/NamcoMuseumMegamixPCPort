#pragma once

// The dev window's Pac-Motos stage editor (src/platform/devtools_g07edit.cpp; tab "Motos editor", only in a single-player MOTOS_GAME stage).
// Kept out of devtools.hpp so a change here does not rebuild the game. NOT part of the 1:1 game.
// G07EditOverride swaps the stage's MAPOO_OBJ for the applied edit (cKingdomMapoo::RegistObj), G07EditCamera replaces the camera with the
// editor's orbit camera (NativeScene::SetCamera), G07EditDraw3D draws the grid and markers (inside the 3D pass, devtools::DrawOverlay3D).

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace kingdom { class cKingdomModelBase; }

namespace devtools
{
void RegisterG07EditPanel();
void G07EditOverride(const std::string& key, const std::uint8_t*& data, std::size_t& size);
void G07EditCamera(float eye[3], float target[3], float& farZ);
void G07EditDraw3D();
// "Preview the edited map": while the tab is shown, the live panels / gimmicks / characters (light sets 1 / 2) are dropped from this frame's
// 3D pass and stand-alone models of the edited map (no tasks) are drawn instead; the background stays (NativeScene::Flush).
void G07EditPreviewFrame(std::vector<kingdom::cKingdomModelBase*>& frame);
bool G07EditFrozen();      // the tab is shown: the stage stands still (Constants::MOTOS_IS_PAUSE, cAppG07MotosGame::Execute)
bool G07SkipItemSet();     // "Skip the item settings": the item setup screen confirms at once (cAppG07MotosGame::StepItemSet), no READY console
// While the tab is shown on the item setup screen, the stock of power / jump / charge (KeepData m_nItemNum[1..3]) reads 99 (the real stock
// is kept aside); G07EditItemRestore puts it back at the stage end / app Term and returns true when it did (the game then skips its subtraction).
void G07EditItemStock(unsigned char* itemNum);
// Difficulty (Menu / Motos editor tabs): 0 normal, 1 easy, 2 hard, the order of the CSV rows / columns and of cMotosGameSys::nGameLevel. Megamix only
// ever reads NORMAL (DOL: only CSV_CHAR_PARAM_NORMAL / CSV_STAGE_PARAM_NORMAL_PARAM are referenced, row 1 / column 1 elsewhere); the EASY / HARD data
// on the disc is unused. G07Level() is 0 in the multiplayer stages (36..45); read when a stage starts.
int& G07LevelSetting();
int G07Level();
std::string G07LevelCsv(const char* normalName);   // "..._NORMAL..." -> "..._EASY..." / "..._HARD..."
bool G07EditItemRestore(unsigned char* itemNum);
}
