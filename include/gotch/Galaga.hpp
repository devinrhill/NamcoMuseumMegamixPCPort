#pragma once

#include <cstdint>

// GALAGA of the arcade collection vol 2 (nmwii2.rso, class GALAGA, game index 12, RSO2 0x80d63d94..0x80d7ed58, the custom chips
// GALAGA_NCI / N51 / N54 at RSO2 0x80d88cec..0x80d8a740, the sound GALAGA's SND_GALAGA_WSG3 at RSO2 0x80d64f0c): the three-Z80 board of the original
// machine (CPU1 main, CPU2 sub, CPU3 sound; one shared RAM) run by the gotch Z80 core, the 06XX-style NCI with its N51 (coins / controls) and N54
// (sound commands) chips, Namco's native patches of the ROMs (RST 0 hooks: CPU1 / CPU2 / CPU3 ExtFuncExtend) and the frontend glue. Same shape as
// RALLYX (include/gotch/RallyX.hpp); the monitor is turned like PACMAN's (portrait: the Console must show this game as `portrait`).
//
// Stubbed (marked at the sites): the screen adjust menu / CROSSHATCH / SCREENADJUST objects, CDraw::FadeSet, MenuMM, MENU_KEYCONV, the key config
// (InitMenu_KeySetting), the 06XX NMI timer (the original raises the NMI from the NCI status in CPU1's ReadMemory8).
namespace gotch
{
class CDraw;
class PrimaryStream;

namespace galaga
{
bool Setup();                // RSO2 0x80d65a84 (false when the player's module lacks the GALAGA data)
bool IsSetup();
void ExitMachine();          // RSO2 0x80d669e4 (Exit) + the save; the next Start runs Setup again
void StorageFrameUpdate();   // CDraw::WaitVSync RSO2 0x80d06d9c runs CDataStorage::FrameUpdate first
int Execute1V();             // RSO2 0x80d66b44; 1 when the machine stopped
void HotReset();             // RSO2 0x80d670c4
void ExitRequest();          // CGAMEIO::ExitRequest (the HOME stand-in)
void SetSoundBufferClear(void (*fn)());
// GALAGA::SoundInt (RSO2 0x80d66fe0, twice per frame) calls CConsole::UpdateStream(console, &time): the integrator binds this to
// Console_UpdateStream(*g_wiiSound, Primary(), time, enabled) (time in 2^40 units per second = half a frame period)
void SetUpdateStream(void (*fn)(std::int64_t time));
PrimaryStream* Primary();
const CDraw& Draw();         // 288 x 224 landscape, the monitor turned (portrait on screen)
}
}
