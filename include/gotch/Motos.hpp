#pragma once

#include <cstdint>

#include "gotch/Machine.hpp"
#include "gotch/Sound.hpp"

// MOTOS of the arcade collection vol 2 (nmwii2.rso, class MOTOS, RSO2 0x80d53be0..0x80d56858 + 0x80d6e240..0x80d70d78): the Namco "Super Pac-Man"
// class board of the original machine, TWO M6809 CPUs ("6809 #0" / "6809 #1", 1.536 MHz), two Namco I/O customs (NCI_68: the 58xx / 56xx
// chips at 0x4800 / 0x4810), the 8-voice NAMCO WSG8 and a tile + 16 x 16 sprite renderer, run by the gotch framework. Same shape as
// include/gotch/RallyX.hpp; the monitor is turned 270 degrees (portrait, like PACMAN).
//
// This header also carries the framework pieces MOTOS and GROBDA share (the module has ONE copy of them, only the games differ): the M6809 core
// (RSO2 0x80d13cac..0x80d1aeac: M6809_SetFlag / Init / Reset / Run, IRQgo / FIRQgo / NMIgo, the opcode dispatcher 0x80d14b48 and the _Emu
// helpers), the NCI_68 object with its N56 / N58 Execute (RSO2 0x80d3a768..0x80d3b0..), SND_NAMCO_WSG8 (ctor 0x80d22270, SetRegister 0x80d22410).
// They live in namespace gotch::arc6809 and are defined in src/gotch/Motos.cpp (Grobda.cpp uses them).
//
// Stubbed (marked at the sites): the screen adjust menu / CROSSHATCH / SCREENADJUST, CDraw::FadeSet, MenuMM, the NOW SAVING / sharing texts.
namespace gotch
{
class CDraw;
class PrimaryStream;

namespace arc6809
{
// the M6809 environment (the original's 0x30-byte struct; the callbacks sit at +8 / +0xc / +0x10, the registers at +0x14..+0x21)
struct M6809_env
{
	std::int32_t cycles = 0;                                          // +0x00: budget, every instruction subtracts its cycles
	std::uint8_t (*ReadMemory8)(std::uint16_t addr) = nullptr;       // +0x08
	void (*WriteMemory8)(std::uint16_t addr, std::uint8_t v) = nullptr;   // +0x0c
	void (*ExtFunc)() = nullptr;                                      // +0x10: the (illegal) opcode 0x14 calls it (Namco's native ROM patches)
	std::uint16_t pc = 0;                                             // +0x14
	std::uint16_t u = 0;                                              // +0x16
	std::uint16_t s = 0;                                              // +0x18
	std::uint16_t x = 0;                                              // +0x1a
	std::uint16_t y = 0;                                              // +0x1c
	std::uint8_t a = 0;                                               // +0x1e
	std::uint8_t b = 0;                                               // +0x1f
	std::uint8_t cc = 0;                                              // +0x20: E F H I N Z V C = 0x80 .. 0x01
	std::uint8_t dp = 0;                                              // +0x21
	std::uint32_t status = 0;                                         // +0x24: 1 IRQ, 2 FIRQ, 4 NMI, 8 SYNC, 0x10 CWAI, 0x20 reset line, 0x40 off, 0x100 stop, 0x800 halt wait
	std::int32_t user = 0;                                            // +0x28 (SetFlag 0xc; Setup stores the CPU number)
};

// CPUFLAG values M6809_SetFlag knows: 0 IRQ, 1 FIRQ, 4 NMI, 5 reset line, 7 CPU off, 9 halt wait, 10 set cycle budget, 11 get it, 12 user value
int M6809_SetFlag(void* env, int flag, void* value);   // RSO2 0x80d13cac (0 = done, 1 = unknown flag)
int M6809_Init(void* env);                             // RSO2 0x80d13de8
int M6809_Reset(void* env);                            // RSO2 0x80d14330
int M6809_Run(void* env);                              // RSO2 0x80d143d8 (0 = budget spent, 2 = halt wait, 4 = undefined opcode)

// NCI_68 (0x58 bytes, RSO2 ctor 0x80d3a768 / Init 0x80d3a7b4 / CoinExec 0x80d3a7f8 / StartButton 0x80d3ab08), the Namco I/O custom: 16 registers
// at +0x40, mode in register 8. kind 56 -> N56::Execute 0x80d3ab24, kind 58 -> N58::Execute 0x80d3afdc (the vtable the game installs)
class Nci68
{
public:
	typedef std::uint8_t (*ReadCb)(int port);        // +0x38 (the game's NCIxxRead)
	typedef void (*WriteCb)(int port, unsigned v);   // +0x3c (the game's NCIxxWrite)
	Nci68(int kind, ReadCb read, WriteCb write);
	void Init();                  // vtable +0xc
	void Execute();               // vtable +0x10
	void CoinExec(int base);      // vtable +0x14
	void StartButton(int player); // RSO2 0x80d3ab08

	std::uint8_t reg[16] = {};    // +0x40
	std::int32_t credits = 0;     // +0x0c

private:
	int kind;
	ReadCb rd;
	WriteCb wr;
	std::int32_t prevCoin = 0;    // +0x04
	std::int32_t prevStart = 0;   // +0x08
	std::int32_t coin1 = 0;       // +0x10
	std::int32_t start = 0;       // +0x14
	std::int32_t unk18 = 0;       // +0x18
	std::int32_t unk1c = 0;       // +0x1c
	std::int32_t count1 = 0;      // +0x20
	std::int32_t count2 = 0;      // +0x24
	std::int32_t set1 = 1;        // +0x28
	std::int32_t set2 = 1;        // +0x2c
	std::int32_t set3 = 1;        // +0x30
	std::int32_t set4 = 1;        // +0x34
	bool coinEnable = true;       // +0x50
	bool startEnable = true;      // +0x51
	std::int32_t startReq = 0;    // +0x54
	std::uint8_t In(int port) const { return rd != nullptr ? rd(port) : 0; }
	void Out(int port, unsigned v) const { if (wr != nullptr) wr(port, v); }
};

// SND_NAMCO_WSG8 (RSO2 ctor 0x80d22270, SetRegister 0x80d22410): 8 voices, mono, clock 48000000.0f, stream "NAMCO WSG8"; the waves are the PROM's low
// nibbles - 8 (only the first 0x100 bytes: 8 waves, the other 8 stay empty like in the original)
class SndNamcoWsg8 : public SndNamcoWsg
{
public:
	SndNamcoWsg8(PrimaryStream& primary, const std::uint8_t* rom);
	// regs = the 0x400-byte sound RAM, voice n at regs + 8 * n
	void SetRegister(const std::uint8_t* regs);
};
}

namespace motos
{
bool Setup();                // RSO2 0x80d543bc (false when the player's module lacks the MOTOS data)
bool IsSetup();
void ExitMachine();          // RSO2 0x80d54e44 (Exit); the next Start runs Setup again
void StorageFrameUpdate();   // CDraw::WaitVSync RSO2 0x80d06d9c runs CDataStorage::FrameUpdate first
int Execute1V();             // RSO2 0x80d54f68; 1 when the machine stopped
void HotReset();             // RSO2 0x80d55628
void ExitRequest();          // CGAMEIO::ExitRequest (the HOME stand-in)
void SetSoundBufferClear(void (*fn)());
PrimaryStream* Primary();
const CDraw& Draw();         // 288 x 224 landscape; the monitor shows it turned 270 degrees (portrait)
}
}
