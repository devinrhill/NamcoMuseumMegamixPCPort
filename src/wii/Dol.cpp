#include "wii/Dol.hpp"

#include <cstdio>
#include <fstream>

#include "WiiSystem.hpp"

namespace wii
{
namespace
{
std::uint32_t Be32(const std::uint8_t* p) { return ((std::uint32_t)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]; }
}

// DOL header: 18 section offsets, 18 load addresses, 18 sizes (7 text, 11 data), all big endian
bool Dol::Load(const std::string& path)
{
	std::ifstream f(path, std::ios::binary | std::ios::ate);
	if (!f) return false;
	m_raw.resize((std::size_t)f.tellg());
	f.seekg(0);
	f.read((char*)m_raw.data(), m_raw.size());
	if (m_raw.size() < 0x100) { m_raw.clear(); return false; }
	m_sections.clear();
	for (int i = 0; i < 18; i++)
	{
		const std::uint32_t off = Be32(&m_raw[4 * i]), addr = Be32(&m_raw[0x48 + 4 * i]), size = Be32(&m_raw[0x90 + 4 * i]);
		if (size && (std::size_t)off + size <= m_raw.size()) m_sections.push_back({off, addr, size});
	}
	return true;
}

const std::uint8_t* Dol::At(std::uint32_t addr, std::size_t size) const
{
	for (const Section& s : m_sections)
		if (addr >= s.addr && (std::uint64_t)addr + size <= (std::uint64_t)s.addr + s.size) return &m_raw[s.offset + (addr - s.addr)];
	return nullptr;
}

const Dol* GameDol()
{
	static Dol dol;
	static bool tried = false;
	if (!tried)
	{
		tried = true;
		if (!dol.Load(WiiSystem::DolPath())) std::fprintf(stderr, "main.dol not found at '%s' (set dol_path in megamix.cfg): engine resources from the executable are unavailable\n", WiiSystem::DolPath().c_str());
	}
	return dol.loaded() ? &dol : nullptr;
}
}
