#pragma once

#include <cstdint>
#include <string>

// File-backed stand-in for the few Wii NAND calls the save code uses (NANDChangeDir / GetLength / Open+Read / Create+Write+Move /
// Delete / Check). Results use the SDK's NAND_RESULT_* values (RVL_SDK/include/revolution/nand.h) so the game's status mapping
// (kingdom::cKingdomSave) works unchanged. The "NAND directory" is WiiSystem::SaveDir().
namespace wii::nand
{
constexpr int OK = 0;
constexpr int ACCESS = -1;
constexpr int ALLOC_FAILED = -2;
constexpr int BUSY = -3;
constexpr int CORRUPT = -4;
constexpr int ECC_CRIT = -5;
constexpr int EXISTS = -6;
constexpr int INVALID = -8;
constexpr int MAXBLOCKS = -9;
constexpr int MAXFD = -10;
constexpr int MAXFILES = -11;
constexpr int NOEXISTS = -12;
constexpr int NOTEMPTY = -13;
constexpr int OPENFD = -14;
constexpr int AUTHENTICATION = -15;
constexpr int MAXDEPTH = -16;
constexpr int UNKNOWN = -64;
constexpr int FATAL_ERROR = -128;

// NANDChangeDir: makes sure the save directory exists (a real console already has the title's data folder).
int ChangeDir();

// NANDOpen + NANDGetLength + NANDClose. NOEXISTS when the file is absent.
int GetLength(const std::string& name, std::uint32_t& length);

// NANDOpen + NANDRead + NANDClose. `read` is the number of bytes actually read.
int ReadFile(const std::string& name, std::uint8_t* buf, std::uint32_t size, std::uint32_t& read);

// The game's safe write (FUN_80022e18): write a temp file, then move it over the destination.
int WriteFile(const std::string& name, const std::uint8_t* buf, std::uint32_t size);

// NANDDelete
int Delete(const std::string& name);

// NANDCheck(blocks, inodes, &answer): answer bit 0/2 = not enough blocks, bit 1/3 = not enough inodes (0 = enough room).
// A NAND block is 16 KiB.
int Check(std::uint32_t blocks, std::uint32_t inodes, std::uint32_t& answer);
}
