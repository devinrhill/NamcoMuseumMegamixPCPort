// Loads RESIDENT_FONT_FRAME / RESIDENT_FONT_MII from resident.arc, prints font info and glyph data for a few characters,
// and writes sheet 0 as a PGM (alpha) next to the given output path when one is passed.
#include <cstdio>
#include <string>
#include "wii/Archive.hpp"
#include "wii/Rfnt.hpp"

int main(int argc, char** argv)
{
	if (argc < 2) return 2;
	wii::Archive arc;
	if (!arc.LoadFile(argv[1])) return 2;
	for (const auto& e : arc.Entries())
	{
		if (e.name.rfind("RESIDENT_FONT_", 0) != 0) continue;
		wii::Rfnt f;
		if (!f.Load(arc.Data(e), e.size)) { std::printf("%s FAILED\n", e.name.c_str()); continue; }
		std::printf("%s h=%d asc=%d lf=%d cell=%dx%d sheets=%d %dx%d enc=%d\n", e.name.c_str(), f.Height(), f.Ascent(), f.Linefeed(), f.CellWidth(),
		            f.CellHeight(), f.SheetCount(), f.SheetWidth(), f.SheetHeight(), f.Encoding());
		for (unsigned c : {65u, 97u, 48u, 32u, 0x3042u, 0x20acu})
		{
			const wii::RfntGlyph g = f.GetGlyph(c);
			std::printf("  U+%04X idx=%d sheet=%d cell=(%d,%d) w=%d/%d/%d\n", c, f.FindGlyphIndex(c), g.sheet, g.cellX, g.cellY, g.widths.left, g.widths.glyphWidth, g.widths.charWidth);
		}
		if (argc > 2 && e.name == "RESIDENT_FONT_FRAME")
		{
			const wii::Image& s = f.Sheet(f.GetGlyph('A').sheet);
			std::FILE* o = std::fopen(argv[2], "wb");
			std::fprintf(o, "P5 %d %d 255\n", s.width, s.height);
			for (int i = 0; i < s.width * s.height; i++) std::fputc(s.rgba[i * 4 + 3], o);
			std::fclose(o);
		}
	}
	return 0;
}
