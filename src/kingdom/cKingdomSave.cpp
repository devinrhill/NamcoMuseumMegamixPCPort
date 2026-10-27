#include "kingdom/cKingdomSave.hpp"

#include <cstring>

#include "kingdom/MsgTable.hpp"
#include "wii/Archive.hpp" // Crc32
#include "wii/Brres.hpp"
#include "wii/Nand.hpp"

namespace kingdom
{

namespace
{
constexpr const char* MUSEUM_BIN = "museum.bin";
constexpr const char* BANNER_BIN = "banner.bin";

uint ReadBE32(const byte* p)
{
	return ((uint)p[0] << 24) | ((uint)p[1] << 16) | ((uint)p[2] << 8) | p[3];
}

void WriteBE32(byte* p, uint v)
{
	p[0] = (byte)(v >> 24);
	p[1] = (byte)(v >> 16);
	p[2] = (byte)(v >> 8);
	p[3] = (byte)v;
}
}

// DOL 0x8000b490 (ctor: vtable, then 0x80022320). The checksum word starts as 0x20070813 (0x80022320); the banner is built by
// InitBanner, which the port calls once the resident archive is attached (boot.cpp Job128_Init).
cKingdomSave::cKingdomSave() : m_nChecksum(0x20070813)
{
}

namespace
{
// DOL 0x80026a98 -> 0x80026b08: UTF-8 (1..3 byte sequences) to UTF-16, at most `max` - 1 characters + a terminator, written big endian
// (the Wii's byte order; the DOL swaps only on a little-endian flag)
void Utf8ToUtf16Be(byte* dst, uint max, const std::string& src)
{
	uint i = 0, n = 0;
	while (i < src.size() && n + 1 < max)
	{
		const byte c = (byte)src[i];
		uint w;
		if ((c & 0x80) == 0) { w = c & 0x7f; i += 1; }
		else if ((c & 0xe0) == 0xc0) { w = ((c & 0x1f) << 6) | ((byte)src[i + 1] & 0x3f); i += 2; }
		else { w = ((c & 0xf) << 12) | (((byte)src[i + 1] & 0x3f) << 6) | ((byte)src[i + 2] & 0x3f); i += 3; }
		dst[n * 2] = (byte)(w >> 8);
		dst[n * 2 + 1] = (byte)w;
		n++;
	}
	dst[n * 2] = dst[n * 2 + 1] = 0;
}

// ResFile::GetResTex(0) (0x802d3890) + the TEX0's data offset (+0x10): `size` bytes of the first texture of BRRES entry `crc`
bool CopyTexture(std::uint32_t crc, byte* dst, uint size)
{
	std::size_t n = 0;
	const std::uint8_t* d = wii::ArchiveStore::Find(crc, n);
	wii::Brres brres;
	if (d == nullptr || !brres.Load(d, n)) return false;
	const auto* tex = brres.Folder("Textures(NW4R)");
	if (tex == nullptr || tex->empty()) return false;
	const std::vector<std::uint8_t>& f = brres.Data();
	const std::uint32_t o = tex->front().second;
	if (o + 0x14 > f.size()) return false;
	const std::uint32_t data = o + (((uint)f[o + 0x10] << 24) | ((uint)f[o + 0x11] << 16) | ((uint)f[o + 0x12] << 8) | f[o + 0x13]);
	if (data + size > f.size()) return false;
	std::memcpy(dst, f.data() + data, size);
	return true;
}
}

// DOL 0x80022320 (banner part): NANDInitBanner(flag 0, "", ""), the title / comment from RESIDENT_US_SAVE_COMMENT (rows 0 / 1, column 1),
// the 192x64 banner from RESIDENT_US_SAVE_BANNER, icons 1..8 from RESIDENT_US_SAVE_ICON_n (CRCs at 0x8038a9b8), every icon speed 1 (fast).
void cKingdomSave::InitBanner()
{
	std::memset(m_Banner, 0, BANNER_SIZE);
	WriteBE32(m_Banner + 0x00, 0x5749424e); // NAND_BANNER_SIGNATURE "WIBN"
	WriteBE32(m_Banner + 0x04, 0);          // flag
	MsgTable csv;
	if (csv.Load("RESIDENT_US_SAVE_COMMENT"))
	{
		Utf8ToUtf16Be(m_Banner + 0x20, 0x40, csv.Cell(0, 1));
		Utf8ToUtf16Be(m_Banner + 0x60, 0x40, csv.Cell(1, 1));
	}
	CopyTexture(0x6136bdc9, m_Banner + 0xa0, 0x6000); // RESIDENT_US_SAVE_BANNER
	static const std::uint32_t ICONS[8] = {0xad1bb336, 0x3412e28c, 0x4315d21a, 0xdd7147b9, 0xaa76772f, 0x337f2695, 0x44781603, 0xd4c70b92};
	uint speed = 0;
	for (uint i = 0; i < 8; i++)
	{
		CopyTexture(ICONS[i], m_Banner + 0x60a0 + i * 0x1200, 0x1200);
		speed = (speed & ~(3u << (i * 2))) | (1u << (i * 2));
	}
	m_Banner[0x08] = (byte)(speed >> 8);
	m_Banner[0x09] = (byte)speed;
}

// The compare tree repeated at 0x80023b94 (and in every helper): NAND result -> status code
uint cKingdomSave::StatusFromNand(int r)
{
	switch (r)
	{
	case wii::nand::OK: return eOK;
	case wii::nand::AUTHENTICATION:
	case wii::nand::ECC_CRIT: return eCORRUPT;
	case wii::nand::CORRUPT: return eBROKEN;
	case wii::nand::BUSY:
	case wii::nand::ALLOC_FAILED:
	case wii::nand::UNKNOWN: return eBUSY;
	case wii::nand::MAXBLOCKS: return eMAX_BLOCKS;
	case wii::nand::MAXFILES: return eMAX_FILES;
	default: return eOTHER;
	}
}

uint cKingdomSave::ComputeChecksum() const
{
	return wii::Crc32(m_Data, DATA_SIZE); // FUN_8013ba2c(0, data, 0xe000): plain CRC-32
}

// DOL 0x80023d1c
bool cKingdomSave::IsDirty() const
{
	return ComputeChecksum() != m_nChecksum;
}

// FUN_80022e18: write to a temp file, then move into the save folder; failures set Status()
bool cKingdomSave::WriteFile(const char* name, const byte* data, uint size)
{
	int r = wii::nand::WriteFile(name, data, size);
	if (r != wii::nand::OK)
	{
		m_nStatus = StatusFromNand(r);
		return false;
	}
	return true;
}

// museum.bin = [BE checksum][data]; banner.bin = banner
bool cKingdomSave::WriteBoth()
{
	static byte file[FILE_SIZE];
	WriteBE32(file, m_nChecksum);
	std::memcpy(file + 4, m_Data, DATA_SIZE);
	return WriteFile(MUSEUM_BIN, file, FILE_SIZE) && WriteFile(BANNER_BIN, m_Banner, BANNER_SIZE);
}

// DOL 0x800226d4
bool cKingdomSave::Check()
{
	if (m_bDisabled)
	{
		m_nStatus = eOK;
		return true;
	}
	int r = wii::nand::ChangeDir();
	m_nStatus = StatusFromNand(r);
	if (r != wii::nand::OK)
	{
		return false;
	}

	// banner.bin: absent -> "no save" (false, status stays OK); present -> must be BANNER_SIZE bytes
	uint len = 0;
	r = wii::nand::GetLength(BANNER_BIN, len);
	if (r != wii::nand::OK)
	{
		m_nStatus = (r == wii::nand::NOEXISTS) ? (uint)eOK : StatusFromNand(r);
		return false;
	}
	m_nStatus = eOK;
	const bool bannerGood = (len == BANNER_SIZE);

	// museum.bin must exist
	uint len2 = 0;
	r = wii::nand::GetLength(MUSEUM_BIN, len2);
	if (r != wii::nand::OK)
	{
		m_nStatus = (r == wii::nand::NOEXISTS) ? (uint)eOK : StatusFromNand(r);
		return false;
	}
	m_nStatus = eOK;

	if (bannerGood)
	{
		return true;
	}
	return WriteFile(BANNER_BIN, m_Banner, BANNER_SIZE); // rewrite a wrong sized banner
}

// DOL 0x80022af4
bool cKingdomSave::CheckSpace()
{
	if (m_bDisabled)
	{
		m_nStatus = eOK;
		return true;
	}
	int r = wii::nand::ChangeDir();
	m_nStatus = StatusFromNand(r);
	if (r != wii::nand::OK)
	{
		return false;
	}
	uint answer = 0;
	r = wii::nand::Check(8, 2, answer);
	if (r != wii::nand::OK)
	{
		m_nStatus = StatusFromNand(r);
		return false;
	}
	if (answer != 0)
	{
		if (answer & 5) m_nStatus = eNO_BLOCKS;
		else if (answer & 0xa) m_nStatus = eNO_INODES;
		return false;
	}
	m_nStatus = eOK;
	m_nChecksum = ComputeChecksum();
	return WriteBoth();
}

// DOL 0x800233e8
bool cKingdomSave::Delete()
{
	if (m_bDisabled)
	{
		m_nStatus = eOK;
		return true;
	}
	int r = wii::nand::ChangeDir();
	m_nStatus = StatusFromNand(r);
	if (r != wii::nand::OK)
	{
		return false;
	}
	for (const char* name : {MUSEUM_BIN, BANNER_BIN})
	{
		r = wii::nand::Delete(name);
		if (r != wii::nand::OK && r != wii::nand::NOEXISTS)
		{
			m_nStatus = StatusFromNand(r);
			return false;
		}
	}
	return true;
}

// DOL 0x800236ec
bool cKingdomSave::Load()
{
	if (m_bDisabled)
	{
		m_nStatus = eOK;
		return true;
	}
	int r = wii::nand::ChangeDir();
	m_nStatus = StatusFromNand(r);
	if (r != wii::nand::OK)
	{
		return false;
	}
	static byte file[FILE_SIZE];
	uint got = 0;
	r = wii::nand::ReadFile(MUSEUM_BIN, file, FILE_SIZE, got);
	if (r != wii::nand::OK)
	{
		m_nStatus = StatusFromNand(r);
		return false;
	}
	if (got != FILE_SIZE) // short read
	{
		m_nStatus = eCORRUPT;
		return false;
	}
	if (ReadBE32(file) != wii::Crc32(file + 4, DATA_SIZE))
	{
		m_nStatus = eCORRUPT;
		return false;
	}
	m_nChecksum = ReadBE32(file);
	std::memcpy(m_Data, file + 4, DATA_SIZE);
	m_nStatus = eOK;
	return true;
}

// DOL 0x80023b58
bool cKingdomSave::Save()
{
	if (m_bDisabled)
	{
		m_nStatus = eOK;
		return true;
	}
	int r = wii::nand::ChangeDir();
	m_nStatus = StatusFromNand(r);
	if (r != wii::nand::OK)
	{
		return false;
	}
	m_nChecksum = ComputeChecksum();
	return WriteBoth();
}

}

namespace kingdom
{
bool cKingdomSave::ReadBlock(uint block, void* dst, uint size) const
{
	if (block >= DATA_SIZE / BLOCK_SIZE || size > BLOCK_SIZE) return false;
	std::memcpy(dst, m_Data + block * BLOCK_SIZE, size);
	return true;
}

bool cKingdomSave::WriteBlock(uint block, const void* src, uint size)
{
	if (block >= DATA_SIZE / BLOCK_SIZE || size > BLOCK_SIZE) return false;
	std::memcpy(m_Data + block * BLOCK_SIZE, src, size);
	return true;
}
}
