#include "wii/Archive.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>

#include "WiiSystem.hpp"

namespace wii
{

std::uint32_t Crc32(const void* data, std::size_t size)
{
	static std::uint32_t table[256];
	static bool ready = false;
	if (!ready)
	{
		for (std::uint32_t i = 0; i < 256; i++)
		{
			std::uint32_t c = i;
			for (int k = 0; k < 8; k++) c = (c & 1) ? 0xedb88320u ^ (c >> 1) : c >> 1;
			table[i] = c;
		}
		ready = true;
	}
	std::uint32_t crc = 0xffffffffu;
	const std::uint8_t* p = static_cast<const std::uint8_t*>(data);
	for (std::size_t i = 0; i < size; i++) crc = table[(crc ^ p[i]) & 0xff] ^ (crc >> 8);
	return ~crc;
}

std::uint32_t Crc32(const std::string& s)
{
	return Crc32(s.data(), s.size());
}

std::size_t Unlzss(const std::uint8_t* src, std::size_t srcLen, std::uint8_t* dst, std::size_t dstLen)
{
	constexpr int EI = 12, EJ = 4, P = 2;
	constexpr int N = 1 << EI, F = 1 << EJ;
	std::uint8_t window[N] = {};
	int r = (N - F) - P;
	const std::uint8_t* end = src + srcLen;
	std::size_t out = 0;
	unsigned flags = 0;
	for (;; flags >>= 1)
	{
		if (!(flags & 0x100))
		{
			if (src >= end) break;
			flags = *src++ | 0xff00;
		}
		if (flags & 1)
		{
			if (src >= end || out >= dstLen) break;
			std::uint8_t c = *src++;
			dst[out++] = c;
			window[r] = c;
			r = (r + 1) & (N - 1);
		}
		else
		{
			if (src + 1 >= end) break;
			int lo = *src++;
			int hi = *src++;
			int pos = lo | ((hi >> EJ) << 8);
			int len = (hi & (F - 1)) + P;
			for (int k = 0; k <= len; k++)
			{
				if (out >= dstLen) return out;
				std::uint8_t c = window[(pos + k) & (N - 1)];
				dst[out++] = c;
				window[r] = c;
				r = (r + 1) & (N - 1);
			}
		}
	}
	return out;
}

static std::uint32_t Le32(const std::uint8_t* p)
{
	return p[0] | (p[1] << 8) | (p[2] << 16) | (std::uint32_t(p[3]) << 24);
}

bool Archive::LoadFile(const std::string& path)
{
	std::ifstream f(path, std::ios::binary);
	if (!f)
	{
		return false;
	}
	std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	return LoadBytes(std::move(bytes));
}

bool Archive::LoadBytes(std::vector<std::uint8_t> bytes)
{
	m_data.clear();
	m_entries.clear();
	m_byCrc.clear();

	if (bytes.size() >= 16 && std::memcmp(bytes.data(), "SSZL", 4) == 0)
	{
		std::uint32_t codeSize = Le32(&bytes[8]), textSize = Le32(&bytes[12]);
		if (Le32(&bytes[4]) != 0 || codeSize > bytes.size() - 16)
		{
			std::fprintf(stderr, "archive: bad SSZL header\n");
			return false;
		}
		m_data.resize(textSize);
		if (Unlzss(&bytes[16], codeSize, m_data.data(), textSize) != textSize)
		{
			std::fprintf(stderr, "archive: SSZL size mismatch\n");
			return false;
		}
	}
	else
	{
		m_data = std::move(bytes);
	}

	if (m_data.size() < 0x40 || std::memcmp(m_data.data(), "VCRA", 4) != 0)
	{
		std::fprintf(stderr, "archive: not a VCRA container\n");
		return false;
	}
	std::uint32_t count = Le32(&m_data[4]);
	if (m_data.size() < 0x40 + std::size_t(count) * 0x40)
	{
		std::fprintf(stderr, "archive: truncated entry table\n");
		return false;
	}
	for (std::uint32_t i = 0; i < count; i++)
	{
		const std::uint8_t* e = &m_data[0x40 + i * 0x40];
		Entry entry;
		entry.offset = Le32(e);
		entry.size = Le32(e + 4);
		entry.name.assign(reinterpret_cast<const char*>(e + 8), strnlen(reinterpret_cast<const char*>(e + 8), 0x38));
		if (std::size_t(entry.offset) + entry.size > m_data.size())
		{
			std::fprintf(stderr, "archive: entry '%s' out of bounds\n", entry.name.c_str());
			return false;
		}
		m_byCrc[Crc32(entry.name)] = m_entries.size();
		m_entries.push_back(entry);
	}
	return true;
}

const Archive::Entry* Archive::Find(std::uint32_t crc) const
{
	auto it = m_byCrc.find(crc);
	return it == m_byCrc.end() ? nullptr : &m_entries[it->second];
}

// ---- ArchiveStore ------------------------------------------------------------------------------

namespace
{
struct Attached
{
	std::string name;
	std::unique_ptr<Archive> archive;
};
std::vector<Attached>& Store()
{
	static std::vector<Attached> s;
	return s;
}
}

bool ArchiveStore::Attach(const std::string& relPath)
{
	for (const Attached& a : Store())
	{
		if (a.name == relPath) return true;
	}
	const std::string base = WiiSystem::DataRoot() + "/museum/";
	std::unique_ptr<Archive> arc(new Archive());
	if (!arc->LoadFile(base + WiiSystem::LanguageDir() + "/" + relPath) && !arc->LoadFile(base + relPath))
	{
		std::fprintf(stderr, "AttachArcData: cannot load '%s' under %s\n", relPath.c_str(), base.c_str());
		return false;
	}
	Store().push_back({relPath, std::move(arc)});
	return true;
}

std::string ArchiveStore::ResolvePath(const std::string& relPath)
{
	const std::string base = WiiSystem::DataRoot() + "/museum/";
	for (const std::string& p : {base + WiiSystem::LanguageDir() + "/" + relPath, base + relPath})
	{
		std::ifstream f(p, std::ios::binary);
		if (f) return p;
	}
	return std::string();
}

bool ArchiveStore::Detach(const std::string& relPath)
{
	auto& s = Store();
	for (auto it = s.begin(); it != s.end(); ++it)
	{
		if (it->name == relPath)
		{
			s.erase(it);
			return true;
		}
	}
	return false;
}

const std::uint8_t* ArchiveStore::Find(std::uint32_t crc, std::size_t& size)
{
	auto& s = Store();
	for (auto it = s.rbegin(); it != s.rend(); ++it) // most recently attached first
	{
		if (const Archive::Entry* e = it->archive->Find(crc))
		{
			size = e->size;
			return it->archive->Data(*e);
		}
	}
	size = 0;
	return nullptr;
}

const std::uint8_t* ArchiveStore::Find(const std::string& name, std::size_t& size)
{
	return Find(Crc32(name), size);
}

}
