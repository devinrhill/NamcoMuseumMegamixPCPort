#pragma once

#include <cstdint>

// The Z80 core of the arcade emulator "gotch" (nmwii2.rso, RSO2 0x80d0d070..0x80d13cac; nmwii.rso has the same core): a 1:1 port of
// Z80_SetFlag / Z80_Init / Z80_Reset / Z80_Run, INTgo / NMIgo / HALTgo, the opcode dispatcher (unnamed static RSO2 0x80d0d7f8) and the
// _Emu_* helpers. Names and argument types come from the module's export table (CodeWarrior mangling, e.g. _Emu_MakeFlagBit__FRCP7Z80_envRUcRCi).
//
// The original addresses the registers through byte-offset tables (B C D E H L (HL) A -> 0x22 0x23 ... 0x20), so Z80_env keeps the
// original byte layout in m[] (big-endian like on the Wii) and the tables keep their original offsets. The quirks of the original are kept
// on purpose (they are listed in src/gotch/Z80.cpp where they occur).
namespace gotch
{
// Z80_SetFlag's second argument (mangled type CPUFLAG); the names are guesses from what each value changes (UNVERIFIED)
enum CPUFLAG
{
	CPUFLAG_IRQ = 0,        // status 0x80: interrupt line
	CPUFLAG_NMI = 4,        // status 0x40: NMI pending
	CPUFLAG_RESET = 5,      // status 0x400: reset line (Z80_Run resets and runs nothing while it is set)
	CPUFLAG_DISABLE = 7,    // status 0x800: CPU off (Z80_Run returns at once)
	CPUFLAG_HALTWAIT = 9,   // status 0x200: Z80_Run returns 2 instead of 0 when it stops on HALT
	CPUFLAG_SETCYCLE = 10,  // cycle budget = (int)value
	CPUFLAG_GETCYCLE = 11,  // *(int*)value = cycle budget
};

struct Z80_env
{
	// byte offsets of the original struct
	enum : int
	{
		CYCLE = 0x00,  // int: cycle budget, every instruction subtracts its count
		A = 0x20, F = 0x21, B = 0x22, C = 0x23, D = 0x24, E = 0x25, H = 0x26, L = 0x27,
		AF = 0x20, BC = 0x22, DE = 0x24, HL = 0x26, AF2 = 0x28, BC2 = 0x2a, DE2 = 0x2c, HL2 = 0x2e,
		IX = 0x30, IY = 0x32, SP = 0x34, PC = 0x36, I = 0x38, R = 0x39,
		STATUS = 0x3a,  // u16, see ST_*
		BUS = 0x3c,     // u16: the data bus value an interrupt reads (IM0 jump target, IM2 vector low byte)
	};
	enum : uint16_t
	{
		ST_IFF1 = 0x001, ST_IFF2 = 0x002, ST_IM = 0x00c, ST_NMI_ACTIVE = 0x010, ST_HALT = 0x020, ST_NMI = 0x040, ST_IRQ = 0x080,
		ST_STOP = 0x100,  // set by an undefined opcode (Uncd): Z80_Run returns 4
		ST_HALTWAIT = 0x200, ST_RESET = 0x400, ST_DISABLE = 0x800,
	};

	// +0x04 is unknown (UNVERIFIED); the callbacks sit at +0x08.. in the original
	std::uint8_t (*ReadM1)(std::uint16_t addr) = nullptr;                 // +0x08 opcode fetch
	std::uint8_t (*ReadMemory8)(std::uint16_t addr) = nullptr;            // +0x0c
	std::uint8_t (*ReadIO8)(std::uint16_t port) = nullptr;                // +0x10
	void (*WriteMemory8)(std::uint16_t addr, std::uint8_t v) = nullptr;   // +0x14
	void (*WriteIO8)(std::uint16_t port, std::uint8_t v) = nullptr;       // +0x18
	void (*ExtFunc)() = nullptr;  // +0x1c: opcode 0xC7 (RST 0) calls it instead (Namco's native patches of the ROMs, e.g. PACMAN::CPU1_ExtFunc)

	std::uint8_t m[0x40] = {};  // the original layout; only +0x00..0x03 (cycles) and +0x20..0x3d are used here

	std::uint8_t& r8(int off) { return m[off]; }
	std::uint16_t r16(int off) const { return (std::uint16_t)(m[off] << 8 | m[off + 1]); }
	void set16(int off, std::uint16_t v)
	{
		m[off] = (std::uint8_t)(v >> 8);
		m[off + 1] = (std::uint8_t)v;
	}
	std::int32_t cycles() const { return (std::int32_t)((std::uint32_t)m[0] << 24 | (std::uint32_t)m[1] << 16 | (std::uint32_t)m[2] << 8 | m[3]); }
	void setCycles(std::int32_t v)
	{
		m[0] = (std::uint8_t)((std::uint32_t)v >> 24);
		m[1] = (std::uint8_t)((std::uint32_t)v >> 16);
		m[2] = (std::uint8_t)((std::uint32_t)v >> 8);
		m[3] = (std::uint8_t)v;
	}
	void addCycles(std::int32_t d) { setCycles(cycles() + d); }
	std::uint16_t status() const { return r16(STATUS); }
	void setStatus(std::uint16_t v) { set16(STATUS, v); }
};

int Z80_SetFlag(Z80_env* env, CPUFLAG flag, void* value);  // RSO2 0x80d0d070; 0 = done, 1 = unknown flag
int Z80_Init(Z80_env* env);                                // RSO2 0x80d0d198
int Z80_Reset(Z80_env* env);                               // RSO2 0x80d0d300
// RSO2 0x80d0d5fc: runs until the cycle budget is used up; 0 = budget spent, 2 / 0 = stopped on HALT (ST_HALTWAIT / not), 4 = undefined opcode
int Z80_Run(Z80_env* env);

// the four flag tables of the module's rodata (RSO2 0x80dca210 SZ, 0x80dca310 SZP, 0x80dca410 INC, 0x80dca510 DEC), generated
// (tests/gotch_z80_test.cpp compares them with the player's module)
const std::uint8_t* Z80_FlagTable(int which);
}
