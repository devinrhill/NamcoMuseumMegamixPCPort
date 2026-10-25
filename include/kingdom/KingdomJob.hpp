#pragma once

#include "common.hpp"

namespace kingdom
{

// Job numbers (table index << 20) of the system jobs, from the table at 0x804400c8 in main.dol.
namespace job
{
constexpr uint KINGDOM_SYS = 0x08000000u;  // 128: creates cKingdomSys
constexpr uint PASS = 0x08200000u;         // 130: -> 132
constexpr uint BOOT = 0x08400000u;         // 132: cAppBoot
constexpr uint CHANGE_FADE = 0x08500000u;  // 133: cAppSaveLoad as transition (with fade)
constexpr uint CHANGE = 0x08600000u;       // 134: cAppSaveLoad as transition (no fade)
constexpr uint ATTRACT = 0x08800000u;      // 136: cAppAttract
constexpr uint TITLE = 0x08900000u;        // 137: cAppTitle
}

// DOL 0x8000db5c. force == false: switch immediately. Otherwise remember `jobNo` in cKingdomSys and go
// through the transition app (fade ? job 133 : job 134), which jumps to the remembered job when it is done.
void Kingdom_SetJobNo(uint jobNo, bool force, bool fade);

}
