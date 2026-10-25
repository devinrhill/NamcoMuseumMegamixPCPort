#include "kingdom/cKingdomSave.hpp"

#include <cstring>

#include "wii/Archive.hpp" // Crc32
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

cKingdomSave::cKingdomSave()
{
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
