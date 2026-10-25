#pragma once

#include <cstdint>

// The engine's debug text: DOL 0x8013be34 (queue a formatted string) and 0x8013c3d4 (draw one string), flushed by cSysTaskGFinish::PostDraw
// (0x801385fc -> 0x8013bf54) on top of everything. Characters are 8x8 glyphs of a 128x64 atlas embedded in the executable (a BRRES at
// 0x80442bc0, texture "ascii", 16 x 8 cells, character code = row * 16 + column) drawn at 16 x 16 pixels, advancing 16 pixels; '\n' returns
// to the start x and moves down 16; codes above 0x7f print '?'. Positions are in the 640 x 480 screen, origin top left, y down.
namespace dbgtext
{
constexpr std::uint32_t DEFAULT_COLOR = 0xffffffffu;   // "use the default": 0xRRGGBBAA, the default is white (0x80484a30 = -1)

// printf-style; queued until Flush(). Prints nothing if main.dol (the font) is unavailable.
void Print(int x, int y, std::uint32_t color, const char* fmt, ...) __attribute__((format(printf, 4, 5)));

// Draws and clears the queue; the platform calls it once per frame after all tasks drew.
void Flush();
}
