#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Wii disc archives as the game reads them: "SSZL" (Okumura LZSS) wrapping a "VCRA" container whose
// entries are looked up by CRC32 of the entry name (e.g. crc32("MAINTITLE_ANITA") == 0x706a47ba).
namespace wii
{
std::uint32_t Crc32(const void* data, std::size_t size);
std::uint32_t Crc32(const std::string& s);

// 4096 byte ring, zero initialised, write cursor at 4078, flag bit 1 = literal, match = 12 bit window
// position + 4 bit length (count = low nibble + 3). Returns the number of bytes produced.
std::size_t Unlzss(const std::uint8_t* src, std::size_t srcLen, std::uint8_t* dst, std::size_t dstLen);

class Archive
{
public:
	struct Entry
	{
		std::string name;
		std::uint32_t offset{};
		std::uint32_t size{};
	};

	// Accepts a file that is SSZL compressed or a bare VCRA. Returns false (with a message on stderr) on error.
	bool LoadFile(const std::string& path);
	bool LoadBytes(std::vector<std::uint8_t> bytes);

	const Entry* Find(std::uint32_t crc) const;
	const Entry* Find(const std::string& name) const { return Find(Crc32(name)); }
	const std::uint8_t* Data(const Entry& e) const { return m_data.data() + e.offset; }
	const std::vector<Entry>& Entries() const { return m_entries; }

private:
	std::vector<std::uint8_t> m_data;
	std::vector<Entry> m_entries;
	std::unordered_map<std::uint32_t, std::size_t> m_byCrc;
};

// The set of archives currently "attached" (cKingdomSys::AttachArcData). Later attachments win.
class ArchiveStore
{
public:
	// `relPath` as the game writes it, e.g. "g10_menu/g10title.lzs". Resolved under <data>/museum/<lang>/ then <data>/museum/.
	static bool Attach(const std::string& relPath);
	static bool Detach(const std::string& relPath);
	// Path of a disc file (g10_menu/attract.thp ...) the way Attach looks for it: museum/<lang>/<rel>, then museum/<rel>. Empty if neither exists.
	static std::string ResolvePath(const std::string& relPath);

	// Bytes of the entry with this name / CRC32, or nullptr. Valid until the owning archive is detached.
	static const std::uint8_t* Find(const std::string& name, std::size_t& size);
	static const std::uint8_t* Find(std::uint32_t crc, std::size_t& size);
};
}
