#pragma once

#include <string>
#include <vector>

// A localized message CSV from the archives (RESIDENT_MSG_NAND_ERROR, ...): header row, then {key, description, English, Japanese, German,
// Spanish, French, Italian, Korean}. The game reads the cells with cJetCSV (DOL 0x801394b4 -> ConvertS32ASCII 0x8013a450 / ShiftJIS 0x8013a648);
// here the column is fixed to English (project rule: locale en). "\n" in a cell is a line break. Bytes that are not valid UTF-8 are kept as is
// (gfx::DrawText reads them as Latin-1).
namespace kingdom
{
class MsgTable
{
public:
	// `entry` = archive entry name. Returns false when it is not in the attached archives.
	bool Load(const char* entry);
	// Message `index` = CSV row index + 1 (row 0 is the header), English column. Empty when out of range.
	std::string Get(int index) const;
	// Raw cell (row 0 = first line of the file, column 0 = first cell); empty when out of range.
	std::string Cell(int row, int col) const;

private:
	std::vector<std::vector<std::string>> m_rows;
};
}
