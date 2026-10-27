#pragma once

#include <cstdint>

// PAC-MAN of the arcade collection vol 2 (nmwii2.rso, class PACMAN, RSO2 0x80d45fa0..0x80d47c40 + 0x80d81700..0x80d86600): the Z80 board of the
// original machine run by the gotch Z80 core, with Namco's native patches of the ROM (RST 0 hooks: CPU1_ExtFuncExtend / ExtFuncExtendCommon)
// and the frontend glue (swork, export data = the save block, DIP switch settings, the CDraw renderer). A 1:1 port of that code; ROMs and
// tables are read from the player's module at Setup, never stored in the source.
//
// Stubbed (marked at the sites): the Wii menu (WII_MENU: options / pause / continue round), CGAMEIO (pause / debug / ROM-RAM skip
// / exit checks), achievements' WiiConnect side (the counters are kept), the crosshatch / screen adjust objects, CDraw::FadeSet / PrintfUTF8 (the "NOW SAVING" text).
namespace gotch
{
class CDraw;
class PrimaryStream;

namespace pacman
{
// RSO2 0x80d465b0: builds the machine (false when the player's module lacks the PACMAN data)
bool Setup();
bool IsSetup();
// RSO2 0x80d46eac (ExecuteExit -> PACMAN::Exit 0x80d46d64 -> ExitExtend 0x80d8417c): the high score into swork, the save block out to the
// bridge (gotch::BridgeSave); the next Start runs Setup again
void ExitMachine();
// CDraw::WaitVSync (RSO2 0x80d06d9c) first runs CDataStorage::FrameUpdate of the console's storage (+0x124 = the machine's, 0x811fa70c)
void StorageFrameUpdate();
// RSO2 0x80d46e94: one video frame (ConvertIO, the CPU + events up to VBLANK_OUT, Render); returns 1 when the machine stopped
int Execute1V();
// RSO2 0x80d470fc
void HotReset();
// the pads are CGAMEIO's (src/gotch/GameIO.cpp: remotes 1 / 2 through the module's translate table); the pause menu is WII_MENU's
// (src/gotch/Menu.cpp). Stand-ins for the menus that are not ported (start / options): sw+0x2f (a game is on), sw+6 / sw+7 (a player paused),
// the HOME menu's exit
bool Playing();
bool Paused();
void ExitRequest();
// the console's CWiiSound::BufferClear, which the pause menu's QUIT GAME calls (machine +0x120 -> +0xc0); set before Setup
void SetSoundBufferClear(void (*fn)());
// RSO2 0x80d81854 (PACMAN::GAMEMENU::GameStartInit): what the Wii menu's START does; value 0x10000 = 1P, 0x10100 = 2P, 0x18100 / 0x18200 = 1P / 2P
// continue
void GameStartInit(int value);

// the machine's PRIMARYSTREAM (machine +0x94; InitMachine binds it to the console: CConsole::BindPrimaryStream RSO2 0x80d00654)
PrimaryStream* Primary();

// the 288 x 224 landscape picture the last Execute1V rendered (0xAARRGGBB, row 0 = top); the cabinet's monitor shows it turned (SCREEN_SPEC
// rotation 270 degrees, RSO2 0x80dc8618): portrait pixel (223 - y, x)
const CDraw& Draw();
}
}
