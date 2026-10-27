#pragma once

#include "common.hpp"

namespace kingdom
{

// The save-data manager at cKingdomSys + 0xa8c (RTTI kingdom::cKingdomSave 0x804767a8). Its object is 0x1d130+ bytes in the DOL; what
// matters is: banner image at +0x40 (0xf0a0 bytes), checksum word at +0xf100, game data at +0xf104 (0xe000 bytes), "disabled" byte at
// +0xf0e0 and the last result code at +0xf0e4. The helpers below are the DOL functions that cAppSaveLoad drives; each returns true on
// success and leaves the reason for a failure in Status().
//
// Files (in the NAND folder 00010000/534e4d45/data, here WiiSystem::SaveDir()):
//   museum.bin  0xe004 bytes  [u32 big endian CRC-32 of the data][0xe000 bytes]   (checked against a real save)
//   banner.bin  0xf0a0 bytes  Wii banner shown in the system menu
class cKingdomSave
{
public:
	static constexpr uint DATA_SIZE = 0xe000;
	static constexpr uint BANNER_SIZE = 0xf0a0;
	static constexpr uint FILE_SIZE = 0xe004;

	cKingdomSave();
	// DOL 0x80022320 (its banner part): fill Banner() from the resident archive's SAVE_COMMENT / SAVE_BANNER / SAVE_ICON_n entries
	void InitBanner();

	// DOL 0x800226d4: save folder reachable, banner.bin present (rewritten if its size is wrong), museum.bin present
	bool Check();
	// DOL 0x80022af4: enough room for a new save (8 blocks, 2 inodes)? If so, creates museum.bin and banner.bin
	bool CheckSpace();
	// DOL 0x800233e8: delete museum.bin and banner.bin (a missing file is fine)
	bool Delete();
	// DOL 0x800236ec: read museum.bin, verify its CRC, copy it into Data()
	bool Load();
	// DOL 0x80023b58: checksum Data() and write museum.bin and banner.bin
	bool Save();
	// DOL 0x80023d1c: Data() no longer matches the stored checksum
	bool IsDirty() const;

	// Per-game blocks of Data(): 7 x 0x2000 bytes (block 0 = Galaga, 6 = the main menu hub). Raw copies, the content is big-endian.
	static constexpr uint BLOCK_SIZE = 0x2000;
	// DOL 0x80023d98: copy `size` bytes of block `block` to `dst`
	bool ReadBlock(uint block, void* dst, uint size) const;
	// DOL 0x80023dd8: copy `size` bytes from `src` into block `block`
	bool WriteBlock(uint block, const void* src, uint size);

	// DOL 0x80023d74 / 0x80023d80: a disabled manager makes every operation above succeed without touching storage
	void SetDisabled(bool disabled) { m_bDisabled = disabled; }
	bool IsDisabled() const { return m_bDisabled; }

	// DOL 0x80023d8c: result of the last failed operation, see EStatus
	uint Status() const { return m_nStatus; }

	byte* Data() { return m_Data; }
	byte* Banner() { return m_Banner; }

	// Result codes (NAND error -> code mapping at 0x80023b94 and 0x80009414's table):
	enum EStatus
	{
		eOK = 0,
		eCORRUPT = 1,       // NAND authentication / ECC error, short read or checksum mismatch -> "delete?" screen (state 8)
		eRETRY = 2,         // unused by the mappings seen; the router retries it three times
		eNO_BLOCKS = 3,     // not enough free blocks (CheckSpace)         -> state 9
		eNO_INODES = 4,     // not enough free files (CheckSpace)          -> state 10
		eMAX_BLOCKS = 5,    // NAND_RESULT_MAXBLOCKS                       -> state 15
		eMAX_FILES = 6,     // NAND_RESULT_MAXFILES                        -> state 16
		eBROKEN = 7,        // NAND_RESULT_CORRUPT                         -> state 17
		eBUSY = 8,          // busy / alloc failed / unknown               -> state 18
		eOTHER = 9          // anything else; the router does not handle it
	};

private:
	bool WriteFile(const char* name, const byte* data, uint size);
	bool WriteBoth();
	uint ComputeChecksum() const;
	static uint StatusFromNand(int nandResult);

	byte m_Banner[BANNER_SIZE]{};   // +0x40
	bool m_bDisabled{};             // +0xf0e0
	uint m_nStatus{};               // +0xf0e4
	uint m_nChecksum{};             // +0xf100
	byte m_Data[DATA_SIZE]{};       // +0xf104
};

}
