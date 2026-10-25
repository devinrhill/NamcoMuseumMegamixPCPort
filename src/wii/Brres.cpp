#include "wii/Brres.hpp"

#include <cstdio>
#include <cstring>

namespace wii
{

namespace
{
struct BlockInfo { int bw, bh, bytes; };

bool Info(int fmt, BlockInfo& b)
{
	switch (fmt)
	{
	case GX_I4: b = {8, 8, 32}; return true;
	case GX_I8: b = {8, 4, 32}; return true;
	case GX_IA4: b = {8, 4, 32}; return true;
	case GX_IA8: b = {4, 4, 32}; return true;
	case GX_RGB565: b = {4, 4, 32}; return true;
	case GX_RGB5A3: b = {4, 4, 32}; return true;
	case GX_RGBA32: b = {4, 4, 64}; return true;
	case GX_C4: b = {8, 8, 32}; return true;
	case GX_C8: b = {8, 4, 32}; return true;
	case GX_C14X2: b = {4, 4, 32}; return true;
	case GX_CMPR: b = {8, 8, 32}; return true;
	default: return false;
	}
}

inline std::uint8_t Expand(unsigned v, int bits)
{
	return bits < 8 ? (std::uint8_t)((v << (8 - bits)) | (v >> (2 * bits - 8))) : (std::uint8_t)v;
}

inline void RGB565(unsigned v, std::uint8_t* o)
{
	o[0] = Expand((v >> 11) & 0x1f, 5);
	o[1] = Expand((v >> 5) & 0x3f, 6);
	o[2] = Expand(v & 0x1f, 5);
	o[3] = 255;
}

inline void RGB5A3(unsigned v, std::uint8_t* o)
{
	if (v & 0x8000)
	{
		o[0] = Expand((v >> 10) & 0x1f, 5);
		o[1] = Expand((v >> 5) & 0x1f, 5);
		o[2] = Expand(v & 0x1f, 5);
		o[3] = 255;
	}
	else
	{
		unsigned a = (v >> 12) & 7;
		o[0] = Expand((v >> 8) & 0xf, 4);
		o[1] = Expand((v >> 4) & 0xf, 4);
		o[2] = Expand(v & 0xf, 4);
		o[3] = (std::uint8_t)((a << 5) | (a << 2) | (a >> 1));
	}
}
}

std::size_t GxLevelSize(int fmt, int w, int h)
{
	BlockInfo b;
	if (!Info(fmt, b)) return 0;
	return (std::size_t)((w + b.bw - 1) / b.bw) * ((h + b.bh - 1) / b.bh) * b.bytes;
}

std::size_t GxImageSize(int fmt, int w, int h, int levels)
{
	std::size_t total = 0;
	for (int i = 0; i < levels; i++)
	{
		total += GxLevelSize(fmt, w >> i ? w >> i : 1, h >> i ? h >> i : 1);
	}
	return total;
}

bool GxDecodePalette(int tlutFmt, const std::uint8_t* d, std::size_t avail, int count, std::vector<std::uint8_t>& rgba)
{
	if (avail < (std::size_t)count * 2) return false;
	rgba.assign((std::size_t)count * 4, 0);
	for (int i = 0; i < count; i++)
	{
		unsigned v = (d[i * 2] << 8) | d[i * 2 + 1];
		std::uint8_t* o = &rgba[(std::size_t)i * 4];
		switch (tlutFmt)
		{
		case GX_TL_IA8: o[0] = o[1] = o[2] = v & 0xff; o[3] = v >> 8; break;
		case GX_TL_RGB565: RGB565(v, o); break;
		case GX_TL_RGB5A3: RGB5A3(v, o); break;
		default: return false;
		}
	}
	return true;
}

bool GxDecode(int fmt, const std::uint8_t* d, std::size_t avail, int w, int h, const std::vector<std::uint8_t>* pal, Image& out)
{
	BlockInfo b;
	if (!Info(fmt, b) || w <= 0 || h <= 0 || avail < GxLevelSize(fmt, w, h)) return false;
	const bool ci = fmt == GX_C4 || fmt == GX_C8 || fmt == GX_C14X2;
	if (ci && pal == nullptr) return false;
	out.width = w;
	out.height = h;
	out.rgba.assign((std::size_t)w * h * 4, 0);

	auto lookup = [&](unsigned idx, std::uint8_t* o) {
		if (idx * 4 + 3 < pal->size()) std::memcpy(o, &(*pal)[idx * 4], 4);
		else std::memset(o, 0, 4);
	};

	std::size_t pos = 0;
	if (fmt == GX_CMPR)
	{
		for (int by = 0; by < h; by += 8)
			for (int bx = 0; bx < w; bx += 8)
				for (int sy = 0; sy < 8; sy += 4)
					for (int sx = 0; sx < 8; sx += 4)
					{
						const std::uint8_t* c = d + pos;
						pos += 8;
						unsigned c0 = (c[0] << 8) | c[1], c1 = (c[2] << 8) | c[3];
						std::uint8_t p[4][4];
						RGB565(c0, p[0]);
						RGB565(c1, p[1]);
						if (c0 > c1)
						{
							for (int k = 0; k < 3; k++)
							{
								p[2][k] = (std::uint8_t)((2 * p[0][k] + p[1][k]) / 3);
								p[3][k] = (std::uint8_t)((p[0][k] + 2 * p[1][k]) / 3);
							}
							p[2][3] = p[3][3] = 255;
						}
						else
						{
							for (int k = 0; k < 3; k++) p[2][k] = (std::uint8_t)((p[0][k] + p[1][k]) / 2);
							p[2][3] = 255;
							p[3][0] = p[3][1] = p[3][2] = p[3][3] = 0;
						}
						for (int y = 0; y < 4; y++)
						{
							unsigned row = c[4 + y];
							for (int x = 0; x < 4; x++)
							{
								int px = bx + sx + x, py = by + sy + y;
								if (px < w && py < h)
								{
									std::memcpy(&out.rgba[((std::size_t)py * w + px) * 4], p[(row >> (6 - 2 * x)) & 3], 4);
								}
							}
						}
					}
		return true;
	}

	for (int by = 0; by < h; by += b.bh)
	{
		for (int bx = 0; bx < w; bx += b.bw)
		{
			const std::uint8_t* blk = d + pos;
			pos += b.bytes;
			for (int y = 0; y < b.bh; y++)
			{
				int py = by + y;
				if (py >= h) continue;
				for (int x = 0; x < b.bw; x++)
				{
					int px = bx + x;
					if (px >= w) continue;
					std::uint8_t* o = &out.rgba[((std::size_t)py * w + px) * 4];
					switch (fmt)
					{
					case GX_RGBA32: // AR in the first 32 bytes, GB in the second
					{
						int i = (y * 4 + x) * 2;
						o[0] = blk[i + 1]; o[1] = blk[32 + i]; o[2] = blk[32 + i + 1]; o[3] = blk[i];
						break;
					}
					case GX_I4:
					{
						unsigned v = blk[(y * 8 + x) / 2];
						v = (x % 2 == 0) ? (v >> 4) : (v & 0xf);
						v = (v << 4) | v;
						o[0] = o[1] = o[2] = o[3] = (std::uint8_t)v;
						break;
					}
					case GX_I8: o[0] = o[1] = o[2] = o[3] = blk[y * 8 + x]; break;
					case GX_IA4:
					{
						unsigned v = blk[y * 8 + x];
						o[0] = o[1] = o[2] = (std::uint8_t)((v & 0xf) * 17);
						o[3] = (std::uint8_t)((v >> 4) * 17);
						break;
					}
					case GX_IA8:
					{
						int i = (y * 4 + x) * 2;
						o[0] = o[1] = o[2] = blk[i + 1];
						o[3] = blk[i];
						break;
					}
					case GX_RGB565: RGB565((blk[(y * 4 + x) * 2] << 8) | blk[(y * 4 + x) * 2 + 1], o); break;
					case GX_RGB5A3: RGB5A3((blk[(y * 4 + x) * 2] << 8) | blk[(y * 4 + x) * 2 + 1], o); break;
					case GX_C4:
					{
						unsigned v = blk[(y * 8 + x) / 2];
						lookup((x % 2 == 0) ? (v >> 4) : (v & 0xf), o);
						break;
					}
					case GX_C8: lookup(blk[y * 8 + x], o); break;
					case GX_C14X2:
					{
						int i = (y * 4 + x) * 2;
						lookup(((blk[i] << 8) | blk[i + 1]) & 0x3fff, o);
						break;
					}
					}
				}
			}
		}
	}
	return true;
}

// ---- BRRES -----------------------------------------------------------------------------------------------------------

std::uint32_t Brres::U32(std::uint32_t o) const
{
	if (o + 4 > m_data.size()) return 0;
	const std::uint8_t* p = &m_data[o];
	return m_be ? ((std::uint32_t)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]
	            : ((std::uint32_t)p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
}

std::uint16_t Brres::U16(std::uint32_t o) const
{
	if (o + 2 > m_data.size()) return 0;
	const std::uint8_t* p = &m_data[o];
	return m_be ? (std::uint16_t)((p[0] << 8) | p[1]) : (std::uint16_t)((p[1] << 8) | p[0]);
}

// BinString: the offset points at the characters, the u32 length sits 4 bytes before
std::string Brres::ReadBinString(std::uint32_t offset) const
{
	if (offset < 4 || offset > m_data.size()) return std::string();
	std::uint32_t len = U32(offset - 4);
	if (offset + len > m_data.size()) return std::string();
	return std::string(reinterpret_cast<const char*>(&m_data[offset]), len);
}

bool Brres::ParseDic(std::uint32_t off, Dic& dic) const
{
	if (off + 8 > m_data.size()) return false;
	std::uint32_t num = U32(off + 4);
	if ((std::uint64_t)off + 8 + 0x10ull * (num + 1) > m_data.size()) return false;
	dic.entries.clear();
	for (std::uint32_t i = 1; i <= num; i++) // node 0 is the trie root
	{
		std::uint32_t n = off + 8 + 0x10 * i;
		std::int32_t nameOff = (std::int32_t)U32(n + 8), dataOff = (std::int32_t)U32(n + 12);
		dic.entries.emplace_back(ReadBinString(off + nameOff), off + dataOff);
	}
	return true;
}

bool Brres::Load(const std::uint8_t* data, std::size_t size)
{
	m_data.assign(data, data + size);
	m_tex.clear();
	m_plt.clear();
	m_mdl.clear();
	m_mdlNames.clear();
	m_texNames.clear();
	m_folders.clear();
	if (size < 0x20 || std::memcmp(data, "bres", 4) != 0) return false;
	unsigned order = (data[4] << 8) | data[5];
	if (order == 0xfeff) m_be = true;
	else if (order == 0xfffe) m_be = false;
	else return false;

	std::uint32_t rootOff = U16(0xc);
	if (rootOff + 8 > size || std::memcmp(data + rootOff, "root", 4) != 0) return false;

	Dic folders;
	if (!ParseDic(rootOff + 8, folders)) return false;
	for (const auto& f : folders.entries)
	{
		const bool isTex = f.first == "Textures(NW4R)", isPlt = f.first == "Palettes(NW4R)", isMdl = f.first == "3DModels(NW4R)";
		Dic dic;
		if (!ParseDic(f.second, dic)) return false;
		m_folders[f.first] = dic.entries;
		if (!isTex && !isPlt && !isMdl) continue;
		for (const auto& e : dic.entries)
		{
			if (isTex)
			{
				m_tex[e.first] = e.second;
				m_texNames.push_back(e.first);
			}
			else if (isMdl)
			{
				m_mdl[e.first] = e.second;
				m_mdlNames.push_back(e.first);
			}
			else
			{
				m_plt[e.first] = e.second;
			}
		}
	}
	return true;
}

bool Brres::DecodeTexture(const std::string& name, Image& out, int level) const
{
	auto it = m_tex.find(name);
	if (it == m_tex.end()) return false;
	const std::uint32_t o = it->second;
	if (o + 0x34 > m_data.size() || std::memcmp(&m_data[o], "TEX0", 4) != 0) return false;
	const std::uint32_t size = U32(o + 4);
	const std::int32_t toData = (std::int32_t)U32(o + 0x10);
	const std::uint32_t flag = U32(o + 0x18);
	const int w = U16(o + 0x1c), h = U16(o + 0x1e), fmt = (int)U32(o + 0x20);
	int mips = (int)U32(o + 0x24);
	if (mips < 1) mips = 1;
	if (level < 0 || level >= mips || toData < 0) return false;
	const std::size_t want = GxImageSize(fmt, w, h, mips);
	if (want == 0 || (std::size_t)toData + want > size || o + toData + want > m_data.size()) return false;

	std::size_t levelOff = level ? GxImageSize(fmt, w, h, level) : 0;
	int lw = w >> level ? w >> level : 1, lh = h >> level ? h >> level : 1;

	std::vector<std::uint8_t> palette;
	const std::vector<std::uint8_t>* pal = nullptr;
	if (flag & 1) // color indexed: the palette of the same name
	{
		auto pt = m_plt.find(name);
		if (pt == m_plt.end()) return false;
		const std::uint32_t p = pt->second;
		if (p + 0x24 > m_data.size() || std::memcmp(&m_data[p], "PLT0", 4) != 0) return false;
		const std::int32_t pData = (std::int32_t)U32(p + 0x10);
		const int pFmt = (int)U32(p + 0x18), pCount = U16(p + 0x1c);
		if (pData < 0 || p + pData + (std::size_t)pCount * 2 > m_data.size()) return false;
		if (!GxDecodePalette(pFmt, &m_data[p + pData], (std::size_t)pCount * 2, pCount, palette)) return false;
		pal = &palette;
	}
	return GxDecode(fmt, &m_data[o + toData + levelOff], want - levelOff, lw, lh, pal, out);
}
}
