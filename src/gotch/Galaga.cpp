// GALAGA of the arcade collection vol 2 (nmwii2.rso, class GALAGA): a 1:1 port of the machine glue around the three-Z80 board (see
// include/gotch/Galaga.hpp; it follows src/gotch/RallyX.cpp and src/gotch/PacMan.cpp). Module globals as file statics; comments cite them as
// "sw+0xNN" (GALAGA::THISGAME_sw 0x81209078 + NN), "mem[0xNNNN]" (GALAGA::CPU1_MainMemory 0x8102d088 + NNNN, the memory every CPU shares above 0x6800),
// "rom2[..]" / "rom3[..]" (CPU2_MainMemory 0x8103d088 / CPU3_MainMemory 0x8103e088) and "save+0xNN" (the export data block, CDataStorage + 4; the
// GALAGA data starts at save+0x190: magic "PPGG0001").
//
// How the board works (all of it ported, not emulated from a table):
//  * CPU1 (main), CPU2 (sub) and CPU3 (sound) run 3.072 MHz Z80s one after the other for every slice of the machine scheduler (Machine.cpp). CPU1 reads and
//    writes the whole 64 KB (ROM below 0x4000, RAM / IO above); CPU2 and CPU3 read their own ROM below 0x6800 and the shared memory above.
//  * IO: 0x6800..0x6807 reads the DIP switch bits (IO[]); writes of 0x6820 / 0x6821 / 0x6822 are the CPU1 IRQ enable, CPU2 IRQ enable and CPU3 NMI disable;
//    0x6823 = CPU2 / CPU3 reset (low). 0x7000..0x700f is the NCI data port and 0x7100 its command port (NCI = the 06XX): the latched command selects chip
//    0 = N51 (coins, controls, DIP bits) or chip 3 = N54 (the sound commands that fire the PCM explosion).
//  * the sound CPU (CPU3) writes its WSG3 registers at mem[0x6800..0x681f] and SND_GALAGA_WSG3 turns them into samples (3 voices) plus one PCM voice (PCM_BOMB).
//  * the picture is the usual Namco tile map (0x8000..0x83ff, colours + 0x400), 64 sprites (0x8b80 / 0x9380 / 0x9b80) and the starfield (the registers
//    0xa000..0xa007 drive the 252 stars of a module table).
//
// Not ported (marked at the sites): the screen adjust menu (mode 9), CROSSHATCH / SCREENADJUST, CDraw::FadeSet, MenuMM, MENU_KEYCONV, InitMenu_KeySetting.
#include "gotch/Galaga.hpp"

#include "gotch/DataStorage.hpp"
#include "gotch/Draw.hpp"
#include "gotch/GameIO.hpp"
#include "gotch/GameState.hpp"
#include "gotch/Machine.hpp"
#include "gotch/Menu.hpp"
#include "gotch/RsoModule.hpp"
#include "gotch/SWork.hpp"
#include "gotch/Sound.hpp"
#include "gotch/Z80.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

