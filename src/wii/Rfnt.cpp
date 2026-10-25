#include "wii/Rfnt.hpp"

#include <cstring>

namespace wii
{
namespace
{
std::uint16_t R16(const std::uint8_t* p) { return (std::uint16_t)((p[0] << 8) | p[1]); }
std::uint32_t R32(const std::uint8_t* p) { return ((std::uint32_t)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]; }
}

bool Rfnt::Load(const std::uint8_t* d, std::size_t size)
{
	if (size < 0x30 || std::memcmp(d, "RFNT", 4) != 0 || R16(d + 4) != 0xfeff) return false;
	const std::size_t finf = R16(d + 12);   // header size: first block
	if (finf + 0x20 > size || std::memcmp(d + finf, "FINF", 4) != 0) return false;
	const std::uint8_t* f = d + finf + 8;
	m_linefeed = (std::int8_t)f[1];
	m_alter = R16(f + 2);
	m_defaultWidth = {(std::int8_t)f[4], f[5], (std::int8_t)f[6]};
	m_encoding = f[7];
	const std::uint32_t pGlyph = R32(f + 8), pWidth = R32(f + 12), pMap = R32(f + 16);
	m_height = f[20];
	m_ascent = f[22];

	// TGLP body
	if (pGlyph + 0x10 + 4 > size) return false;
	const std::uint8_t* g = d + pGlyph;
	m_cellW = g[0]; m_cellH = g[1]; m_baseline = (std::int8_t)g[2]; m_maxCharWidth = g[3];
	m_sheetSize = R32(g + 4);
	const int sheetNum = R16(g + 8);
	m_sheetFmt = R16(g + 10) & 0x7fff;
	m_sheetRow = R16(g + 12); m_sheetLine = R16(g + 14);
	m_sheetW = R16(g + 16); m_sheetH = R16(g + 18);
	const std::uint32_t image = R32(g + 20);
	if (m_sheetRow == 0 || m_sheetLine == 0 || (std::size_t)image + (std::size_t)sheetNum * m_sheetSize > size) return false;
	for (int i = 0; i < sheetNum; i++)
		m_sheets.emplace_back(d + image + (std::size_t)i * m_sheetSize, d + image + (std::size_t)(i + 1) * m_sheetSize);
	m_decoded.resize(sheetNum);
	m_done.assign(sheetNum, false);

	// CWDH chain
	for (std::uint32_t p = pWidth; p != 0;)
	{
		if (p + 8 > size) return false;
		Width w;
		w.begin = R16(d + p); w.end = R16(d + p + 2);
		const std::uint32_t next = R32(d + p + 4);
		if (w.end < w.begin || p + 8 + 3 * (std::size_t)(w.end - w.begin + 1) > size) return false;
		for (int i = 0; i <= w.end - w.begin; i++)
		{
			const std::uint8_t* c = d + p + 8 + 3 * i;
			w.table.push_back({(std::int8_t)c[0], c[1], (std::int8_t)c[2]});
		}
		m_widths.push_back(std::move(w));
		p = next;
	}
	// CMAP chain
	for (std::uint32_t p = pMap; p != 0;)
	{
		if (p + 12 > size) return false;
		Map m;
		m.begin = R16(d + p); m.end = R16(d + p + 2); m.method = R16(d + p + 4);
		const std::uint32_t next = R32(d + p + 8);
		std::size_t words = 0;
		if (m.method == 0) words = 1;
		else if (m.method == 1) words = (std::size_t)m.end - m.begin + 1;
		else if (m.method == 2) { if (p + 14 > size) return false; words = 1 + 2 * (std::size_t)R16(d + p + 12); }
		else return false;
		if (p + 12 + 2 * words > size) return false;
		for (std::size_t i = 0; i < words; i++) m.info.push_back(R16(d + p + 12 + 2 * i));
		m_maps.push_back(std::move(m));
		p = next;
	}
	return true;
}

std::uint16_t Rfnt::FindGlyphIndex(std::uint32_t c) const
{
	for (const Map& m : m_maps)
	{
		if (c < m.begin || c > m.end) continue;
		switch (m.method)
		{
		case 0: return (std::uint16_t)(c - m.begin + m.info[0]);
		case 1: return m.info[c - m.begin];
		case 2:
		{
			const int n = m.info[0];
			int lo = 0, hi = n - 1;
			while (lo <= hi)
			{
				const int mid = (lo + hi) / 2;
				const std::uint16_t cc = m.info[1 + 2 * mid];
				if (cc < c) lo = mid + 1;
				else if (c < cc) hi = mid - 1;
				else return m.info[2 + 2 * mid];
			}
			return NOT_FOUND;
		}
		}
	}
	return NOT_FOUND;
}

RfntCharWidths Rfnt::WidthsOfIndex(std::uint16_t index) const
{
	for (const Width& w : m_widths)
		if (w.begin <= index && index <= w.end) return w.table[index - w.begin];
	return m_defaultWidth;
}

RfntGlyph Rfnt::GetGlyph(std::uint32_t code) const
{
	std::uint16_t index = FindGlyphIndex(code);
	if (index == NOT_FOUND) index = m_alter;
	const int perSheet = m_sheetRow * m_sheetLine;
	RfntGlyph g;
	g.sheet = index / perSheet;
	const int cell = index % perSheet;
	g.cellX = (cell % m_sheetRow) * (m_cellW + 1) + 1;
	g.cellY = (cell / m_sheetRow) * (m_cellH + 1) + 1;
	g.widths = WidthsOfIndex(index);
	return g;
}

const Image& Rfnt::Sheet(int n) const
{
	static const Image empty;
	if (n < 0 || n >= (int)m_sheets.size()) return empty;
	if (!m_done[n])
	{
		m_done[n] = true;
		GxDecode(m_sheetFmt, m_sheets[n].data(), m_sheets[n].size(), m_sheetW, m_sheetH, nullptr, m_decoded[n]);
	}
	return m_decoded[n];
}
}
