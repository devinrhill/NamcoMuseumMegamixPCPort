#pragma once

#include <cstdint>

// NEW RALLY-X of the arcade collection vol 2 (nmwii2.rso, class NEWRALLYX, RSO2 0x80d59870..0x80d92e00): the Z80 board of the original machine run
// by the gotch Z80 core, Namco's native patches of the ROM (RST 0 hooks: CPU1_ExtFuncExtend) and the frontend glue. Same shape as PACMAN
// (include/gotch/RallyX.hpp); the screen is NOT turned (landscape 288 x 224, CreateScreenSpec's rotation 0).
//
// Stubbed (marked at the sites): the screen adjust menu / CROSSHATCH / SCREENADJUST objects, CDraw::FadeSet, MenuMM, MENU_KEYCONV.
namespace gotch
{
class CDraw;
class PrimaryStream;

namespace newrallyx
{
bool Setup();                // RSO2 0x80d59948 (false when the player's module lacks the RALLYX data)
bool IsSetup();
void ExitMachine();          // RSO2 0x80d59e74 (Exit) + the save; the next Start runs Setup again
void StorageFrameUpdate();   // CDraw::WaitVSync RSO2 0x80d06d9c runs CDataStorage::FrameUpdate first
int Execute1V();             // RSO2 0x80d59f0c; 1 when the machine stopped
void HotReset();             // RSO2 0x80d5a134
void ExitRequest();          // CGAMEIO::ExitRequest (the HOME stand-in)
void SetSoundBufferClear(void (*fn)());
PrimaryStream* Primary();
const CDraw& Draw();         // 288 x 224 landscape, shown unturned
}
}
