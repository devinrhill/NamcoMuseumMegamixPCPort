#pragma once

#include <cstdint>
#include <vector>

// The save data of the gotch frontend (nmwii*.rso): CCDataStorage (three blocks of the same size) + CDataStorage (the request / frame state
// machine). The module never touches the NAND itself: a save is copied into the host's bridge (`ToseGotchBridge + 0x1c`, 0x800 bytes) and the
// host (main.dol job 142, cAppRSOOriginal) moves that block in and out of the Megamix save file (block 6: vol 1 = its first 0x800 bytes,
// vol 2 = the next 0x800). Every block is the module's own byte layout (big-endian words, as on the Wii).
namespace gotch
{
// sRSOBridge +0x1c (the host's 0x800-byte save area); zero = no save yet (a first boot)
std::uint8_t* BridgeSave();
constexpr std::uint32_t kBridgeSaveSize = 0x800;

class CDataStorage
{
public:
	// RSO2 0x80d022fc (CDataStorage(user, size): size 0 = 0x800; file name "DataStorageWii.bin", +0x34 = 0, +0x38 = 4) and the
	// CCDataStorage ctor 0x80d45214 (three zeroed blocks, +0x24 = 1, +0x14 = 0, +0x18 = 1)
	explicit CDataStorage(std::uint32_t size = 0);
	std::uint32_t SaveRequest();          // RSO2 0x80d02424 (returns the state +0x18)
	std::uint32_t LoadRequest();          // RSO2 0x80d025b8
	std::uint32_t FrameUpdate();          // RSO2 0x80d02634
	void ActivateLoadedData();            // RSO2 0x80d02838
	std::int32_t GetCheckData(const std::uint8_t* block) const;   // RSO2 0x80d0285c: the byte sum of block[0x30 .. size)
	std::uint32_t CheckComplete();        // RSO2 0x80d02924
	void ZeroFillDataBlock();             // RSO2 0x80d456c0

	bool Valid() const { return valid; }   // +0x24
	std::uint8_t* Data() { return work.data(); }   // +0x4: the block the game reads and writes (PACMAN: the export data)
	std::uint32_t magicOffset = 0;        // +0x34: where SaveRequest stores 0x1f (the game sets it: PACMAN 0x28)
	std::uint32_t sumOffset = 4;          // +0x38: where it stores the check sum (PACMAN 0x2c)

private:
	std::vector<std::uint8_t> saved;      // +0x0: the last block written to / read from the bridge
	std::vector<std::uint8_t> work;       // +0x4
	std::vector<std::uint8_t> transfer;   // +0x8
	std::uint32_t size;                   // +0xc
	std::uint32_t request = 0;            // +0x14: 0 idle, 1 save, 2 load
	std::uint32_t state = 1;              // +0x18: 0 done, 2 busy, 3 load failed (no / bad data), 5 error
	bool valid = true;                    // +0x24
	std::int32_t wait = 0;                // +0x2c: frames until the copy
};

// GetDataStorageBusy (RSO2 0x80d02978): DAT_811dca08, set while a request is pending
bool GetDataStorageBusy();
}
