#pragma once

#include <cstdint>
#include <string>

// Text drawing with the game's NW4R fonts (wii::Rfnt) on top of gfx::DrawQuad: the native stand-in for nw4r::ut::TextWriter as used by
// kingdom::cKingdomFontWriter / cTaskFontWriter (vtables 0x803893b8 / 0x804381c0). Fonts are archive entries ("RESIDENT_FONT_FRAME",
// "RESIDENT_FONT_MII"), found through the attached archives like every other resource. Text is UTF-8 (FINF encoding 0 for both fonts).
// UNVERIFIED: nw4r tag processing (cTagProcessor 0x8038a038) is not implemented, '\n' is the only control character.
//
// Coordinates are screen pixels of the 640 x 480 screen, origin TOP-LEFT, y down (the convention of dbgtext and of the DOL's text code);
// (x, y) is the draw origin: x per TextAlign, y per TextStyle::vorigin (nw4r::ut::TextWriterBase draw flags).
namespace gfx
{
struct TextStyle
{
	const char* font = "RESIDENT_FONT_FRAME";
	float scaleX = 1.0f;
	float scaleY = 1.0f;
	std::uint32_t color = 0xffffffffu;    // 0xRRGGBBAA
	float extraSpacing = 0.0f;            // added to every advance (unscaled font pixels)
	// nw4r VERTICAL_ORIGIN_*: y is the top (ascender line) / middle / bottom of the text block, or the FIRST line's baseline.
	// kingdom::cKingdomFontWriter queue modes 0..2 (flags 0x300 / 0x322 / 0x311) are Baseline, mode 3 (0x111) is Middle.
	enum class VOrigin { Top, Middle, Bottom, Baseline } vorigin = VOrigin::Top;
};

enum class TextAlign { Left, Center, Right };

// Width of the widest line / total height of the text in screen pixels. Zero when the font cannot be loaded.
float MeasureTextWidth(const std::string& utf8, const TextStyle& style);
float MeasureTextHeight(const std::string& utf8, const TextStyle& style);

// FINF height of a font (nw4r SetFontSize(size) = scale size / height). Zero when the font cannot be loaded.
int FontHeight(const char* font);

// Draws the text. For Center / Right, x is the centre / right edge of each line. Returns false when the font is unavailable.
bool DrawText(float x, float y, const std::string& utf8, const TextStyle& style = {}, TextAlign align = TextAlign::Left);

// Drops cached fonts and their GPU sheets (call when the attached archives change).
void ResetFonts();
}
