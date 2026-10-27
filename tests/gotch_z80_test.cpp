// The gotch Z80 core (src/gotch/Z80.cpp): the generated flag tables must equal the module's rodata, and a few small programs must give the
// results the original's handlers compute. The table check skips itself when the player's nmwii2.rso is not there.
#include <cstdio>
#include <cstring>
#include "WiiSystem.hpp"
#include "gotch/RsoModule.hpp"
#include "gotch/Z80.hpp"

#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); return 1; } } while (0)

static std::uint8_t mem[0x10000];
static std::uint8_t RdM(std::uint16_t a) { return mem[a]; }
static void WrM(std::uint16_t a, std::uint8_t v) { mem[a] = v; }
static std::uint8_t RdIO(std::uint16_t) { return 0x5a; }
static int ioWrites = 0;
static void WrIO(std::uint16_t, std::uint8_t) { ioWrites++; }
static int extCalls = 0;
static gotch::Z80_env* cur = nullptr;
static void Ext()
{
	extCalls++;
	cur->set16(gotch::Z80_env::PC, (std::uint16_t)(cur->r16(gotch::Z80_env::PC) + 1));
}

// runs code placed at 0 until HALT
static void Run(gotch::Z80_env& z, std::initializer_list<std::uint8_t> code)
{
	std::memset(mem, 0, sizeof mem);
	std::memcpy(mem, code.begin(), code.size());
	gotch::Z80_Init(&z);
	z.set16(gotch::Z80_env::SP, 0xf000);
	z.setCycles(100000);
	gotch::Z80_Run(&z);
}

int main()
{
	using gotch::Z80_env;
	WiiSystem::Init("/nonexistent.cfg");
	const gotch::RsoModule& m = gotch::RsoModule::Volume(2);
	const gotch::RsoModule::Export* e = m.Find("CPU_IF_Z80");  // RSO2 0x80dca1f8; the tables follow at +0x18
	if (e == nullptr)
	{
		std::printf("SKIP flag tables: nmwii2.rso not found\n");
	}
	else
	{
		for (int t = 0; t < 4; t++) CHECK(std::memcmp(e->data + 0x18 + t * 0x100, gotch::Z80_FlagTable(t), 256) == 0);
	}

	Z80_env z;
	z.ReadM1 = RdM;
	z.ReadMemory8 = RdM;
	z.WriteMemory8 = WrM;
	z.ReadIO8 = RdIO;
	z.WriteIO8 = WrIO;
	z.ExtFunc = Ext;
	cur = &z;

	// LD A,0x15; ADD A,0x27; DAA; HALT -> 0x42
	Run(z, {0x3e, 0x15, 0xc6, 0x27, 0x27, 0x76});
	CHECK(z.r8(Z80_env::A) == 0x42);
	// LD HL,0x100; LD DE,0x200; LD BC,3; LDIR; HALT
	std::memset(mem, 0, sizeof mem);
	Run(z, {0x21, 0x00, 0x01, 0x11, 0x00, 0x02, 0x01, 0x03, 0x00, 0xed, 0xb0, 0x76});
	CHECK(z.r16(Z80_env::BC) == 0 && z.r16(Z80_env::DE) == 0x203 && z.r16(Z80_env::PC) == 0x0c);
	// CALL 0x10; HALT; ... 0x10: LD B,7; RET -> B = 7, SP back
	Run(z, {0xcd, 0x10, 0x00, 0x76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x06, 0x07, 0xc9});
	CHECK(z.r8(Z80_env::B) == 7 && z.r16(Z80_env::SP) == 0xf000 && z.r16(Z80_env::PC) == 4);
	// LD A,0x80; DEC A -> 0x7f with P/V (overflow), H, N
	Run(z, {0x3e, 0x80, 0x3d, 0x76});
	CHECK(z.r8(Z80_env::A) == 0x7f && (z.r8(Z80_env::F) & 0x16) == 0x16);
	// RST 0 (0xC7) is the native hook
	Run(z, {0xc7, 0x76});
	CHECK(extCalls == 1 && z.r16(Z80_env::PC) == 2);
	// quirk: ADD IY,IY adds IX (LD IX,0x1234; LD IY,1; ADD IY,IY -> 0x1235)
	Run(z, {0xdd, 0x21, 0x34, 0x12, 0xfd, 0x21, 0x01, 0x00, 0xfd, 0x29, 0x76});
	CHECK(z.r16(Z80_env::IY) == 0x1235);
	// IM 1 interrupt: EI; IM 1; HALT -> IRQ -> 0x38
	Run(z, {0xfb, 0xed, 0x56, 0x76});
	CHECK(z.status() & Z80_env::ST_HALT);
	gotch::Z80_SetFlag(&z, gotch::CPUFLAG_IRQ, (void*)1);
	mem[0x38] = 0x76;
	z.setStatus((std::uint16_t)(z.status() & ~Z80_env::ST_HALT));
	z.setCycles(1000);
	gotch::Z80_Run(&z);
	CHECK(z.r16(Z80_env::PC) == 0x39 && z.r16(Z80_env::SP) == 0xeffe);
	// undefined opcode stops the run with 4
	std::memset(mem, 0, sizeof mem);
	mem[0] = 0xed;
	mem[1] = 0x00;
	gotch::Z80_Init(&z);
	z.setCycles(100);
	CHECK(gotch::Z80_Run(&z) == 4);
	std::printf("gotch z80 tests passed\n");
	return 0;
}
