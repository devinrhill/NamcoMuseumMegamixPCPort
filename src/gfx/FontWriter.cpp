#include "gfx/FontWriter.hpp"

#include <map>
#include <memory>
#include <vector>

#include "gfx/Gfx.hpp"
#include "wii/Archive.hpp"
#include "wii/Rfnt.hpp"

namespace gfx
{
namespace
{
struct LoadedFont
{
	wii::Rfnt font;
	std::vector<TexId> sheets;
};
std::map<std::string, std::unique_ptr<LoadedFont>> g_fonts;

LoadedFont* GetFont(const std::string& name)
{
	auto it = g_fonts.find(name);
	if (it != g_fonts.end()) return it->second.get();
	std::unique_ptr<LoadedFont> lf;
	std::size_t size = 0;
	const std::uint8_t* d = wii::ArchiveStore::Find(name.c_str(), size);
	if (d != nullptr)
	{
		lf = std::make_unique<LoadedFont>();
		if (!lf->font.Load(d, size)) lf.reset();
		else lf->sheets.assign(lf->font.SheetCount(), NO_TEX);
	}
	LoadedFont* r = lf.get();
	g_fonts[name] = std::move(lf);   // a missing font is remembered as null until ResetFonts
	return r;
}

TexId SheetTex(LoadedFont& f, int n)
{
	if (n < 0 || n >= (int)f.sheets.size()) return NO_TEX;
	if (f.sheets[n] == NO_TEX)
	{
		const wii::Image& img = f.font.Sheet(n);
		if (!img.rgba.empty()) f.sheets[n] = CreateTexture(img.width, img.height, img.rgba.data());
	}
	return f.sheets[n];
}

// Next code point of a UTF-8 string; invalid bytes are taken as Latin-1 (the message CSVs mix encodings).
std::uint32_t NextCode(const std::string& s, std::size_t& i)
{
	const unsigned char c = (unsigned char)s[i++];
	int extra = c >= 0xf0 ? 3 : c >= 0xe0 ? 2 : c >= 0xc0 ? 1 : 0;
	if (c >= 0x80 && extra == 0) return c;
	std::uint32_t code = extra == 3 ? (c & 7) : extra == 2 ? (c & 15) : extra == 1 ? (c & 31) : c;
	if (i + extra > s.size()) return c;
	for (int k = 0; k < extra; k++)
	{
		const unsigned char n = (unsigned char)s[i + k];
		if ((n & 0xc0) != 0x80) return c;
		code = (code << 6) | (n & 0x3f);
	}
	i += extra;
	return code;
}

float LineWidth(LoadedFont& f, const std::string& s, std::size_t& i, const TextStyle& st)
{
	float w = 0;
	while (i < s.size())
	{
		if (s[i] == '\n') { i++; return w; }
		w += (f.font.GetGlyph(NextCode(s, i)).widths.charWidth + st.extraSpacing) * st.scaleX;
	}
	return w;
}
}

float MeasureTextWidth(const std::string& s, const TextStyle& st)
{
	LoadedFont* f = GetFont(st.font);
	if (f == nullptr) return 0;
	float best = 0;
	for (std::size_t i = 0; i < s.size();) best = std::max(best, LineWidth(*f, s, i, st));
	return best;
}

float MeasureTextHeight(const std::string& s, const TextStyle& st)
{
	LoadedFont* f = GetFont(st.font);
	if (f == nullptr) return 0;
	int lines = 1;
	for (char c : s) if (c == '\n') lines++;
	return (f->font.Height() + (lines - 1) * f->font.Linefeed()) * st.scaleY;
}

int FontHeight(const char* font)
{
	LoadedFont* f = GetFont(font);
	return f != nullptr ? f->font.Height() : 0;
}

bool DrawText(float x, float y, const std::string& s, const TextStyle& st, TextAlign align)
{
	LoadedFont* f = GetFont(st.font);
	if (f == nullptr) return false;
	const wii::Rfnt& font = f->font;
	const std::uint8_t r = (std::uint8_t)(st.color >> 24), g = (std::uint8_t)(st.color >> 16), b = (std::uint8_t)(st.color >> 8), a = (std::uint8_t)st.color;
	// nw4r ut_TextWriterBase.cpp AdjustCursor: the cursor ends on the first line's baseline (y, or the block's top + ascent);
	// Print moves each glyph cell up by the TGLP baseline position, so the cell top is baseline - baselinePos (all scaled).
	float base = y;
	if (st.vorigin != TextStyle::VOrigin::Baseline)
	{
		const float h = MeasureTextHeight(s, st);
		const float top = st.vorigin == TextStyle::VOrigin::Middle ? y - h / 2 : st.vorigin == TextStyle::VOrigin::Bottom ? y - h : y;
		base = top + font.Ascent() * st.scaleY;
	}
	float cy = base - font.BaselinePos() * st.scaleY;
	for (std::size_t i = 0; i < s.size();)
	{
		std::size_t probe = i;
		const float w = LineWidth(*f, s, probe, st);
		float cx = align == TextAlign::Center ? x - w / 2 : align == TextAlign::Right ? x - w : x;
		while (i < s.size() && s[i] != '\n')
		{
			const wii::RfntGlyph gl = font.GetGlyph(NextCode(s, i));
			const TexId tex = SheetTex(*f, gl.sheet);
			if (tex != NO_TEX && gl.widths.glyphWidth > 0)
			{
				const wii::Image& sh = font.Sheet(gl.sheet);
				const float x0 = cx + gl.widths.left * st.scaleX - 320, x1 = x0 + gl.widths.glyphWidth * st.scaleX;
				const float y0 = 240 - cy, y1 = y0 - font.CellHeight() * st.scaleY;
				const float u0 = (float)gl.cellX / sh.width, u1 = (float)(gl.cellX + gl.widths.glyphWidth) / sh.width;
				const float v0 = (float)gl.cellY / sh.height, v1 = (float)(gl.cellY + font.CellHeight()) / sh.height;
				const float xy[8] = {x0, y0, x0, y1, x1, y0, x1, y1};
				const float uv[8] = {u0, v0, u0, v1, u1, v0, u1, v1};
				DrawQuad(tex, xy, uv, r, g, b, a, BLEND_ALPHA, true);
			}
			cx += (gl.widths.charWidth + st.extraSpacing) * st.scaleX;
		}
		if (i < s.size()) i++;   // the '\n'
		cy += font.Linefeed() * st.scaleY;
	}
	return true;
}

void ResetFonts()
{
	for (auto& kv : g_fonts)
		if (kv.second)
			for (TexId t : kv.second->sheets) if (t != NO_TEX) DestroyTexture(t);
	g_fonts.clear();
}
}