namespace gotch
{
namespace galaga
{
namespace
{
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using s32 = std::int32_t;
using s64 = std::int64_t;
using u64 = std::uint64_t;

// ---- module data (read from the player's nmwii2.rso at Setup) ----
const u8* kAsciiToGame = nullptr;   // AsciiCodeToGameCodeConvertTable__6GALAGA (RSO2 0x811476d0)
const u8* kFontPatch = nullptr;     // FontPatchTable__6GALAGA (0x8102cde0)
const u8* kMenuPalet = nullptr;     // MENUPALET__6GALAGA (0x8103f088, 0x25 palettes x 4 big-endian u32)
const u8* kPaletRom = nullptr;      // PALET_ROM__6GALAGA (0x8103f2d8)
const u8* kBgClut = nullptr;        // BG_CLUT_ROM__6GALAGA (0x8103f2f8)
const u8* kSpClut = nullptr;        // SP_CLUT_ROM__6GALAGA (0x8103f3f8)
const u8* kWsgRom = nullptr;        // WSGROM__6GALAGA (0x8103f4f8)
const u8* kCgromA = nullptr;        // CGROM_A__6GALAGA (0x8103f5f8: the background tiles)
const u8* kCgromB = nullptr;        // CGROM_B__6GALAGA (0x810405f8: the sprites)
const u8* kCgromConfig = nullptr;   // CGROM_CONFIG_V (the menu font, shared)
const u8* kLayoutBg = nullptr;      // unexported CGen layouts: background (0x81029de0), sprites (0x8102a5e0), menu font (0x8102c5e0)
const u8* kLayoutSprite = nullptr;
const u8* kLayoutConfig = nullptr;
const u8* kStarTable = nullptr;     // the 252 stars: 4 bytes each (x, y, colour, mask) at RSO2 0x810299e8 (StarDraw)
const u8* kStarSpeed = nullptr;     // GALAGA::@LOCAL@StarMove (0x8102cee0): the scroll step of the 8 speed settings (ints)
const u8* kOldL3aeb = nullptr;      // GALAGA::OLD_l3aeb (0x8102cf88): ten bytes patched into mem[0x3b9f]
const u8* kHiscoreInit = nullptr;   // HISCORE_INITDATA__6GALAGA (0x811477d0): the 0x2d bytes of one bank's records
const u8* kGameSettingDefault = nullptr;   // GAME_SETTING_DEFAULT__6GALAGA (0x81147800, big-endian ints, -1 ends)
const u8* kSoundTestCodes = nullptr;       // SoundTest_CodeTable__6GALAGA (0x81147c8c, 2 bytes per entry)
const u8* kPcmBomb = nullptr;       // PCM_BOMB__6GALAGA (0x810435f8)
// the PCM's end: SetPCM gets the address 0x8107d0ca (DAT_8107d0ca), 0x16 bytes before the export's end
const u32 kPcmBombSize = 0x8107d0cau - 0x810435f8u;
std::string sCopyright1, sCopyright2, sNamcoLogo;   // Copyright1_String / Copyright2_String / NamcoLogo_String (C strings)

// ---- the module's globals ----
u8 mem[0x10000];             // GALAGA::CPU1_MainMemory 0x8102d088
u8 rom2[0x1000];             // CPU2_MainMemory 0x8103d088
u8 rom3[0x1000];             // CPU3_MainMemory 0x8103e088
Z80_env env1, env2, env3;    // CPU1_Z80_ENV 0x812092a4 / CPU2_Z80_ENV 0x81209330 / CPU3_Z80_ENV 0x812093c0
CPU_SPEC z80cpu1, z80cpu2, z80cpu3;   // CPU1_Z80 0x812092e8 / CPU2_Z80 0x81209378 / CPU3_Z80 0x81209408
MACHINE_SPEC machine;        // GALAGA::Machine 0x81209450
EVENT_SPEC timeSlicer;       // GALAGA::TimeSlicer 0x81209580 ("TIME SLICE")
EVENT_SPEC soundEvent;       // GALAGA::SOUND 0x812095a8 ("SOUND NMI")
SWork sw;                    // GALAGA::THISGAME_sw 0x81209078
u8 io[8];                    // GALAGA::IO 0x812098d8 (the DIP switch bits mem[0x6800..0x6807] reads)
bool cpu23ResetActive = true;    // GALAGA::CPU23_RESET_ACTIVE 0x812098e0
int sliceCount = 0;              // DAT_812098e4
bool bGameOverWaitSkip = false;  // GALAGA::bGameOverWaitSkip 0x8120ac80
bool bGameScreenFlip = false;    // GALAGA::bGameScreenFlip 0x81208ebc
u8 copyrightFlag = 0;            // *GALAGA::Copyright (0x8120ac40: a byte nothing sets)
s32 starScroll = 0;              // DAT_81205f70
u8 dipswA = 0xf4, dipswB = 0x97;   // GALAGA::DIPSW_A_CURRENT 0x8120adb8 / DIPSW_B_CURRENT 0x8120adb9
s32 gsCurrent[0x120];            // GAME_SETTING_CURRENT 0x81147818 (the original keeps 0x11b ints)
s32 gsCurrentPtr = 0;            // GAME_SETTING_CURRENT_PTR 0x8120adb4
s32 gsDefault[7];               // GAME_SETTING_DEFAULT 0x81147800 (6 big-endian ints in the module, host order here; -1 ends)
u8 loopSoundBuf = 0xff;          // GALAGA::LoopSoundBuf 0x8120adac
std::unique_ptr<GameState> gameState;    // GALAGA::GameState
std::unique_ptr<CDataStorage> storage;   // *0x81209574
u8* save = nullptr;          // *(CDataStorage + 4): the export data (save block)
std::unique_ptr<CDraw> draw;
std::unique_ptr<PrimaryStream> primary;
std::unique_ptr<CGameIO> gameio;
std::unique_ptr<WiiMenu> gameMenu;       // GALAGA::GameMenu (GAMEMENU = WII_MENU with the overrides below)
void (*soundBufferClear)() = nullptr;
void (*updateStream)(s64) = nullptr;
int texBg = -1, texSprite = -1, texConfig = -1, texStar = -1;   // wdh_cgrom_bg / _sprite / _config, the stars' one-pixel characters
bool isSetup = false;
bool warnedHook = false;
const int kLocale = 1;
const int kMenuPaletOffset = 0x81;   // GALAGA::MENU_PaletOffset

// GALAGA::OptionsMenuItem 0x8120ad90..: BONUS, LIFE, SOUND TEST, ATTRACT SOUND, SCREEN ADJUSTMENT, DEFAULT, EXIT (rebuilt every frame by OptionsMenuBuild)
MenuItem* optionsItem[7] = {};
char soundTestText[0x40][8] = {};   // GALAGA::SOUND_TEST_NO_Buffer
struct OptionStrings
{
	const char* options = "";
	const char* bonus = "";
	const char* none = "";
	const char* life = "";
	const char* soundTest = "";
	const char* attract = "";
	const char* on = "";
	const char* off = "";
	const char* screenAdjust = "";
	const char* deflt = "";
	const char* exit = "";
	const char* left5[7] = {};   // MenuStr_GALAGA_OPTIONS_EXTEND_LEFT5_{1..7}of8
	const char* left3[7] = {};   // ..._LEFT3_...
} optStr;

// ---- helpers over the original layouts ----
// (the hooks address the registers through the env of their CPU; 16-bit pairs are big-endian words of the original struct)
u8& A(Z80_env& e) { return e.r8(Z80_env::A); }
u8& Breg(Z80_env& e) { return e.r8(Z80_env::B); }
u8& Creg(Z80_env& e) { return e.r8(Z80_env::C); }
u8& Dreg(Z80_env& e) { return e.r8(Z80_env::D); }
u8& Ereg(Z80_env& e) { return e.r8(Z80_env::E); }
u16 HL(Z80_env& e) { return e.r16(Z80_env::HL); }
void SetHL(Z80_env& e, u16 v) { e.set16(Z80_env::HL, v); }
u16 DE(Z80_env& e) { return e.r16(Z80_env::DE); }
void SetDE(Z80_env& e, u16 v) { e.set16(Z80_env::DE, v); }
u16 SP(Z80_env& e) { return e.r16(Z80_env::SP); }
u16 PC(Z80_env& e) { return e.r16(Z80_env::PC); }
void SetPC(Z80_env& e, u16 v) { e.set16(Z80_env::PC, v); }
s32 Sw32(int off) { return sw.i32(off); }
void SetSw32(int off, s32 v) { sw.set32(off, v); }
// the save block holds big-endian words (the Wii's byte order)
void SetSave32(int off, u32 v)
{
	save[off] = (u8)(v >> 24);
	save[off + 1] = (u8)(v >> 16);
	save[off + 2] = (u8)(v >> 8);
	save[off + 3] = (u8)v;
}
u8 Bank() { return sw.b(0xa3); }   // GALAGA::GetCurrentBankNumber 0x80d7a408

// s64 arithmetic of the module's float math (the same steps as every static initialiser / event set-up: single-precision conversions, the multiply
// by 2^40 in double, __cvt_dbl_usll truncating to unsigned 64 bits; negative gives 0, UNVERIFIED for the runtime routine)
u64 ToU64(double d) { return d <= 0.0 ? 0 : (u64)d; }
// (u64)(float)((double)(float)((double)2^40 * (double)(float)t) / (double)(float)(d << 40)): GALAGA::Setup's event periods, Slice, SoundInt
s64 EtimeDiv(s64 t, int d)
{
	const float f = (float)((double)kETimeSecond * (double)(float)t);
	const double q = (double)f / (double)(float)((s64)d << 40);
	return (s64)ToU64((double)(float)q);
}
// (u64)((float)((double)(float)t * (double)(float)(m << 40)) / 2^40): the first step of VBlankIn's slice period
s64 EtimeMul(s64 t, int m)
{
	const float f = (float)((double)(float)t * (double)(float)((s64)m << 40));
	return (s64)ToU64((double)(f / kETimeSecond));
}

// ---- the custom chips (NCI + N51 + N54), the structs keep the original byte layout in their comments ----
// GALAGA_N51 (0x160 bytes at NCI +8; ctor / N51Init RSO2 0x80d89a20): the coin / start / controls chip
struct N51
{
	u8 sel = 0;        // +0: selected chip row (N51_CMD_Write: nobody calls it, stays 0)
	u8 prev = 0;       // +1: the coin inputs of the last read, inverted
	u8 flag = 0;       // +2
	u8 credits = 0;    // +3
	u8 mode = 1;       // +4: 1 = switch mode (reads the raw port), 0 = credit mode
	u8 coinIO = 1;     // +5
	u8 startIO = 1;    // +6
	u8 f7 = 0;         // +7
	s32 i8 = 0;        // +8: coins per credit (0 = coins do nothing)
	s32 ic = 0;        // +0xc
	s32 i20 = 0;      // +0x20: credits per coin step
	s32 i24 = 0;       // +0x24
	s32 i38 = 0;       // +0x38: the coin counter
	s32 i3c = 0;       // +0x3c: the number of setup bytes still to come
	u8 in[0x20];       // +0x40: the input ports (IO_Convert fills in[0..2])
	u8 tab[0x100];     // +0x60

