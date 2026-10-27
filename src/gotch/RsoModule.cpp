#include "gotch/RsoModule.hpp"

#include "WiiSystem.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>

namespace gotch
{
namespace
{
std::uint32_t U32(const std::vector<std::uint8_t>& d, std::size_t o)
{
	if (o + 4 > d.size()) return 0;
	return (std::uint32_t)d[o] << 24 | (std::uint32_t)d[o + 1] << 16 | (std::uint32_t)d[o + 2] << 8 | d[o + 3];
}
}

// RSO header (SDK rso.h): +0x08 section count, +0x0c section table (offset, size), +0x40 / +0x44 export table (16 B entries: name offset, offset in
// section, section, hash), +0x48 export names. Exports of the bss (no file offset) are skipped.
bool RsoModule::Load(const std::string& path)
{
	m_image.clear();
	m_exports.clear();
	m_pointers.clear();
	std::ifstream f(WiiSystem::DataRoot() + "/" + path, std::ios::binary);
	if (!f) return false;
	m_image.assign(std::istreambuf_iterator<char>(f), {});
	const std::uint32_t nsec = U32(m_image, 0x08), secTab = U32(m_image, 0x0c);
	const std::uint32_t expOff = U32(m_image, 0x40), expSize = U32(m_image, 0x44), expNames = U32(m_image, 0x48);
	if (nsec == 0 || nsec > 64 || secTab + nsec * 8 > m_image.size() || expOff + expSize > m_image.size())
	{
		m_image.clear();
		return false;
	}
	std::vector<std::uint32_t> secOff(nsec), secSize(nsec);
	for (std::uint32_t i = 0; i < nsec; i++)
	{
		secOff[i] = U32(m_image, secTab + i * 8);
		secSize[i] = U32(m_image, secTab + i * 8 + 4);
	}
	// sizes: distance to the next export of the same section (sorted by offset), the last one runs to the section end
	std::map<std::pair<int, std::uint32_t>, std::string> sorted;
	for (std::uint32_t i = 0; i < expSize / 16; i++)
	{
		const std::uint32_t e = expOff + i * 16;
		const std::uint32_t nameOff = U32(m_image, e), off = U32(m_image, e + 4), sec = U32(m_image, e + 8);
		if (sec >= nsec || secOff[sec] == 0 || expNames + nameOff >= m_image.size()) continue;
		const char* name = (const char*)&m_image[expNames + nameOff];
		sorted[{(int)sec, off}] = std::string(name, strnlen(name, m_image.size() - expNames - nameOff));
	}
	for (auto it = sorted.begin(); it != sorted.end(); ++it)
	{
		const int sec = it->first.first;
		const std::uint32_t off = it->first.second;
		auto next = std::next(it);
		const std::uint32_t end = (next != sorted.end() && next->first.first == sec) ? next->first.second : secSize[sec];
		if (secOff[sec] + off > m_image.size()) continue;
		Export x;
		x.data = &m_image[secOff[sec] + off];
		x.size = std::min<std::uint32_t>(end - off, (std::uint32_t)m_image.size() - secOff[sec] - off);
		x.section = sec;
		m_exports[it->second] = x;
	}
	// internal relocations (+0x30 / +0x34, 12 B entries: offset, section << 8 | type, addend); type 1 = R_PPC_ADDR32
	const std::uint32_t relOff = U32(m_image, 0x30), relSize = U32(m_image, 0x34);
	for (std::uint32_t i = 0; relOff + i * 12 + 12 <= m_image.size() && i < relSize / 12; i++)
	{
		const std::uint32_t e = relOff + i * 12;
		const std::uint32_t off = U32(m_image, e), info = U32(m_image, e + 4), add = U32(m_image, e + 8);
		const std::uint32_t sec = info >> 8;
		if ((info & 0xff) != 1 || sec >= nsec || secOff[sec] == 0) continue;
		m_pointers[off] = secOff[sec] + add;
	}
	return true;
}

const RsoModule::Export* RsoModule::Find(const std::string& name) const
{
	auto it = m_exports.find(name);
	return it == m_exports.end() ? nullptr : &it->second;
}

std::uint32_t RsoModule::Copy(const std::string& name, void* dst, std::uint32_t n) const
{
	const Export* e = Find(name);
	if (e == nullptr) return 0;
	const std::uint32_t k = std::min(n, e->size);
	std::memcpy(dst, e->data, k);
	return k;
}

const std::uint8_t* RsoModule::At(std::uint32_t addr, std::uint32_t n) const
{
	constexpr std::uint32_t kBase = 0x80d00000;
	if (addr < kBase || addr - kBase > m_image.size() || n > m_image.size() - (addr - kBase)) return nullptr;
	return &m_image[addr - kBase];
}

const char* RsoModule::Pointer(std::uint32_t addr) const
{
	auto it = m_pointers.find(addr - 0x80d00000);
	if (addr < 0x80d00000 || it == m_pointers.end() || it->second >= m_image.size()) return nullptr;
	return (const char*)&m_image[it->second];
}

std::uint32_t RsoModule::Word(std::uint32_t addr) const
{
	if (addr >= 0x80d00000)
	{
		auto it = m_pointers.find(addr - 0x80d00000);
		if (it != m_pointers.end()) return 0x80d00000 + it->second;
	}
	const std::uint8_t* p = At(addr, 4);
	return p == nullptr ? 0 : (std::uint32_t)p[0] << 24 | (std::uint32_t)p[1] << 16 | (std::uint32_t)p[2] << 8 | p[3];
}

const RsoModule& RsoModule::Volume(int vol)
{
	static RsoModule mods[2];
	static bool tried[2] = {};
	const int i = vol == 1 ? 0 : 1;
	if (!tried[i])
	{
		tried[i] = true;
		const std::string path = "museum/" + WiiSystem::LanguageDir() + "/rso_original/" + (i == 0 ? "nmwii.rso" : "nmwii2.rso");
		if (!mods[i].Load(path)) std::fprintf(stderr, "gotch: cannot read %s (arcade games of vol %d)\n", path.c_str(), i + 1);
	}
	return mods[i];
}
}
