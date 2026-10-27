#include "gfx/DebugText.hpp"

#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>

#include "gfx/Gfx.hpp"
#include "wii/Brres.hpp"
#include "wii/Dol.hpp"

namespace dbgtext
{
namespace
{
constexpr std::uint32_t kFontBres = 0x80442bc0;   // embedded BRRES (8448 bytes) with the 128 x 64 "ascii" texture
constexpr int kQueueBytes = 0x3000;               // the DOL's buffer (0x80484a20 + 0x4c)

struct Entry { int x, y; std::uint32_t color; std::string text; };
std::vector<Entry> g_queue;
std::size_t g_bytes = 0;
gfx::TexId g_tex = gfx::NO_TEX;
bool g_tried = false;

bool EnsureFont()
{
	if (g_tried) return g_tex != gfx::NO_TEX;
	g_tried = true;
	wii::Brres b;
	wii::Image img;
	std::vector<std::uint8_t> file;
	{   // the font BRRES inside the executable (DOL 0x80442bc0)
		const wii::Dol* dol = wii::GameDol();
		if (dol == nullptr) return false;
		const std::uint8_t* p = dol->At(kFontBres, 0x10);
		if (p == nullptr) return false;
		const std::size_t size = ((std::size_t)p[8] << 24) | (p[9] << 16) | (p[10] << 8) | p[11];
		p = dol->At(kFontBres, size);
		if (p == nullptr) return false;
		file.assign(p, p + size);
	}
	if (!b.Load(file.data(), file.size()) || !b.HasTexture("ascii") || !b.DecodeTexture("ascii", img)) return false;
	g_tex = gfx::CreateTexture(img.width, img.height, img.rgba.data());
	return g_tex != gfx::NO_TEX;
}
}

// DOL 0x8013be34: the string is formatted into the queue (entries {x, y, color, text} in a 0x3000 byte buffer, 4 byte aligned,
// text NUL terminated); when the buffer is full the call is dropped.
void Print(int x, int y, std::uint32_t color, const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	std::vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);
	const std::size_t need = 12 + ((std::strlen(buf) + 1 + 3) & ~3u);
	if (g_bytes + need >= (std::size_t)kQueueBytes) return;
	g_bytes += need;
	g_queue.push_back({x, y, color, buf});
}

// DOL 0x8013c3d4 per entry: 4 vertices per glyph, u = col * 8 / 128, v = row * 8 / 64
void Flush()
{
	if (!g_queue.empty() && EnsureFont())
	{
		for (const Entry& e : g_queue)
		{
			int cx = e.x, cy = e.y;
			const std::uint8_t r = (std::uint8_t)(e.color >> 24), g = (std::uint8_t)(e.color >> 16), b = (std::uint8_t)(e.color >> 8), a = (std::uint8_t)e.color;
			for (unsigned char c : e.text)
			{
				if (c > 0x7f) c = '?';
				if (c == '\n') { cx = e.x; cy += 16; continue; }
				const int col = c & 15, row = c >> 4;
				const float u0 = col * 8 / 128.0f, u1 = (col * 8 + 8) / 128.0f, v0 = row * 8 / 64.0f, v1 = (row * 8 + 8) / 64.0f;
				// game screen space is centre origin, y up: x - 320, 240 - y; corners TL, BL, TR, BR
				const float x0 = (float)cx - 320, x1 = (float)cx + 16 - 320, y0 = 240.0f - cy, y1 = 240.0f - (cy + 16);
				const float xy[8] = {x0, y0, x0, y1, x1, y0, x1, y1};
				const float uv[8] = {u0, v0, u0, v1, u1, v0, u1, v1};
				gfx::DrawQuad(g_tex, xy, uv, r, g, b, a, gfx::BLEND_ALPHA, false);
				cx += 16;
			}
		}
	}
	g_queue.clear();
	g_bytes = 0;
}
}
