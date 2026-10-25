#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "wii/Brres.hpp"

// Reader for NW4R font resources ("RFNT", ut::ResFont): FINF (font info), TGLP (glyph sheets, GX textures), CWDH (widths), CMAP (code maps).
// Layout and lookup rules follow nw4r/ut/fontResourceFormat.h and ut_ResFontBase.cpp (NintendoWare/Revolution). The file is big endian;
// the pointers in FINF / CWDH / CMAP are file offsets of the block *bodies* (block start + 8).
namespace wii
{
struct RfntCharWidths
{
	int left{};       // left space
	int glyphWidth{}; // pixel width of the glyph
	int charWidth{};  // advance
};

struct RfntGlyph
{
	int sheet{};
	int cellX{}, cellY{};   // pixel position of the glyph's cell on its sheet (1 pixel border already skipped)
	RfntCharWidths widths;
};

class Rfnt
{
public:
	static constexpr std::uint16_t NOT_FOUND = 0xffff;

	bool Load(const std::uint8_t* data, std::size_t size);   // keeps no pointer into `data`

	int Height() const { return m_height; }          // FINF height
	int Ascent() const { return m_ascent; }
	int Linefeed() const { return m_linefeed; }      // FINF leading
	int CellWidth() const { return m_cellW; }
	int CellHeight() const { return m_cellH; }
	int BaselinePos() const { return m_baseline; }   // TGLP baseline: cell top -> baseline
	int SheetCount() const { return (int)m_sheets.size(); }
	int SheetWidth() const { return m_sheetW; }
	int SheetHeight() const { return m_sheetH; }
	int Encoding() const { return m_encoding; }      // 0 UTF-8, 1 UTF-16, 2 Shift-JIS, 3 CP1252

	// Glyph index of a character code, NOT_FOUND when the font does not contain it (ResFontBase::FindGlyphIndex).
	std::uint16_t FindGlyphIndex(std::uint32_t code) const;
	// Glyph of a code; unknown codes use the font's alternate glyph (ResFontBase::GetGlyphIndex).
	RfntGlyph GetGlyph(std::uint32_t code) const;
	RfntCharWidths WidthsOfIndex(std::uint16_t index) const;
	// Decoded RGBA sheet n (row 0 = top), decoded on first use. Empty on error.
	const Image& Sheet(int n) const;

private:
	struct Map { std::uint16_t begin{}, end{}, method{}; std::vector<std::uint16_t> info; };
	struct Width { std::uint16_t begin{}, end{}; std::vector<RfntCharWidths> table; };

	int m_height{}, m_ascent{}, m_linefeed{}, m_encoding{};
	std::uint16_t m_alter{};
	RfntCharWidths m_defaultWidth;
	int m_cellW{}, m_cellH{}, m_baseline{}, m_maxCharWidth{};
	int m_sheetFmt{}, m_sheetRow{}, m_sheetLine{}, m_sheetW{}, m_sheetH{};
	std::uint32_t m_sheetSize{};
	std::vector<Map> m_maps;
	std::vector<Width> m_widths;
	std::vector<std::vector<std::uint8_t>> m_sheets;   // raw GX data
	mutable std::vector<Image> m_decoded;
	mutable std::vector<bool> m_done;
};
}