	N51() { Init(); }
	void Init()   // RSO2 0x80d89a20
	{
		sel = prev = flag = credits = 0;
		mode = 1;
		i8 = ic = i20 = i24 = i38 = i3c = 0;
		coinIO = startIO = 1;
		f7 = 0;
		std::memset(in, 0xff, sizeof(in));
		std::memset(tab, 0xff, sizeof(tab));
	}
	// RSO2 0x80d89c20 (mode = the NCI's latched command, unused)
	u8 DataRead(u32 addr)
	{
		const u32 a = addr & 0xf;
		if (a == 0)
		{
			if (mode == 1)
			{
				tab[sel * 0x10 + 2] = in[2];
				return tab[sel * 0x10 + 2];
			}
			if (mode == 0)
			{
				tab[sel * 0x10 + 2] = in[2];
				const u8 old = prev;
				const u8 now = (u8)~tab[sel * 0x10 + 2];
				prev = now;
				const u8 changed = old ^ now;
				if (flag != 0 && startIO != 0)
				{
					if ((changed & 4 & now) != 0 && credits != 0)
					{
						credits--;
						flag = 0;
					}
					if ((changed & 8 & prev) != 0 && 1 < credits)
					{
						credits = (u8)(credits - 2);
						flag = 0;
					}
				}
				if (0 < i8 && coinIO != 0 && (changed & 0x70 & prev) != 0 && credits < 99)
				{
					const s32 n = i38++;
					if (i8 <= n + 1)
					{
						i38 = 0;
						credits = (u8)(credits + (u8)i20);
					}
				}
				return (u8)((credits + (credits / 10) * 6) & 0xff);   // BCD
			}
		}
		if (a == 1)
		{
			tab[sel * 0x10] = in[0];
			return tab[sel * 0x10];
		}
		if (a != 2) return tab[a + sel * 0x10];
		tab[sel * 0x10 + 1] = in[1];
		return tab[sel * 0x10 + 1];
	}
	// RSO2 0x80d89e34
	void DataWrite(u32 addr, u32 value)
	{
		const s32 n = i3c;
		if (n != 0)
		{
			i3c = n - 1;
			if (n == 4) i8 = (s32)value;
			else if (n == 3) i20 = (s32)value;
			else if (n == 2) ic = (s32)value;
			else if (n == 1) i24 = (s32)value;
			tab[(addr & 0xf) + sel * 0x10] = (u8)value;
			return;
		}
		switch (value & 7)
		{
		case 1: i3c = 4; credits = 0; break;
		case 2: mode = 0; flag = 1; break;
		case 3: f7 = 0; break;
		case 4: f7 = 1; break;
		case 5: mode = 1; break;
		default: break;
		}
	}
	void CoinIOControl(u8 v) { coinIO = v; }    // RSO2 0x80d89f20
	void StartIOControl(u8 v) { startIO = v; }  // RSO2 0x80d89f28
	void SetCredits(u8 v) { credits = v; }      // RSO2 0x80d89f30
	// RSO2 0x80d89f40: a start button takes `n` credits when there are enough
	void StartButton(int n)
	{
		if ((int)credits < n) return;
		credits = (u8)(credits - n);
		flag = 0;
	}
};

// GALAGA_N54 (0x30 bytes at NCI +0x168; ctor / N54Init RSO2 0x80d8a440): the sound command chip
struct N54
{
	u8 sel = 0;                       // +0
	void (*callback)(int cmd) = nullptr;   // +4: GALAGA::N54_Callback (the original passes the data pointer too: nobody reads it)
	u8 buf[0x18];                     // +8: three 8-byte command buffers
	s32 state = 0;                    // +0x20
	s32 count = 0;                    // +0x24
	u8 in[0x10];                      // +0x28: the input bytes (the top DIP switch bits)

	N54() { Init(); }
	void Init()
	{
		sel = 0;
		std::memset(in, 0xff, sizeof(in));
		std::memset(buf, 0x11, sizeof(buf));
		state = count = 0;
		callback = nullptr;
	}
	// RSO2 0x80d8a4e4
	u8 DataRead(u32 addr) { return in[addr & 7]; }
	// RSO2 0x80d8a54c
	void DataWrite(u32 value)
	{
		const s32 st = state;
		if ((u32)(st - 1) < 2)
		{
			buf[(st - 1) * 8 + count] = (u8)value;
			const s32 n = ++count;
			if (n == 4)
			{
				if (callback != nullptr) callback(st * 0x10 + 0x20);
				state = 0;
			}
		}
		else if (st == 0)
		{
			if (value == 0x30) { state = 1; count = 0; }
			else if (value == 0x40) { state = 2; count = 0; }
			else if (value == 0x60) { state = 3; count = 0; }
			else if (value == 0x10) { if (callback != nullptr) callback(0x10); }
			else if (value == 0x20) { if (callback != nullptr) callback(0x20); }
			else if (value == 0x50) { if (callback != nullptr) callback(0x50); }
			else if (((value + 0x90) & 0xff) < 0x10) { if (callback != nullptr) callback((int)value); }
		}
		else if (st == 3)
		{
			buf[0x10 + count] = (u8)value;
			const s32 n = ++count;
			if (n == 5)
			{
				if (callback != nullptr) callback(0x60);
				state = 0;
			}
		}
	}
};

// GALAGA_NCI (RSO2 0x80d88cec; GALAGA_NCIx is its subclass holding the chips, 0x1a0 bytes): the command latch and the dispatch to the chips
struct Nci
{
	u8 f4 = 0;          // +4 (Setup stores 0)
	u8 cmd = 0x10;      // +5: the latched command (lowest set bit of the low nibble selects the chip)
	u8 cmdRead = 0x10;  // +6: what a read of the command port returns
	u8 nmiEnable = 0;   // +7
	N51 n51;            // +8
	N54 n54;            // +0x168

