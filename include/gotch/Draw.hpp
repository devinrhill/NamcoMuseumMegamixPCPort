#pragma once

#include <cstdint>
#include <vector>

// Native stand-in for the gotch framework's CDraw (nmwii2.rso, RSO2 0x80d037b8..): the arcade games queue 2D character quads (DrawChrEx) between
// Begin / End and CDraw flushes them through GX. The port keeps the game-facing calls (same arguments, same queue fields) and paints the queue into a
// software RGBA frame buffer of the game's screen size instead of GX. What is NOT GX-exact (UNVERIFIED, marked at the sites): the depth order
// inside one flush (sorted far to near, smaller z = nearer), blending (texels with alpha 0 are skipped, others replace), scale factors other than 1.
namespace gotch
{
// CGENFORMAT, the layout AssignTextureAndConvertFromCGen reads (+0 width, +4 height, +8 bits per pixel, +0xc bytes per character, +0x10 pixel layout:
// width * height entries of 8 u32 each, one per bit plane (top 4 bits = source ROM index, 0xf = no plane; low 28 bits = bit address in the character))
struct CGENFORMAT
{
	int width = 0;
	int height = 0;
	int bpp = 0;
	int charBytes = 0;
	const std::uint8_t* layout = nullptr;  // big-endian u32 entries in the module
};

class CDraw
{
public:
	CDraw(int screenW, int screenH);

	int ScreenW() const { return m_w; }
	int ScreenH() const { return m_h; }

	// RSO2 0x80d3596c: decodes `count` characters of the CGen ROMs (src[plane source]) into a texture; returns its handle (the original writes it to
	// *handle and returns 1), -1 when a layout entry names a source above 7
	int AssignTextureAndConvertFromCGen(const CGENFORMAT& fmt, int count, const std::uint8_t* const* src, int add);
	// RSO2 0x80d090c0: palette `index` colours start .. start + n - 1 (0xAARRGGBB as the games build them)
	int SetPalet(int index, int start, const std::uint32_t* colours, int n);
	std::uint32_t GetPalet(int index, int colour) const;

	void Begin(std::uint32_t mode);  // RSO2 0x80d0adc8
	void End();                      // RSO2 0x80d0aef4
	// RSO2 0x80d0b0d4: one character. flip bit 0 / 1 = mirror the image horizontally / vertically, flip bits 8..9 = blend mode;
	// mirror bit 0 / 1 = mirror the position (and the image) inside the screen
	int DrawChrEx(float scaleX, float scaleY, int x, int y, int z, int tex, std::uint32_t code, int palette, std::uint32_t flip, std::uint32_t mirror);

	// RSO2 0x80d0b618 for the menus' box (type 0x98 = quads, colours looked up in `palette`): xy = 4 corners, colour = the palette entry
	// (UNVERIFIED: the blend of RenderModeSet, taken as source alpha over the frame; only axis-aligned rectangles, all the callers draw)
	void DrawPoints(const int* xy, int n, int palette, int colour);
	// RSO2 0x80d334b8 / 0x80d33640: a text in 8 x 8 characters of texture `tex`. rect = left, top, right, bottom (left / right only align);
	// flags & 0xfff = the screen rotation (0, 90, 180, 270), (flags >> 12) & 0xf = 0 left / 1 right / 2 centred, 0x10000 = measure only,
	// (flags >> 12) & 0xf00 = the characters' flip / blend bits. conv (may be null) maps codes < 0x100. out (may be null) = the bounding box.
	void PrintfUTF8(const int* rect, int z, int tex, int palette, const char* s, std::uint32_t flags, int lineHeight, const std::uint16_t* conv,
	                int* out);
	void PrintfUCS2(const int* rect, int z, int tex, int palette, const std::uint16_t* s, std::uint32_t flags, int lineHeight,
	                const std::uint16_t* conv, int* out);
	// the accented letters' glyph table (RSO2 0x80dd27d8: 21 entries of base glyph, accent glyph, accent flip; big-endian words of the module)
	void SetAccentTable(const std::uint8_t* table) { m_accent = table; }

	// the frame buffer (row 0 = top, 0xAARRGGBB); ClearFrame paints it black before a frame is rendered
	void ClearFrame();
	// GX keeps the depth buffer across batches (Begin / End pairs); the first ported game (PACMAN) never relies on it, RALLYX draws sprites and
	// tiles in separate batches. Off by default: a nearer-or-equal quad wins, as the flush's sort already does inside one batch
	void SetDepthTest(bool on) { m_depthTest = on; }
	const std::vector<std::uint32_t>& Frame() const { return m_frame; }

private:
	struct Texture
	{
		int w = 0, h = 0, count = 0;
		std::vector<std::uint8_t> pix;  // count * w * h colour indices
	};
	// one DrawChrEx entry (the original's 0x28-byte queue record: +0 x, +2 y, +4 w, +6 h, +8 z, +0xc blend, +0x10 palette, +0x14 texture, uv, scale)
	struct Quad
	{
		int x, y, z, w, h;
		int tex, code, palette;
		bool flipU, flipV;
		int blend;
		std::uint32_t order;
	};
	void Flush();  // RSO2 0x80d0a2d0
	// RSO2 0x80d3312c: a text position in the menu's (portrait) space -> the frame, and the mirror bits DrawChrEx gets
	void RotateDrawStringPos(int rot, int x, int y, int* ox, int* oy, std::uint32_t* mirror, std::uint32_t* flip) const;
	const std::uint8_t* m_accent = nullptr;

	int m_w, m_h;
	std::vector<std::uint32_t> m_frame;
	std::vector<Texture> m_tex;
	std::uint32_t m_palet[0x100][0x100] = {};  // 256 palettes of up to 256 colours (SetPalet refuses index >= 0x100)
	std::vector<Quad> m_queue;
	bool m_open = false;       // +0x1411a9
	std::uint32_t m_mode = 0;  // +0x141220
	bool m_alphaTest = false;  // +0x1411aa: Begin bit 0
	bool m_depthTest = false;
	std::vector<int> m_depth;  // per pixel, reset by ClearFrame
};
}
