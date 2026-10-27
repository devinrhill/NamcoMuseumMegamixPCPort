// Native stand-in for the gotch CDraw (see include/gotch/Draw.hpp): the game-facing calls follow the original; the GX flush is a software painter.
#include "gotch/Draw.hpp"

#include <algorithm>
#include <cstring>
#include <vector>

namespace gotch
{
namespace
{
std::uint32_t BE32(const std::uint8_t* p) { return (std::uint32_t)p[0] << 24 | (std::uint32_t)p[1] << 16 | (std::uint32_t)p[2] << 8 | p[3]; }
}

CDraw::CDraw(int screenW, int screenH) : m_w(screenW), m_h(screenH), m_frame((std::size_t)screenW * screenH, 0xff000000u) {}

void CDraw::ClearFrame()
{
	std::fill(m_frame.begin(), m_frame.end(), 0xff000000u);
	m_depth.assign(m_frame.size(), 0x7fffffff);
}

// RSO2 0x80d3596c. The original packs the characters into one GX texture (4 bpp when at most 4 planes are used, else 8 bpp) and remembers each
// character's uv; the port keeps a plain array of colour indices per character.
int CDraw::AssignTextureAndConvertFromCGen(const CGENFORMAT& fmt, int count, const std::uint8_t* const* src, int add)
{
	Texture t;
	t.w = fmt.width;
	t.h = fmt.height;
	t.count = count;
	t.pix.assign((std::size_t)count * t.w * t.h, 0);
	for (int c = 0; c < count; c++)
	{
		for (int i = 0; i < t.w * t.h; i++)
		{
			const std::uint8_t* entry = fmt.layout + i * 0x20;
			unsigned v = 0;
			for (int plane = 0; plane < 8; plane++)
			{
				const std::uint32_t e = BE32(entry + plane * 4);
				const std::uint32_t srcIndex = e >> 28;
				if (srcIndex < 8)
				{
					// bit (e & 7) of byte (e >> 3) of this character; plane 0 is the top bit of the colour index
					const std::uint8_t b = src[srcIndex][((e >> 3) & 0x1ffffff) + fmt.charBytes * c];
					if (b & (1u << (e & 7))) v |= 0x80u >> plane;
				}
				else if (srcIndex != 0xf)
				{
					return -1;
				}
			}
			t.pix[(std::size_t)c * t.w * t.h + i] = (std::uint8_t)(add + v);
		}
	}
	m_tex.push_back(std::move(t));
	return (int)m_tex.size() - 1;
}

// RSO2 0x80d090c0 (the original swizzles the colours into its palette texture; n == 0 returns 1, index >= 0x100 returns 0)
int CDraw::SetPalet(int index, int start, const std::uint32_t* colours, int n)
{
	if (n == 0) return 1;
	if (index >= 0x100) return 0;
	for (int i = 0; i < n && start + i < 0x100; i++) m_palet[index][start + i] = colours[i];
	return 1;
}

std::uint32_t CDraw::GetPalet(int index, int colour) const { return m_palet[index & 0xff][colour & 0xff]; }

// RSO2 0x80d0adc8: an open batch is flushed first. Mode bit 0 = alpha test (GX_GEQUAL 1); the other bits (2 = indirect palette look-up,
// 0x20 = vertex colour, ...) change GX state only
void CDraw::Begin(std::uint32_t mode)
{
	if (m_open) Flush();
	m_mode = mode;
	m_open = true;
	m_alphaTest = (mode & 1) != 0;
}

// RSO2 0x80d0aef4
void CDraw::End()
{
	if (m_open)
	{
		Flush();
		m_open = false;
	}
}

// RSO2 0x80d0b0d4 (the branch for +0x141224 == 0; the other branch flips the uv instead of negating the width: same picture)
int CDraw::DrawChrEx(float scaleX, float scaleY, int x, int y, int z, int tex, std::uint32_t code, int palette, std::uint32_t flip, std::uint32_t mirror)
{
	(void)scaleX;  // UNVERIFIED: every caller ported so far passes 1.0 (RSO2 0x80dc8628); the original stores both into the queue record
	(void)scaleY;
	if (tex < 0 || tex >= (int)m_tex.size()) return 1;
	const Texture& t = m_tex[tex];
	Quad q;
	q.x = (std::int16_t)x;
	q.y = (std::int16_t)y;
	q.z = z;
	q.w = t.w;
	q.h = t.h;
	q.tex = tex;
	q.code = (int)code;
	q.palette = palette;
	q.flipU = (flip & 1) != 0;
	q.flipV = (flip & 2) != 0;
	q.blend = (int)((flip >> 8) & 3);
	q.order = (std::uint32_t)m_queue.size();
	if (mirror & 1)
	{
		q.x = m_w - q.w - q.x;
		q.flipU = !q.flipU;
	}
	if (mirror & 2)
	{
		q.y = m_h - q.h - q.y;
		q.flipV = !q.flipV;
	}
	m_queue.push_back(q);
	return 1;
}

// RSO2 0x80d0a2d0: GX draws the queue grouped by texture with z = prio - 0x8000 and a depth test. Here: far (large z) first, then nearer ones on top
// (UNVERIFIED: the compare function, read from +0x14119c); queue order among equal z. Blend mode 0 is taken as "replace where alpha != 0"
// (UNVERIFIED for modes 1..3, unused by the ported games so far).
void CDraw::Flush()
{
	std::stable_sort(m_queue.begin(), m_queue.end(), [](const Quad& a, const Quad& b) { return a.z > b.z; });
	// Begin without the alpha test (bit 0) and without the blend (bit 1): GX writes every texel, a zero-alpha one as its (black) colour (RALLYX's
	// side panel tiles hide the playfield this way; UNVERIFIED against the module's TEV setup)
	const bool opaque = !m_alphaTest && (m_mode & 2) == 0;
	for (const Quad& q : m_queue)
	{
		const Texture& t = m_tex[q.tex];
		if (q.code < 0 || q.code >= t.count) continue;
		const std::uint8_t* pix = &t.pix[(std::size_t)q.code * t.w * t.h];
		for (int ty = 0; ty < q.h; ty++)
		{
			const int sy = q.y + ty;
			if (sy < 0 || sy >= m_h) continue;
			const int vy = q.flipV ? q.h - 1 - ty : ty;
			for (int tx = 0; tx < q.w; tx++)
			{
				const int sx = q.x + tx;
				if (sx < 0 || sx >= m_w) continue;
				const int ux = q.flipU ? q.w - 1 - tx : tx;
				const std::uint32_t c = m_palet[q.palette & 0xff][pix[vy * t.w + ux]];
				if ((c >> 24) == 0 && !opaque) continue;  // alpha test GEQUAL 1 (and the blend's zero-alpha texels)
				if (m_depthTest)
				{
					int& d = m_depth[(std::size_t)sy * m_w + sx];
					if (q.z > d) continue;
					d = q.z;
				}
				m_frame[(std::size_t)sy * m_w + sx] = c | 0xff000000u;
			}
		}
	}
	m_queue.clear();
}

// RSO2 0x80d0b618 (type 0x98, indexed colours: the module looks the colour up in the palette texture, GX blends it with RenderModeSet)
void CDraw::DrawPoints(const int* xy, int n, int palette, int colour)
{
	if (n < 4) return;
	int x0 = xy[0], x1 = xy[0], y0 = xy[1], y1 = xy[1];
	for (int i = 1; i < 4; i++)
	{
		x0 = std::min(x0, xy[i * 2]);
		x1 = std::max(x1, xy[i * 2]);
		y0 = std::min(y0, xy[i * 2 + 1]);
		y1 = std::max(y1, xy[i * 2 + 1]);
	}
	const std::uint32_t c = m_palet[palette & 0xff][colour & 0xff];
	const std::uint32_t a = c >> 24;
	if (a == 0) return;
	x0 = std::max(x0, 0);
	y0 = std::max(y0, 0);
	x1 = std::min(x1, m_w);
	y1 = std::min(y1, m_h);
	for (int y = y0; y < y1; y++)
	{
		for (int x = x0; x < x1; x++)
		{
			std::uint32_t& d = m_frame[(std::size_t)y * m_w + x];
			std::uint32_t o = 0xff000000u;
			for (int sh = 0; sh < 24; sh += 8)
			{
				const std::uint32_t s = (c >> sh) & 0xff, t = (d >> sh) & 0xff;
				o |= ((s * a + t * (255 - a)) / 255) << sh;
			}
			d = o;
		}
	}
}

// RSO2 0x80d3312c (screen spec +0x20 / +0x24 = the frame's width / height)
void CDraw::RotateDrawStringPos(int rot, int x, int y, int* ox, int* oy, std::uint32_t* mirror, std::uint32_t* flip) const
{
	if (rot == 0x10e)
	{
		*mirror = 0;
		*ox = y;
		*oy = (m_h - x) - 8;
	}
	else if (rot == 0xb4)
	{
		*mirror = 3;
		*ox = x;
		*oy = y;
	}
	else if (rot == 0x5a)
	{
		*mirror = 2;
		*flip ^= 3;
		*ox = (m_w - y) - 8;
		*oy = (m_h - x) - 8;
	}
	else
	{
		*mirror = 0;
		*ox = x;
		*oy = y;
	}
}

// RSO2 0x80d334b8 (CUTF8::ConvertUCS2 0x80d30758 into a heap copy; a broken sequence ends the text there)
void CDraw::PrintfUTF8(const int* rect, int z, int tex, int palette, const char* s, std::uint32_t flags, int lineHeight, const std::uint16_t* conv,
                       int* out)
{
	std::vector<std::uint16_t> w(std::strlen(s) + 1, 0);
	std::uint16_t* o = w.data();
	const unsigned char* p = (const unsigned char*)s;
	for (;;)
	{
		const unsigned c = *p++;
		if (c == 0) break;
		if ((c & 0x80) == 0)
			*o = (std::uint16_t)c;
		else if ((c & 0xe0) == 0xc0)
		{
			if (*p == 0) break;
			*o = (std::uint16_t)(((c << 6) & 0x7c0) | (*p++ & 0x3f));
		}
		else if ((c & 0xf0) == 0xe0)
		{
			if (p[0] == 0 || p[1] == 0) break;
			*o = (std::uint16_t)((c << 12) | ((p[0] << 6) & 0xfc0) | (p[1] & 0x3f));
			p += 2;
		}
		else
			break;
		o++;
	}
	*o = 0;
	int r[4] = {rect[0], rect[1], rect[2], rect[3]};
	PrintfUCS2(r, z, tex, palette, w.data(), flags, lineHeight, conv, out);
}

namespace
{
// the accented letters (two characters: the base letter and the accent 8 pixels above): index into the table at RSO2 0x80dd27d8
int AccentIndex(unsigned c)
{
	static const unsigned kChars[21] = {0xc0, 0xc1, 0xc4, 0xc8, 0xc9, 0xcb, 0xcc, 0xcd, 0xcf, 0xd2, 0xd3,
	                                    0xd6, 0xd9, 0xda, 0xdc, 0x106, 0x139, 0x143, 0x154, 0x15a, 0x179};
	for (int i = 0; i < 21; i++)
		if (kChars[i] == c) return i;
	return -1;
}
// the single characters drawn with a fixed glyph of the font (0 = not one of them)
int FixedGlyph(unsigned c)
{
	switch (c)
	{
	case 0xa9: return 0x18;
	case 0xb0:
	case 0xba: return 0x8d;
	case 0xd7: return 0x92;
	case 0x201d: return 0x22;
	case 0x201f: return 0x10;
	case 0x2190: return 0x98;
	case 0x2191: return 0x95;
	case 0x2192: return 0x97;
	case 0x2193: return 0x96;
	case 0x25a1: return 0x91;
	case 0x25b3: return 0x90;
	case 0x25cb: return 0x8f;
	case 0xff2c: return 0x93;
	case 0xff32: return 0x94;
	default: return 0;
	}
}
}

// RSO2 0x80d33640
void CDraw::PrintfUCS2(const int* rect, int z, int tex, int palette, const std::uint16_t* s, std::uint32_t flags, int lineHeight,
                       const std::uint16_t* conv, int* out)
{
	const bool drawIt = (flags & 0x10000) == 0;
	const int rot = (int)(flags & 0xfff);
	const std::uint32_t fl = (flags >> 12) & 0xf00;
	const unsigned align = (flags >> 12) & 0xf;
	const float kScale = 1.0f;   // DAT_80dc82e8
	int top = rect[1];
	int minX = 0x7fffffff, minY = 0x7fffffff, maxX = -1, maxY = -1;
	auto lineStart = [&](const std::uint16_t* p) {
		int n = 0;
		while (p[n] != 0 && p[n] != 10) n++;
		const int width = (n * 8) & ~7;
		if (align == 1) return rect[2] - width;
		if (align == 2) return rect[0] + ((rect[2] - rect[0]) - width) / 2;
		return rect[0];   // 0 (and, uninitialised in the original, 3..15)
	};
	int x = lineStart(s);
	int y = top;
	bool newline = false;
	for (;;)
	{
		const unsigned c = *s++;
		minX = std::min(minX, x);
		minY = std::min(minY, y);
		bool end = false;
		std::uint32_t flip = fl, mirror = 0;
		int ox = 0, oy = 0;
		auto glyph = [&](unsigned code, std::uint32_t chrFlip) {
			RotateDrawStringPos(rot, x, y, &ox, &oy, &mirror, &flip);
			DrawChrEx(kScale, kScale, ox, oy, z, tex, code, palette, chrFlip == ~0u ? flip : chrFlip, mirror);
		};
		const int accent = AccentIndex(c);
		if (c == 0)
			end = true;
		else if (c == 10)
			newline = true;
		else if (c == 0x20)
			;
		else if (accent >= 0 && m_accent != nullptr)
		{
			if (drawIt)
			{
				const std::uint8_t* e = m_accent + accent * 12;
				unsigned base = BE32(e), acc = BE32(e + 4);
				const std::uint32_t accFlip = BE32(e + 8);
				if (conv != nullptr)
				{
					base = conv[base];
					acc = conv[acc];
				}
				std::uint32_t f2 = fl;
				RotateDrawStringPos(rot, x, y, &ox, &oy, &mirror, &f2);
				DrawChrEx(kScale, kScale, ox, oy, z, tex, base, palette, fl, mirror);
				RotateDrawStringPos(rot, x, y - 8, &ox, &oy, &mirror, &f2);
				DrawChrEx(kScale, kScale, ox, oy, z, tex, acc, palette, accFlip | fl, mirror);
			}
		}
		else if (c == 0xbf)
		{
			if (drawIt) glyph(0x3f, 2 | fl);   // an upside-down '?'
		}
		else if (FixedGlyph(c) != 0)
		{
			if (drawIt) glyph((unsigned)FixedGlyph(c), ~0u);
		}
		else if (((c - 0x21) & 0xffff) < 0x5f)
		{
			if (drawIt) glyph(c < 0x100 && conv != nullptr ? conv[c] : c, ~0u);
		}
		else if (drawIt)
			glyph(0x3f, ~0u);   // anything else is drawn as '?'
		if (!newline)
		{
			if (!end)
			{
				x += 8;
				maxX = std::max(maxX, x);
				maxY = std::max(maxY, y + lineHeight);
			}
		}
		else
		{
			top += lineHeight;
			x = lineStart(s);
			y = top;
			newline = false;
		}
		if (end)
		{
			if (out != nullptr)
			{
				out[0] = minX;
				out[1] = minY;
				out[2] = maxX;
				out[3] = maxY;
			}
			return;
		}
	}
}
}