	// RSO2 0x80d88dc4
	void CmdWrite(u32 v)
	{
		if (v == 0x10)
		{
			nmiEnable = 0;
			cmdRead = 0x10;
			cmd = 0x10;
			return;
		}
		cmd = (u8)v;
		cmdRead = (u8)v;
		nmiEnable = 1;
	}
	// RSO2 0x80d88df8 (the chips 1 / 2 are empty: Data_Read1 / 2 return 0 and the writes do nothing; none selected: 0xff)
	u32 DataRead(u32 addr)
	{
		if (cmd & 1) return n51.DataRead(addr);   // GALAGA_NCIx::Data_Read0 RSO2 0x80d671b4
		if (cmd & 2) return 0;                    // Data_Read1 0x80d6718c
		if (cmd & 4) return 0;                    // Data_Read2 0x80d67184
		if (cmd & 8) return n54.DataRead(addr);   // Data_Read3 0x80d671a4
		return 0xff;
	}
	// RSO2 0x80d88e74
	void DataWrite(u32 addr, u32 value)
	{
		if (cmd & 1) n51.DataWrite(addr, value);   // Data_Write0 0x80d6719c
		else if (cmd & 2) {}                       // Data_Write1 0x80d67180
		else if (cmd & 4) {}                       // Data_Write2 0x80d6717c
		else if (cmd & 8) n54.DataWrite(value);    // Data_Write3 0x80d67194
	}
};
std::unique_ptr<Nci> nci;   // GALAGA::GALAGA_NCI2

// ---- the sound chip (SND_GALAGA_WSG3, RSO2 0x80d64f0c): 3 wave voices + one PCM voice, mono ----
class SndGalagaWsg3 : public SoundModule
{
public:
	// RSO2 0x80d64f0c: the waves are the PROM's low nibbles - 8 (8 waves x 32 samples); the factor = 12582912000000 / (1000 * rate)
	SndGalagaWsg3(PrimaryStream& p, const u8* rom) : rate(p.Rate())
	{
		const double r = (double)(float)((s64)rate << 40);
		factor = (u32)ToU64((double)(12582912000000.0f / (1000.0f * (float)(r / (double)kETimeSecond))));
		stream = p.CreateStream("NAMCO WSG3", 1, 0x2000, this);
		for (int i = 0; i < 0x100; i++) waves[i] = (std::int8_t)((rom[i] & 0xf) - 8);
		Reset();
	}
	// RSO2 0x80d651a0
	void Update(int n) override
	{
		for (int k = 0; k < n; k++)
		{
			std::int16_t sum = 0;
			for (int v = 0; v < 3; v++)
			{
				const u32 st = step[v];
				if (st == 0) continue;
				const u32 q = st >> 2;
				const u32 p1 = phase[v] + q;
				const u32 p2 = p1 + q;
				const u32 p3 = p2 + q;
				const std::int8_t* w = waves + wave[v];
				const int s1 = w[(p1 >> 16) & 0x1f] * 13107;   // SND_GALAGA_WSG3::@LOCAL@Update 0x8102cf10 .. 0x8102cf1c
				const int s2 = w[(p2 >> 16) & 0x1f] * 19660;
				const int s3 = w[(p3 >> 16) & 0x1f] * 19660;
				const int s4 = w[((p3 + q) >> 16) & 0x1f] * 13107;
				phase[v] += st;
				sum = (std::int16_t)(sum + (std::int16_t)(((s1 + s2 + s3 + s4) * (int)vol[v]) >> 10));
			}
			if (pcmFlags & 1)
			{
				const u32 posHi = (u32)(pcmPos >> 32), posLo = (u32)pcmPos;
				const u32 idx = (posHi << 16) | (posLo >> 16);
				if (idx < pcmLength)
				{
					const u32 at = (posHi << 16) | ((posLo >> 16) & 0xfffe);
					const std::int16_t s = (std::int16_t)((u16)pcmData[at] << 8 | pcmData[at + 1]);
					pcmPos += pcmStep;
					sum = (std::int16_t)(sum + (std::int16_t)((u32)(pcmVolume * (int)s) >> 16));
				}
				else
				{
					pcmFlags &= ~1u;
				}
			}
			stream->Put(sum);
		}
		stream->written += n;
	}
	// RSO2 0x80d65340: the steps are NOT reset (they sit at +0x14..)
	void Reset() override
	{
		std::memset(phase, 0, sizeof(phase));
		std::memset(vol, 0, sizeof(vol));
		std::memset(wave, 0, sizeof(wave));
		stream->writeIndex = 0;
		stream->readIndex = 0;
		pcmFlags = 0;
	}
	// RSO2 0x80d653b0 (regs = the memory at 0x6800: the voices' frequency / wave / volume bytes at +0x10.. / +5 / +0xa / +0xf)
	void SetRegister(const u8* r)
	{
		auto stepOf = [&](u32 freq) { return (u32)(((u64)factor * freq) >> 16); };
		step[0] = stepOf((r[0x10] & 0xfu) | (r[0x11] & 0xfu) << 4 | (r[0x12] & 0xfu) << 8 | (r[0x13] & 0xfu) << 12 | (r[0x14] & 0xfu) << 16);
		step[1] = stepOf((r[0x16] & 0xfu) << 4 | (r[0x17] & 0xfu) << 8 | (r[0x18] & 0xfu) << 12 | (r[0x19] & 0xfu) << 16);
		step[2] = stepOf((r[0x1b] & 0xfu) << 4 | (r[0x1c] & 0xfu) << 8 | (r[0x1d] & 0xfu) << 12 | (r[0x1e] & 0xfu) << 16);
		vol[0] = r[0x15] & 0xf;
		wave[0] = (u16)((r[0x05] & 7) << 5);
		vol[1] = r[0x1a] & 0xf;
		wave[1] = (u16)((r[0x0a] & 7) << 5);
		vol[2] = r[0x1f] & 0xf;
		wave[2] = (u16)((r[0x0f] & 7) << 5);
	}
	// RSO2 0x80d654cc: data / length (end - start) / loop (0 = none) / volume (1.0 = 0x10000) / the sample's rate; step = (rate << 16) / audio rate, << 1
	void SetPCM(int srate, s32 volume, const u8* data, u32 length)
	{
		pcmFlags = 0;
		pcmData = data;
		pcmLength = length;
		pcmVolume = volume;
		const float q = (float)(double)((u32)srate << 16) / (float)rate;
		pcmStep = (u64)(double)q << 1;
	}
	void Play() { pcmPos = 0; pcmFlags |= 1; }   // RSO2 0x80d65588
	void Stop() { pcmFlags &= ~1u; }             // RSO2 0x80d655a4
	void Cont() { pcmFlags |= 1; }               // RSO2 0x80d655b4

private:
	int rate;
	u32 factor = 0;                  // +0x154 (+0x150 = 0)
	u32 phase[3] = {};               // +8
	u32 step[3] = {};                // +0x14
	u16 wave[3] = {};                // +0x20
	u16 vol[3] = {};                 // +0x26
	std::int8_t waves[0x100] = {};   // +0x41
	SoundStream* stream = nullptr;   // +0x148
	u32 pcmFlags = 0;                // +0x158
	u32 pcmLength = 0;               // +0x160
	const u8* pcmData = nullptr;     // +0x168
	s32 pcmVolume = 0;               // +0x16c
	u64 pcmPos = 0;                  // +0x170 (hi) / +0x174 (lo)
	u64 pcmStep = 0;                 // +0x178 / +0x17c
};
std::unique_ptr<SndGalagaWsg3> wsg;   // DAT_812095d0

// ---- forward declarations ----
void CopyrightStringSet();
void SaveDataToGameSetting();
void OptionsWork_to_System();
void EntryGameSetting(const s32* list);
void DIPSWSet();
void CRIGHT_POP();
void ReflectGameSetting();
void ExportDataDefaultReset();
void OptionsMenuBuild();

// ---- the Z80 buses (RSO2 0x80d65708..0x80d65a74) ----
// CPU2 / CPU3 read their ROM through a pointer into the module's data (the arrays are only 0x1000 bytes: a read past them runs into the next array
// of the module, as in the original)
u8 RomRead(const u8* rom, u32 romBase, u16 a)
{
	if (a < 0x1000) return rom[a];
	const u8* p = RsoModule::Volume(2).At(romBase + a, 1);
	return p != nullptr ? *p : 0;
}

// RSO2 0x80d65768. Quirk: whenever the NCI's status byte (+7) is set and the CPU's NMI-active bit (status 0x10) is clear, the read raises CPU1's NMI
u8 CPU1_ReadMemory8(u16 a)
{
	if (nci->nmiEnable != 0 && (env1.status() & 0x10) == 0) z80cpu1.SetFlag(&env1, CPUFLAG_NMI, (void*)1);
	if (a < 0x6800) return mem[a];
	const u16 m = a & 0xfff8;
	if (m == 0x6800) return io[a & 7];
	if (m == 0x7000) return (u8)nci->DataRead(a);
	if (m == 0x7100) return nci->cmdRead;   // GALAGA_NCI::CMD_Read RSO2 0x80d88db4
	return mem[a];
}
u8 CPU1_ReadM1(u16 a) { return mem[a]; }   // RSO2 0x80d65848
// RSO2 0x80d65858
void CPU1_WriteMemory8(u16 a, u8 v)
{
	if (a < 0x6800) return;
	if (a == 0x6823) cpu23ResetActive = (v & 1) == 0;
	if ((a & 0xfff0) != 0x7000)
	{
		if ((a & 0xfff0) != 0x7100)
		{
			mem[a] = v;
			return;
		}
		nci->CmdWrite(v);
		return;
	}
	nci->DataWrite(a, v);
}
u8 CPU1_ReadIO8(u16) { return 0xff; }   // RSO2 0x80d658c8
void CPU1_WriteIO8(u16, u8) {}          // RSO2 0x80d658d0

// RSO2 0x80d658d4
u8 CPU2_ReadMemory8(u16 a)
{
	if (a < 0x6800) return RomRead(rom2, 0x8103d088u, a);
	const u16 m = a & 0xfff8;
	if (m == 0x6800) return io[a & 7];
	if (m == 0x7000) return (u8)nci->DataRead(a);
	if (m != 0x7100) return mem[a];
	return nci->cmdRead;
}
u8 CPU2_ReadM1(u16 a) { return RomRead(rom2, 0x8103d088u, a); }   // RSO2 0x80d6594c
// RSO2 0x80d6595c
void CPU2_WriteMemory8(u16 a, u8 v)
{
	if (a < 0x6800) return;
	if (a == 0x7100)
	{
		nci->CmdWrite(v);
		return;
	}
	mem[a] = v;
}
u8 CPU2_ReadIO8(u16) { return 0xff; }
void CPU2_WriteIO8(u16, u8) {}

// RSO2 0x80d65994
u8 CPU3_ReadMemory8(u16 a)
{
	if (a < 0x6800) return RomRead(rom3, 0x8103e088u, a);
	const u16 m = a & 0xfff8;
	if (m == 0x6800) return io[a & 7];
	if (m == 0x7000) return (u8)nci->DataRead(a);
	if (m != 0x7100) return mem[a];
	return nci->cmdRead;
}
u8 CPU3_ReadM1(u16 a) { return RomRead(rom3, 0x8103e088u, a); }   // RSO2 0x80d65a0c
// RSO2 0x80d65a1c: the sound CPU's writes to 0x6800..0x681f also feed the WSG3 registers
void CPU3_WriteMemory8(u16 a, u8 v)
{
	if (a < 0x6800) return;
	if (a == 0x7100)
	{
		nci->CmdWrite(v);
		return;
	}
	mem[a] = v;
	if ((u16)(a + 0x9800) < 0x20) wsg->SetRegister(&mem[0x6800]);
}
u8 CPU3_ReadIO8(u16) { return 0xff; }
void CPU3_WriteIO8(u16, u8) {}

// RSO2 0x80d66f88: the N54's callback, only the PCM commands do something
void N54_Callback(int cmd)
{
	if (cmd == 0x10) wsg->Play();
	else if (cmd == 0x20) wsg->Play();
}

// ---- the game's helpers the hooks share ----
// RSO2 0x80d7b828
int GetContinueRound() { return gameMenu->GetContinueRound(gameMenu->players); }

// RSO2 0x80d7c314 (called from the 0x3a1 / 0x18ca hooks only when *GALAGA::Copyright is set, which nothing does): the copyright lines of mem[0x7800..] /
// mem[0x7880..] are written into the tile map, one column down (-0x20 per character), colour in mem[+0x400]
void CRIGHT_POP()
{
	const u8 c1 = mem[0x7802];
	u16 p = (u16)(mem[0x7801] << 8 | mem[0x7800]);
	for (u16 s = 0x7803;; s++)
	{
		const u8 ch = mem[s];
		if (ch == 0x2f) break;
		mem[(u16)(p + 0x400)] = c1;
		mem[p] = ch;
		p = (u16)(p - 0x20);
	}
	const u8 c2 = mem[0x7882];
	p = (u16)(mem[0x7881] << 8 | mem[0x7880]);
	for (u16 s = 0x7883;; s++)
	{
		const u8 ch = mem[s];
		if (ch == 0x2f) break;
		mem[(u16)(p + 0x400)] = c2;
		mem[p] = ch;
		p = (u16)(p - 0x20);
	}
}

// the hi score block: the game's five 9-byte records at mem[0x8a20..0x8a4c] <-> the bank's records at save+0x198 + bank * 0x2e
// RSO2 0x80d7beb0
void HiScorePush()
{
	for (int i = 0; i < 0x2d; i++) save[0x198 + Bank() * 0x2e + i] = mem[0x8a20 + i];
	sw.b(0x53) = 1;
}
// RSO2 0x80d7c000
bool HiScorePop()
{
	if (sw.b(0x53) == 0) return false;
	for (int i = 0; i < 0x2d; i++) mem[0x8a20 + i] = save[0x198 + Bank() * 0x2e + i];
	return true;
}
// RSO2 0x80d7a418 (the 0x19a hook has it inlined): the furthest round of the bank, 1 less than the game's round counters
void ArrivalRoundUpdate()
{
	if (sw.b(0x2f) == 0) return;
	if (((sw.b(0xae) >> 5) & 1) != 0) return;
	u8 r = mem[0x9821];
	if (mem[0x9821] < mem[0x9861]) r = mem[0x9861];
	u8& best = save[0x1c5 + Bank() * 0x2e];
	if (r <= best) return;
	best = (u8)(r - 1);
	sw.b(0x54) = 1;
}
// RSO2 0x80d7c554
void HiScoreDisp(int show)
{
	for (int i = 0; i < 6; i++) mem[0x83ed + i] = show != 0 ? mem[0x8a20 + i] : 0x24;
}

// the hook's "yield": the original stores 0 into the CPU's cycle counter (the first word of its env), ending the slice; PC stays on the patched address
void Yield(Z80_env& e) { e.setCycles(0); }

// the game's own part of BootStart / the 0x3382 hook
void BootStartBody()
{
	CopyrightStringSet();
	sw.b(0x52) = 1;
	if (sw.b(0x51) == 0)
	{
		Breg(env1) = 10;
		SetPC(env1, 0x3384);
	}
	else
	{
		SetSw32(0x14, 1);
		SetSw32(0x18, 1);
		SetPC(env1, 0x346f);
	}
	SaveDataToGameSetting();
	OptionsWork_to_System();
	dipswA = 0xf4;
	dipswB = 0x97;
	EntryGameSetting(gsDefault);
	if (gsCurrentPtr != 0) EntryGameSetting(gsCurrent);
	DIPSWSet();
}

// RSO2 0x80d76c44: the hooks shared by the Galaga variants (CPU1)
void CPU1_ExtFuncExtendCommon()
{
	switch (PC(env1))
	{
	case 0x33b2: SetPC(env1, 0x33b5); break;
	case 0x33eb: SetPC(env1, 0x33ee); break;
	case 0x3424: SetPC(env1, 0x3427); break;
	case 0x34bc: SetPC(env1, 0x34bf); break;
	case 0x353e: SetPC(env1, 0x34c9); break;
	case 0x6f6:
	{
		// a push of the return address 0x06f7 straight into mem (s_wiinewdel_h_8102d07c[SP + 0xb] / [SP + 10] = mem[SP - 1] / mem[SP - 2])
		SetPC(env1, 0x37f6);
		const u16 sp = SP(env1);
		mem[(u16)(sp - 1)] = 0x06;
		mem[(u16)(sp - 2)] = 0xf7;
		env1.set16(Z80_env::SP, (u16)(sp - 2));
		break;
	}
	case 0x3604:
		A(env1) = 0xa8;
		SetPC(env1, 0x3609);
		if (sw.b(0x6c) != 0)
		{
			sw.b(0x6c) = 0;
			SetPC(env1, 0x3606);
		}
		break;
	default:
		// the original returns silently (the patched byte would run again for ever): say so once
		if (!warnedHook)
		{
			std::fprintf(stderr, "gotch::galaga: CPU1 has no hook for PC 0x%04x (the original hangs here)\n", (unsigned)PC(env1));
			warnedHook = true;
		}
		break;
	}
}

// the pair of wait loops "if (cond) yield, else jump": the shape of most of the unpatched handlers
void WaitOr(Z80_env& e, bool stay, u16 next)
{
	if (stay) Yield(e);
	else SetPC(e, next);
}

// RSO2 0x80d7c944 (CPU1_ExtFuncExtend; GALAGA::CPU1_ExtFunc 0x80d65a78 is a thunk to it): RST 0 over the patched ROM addresses, PC = the patched address.
// The patched addresses are the ones EntryExtfuncExtend (0x80d7b658) writes 0xC7 over; a few handlers below belong to addresses nothing patches (the
// developers' other builds): they are kept for completeness.
void CPU1_ExtFuncExtend()
{
	Z80_env& e = env1;
	const u8 a0 = A(e);   // (bVar6)
	const u8 bVar3 = mem[0x9828];
	switch (PC(e))
	{
	case 0x1125:
		if ((CPU1_ReadMemory8(0x9840) & 1) != 0)
		{
			sw.b(4) = 0;
			sw.b(5) = 1;
		}
		else
		{
			sw.b(4) = 1;
			sw.b(5) = 0;
		}
		SetHL(e, 0x8800);
		SetPC(e, 0x1128);
		break;
	case 0x414:   // (unpatched)
		A(e) = mem[HL(e)];
		if (A(e) != 0) Yield(e);
		else SetPC(e, 0x418);
		break;
	case 0x35a:
		gameState->ChangeState(sw.b(0x34) == 0 && sw.b(0x35) == 0 && sw.b(0x33) == 0 ? 3 : 4);
		sw.b(0x52) = 0;
		A(e) = 0;
		SetPC(e, 0x35b);
		sw.b(0x2f) = 0;
		HiScorePush();
		HiScorePop();
		for (int i = 0; i < 6; i++) mem[0x83ed + i] = mem[0x8a20 + i];
		sw.b(0x20) = 1;
		gameMenu->EnableMenu();
		SetSw32(0x18, 1);
		sw.b(0x75) = 0;
		sw.b(0x114) = 0;
		sw.b(0x115) = 0;
		sw.b(0x118) = 0;
		sw.b(0x119) = 0;
		if (sw.b(0x33) == 0 && sw.b(0x34) == 0 && sw.b(0x35) == 0)
		{
			SetSw32(0x18, 1);
			sw.b(0x75) = 0;
			sw.b(0x114) = 0;
			sw.b(0x115) = 0;
			sw.b(0x118) = 0;
			sw.b(0x119) = 0;
			break;
		}
		nci->n51.SetCredits(2);
		SetSw32(0x18, 10);
		sw.b(0x20) = 0;
		sw.b(0x33) = 0;
		break;
	case 0x185:
		sw.b(0x75) = 1;
		SetHL(e, 0x9821);
		SetPC(e, 0x188);
		break;
	case 0x18:   // (unpatched) memset(HL, A, B)
		std::memset(&mem[HL(e)], A(e), std::min<size_t>(Breg(e), 0x10000 - HL(e)));
		e.addCycles(-(s32)(Breg(e) * 0x1a - 5));   // cycles = cycles - (B * 0x1a - 5)
		SetHL(e, (u16)(HL(e) + Breg(e)));
		Breg(e) = 0;
		SetPC(e, 0x1c);
		break;
	case 0x10:   // (unpatched) RST 10h: return, HL += A
	{
		const u16 sp = SP(e);
		SetPC(e, (u16)(mem[(u16)(sp + 1)] << 8 | mem[sp]));
		e.addCycles(-0x1b);
		SetHL(e, (u16)(HL(e) + A(e)));
		e.set16(Z80_env::SP, (u16)(sp + 2));
		break;
	}
	case 0:
		if (sw.b(0x34) == 0 && sw.b(0x35) == 0)
		{
			gameState->ChangeState(2);
			if (sw.b(0x33) != 0) machine.skipFrames = 0x14;
		}
		else
		{
			gameState->ChangeState(4);
			machine.skipFrames = 0x14;
		}
		A(e) = 0x10;
		SetPC(e, 2);
		break;
	case 0xdc:
		SetPC(e, 0xdf);
		Breg(e) = 5;
		if (sw.b(0x53) != 0)
		{
			for (int i = 0; i < 0x2d; i++) mem[0x8a20 + i] = save[0x198 + Bank() * 0x2e + i];
			Breg(e) = 6;
			SetHL(e, 0x8a20);
		}
		break;
	case 0x1bf:   // (unpatched)
		A(e) = mem[0x92ae];
		if (mem[0x92ae] != 0) Yield(e);
		else SetPC(e, 0x1c5);
		break;
	case 0x19a:
	{
		const u8 l = (u8)HL(e);
		SetHL(e, (u16)(HL(e) & 0xff));
		SetPC(e, 0x19c);
		if (((save[0x1f6] >> 3) & 1) != 0 && mem[0x9821] == 0)
		{
			mem[0x9821] = 1;
			SetHL(e, (u16)(u8)(l + 1));
		}
		if (sw.b(0x2f) == 0) break;
		if (((sw.b(0xae) >> 5) & 1) != 0) break;
		const u8 arrival = save[0x1c5 + Bank() * 0x2e];
		u8 r = mem[0x9821];
		if (r < mem[0x9861]) r = mem[0x9861];
		if (r <= arrival) break;
		save[0x1c5 + Bank() * 0x2e] = (u8)(r - 1);
		sw.b(0x54) = 1;
		break;
	}
	case 0x189:
		A(e) = mem[HL(e)];
		SetPC(e, 0x18a);
		if (mem[0x9821] == 0) sw.b(0x118 + mem[0x9840]) = (u8)(sw.b(0x118 + mem[0x9840]) + 1);
		break;
	case 0x2aa:
		CPU1_WriteMemory8(0x6820, A(e));
		SetPC(e, 0x2ad);
		if (sw.b(0x6d) != 0)
		{
			// the pushes of the return address 0x02ad through the bus (a write below 0x6800 would be ignored: the stack is RAM)
			CPU1_WriteMemory8((u16)(SP(e) - 1), 2);
			CPU1_WriteMemory8((u16)(SP(e) - 2), (u8)PC(e));
			e.set16(Z80_env::SP, (u16)(SP(e) - 2));
			SetPC(e, 0x37fe);
			break;
		}
		if ((CPU1_ReadMemory8(0x9201) & 0xff) != 2) break;
		if (sw.b(0x34) != 0)
		{
			sw.b(0x34) = 0;
			swork_SetWork_GameStart(sw, -1, -1);
			nci->n51.StartButton(1);
			gameState->ChangeState(4);
			swork_SetWork_1P_PauseStatus(sw, 1, 10);
			swork_SetWork_2P_PauseStatus(sw, 0, 10);
		}
		if (sw.b(0x35) != 0)
		{
			sw.b(0x35) = 0;
			swork_SetWork_GameStart(sw, -1, -1);
			nci->n51.StartButton(2);
			gameState->ChangeState(4);
			swork_SetWork_1P_PauseStatus(sw, 1, 10);
			swork_SetWork_2P_PauseStatus(sw, 0, 10);
		}
		break;
	case 0x3c4:
		Creg(e) = 0x1c;
		SetPC(e, sw.b(0x2e) != 0 ? 0x3c6 : 0x3c9);
		break;
	case 0x3a4:
		Creg(e) = 1;
		SetPC(e, sw.b(0x2e) != 0 ? 0x3a6 : 0x3a7);
		break;
	case 0x3b5:
		Creg(e) = 0x1b;
		SetPC(e, sw.b(0x2e) != 0 ? 0x3b7 : 0x3ba);
		break;
	case 0x3d3:
		Creg(e) = 0x1d;
		SetPC(e, sw.b(0x2e) != 0 ? 0x3d5 : 0x3d8);
		break;
	case 0x3a1:
	case 0x18ca:
		if (PC(e) == 0x18ca) sw.b(0x73) = 1;
		if (copyrightFlag == 0)
		{
			SetPC(e, (u16)(PC(e) + 3));
			break;
		}
		if (PC(e) == 0x3a1)
		{
			SetPC(e, 0x3a4);
			break;
		}
		if (sw.b(0x29) == 0) CRIGHT_POP();
		SetPC(e, (u16)(PC(e) + 3));
		break;
	case 0x38d:   // (unpatched)
		A(e) = mem[0x9201];
		if (mem[0x9201] == 1) Yield(e);
		else SetPC(e, 0x393);
		break;
	case 0x3d8:   // (unpatched)
		A(e) = mem[0x9201];
		if (mem[0x9201] == 2) Yield(e);
		else SetPC(e, 0x3df);
		break;
	case 0x40f:
		if (sw.b(0x39) != 0)
		{
			mem[0x9821] = (u8)GetContinueRound();
			mem[0x9861] = (u8)GetContinueRound();
		}
		SetHL(e, 0x92af);
		SetPC(e, 0x412);
		break;
	case 0x6ac:   // (unpatched)
		A(e) = mem[0x92a0] & 0xf;
		if ((mem[0x92a0] & 0xf) == 0) Yield(e);
		else SetPC(e, 0x6b3);
		break;
	case 0x4fd:
		if (sw.b(0x2e) == 0) nci->n51.SetCredits(0);
		if (mem[0x99b3] == 0)
		{
			sw.b(5) = 0;
			sw.b(4) = 0;
			sw.b(7) = 0;
			sw.b(6) = 0;
		}
		else if (mem[0x9840] == 0)
		{
			sw.b(4) = 0;
			sw.b(6) = 0;
		}
		else
		{
			sw.b(5) = 0;
			sw.b(7) = 0;
		}
		Creg(e) = 2;
		SetPC(e, 0x4ff);
		break;
	case 0x4c1:   // (unpatched)
		A(e) = mem[HL(e)];
		if (A(e) != 0)
		{
			Yield(e);
			SetPC(e, 0x4a4);
		}
		else
		{
			SetPC(e, 0x4c5);
		}
		break;
	case 0x4b9:   // (unpatched)
		A(e) = mem[0x901d];
		if (mem[0x901d] != 0) Yield(e);
		else SetPC(e, 0x4bf);
		break;
	case 0x4a4:   // (unpatched)
		A(e) = mem[0x901d];
		if (mem[0x901d] == 0)
		{
			Yield(e);
			SetPC(e, 0x4c1);
		}
		else
		{
			SetPC(e, 0x4aa);
		}
		break;
	case 0x4e2:
		SetHL(e, 0x9820);
		SetPC(e, ((sw.b(0xae) >> 4) & 1) == 0 ? 0x4e5 : 0x579);
		break;
	case 0x565:
		SetSw32(0x58, 0xf);
		A(e) = mem[0x99b3];
		SetPC(e, 0x568);
		break;
	case 0x54a:
		sw.b(0x54) = 1;
		A(e) = 0;
		SetPC(e, 0x54b);
		break;
	case 0x540:
		SetPC(e, 0x541);
		if (bGameOverWaitSkip)
		{
			bGameOverWaitSkip = false;
			mem[HL(e)] = 0;
		}
		A(e) = mem[HL(e)];
		if (A(e) != 0) Yield(e);
		else SetPC(e, (u16)(PC(e) + 3));
		break;
	case 0x69b:   // (unpatched)
		A(e) = mem[0x92a0] & 0xf;
		if ((mem[0x92a0] & 0xf) != 0) Yield(e);
		else SetPC(e, 0x6a2);
		break;
	case 0x9a4:
		SetDE(e, 0x803c);
		SetPC(e, sw.b(0x2e) != 0 ? 0x9a7 : 0x9e1);
		break;
	case 0x850:   // (unpatched)
		if (mem[0x92d7] == 1) Yield(e);
		else SetPC(e, 0x856);
		break;
	case 0x7f8:   // (unpatched)
		A(e) = 0;
		SetPC(e, 0x7f9);
		if (HL(e) == 0x83fd || HL(e) == 0x83e8) sw.b(0x114 + mem[0x9840]) = (u8)(sw.b(0x114 + mem[0x9840]) + 1);
		break;
	case 0x741:   // (unpatched)
		A(e) = mem[HL(e)];
		if (A(e) == 0)
		{
			Yield(e);
			SetPC(e, 0x762);
		}
		else
		{
			SetPC(e, 0x745);
		}
		break;
	case 0x977:
		SetPC(e, 0x97a);
		A(e) = CPU1_ReadMemory8(0x99b5);
		if (sw.b(0x33) == 0 && sw.b(0x34) == 0 && sw.b(0x35) == 0) break;
		nci->n51.SetCredits(2);
		SetSw32(0x18, 10);
		sw.b(0x20) = 0;
		sw.b(0x33) = 0;
		break;
	case 0xa14:
		sw.b(4) = 1;
		SetSw32(0xc, 1);
		sw.b(0x26) = 1;
		A(e) = 3;
		SetPC(e, 0xa16);
		break;
	case 0xa0c:
		sw.b(0x2f) = 1;
		A(e) = (u8)(A(e) - 1);
		SetPC(e, 0xa0d);
		break;
	case 0xa23:
		if (sw.b(0x29) != 0)
		{
			SetPC(e, 0xa26);
			break;
		}
		mem[0x9a79] = A(e);
		if (sw.b(0x2e) == 0 && 1 < A(e)) mem[0x9a79] = 1;
		SetPC(e, 0xa26);
		break;
	case 0x35f3:   // (unpatched)
		A(e) = mem[0x92a0];
		if (mem[0x92a0] != 2) Yield(e);
		else SetPC(e, 0x35fa);
		break;
	case 0x19d7:
	{
		mem[0x92ad] = A(e);
		if (sw.b(0x90) != 0)
		{
			SetPC(e, 0x19da);
			break;
		}
		const u8 idx = mem[0x9828] & 7;
		const u8 next = (u8)(idx + 1);
		mem[(u16)(0x9300 | idx)] = mem[0x9362];
		mem[0x9362] = 0;
		mem[(u16)(0x9300 | next)] = mem[0x9363];
		A(e) = mem[0x9b63];
		mem[(u16)(0x9b00 | next)] = mem[0x9b63];
		mem[(u16)(0x8b00 | next)] = 7;
		mem[(u16)(0x8b00 | (bVar3 & 7))] = 7;
		SetDE(e, (u16)(0x9b00 | next));
		SetHL(e, (u16)(0x8b00 | (bVar3 & 7)));
		SetPC(e, 0x19de);
		break;
	}
	case 0x134c:   // (unpatched)
		A(e) = mem[0x9287];
		if (mem[0x9287] != 0) Yield(e);
		else SetPC(e, 0x1352);
		break;
	case 0x121d:   // (unpatched)
		A(e) = (u8)(mem[0x92a0] - Dreg(e));
		if (A(e) != 0) Yield(e);
		else SetPC(e, 0x1223);
		break;
	case 0x1192:
		A(e) = mem[HL(e)];
		SetPC(e, sw.b(0x90) != 0 ? 0x1193 : 0x1197);
		break;
	case 0x1184:
		A(e) = mem[HL(e)];
		SetPC(e, sw.b(0x90) != 0 ? 0x1185 : 0x1189);
		break;
	case 0x1337:
	{
		SetPC(e, 0x1338);
		const u16 ret = (u16)(mem[(u16)(SP(e) + 3)] << 8 | mem[(u16)(SP(e) + 2)]);
		if ((ret == 0x503 || ret == 0x506) && bGameOverWaitSkip)
		{
			bGameOverWaitSkip = false;
			mem[HL(e)] = 0;
		}
		A(e) = mem[HL(e)];
		if (A(e) != 0) Yield(e);
		else SetPC(e, (u16)(PC(e) + 3));
		break;
	}
	case 0x13b7:
		A(e) = Creg(e);
		if (copyrightFlag != 0 || (Creg(e) != 0xe && Creg(e) != 0x13)) SetPC(e, 0x13b8);
		else SetPC(e, 0x13ef);
		break;
	case 0x1399:
	{
		const u8 nd = (u8)(Dreg(e) + 1);
		Dreg(e) = nd;
		if (sw.b(0x90) != 0)
		{
			SetPC(e, 0x139a);
			break;
		}
		A(e) = 5;
		Breg(e) = nd;
		while (A(e) != 0)
		{
			if (Ereg(e) == A(e)) Breg(e) = 0x24;
			const u8 l = (u8)((u8)HL(e) - 2);
			A(e) = (u8)(A(e) - 1);
			mem[HL(e)] = Breg(e);
			SetHL(e, (u16)((HL(e) & 0xff00) | l));
		}
		SetPC(e, 0x13b1);
		break;
	}
	case 0x1382:
		A(e) = (u8)(A(e) + (sw.b(0x90) == 0 ? 6 : 9));
		SetPC(e, 0x1384);
		(void)a0;
		break;
	case 0x3382:
		BootStartBody();
		break;
	case 0x30c2:   // (unpatched)
		A(e) = mem[0x92a0];
		WaitOr(e, Creg(e) == mem[0x92a0], 0x30c8);
		break;
	case 0x30aa:   // (unpatched)
		A(e) = mem[0x92ae];
		WaitOr(e, mem[0x92ae] != 0, 0x30b0);
		break;
	case 0x1f05:
		SetPC(e, 0x1f09);
		if (sw.b(0x90) == 0) break;
		SetHL(e, (u16)(0x8800 | (u8)HL(e)));
		mem[HL(e)] = 0x80;
		break;
	case 0x3179:   // (unpatched)
		A(e) = mem[0x92a0];
		WaitOr(e, mem[0x92a0] != 0, 0x317f);
		break;
	case 0x352b:   // (unpatched)
		SetPC(e, 0x34c9);
		break;
	case 0x3472:
		if (sw.b(0x51) != 0)
		{
			SetPC(e, 0x3478);
			break;
		}
		SetHL(e, 0x3b8b);
		SetPC(e, 0x3475);
		break;
	case 0x35bf:
		if (sw.b(0x51) != 0)
		{
			SetPC(e, 0x35c5);
			break;
		}
		SetHL(e, 0x3b95);
		SetPC(e, 0x35c2);
		break;
	case 0x3819:
		A(e) = Creg(e);
		if (sw.b(0x6d) == 0)
		{
			SetPC(e, 0x381a);
		}
		else
		{
			sw.b(0x6d) = 0;
			SetPC(e, 0x381d);
		}
		break;
	case 0x3719:   // (unpatched)
		A(e) = (u8)(mem[0x92a0] << 1);
		WaitOr(e, (mem[0x92a0] & 0x80) == 0, 0x371f);
		break;
	case 0x36c8:   // (unpatched)
		A(e) = mem[0x92a0];
		WaitOr(e, 8 < mem[0x92a0], 0x36cf);
		break;
	case 0x362e:   // (unpatched)
		A(e) = mem[0x92a0];
		WaitOr(e, mem[0x92a0] == Creg(e), 0x3634);
		break;
	case 0x3623:   // (unpatched)
		A(e) = mem[0x92a0];
		WaitOr(e, (mem[0x92a0] & 8) == 0, 0x362a);
		break;
	case 0x36d9:
		if (sw.b(0x51) != 0)
		{
			SetPC(e, 0x3725);
			break;
		}
		SetHL(e, 0x8000);
		SetPC(e, 0x36dc);
		break;
	case 0x376d:
		sw.b(0x51) = 1;
		SetPC(e, 0x2d3);
		break;
	case 0x3769:
		A(e) = 0;
		SetPC(e, 0x376d);
		break;
	case 0x372e:   // (unpatched)
		A(e) = mem[0x92a0];
		WaitOr(e, mem[0x92a0] != 0, 0x3734);
		break;
	case 0x3808:
		if (sw.b(0x51) != 0)
		{
			SetPC(e, 0x380f);
			break;
		}
		SetHL(e, 0x3ad6);
		SetPC(e, 0x380b);
		break;
	case 0x38af:
		if (sw.b(0x51) != 0)
		{
			SetPC(e, 0x38b5);
			break;
		}
		SetHL(e, 0x3b11);
		SetPC(e, 0x38b2);
		break;
	case 0x3881:
		if (sw.b(0x51) != 0)
		{
			SetPC(e, 0x38b5);
			break;
		}
		SetDE(e, 0x82e8);
		SetPC(e, 0x3884);
		break;
	case 0x384b:
		if (sw.b(0x51) != 0)
		{
			SetPC(e, 0x3855);
			break;
		}
		A(e) = (u8)(A(e) + 1);
		SetPC(e, 0x384c);
		break;
	case 0x382d:
		if (sw.b(0x51) != 0)
		{
			SetPC(e, 0x383c);
			break;
		}
		SetHL(e, 0x3a72);
		SetPC(e, 0x3830);
		break;
	case 0x389a:
		SetHL(e, sw.b(0x90) != 0 ? 0x3b00 : 0x3b9f);
		SetPC(e, 0x389d);
		break;
	case 0x3937:
		if (sw.b(0x51) != 0)
		{
			SetPC(e, 0x393d);
			break;
		}
		SetHL(e, 0x3b71);
		SetPC(e, 0x393a);
		break;
	case 0x38dc:
		if (sw.b(0x51) != 0)
		{
			SetPC(e, 0x3927);
			break;
		}
		Creg(e) = 1;
		SetPC(e, 0x38de);
		break;
	case 0x3a1b:
		if (sw.b(0x51) != 0)
		{
			SetPC(e, 0x3a28);
			break;
		}
		SetHL(e, 0x822e);
		SetPC(e, 0x3a1e);
		break;
	default:
		CPU1_ExtFuncExtendCommon();
		break;
	}
}
void CPU1_ExtFunc() { CPU1_ExtFuncExtend(); }   // RSO2 0x80d65a78 (a thunk)

// RSO2 0x80d76d34
void CPU2_ExtFuncExtendCommon()
{
	if (PC(env2) != 0x593) return;
	A(env2) = 0xff;
	SetPC(env2, 0x595);
}
// RSO2 0x80d7e720 (GALAGA::CPU2_ExtFunc 0x80d65a7c is the same code inlined)
void CPU2_ExtFuncExtend()
{
	Z80_env& e = env2;
	switch (PC(e))
	{
	case 0x57f:   // (unpatched)
		A(e) = mem[DE(e)];
		if (A(e) != 0) Yield(e);
		else SetPC(e, 0x583);
		break;
	case 0x596:   // (unpatched)
		A(e) = mem[DE(e)];
		if (A(e) != 0) Yield(e);
		else SetPC(e, 0x59a);
		break;
	case 0x5bf:   // (unpatched)
	case 0x5e7:
		if (mem[0x92d6] == 1) Yield(e);
		else SetPC(e, 0x5ed);
		break;
	case 0x5b4:   // (unpatched)
		Yield(e);
		e.setStatus((u16)(e.status() | 0x20));
		SetPC(e, 0x5b1);
		break;
	case 0x69c:
		A(e) = mem[0x9008];
		SetPC(e, 0x69f);
		if (((sw.b(0xae) >> 6) & 1) != 0) SetPC(e, 0x6b6);
		break;
	default:
		CPU2_ExtFuncExtendCommon();
		break;
	}
}
void CPU2_ExtFunc() { CPU2_ExtFuncExtend(); }

// RSO2 0x80d76d5c
void CPU3_ExtFuncExtendCommon()
{
	if (PC(env3) != 0xa1) return;
	A(env3) = 0xff;
	SetPC(env3, 0xa3);
}
// RSO2 0x80d7e85c
void CPU3_ExtFuncExtend()
{
	Z80_env& e = env3;
	switch (PC(e))
	{
	case 0x8d:   // (unpatched)
		A(e) = mem[DE(e)];
		if (A(e) != 0) Yield(e);
		else SetPC(e, 0x91);
		break;
	case 0xa4:   // (unpatched)
		A(e) = mem[DE(e)];
		if (A(e) != 0) Yield(e);
		else SetPC(e, 0xa8);
		break;
	case 0xb9:   // (unpatched)
		Yield(e);
		e.setStatus((u16)(e.status() | 0x20));
		SetPC(e, 0xb9);
		break;
	case 0x299:
		SetPC(e, 0x33c);
		if (sw.b(0x90) == 0) SetPC(e, 0x29c);
		break;
	case 0x546:
		SetPC(e, 0x54a);
		if (sw.b(0x90) != 0 && A(e) == 7) SetPC(e, 0x563);
		break;
	case 0x270:
		SetPC(e, 0x273);
		if (sw.b(0x90) != 0) mem[0x9a70] = A(e);
		else mem[0x9a72] = A(e);
		break;
	default:
		CPU3_ExtFuncExtendCommon();
		break;
	}
}
void CPU3_ExtFunc() { CPU3_ExtFuncExtend(); }

// RSO2 0x80d76c00 / 0x80d7b658: RST 0 (0xC7) over the patched ROM addresses
void EntryExtfuncExtend()
{
	// EntryExtfuncExtendCommon
	for (u16 a : {0x33b2, 0x33eb, 0x3424, 0x34bc, 0x353e, 0x6f6, 0x3604}) mem[a] = 0xc7;
	rom2[0x593] = 0xc7;
	rom3[0xa1] = 0xc7;
	// EntryExtfuncExtend
	for (u16 a : {0x1184, 0x1192, 0x1382, 0x1399, 0x19d7, 0x1f05, 0x389a, 0x0000, 0x3769, 0x376d, 0x3382, 0x36d9, 0x3472, 0x3a1b, 0x35bf, 0x3808, 0x382d, 0x384b,
	              0x3881, 0x38af, 0x38dc, 0x3937, 0x0a0c, 0x03a1, 0x18ca, 0x13b7, 0x035a, 0x09a4, 0x03a4, 0x03b5, 0x03c4, 0x03d3, 0x0a14, 0x0a23, 0x04fd, 0x0189,
	              0x00dc, 0x054a, 0x0565, 0x02aa, 0x3819, 0x019a, 0x1337, 0x0540, 0x040f, 0x04e2, 0x0185, 0x0977, 0x1125})
		mem[a] = 0xc7;
	rom3[0x299] = 0xc7;
	rom3[0x546] = 0xc7;
	rom3[0x270] = 0xc7;
	rom2[0x69c] = 0xc7;
}
