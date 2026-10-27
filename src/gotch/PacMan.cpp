// PAC-MAN of the arcade collection vol 2 (nmwii2.rso, class PACMAN): a 1:1 port of the machine glue around the Z80 board (see include/gotch/PacMan.hpp).
// The original keeps everything in module globals (THISGAME_sw 0x811fa330, CPU1_Z80_ENV 0x811fa558, CPU1_MainMemory 0x80de71d0, ...); so does
// the port (file statics). Comments cite the original globals as "sw+0xNN" / "mem[0xNNNN]" (address rules: reversing notes, "PACMAN decode").
//
// Stubbed services (no code behind them yet; the calls are kept as comments where they occur):
//  * the Wii menu (WII_MENU: start / options / pause / continue round, MenuExec / MenuRender); GetContinueRound returns 0 (UNVERIFIED)
//  * the achievements' online side (the counters in the 0x204-byte object are kept)
//  * CROSSHATCH / SCREENADJUST, CDraw::FadeSet / PrintfUTF8 ("NOW SAVING"), ACHIEVEMENT::DebugRender
#include "gotch/PacMan.hpp"

#include "gotch/DataStorage.hpp"
#include "gotch/GameIO.hpp"
#include "gotch/GameState.hpp"
#include "gotch/Draw.hpp"
#include "gotch/Machine.hpp"
#include "gotch/Menu.hpp"
#include "gotch/RsoModule.hpp"
#include "gotch/Sound.hpp"
#include "gotch/SWork.hpp"
#include "gotch/Z80.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

namespace gotch
{
namespace pacman
{
namespace
{
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using s32 = std::int32_t;

// ---- module data (read from the player's nmwii2.rso at Setup) ----
const u8* kAsciiToGame = nullptr;   // AsciiCodeToGameCodeConvertTable__6PACMAN (RSO2 0x80de6ff8)
const u8* kInitTable4ff2 = nullptr; // InitTable4ff2__6PACMAN (0x80de7130)
const u8* kMenuPalet = nullptr;     // MENUPALET__6PACMAN (0x80df71d0, 0x25 palettes x 4 big-endian u32)
const u8* kPaletRom = nullptr;      // PALET_ROM__6PACMAN (0x80df7520)
const u8* kPaletClut = nullptr;     // PALET_CLUT__6PACMAN (0x80df7540)
const u8* kRandomTable = nullptr;   // RANDOM_TABLE__6PACMAN (0x80df7640; RANDOM_ptr reads run past its 0x800 bytes into the module)
const u8* kCgromA = nullptr;        // CGROM_A__6PACMAN (0x80df7e40)
const u8* kCgromB = nullptr;        // CGROM_B__6PACMAN (0x80df8e40)
const u8* kCgromConfig = nullptr;   // CGROM_CONFIG_V (0x80dd0278)
const u8* kFontPatch = nullptr;     // FontPatchTable__6PACMAN (0x80e02250)
const u8* kTable2c5e = nullptr;     // Table2c5e__6PACMAN (0x81148628)
const u8* kVersionJ = nullptr;      // VersionJStringTable__6PACMAN (0x81148494, 0x12 x 6 bytes)
const u8* kDipDefault = nullptr;    // IgnoreScreenFlip__6PACMAN (0x80de6ff0): +1 = DSW A default, +2 = DSW B default
const u8* kLayoutBg = nullptr;      // unexported CGen layouts (RSO2 0x80dff250 / 0x80dffa50 / 0x80e01a50)
const u8* kLayoutSprite = nullptr;
const u8* kLayoutConfig = nullptr;
std::string sNamcoLogo, sCopyright1, sCopyright2;  // NamcoLogo_String / Copyright1_String / Copyright2_String (inline C strings)
s32 kGameSettingDefault[4] = {-1, -1, -1, -1};      // GAME_SETTING_DEFAULT__6PACMAN (0x81147f68: {6, 1, 9, -1} in the retail module)
std::uint32_t kRandomAvail = 0;                     // bytes readable from RANDOM_TABLE on (the module file ends somewhere)

// ---- the module's globals ----
u8 mem[0x10000];             // CPU1_MainMemory 0x80de71d0 (the ROM image + RAM, copied from the module)
Z80_env env;                 // CPU1_Z80_ENV 0x811fa558
CPU_SPEC z80;                // Z80 0x811fa5a0
MACHINE_SPEC machine;        // Machine 0x811fa5e8
SWork sw;                    // THISGAME_sw 0x811fa330
u8 io[0xc];                  // IO 0x811fa830: IN0, IN1, DSW
u8 z80io[0x100];             // Z80IO 0x811fa730
u32 extendedScore[4];        // ExtendedScoreCounter 0x811fa320: {hi, lo} x 2 players
u16 randomPtr = 0;           // RANDOM_ptr 0x8120b4b8
s32 gsCurrent[256];          // GAME_SETTING_CURRENT 0x81147f78
s32 gsCurrentPtr = 0;        // GAME_SETTING_CURRENT_PTR
u8 dswA = 0, dswB = 0;       // DIPSW_A_CURRENT / DIPSW_B_CURRENT 0x8120b36c / d
u8 oldAtr[12], oldXY[12];    // CallRender's copies of the previous frame's sprite registers (0x811fb868 / 0x811fb874)
std::unique_ptr<GameState> gameState;   // PACMAN__GameState (written only: the frontend's view of the game)
std::unique_ptr<CDataStorage> storage;   // *0x811fa70c (InitMachine)
u8* save = nullptr;          // *(CDataStorage 0x811fa70c + 4): the export data (save block)
u8 achievement[0x204];       // *Achievement 0x811fa720
u8 copyright[8];             // *Copyright 0x811fa724
std::unique_ptr<CDraw> draw; // the machine's CDraw (*0x811fa704)
std::unique_ptr<PrimaryStream> primary;  // the audio spec's PRIMARYSTREAM (machine +0x94)
std::unique_ptr<SndNamcoWsg3> wsg;       // PACMAN::WSG
int texBg = -1, texSprite = -1, texConfig = -1;  // wdh_cgrom_bg / _sprite / _config 0x811fa858..
std::unique_ptr<CGameIO> gameio;   // *0x811fa700 (InitMachine: machine +0x118)
std::unique_ptr<WiiMenu> gameMenu;   // PACMAN__GameMenu (GAMEMENU = WII_MENU with empty PauseIn / PauseOut)
void (*soundBufferClear)() = nullptr;   // the console's CWiiSound::BufferClear (machine +0x120 -> +0xc0)
// PACMAN__OptionsMenuItem 0x8120b480..: BONUS (EXTEND), LIFE (LIVES), SOUND TEST, SCREEN ADJUSTMENT, DEFAULT, EXIT (owned by the options menu,
// rebuilt every frame by OptionsMenuBuild)
MenuItem* optionsItem[6] = {};
char soundTestText[0x40][8] = {};   // PACMAN__SOUND_TEST_NO_Buffer (0x40 buffers of 8 bytes, OptionsConstruct / MenuConstruct)
struct OptionStrings
{
	const char* options = "";        // MenuStr_OPTIONS (= MenuStr_STARTMENU_OPTIONS_LOCALE_NEUTRAL, NeutralResourceSet)
	const char* bonus = "";          // MenuStr_BONUS
	const char* none = "";           // MenuStr_NONE
	const char* life = "";           // MenuStr_LIFE
	const char* soundTest = "";      // MenuStr_SOUND_TEST
	const char* screenAdjust = "";   // MenuStr_NMREMIX_WII_SCREEN_ADJUST_TITLE
	const char* deflt = "";          // MenuStr_DEFAULT
	const char* exit = "";           // MenuStr_EXIT
} optStr;
bool isSetup = false;
const int kLocale = 1;  // SetGameLocale(1) in Setup; GetGameLocale returns it from then on

// ---- helpers over the original layouts ----
u8& A() { return env.r8(Z80_env::A); }
u8& B() { return env.r8(Z80_env::B); }
u8& C() { return env.r8(Z80_env::C); }
u8& E() { return env.r8(Z80_env::E); }
u8 F() { return env.r8(Z80_env::F); }
u16 HL() { return env.r16(Z80_env::HL); }
void SetHL(u16 v) { env.set16(Z80_env::HL, v); }
u16 DE() { return env.r16(Z80_env::DE); }
void SetDE(u16 v) { env.set16(Z80_env::DE, v); }
void SetBC(u16 v) { env.set16(Z80_env::BC, v); }
u16 IX() { return env.r16(Z80_env::IX); }
void SetIX(u16 v) { env.set16(Z80_env::IX, v); }
u16 SP() { return env.r16(Z80_env::SP); }
u16 PC() { return env.r16(Z80_env::PC); }
void SetPC(u16 v) { env.set16(Z80_env::PC, v); }
// the hooks push a return address straight into mem (s_wiinewdel_h_80de71c4[SP + 0xb] / [SP + 10] = mem[SP - 1] / mem[SP - 2])
void Push(u16 v)
{
	const u16 sp = SP();
	mem[(u16)(sp - 1)] = (u8)(v >> 8);
	mem[(u16)(sp - 2)] = (u8)v;
	env.set16(Z80_env::SP, (u16)(sp - 2));
}

s32 Sw32(int off) { return sw.i32(off); }
void SetSw32(int off, s32 v) { sw.set32(off, v); }
s32 Ach32(int off)
{
	s32 v;
	std::memcpy(&v, &achievement[off], 4);
	return v;
}
void SetAch32(int off, s32 v) { std::memcpy(&achievement[off], &v, 4); }
void SetSave32(int off, s32 v) { save[off] = (u8)(v >> 24); save[off + 1] = (u8)(v >> 16); save[off + 2] = (u8)(v >> 8); save[off + 3] = (u8)v; }   // big-endian, as on the Wii
// ACHIEVEMENT +0x1a8: the achievements are active (set by the frontend; never set here, so the flags below stay 0)
bool AchOn() { return achievement[0x1a8] != 0; }
void AchFlag(int off)
{
	if (AchOn()) achievement[off] = 1;
}

[[noreturn]] void Spin(const char* where)
{
	// the original spins forever here (an unsupported sw+0x90 version mode)
	std::fprintf(stderr, "gotch::pacman: %s: sw+0x90 = %d (the original hangs here)\n", where, (int)Sw32(0x90));
	std::abort();
}

// RSO2 0x80d83aac
int GetCurrentBankNumber() { return sw.b(0xa3); }

// ---- forward declarations ----
void OptionsWork_to_System();
void OptionsMenuBuild();
void ArrivalRoundUpdate();

// RSO2 0x80d84a48
void HiScoreDisp(int show)
{
	if (show != 0)
	{
		// the bank's 3 BCD bytes (+0x3c..0x3e) into the score digits mem[0x43ed..0x43f2], leading zeros blank (0x40)
		u16 dst = 0x43f2;
		int src = GetCurrentBankNumber() * 5 + 0x3e;
		bool digit = false;
		for (int k = 0; k < 3; k++)
		{
			u8 hi = save[src] >> 4;
			if (hi != 0) digit = true;
			if (!digit && hi == 0) hi = 0x40;
			mem[dst] = hi;
			const u8 b = save[src];
			src--;
			u8 lo = b & 0xf;
			if ((b & 0xf) != 0) digit = true;
			if (!digit && (b & 0xf) == 0) lo = 0x40;
			mem[(u16)(dst - 1)] = lo;
			dst -= 2;
		}
		return;
	}
	for (u16 a = 0x43ed; a <= 0x43f2; a++) mem[a] = 0x40;
	for (u16 a = 0x43cb; a <= 0x43d4; a++) mem[a] = 0x40;
}

// RSO2 0x80d46088
void ExportDataDefaultReset()
{
	sw.b(0xa0) = kDipDefault[1];
	sw.b(0xa2) = kDipDefault[2];
	// every locale branch writes the same two values
	save[0x4b] = 2;
	save[0x49] = 1;
	save[0x46] = 0;
	save[0x47] = 2;
	save[0x48] = 1;
	save[0x4a] = 1;
	save[0x51] = 1;
	sw.b(0xad) = 0;
	save[0x4c] = 1;
	save[0x4d] = 0;
	save[0x4e] = 0;
	save[0x50] = 0;
	sw.b(0x51) = 1;
	save[0x52] = 1;
	save[0x59] = save[0x46];
	save[0x5a] = save[0x47];
	for (int off : {0x5c, 0x60, 0x70, 0x6c, 0x78, 0x74, 0x68, 0x64}) SetSave32(off, 0);
	// CDraw +0x6c / +0x70 / +0x64 / +0x68 (zoom / aspect from the options): not ported (the CDraw stand-in has no screen adjust)
	save[0x58] = 1;
	sw.b(0x54) = 1;
	SetSw32(0x58, 1);
}

// RSO2 0x80d462b8
void ExportData_Init()
{
	std::memcpy(&save[0x34], "PPPC0007", 8);
	ExportDataDefaultReset();
	for (int off : {0x5c, 0x60, 0x70, 0x6c, 0x78, 0x74, 0x68, 0x64}) SetSave32(off, 0);
	save[0x58] = 1;
	for (int i = 0x3c; i <= 0x45; i++) save[i] = 0;
	// quirk kept: strlen is taken again after each store, so only the first character is cleared
	for (int i = 0; i < (int)strnlen((const char*)&save[0x7c], 0x20); i++) save[0x34 + i + 0x48] = 0;
	for (int i = 0; i < (int)strnlen((const char*)&save[0x9c], 0x20); i++) save[0x34 + i + 0x68] = 0;
	for (int i = 0; i < (int)strnlen((const char*)&save[0xbc], 0x20); i++) save[0x34 + i + 0x88] = 0;
	save[0x56] = 0;
	save[0x57] = 0;
	if (sw.b(0x2f) == 0)
	{
		sw.b(0xae) &= 0x83;
		sw.b(0xaf) = (u8)(kLocale << 4 | (sw.b(0xaf) & 7) | 4);
	}
}

// RSO2 0x80d82f7c (AddGameSetting, inlined into SaveDataToGameSetting): append v unless it is already in the list
void AddGameSetting(s32 v)
{
	for (int i = 0; i < 256; i++)
		if (gsCurrent[i] == v) return;
	gsCurrent[gsCurrentPtr++] = v;
	gsCurrent[gsCurrentPtr] = -1;
}

// RSO2 0x80d8338c
void SaveDataToGameSetting()
{
	gsCurrentPtr = 0;
	for (s32& v : gsCurrent) v = -1;
	SetSw32(0x80, save[0x46]);
	SetSw32(0x84, save[0x47]);
	SetSw32(0x8c, save[0x48]);
	SetSw32(0x88, save[0x49]);
	SetSw32(0x90, save[0x4b]);
	if (sw.b(0xa3) != 0)
	{
		SetSw32(0x80, 1);
		SetSw32(0x8c, 1);
		SetSw32(0x84, 1);
		SetSw32(0x88, 0);
		SetSw32(0x90, 1);
	}
	// lives (sw+0x80 0..3 -> setting 1 / 2 / 3 / 0), then the bonus (save+0x47 0..3 -> setting 4..7)
	switch (Sw32(0x80))
	{
	case 0: AddGameSetting(1); break;
	case 1: AddGameSetting(2); break;
	case 2: AddGameSetting(3); break;
	case 3: AddGameSetting(0); break;
	}
	if (save[0x47] <= 3) AddGameSetting(4 + save[0x47]);
}

// RSO2 0x80d8302c
void EntryGameSetting(const s32* list)
{
	for (;; list++)
	{
		switch (*list)
		{
		case 0: dswA = (dswA & 0xcf) | 0x30; break;
		case 1: dswA = dswA & 0xcf; break;
		case 2: dswA = (dswA & 0xcf) | 0x10; break;
		case 3: dswA = (dswA & 0xcf) | 0x20; break;
		case 4: dswA = dswA & 0xf3; break;
		case 5: dswA = (dswA & 0xf3) | 4; break;
		case 6: dswA = (dswA & 0xf3) | 8; break;
		case 7: dswA = (dswA & 0xf3) | 0xc; break;
		case 8: dswB = (dswB & 0x7f) | 0x80; break;
		case 9: dswB = dswB & 0x7f; break;
		case -1: return;
		}
	}
}

// the DIP switch rebuild BootStart and GameStartInit share
void RebuildDipSwitches()
{
	dswA = 0xc9;
	dswB = 0;
	EntryGameSetting(kGameSettingDefault);
	if (gsCurrentPtr != 0) EntryGameSetting(gsCurrent);
	sw.b(0xa0) = dswA;
	sw.b(0xa2) = dswB;
}

// RSO2 0x80d82bec
void OptionsWork_to_System()
{
	if (sw.b(0xa3) == 0)
	{
		SetSw32(0x80, save[0x46]);
		SetSw32(0x84, save[0x47]);
		SetSw32(0x8c, save[0x48]);
		SetSw32(0x88, save[0x49]);
		SetSw32(0x90, save[0x4b]);
	}
	if (optionsItem[0] != nullptr)
	{
		optionsItem[0]->SetItemCursor(Sw32(0x80));
		optionsItem[1]->SetItemCursor(Sw32(0x84));
		optionsItem[2]->SetItemCursor(sw.b(0xad));
	}
	// sw+0x30d / +0x328 / +0x329 (crosshatch, screen ratio), CDraw zoom / aspect / offsets, FadeSet: not ported (screen adjust)
}

// RSO2 0x80d86484
void ArrivalRoundUpdate()
{
	const int base = 0x34;
	if (sw.b(0x2f) != 0 && ((sw.b(0xae) >> 5) & 1) == 0)
	{
		u8 round = mem[0x4e13];
		if (round < mem[0x4e41]) round = mem[0x4e41];
		if (save[base + GetCurrentBankNumber() * 5 + 0xc] < round)
		{
			save[base + GetCurrentBankNumber() * 5 + 0xc] = round;
			sw.b(0x54) = 1;
		}
	}
}

// RSO2 0x80d84b40 (PACMAN30th_CopyrightStringSet): the 30th anniversary copyright lines written into the ROM image
void CopyrightStringSet()
{
	auto put = [](u16 at, const std::string& s, bool convert, u8 end) {
		for (char ch : s) mem[at++] = convert ? kAsciiToGame[(u8)ch] : (u8)ch;
		mem[at] = 0x2f;
		mem[(u16)(at + 1)] = end;
		mem[(u16)(at + 2)] = 0x2f;
		mem[(u16)(at + 3)] = 0x80;
	};
	mem[0x7000] = 0x5a;
	mem[0x7001] = 3;
	put(0x7002, sCopyright1, true, 0x83);
	mem[0x36bb] = 0;
	mem[0x36bc] = 0x70;
	mem[0x36cb] = 0;
	mem[0x36cc] = 0x70;
	mem[0x7080] = 0x5c;
	mem[0x7081] = 3;
	put(0x7082, sCopyright2, true, 0x83);
	mem[0x7100] = 0x5f;
	mem[0x7101] = 2;
	put(0x7102, sNamcoLogo, false, 0x8f);
	std::memset(&mem[0x7200], 0, 0x80);
	for (int i = 0; i < 0x6c; i++) mem[0x7200 + i] = mem[0x36a5 + i];
	mem[0x7280] = 0x80;
	mem[0x7281] = 0x70;
	mem[0x7282] = 0;
	mem[0x7283] = 0x71;
	std::memset(&mem[0x7300], 0, 0x80);
	for (int i = 0; i < 0x12 * 6; i++)
	{
		const u8 c = kVersionJ[i];
		mem[0x7300 + i] = c != 0 ? c : mem[0x36a5 + i];
	}
	mem[0x7380] = 0x80;
	mem[0x7381] = 0x70;
	mem[0x7382] = 0;
	mem[0x7383] = 0x71;
}

// RSO2 0x80d85284
void BootStart()
{
	CopyrightStringSet();
	gameState->ChangeState(sw.b(0x34) == 0 && sw.b(0x35) == 0 && sw.b(0x33) == 0 ? 2 : 4);
	if (sw.b(0x51) == 0)
	{
		sw.b(0x52) = 1;
		SetHL(0);
		SetPC(0x3003);
	}
	else
	{
		SetPC(0x3174);
	}
	SaveDataToGameSetting();
	OptionsWork_to_System();
	RebuildDipSwitches();
}

// RSO2 0x80d86594: the hooks shared by every PAC-MAN variant; 1 = this PC is not one of them
int ExtFuncExtendCommon()
{
	const u16 pc = PC();
	const int idx = mem[0x4e09] * 4;  // the current player (DAT_80debfd9)
	switch (pc)
	{
	case 0x2a31:  // the random number: RANDOM_TABLE (+ the version 1 fixes) instead of the ROM bytes
	{
		const u16 p = randomPtr;
		const u8 table = p < kRandomAvail ? kRandomTable[p] : 0;  // UNVERIFIED past the module file's end (never reached in tests)
		if (Sw32(0x90) == 1)
		{
			if (p == 5) A() = 0xf1;
			else if (p == 0x1c0) A() = 4;
			else if (p == 0x3ae) A() = 0xc3;
			else if (p == 0x6a2) A() = 0xc;
			else if (p == 0x6c1) A() = 0xf1;
			else if (p == 0x73a) A() = 0xc3;
			else A() = table;
		}
		else
		{
			A() = table;
		}
		mem[0x4dc9] = env.r8(Z80_env::L);
		mem[0x4dca] = env.r8(Z80_env::H);
		SetPC(0x2a34);
		randomPtr++;
		return 0;
	}
	case 0xa93:  // round cleared: achievements, the arrival round
	{
		if (sw.b(0x2f) != 0)
		{
			switch (mem[0x4e13])
			{
			case 4: AchFlag(0x1bd); break;
			case 0x14: AchFlag(0x1be); break;
			case 0: AchFlag(0x1c8); break;
			case 1:
				AchFlag(0x1c0);
				AchFlag(0x1c9);
				break;
			case 0x10: AchFlag(0x1c1); break;
			case 0x13: AchFlag(0x1c2); break;
			case 0x1e: AchFlag(0x1c3); break;
			}
		}
		if (Ach32(0x1e4 + idx) > 3 && Ach32(0x1ec + idx) > 1 && AchOn()) achievement[0x1c4] = 1;
		SetAch32(0x1e4, 0);
		SetAch32(0x1e8, 0);
		SetAch32(0x1ec, 0);
		SetAch32(0x1f0, 0);
		if ((std::int8_t)mem[HL()] == -1) save[GetCurrentBankNumber() * 5 + 0x40] = 0xff;
		mem[HL()]++;
		ArrivalRoundUpdate();  // inlined in the original
		// TaikenCheck(mem[HL]) (RSO2 0x80d83ab8): empty
		SetPC(0xa94);
		return 0;
	}
	case 0x609:
		mem[0x4dd6] = 1;
		Push(0x060b);
		SetBC(0x1c40);
		SetPC(0x42);
		return 0;
	case 0x4fa:
		Push(0x04fb);
		A() = 0;
		SetBC(0x1c41);
		SetPC(0x42);
		return 0;
	case 0x4e8:
		mem[HL()]--;
		Push(0x04e9);
		SetBC(0x1c40);
		SetPC(0x42);
		return 0;
	case 0x1d4:
		sw.b(0x74) = 0;
		A() = 1;
		SetPC(0x1d6);
		return 0;
	case 0x5fc:
		if (sw.b(0x2e) == 0)
		{
			SetPC(0x5ff);
			return 0;
		}
		SetPC(0x28);
		Push(0x05fd);
		return 0;
	case 0x60e:
		if (sw.b(0x2e) == 0)
		{
			SetPC(0x61a);
			return 0;
		}
		A() = mem[0x4e71];
		SetPC(0x611);
		return 0;
	case 0x60b:
		Push(0x060e);
		SetBC(0x1c41);
		SetPC(0x42);
		return 0;
	case 0x629:
		if (sw.b(0x2e) == 0)
		{
			SetPC(0x62c);
			return 0;
		}
		SetPC(0x2c5e);
		Push(0x062c);
		return 0;
	case 0x19b2:
		B() = 0x19;
		SetPC(0x19b4);
		if (sw.b(0x2f) != 0) SetAch32(0x1ec + idx, Ach32(0x1ec + idx) + 1);
		return 0;
	case 0x1775:
		if (A() != 0)
		{
			SetPC(0x1776);
			return 0;
		}
		if (sw.b(0x2f) != 0 && ((sw.b(0xae) >> 6) & 1) != 0)
		{
			mem[0x4da4] = 0;
			mem[0x4da5] = 0;
		}
		SetPC(0x1788);
		return 0;
	case 0x1700:
		if ((u32)Sw32(0x90) < 2)
		{
			mem[(u16)(IX() + 0xa)] = 0x30;
			SetPC(0x1704);
			return 0;
		}
		if (Sw32(0x90) == 2)
		{
			E() = 0x30;
			SetPC(0x170b);
			return 0;
		}
		Spin("hook 0x1700");
	case 0x1366:
		SetHL(0x4e14);
		SetPC(((sw.b(0xae) >> 4) & 1) == 0 ? 0x1369 : 0x136e);
		return 0;
	case 0x177a:
		SetHL(0x4dd0);
		SetPC(0x177d);
		if (sw.b(0x2f) == 0) return 0;
		SetAch32(0x1f4 + idx, Ach32(0x1f4 + idx) + 1);
		if (Ach32(0x1f4 + idx) == 100) AchFlag(0x1c6);
		return 0;
	case 0x26a2:
		A() = 0;
		SetPC(0x26a3);
		randomPtr = 0;
		return 0;
	case 0x19e0:
		SetIX(0x4e0e);
		SetPC(0x19e4);
		if (sw.b(0x2f) == 0) return 0;
		SetAch32(0x1fc + idx, Ach32(0x1fc + idx) + 1);
		if (Ach32(0x1fc + idx) == 10000) AchFlag(0x1c7);
		return 0;
	case 0x2926:
		Push(0x2929);
		SetPC(0x2a23);
		return 0;
	case 0x31e7:
	case 0x31f9:
	case 0x3204:
	case 0x3209:
	case 0x322f:
	case 0x323b:  // LAB_80d867dc: the text calls go through 0x2c5e unless the ROM / RAM test is skipped
	{
		const u16 next = (u16)(pc + 3);
		if (sw.b(0x51) != 0)
		{
			SetPC(next);
			return 0;
		}
		Push(next);
		SetPC(0x2c5e);
		return 0;
	}
	case 0x2e29:
		A() = mem[(u16)(IX() + 2)];
		SetPC(sw.b(0x74) == 0 ? 0x2e2c : 0x2e2f);
		return 0;
	case 0x2c5e:  // the text writer: HL = the string table of the version / locale
	{
		if ((u32)Sw32(0x90) - 1 < 2) SetHL(0x7300);
		else if (Sw32(0x90) != 0) Spin("hook 0x2c5e");
		else SetHL(0x7200);
		const int loc = kLocale;
		if ((u32)(loc - 0xc) < 2 || loc == 0) SetHL(0x7300);
		else SetHL(0x7200);  // locale 1 and the rest
		SetPC(0x2c61);
		if (sw.b(0x2e) == 0 && kTable2c5e[B() & 0x7f] == 0) SetPC(0x2ca3);
		const u8 b = B() & 0x7f;
		if (b != 0xb && b != 0x13 && b != 0x40) return 0;
		if (copyright[0] != 0)
		{
			sw.b(0x73) = 1;
			return 0;
		}
		SetPC(0x2ca3);
		return 0;
	}
	case 0x2a68:  // score add: the extended counter (BCD DE added, clamped at 999999990)
	{
		const int k = HL() == 0x4e80 ? 0 : 2;
		const u32 de = DE();
		const u32 add = (de & 0xf) + ((de >> 4) & 0xf) * 10 + ((de >> 8) & 0xf) * 100 + (de >> 12) * 1000;
		const u32 lo = extendedScore[k + 1];
		const u32 sum = lo + add;
		extendedScore[k + 1] = sum;
		extendedScore[k] += (sum < lo) ? 1 : 0;
		if ((u32)(0x3b9ac9f6 < sum) + extendedScore[k] != 0)
		{
			extendedScore[k + 1] = 0x3b9ac9f6;
			extendedScore[k] = 0;
		}
		A() = E();
		SetPC(0x2a69);
		return 0;
	}
	case 0x2a60:  // bonus fruit eaten (B = the fruit)
		if (((sw.b(0xae) >> 2) & 1) != 0 && B() == 0) B() = 0xd;
		SetHL(0x2b17);
		SetPC(0x2a63);
		if (sw.b(0x2f) == 0) return 0;
		if (B() == 5)
		{
			AchFlag(0x1bc);
			SetAch32(0x1e4 + idx, Ach32(0x1e4 + idx) + 1);
			if (Ach32(0x1e4 + idx) > 3) AchFlag(0x1bf);
			if (Ach32(0x1e4 + idx) > 1) AchFlag(0x1ca);
			if (Ach32(0x1e4 + idx) >= 3) AchFlag(0x1cb);
		}
		else if (B() >= 6 && B() <= 0xd)
		{
			AchFlag(0x1b4 + (B() - 6));
		}
		return 0;
	case 0x2d59:
		A() = mem[(u16)(IX() + 2)];
		SetPC(sw.b(0x74) == 0 ? 0x2d5c : 0x2d5f);
		return 0;
	case 0x3018:
		if (sw.b(0x50) != 0)
		{
			SetPC(0x301c);
			return 0;
		}
		A() = C();
		SetPC(0x3019);
		return 0;
	case 0x3000:
		BootStart();
		return 0;
	case 0x3020:
		if ((u32)Sw32(0x90) - 1 < 2) SetPC(A() != 0x40 ? 0x3003 : 0x3025);
		else SetPC(A() != 0x30 ? 0x3003 : 0x3025);  // sw+0x90 == 0 and the rest test the same value
		return 0;
	case 0x320d:
		SetDE(DE() & 0xff);
		SetPC(0x322d);
		return 0;
	case 0x324b:
		if (sw.b(0x51) != 0)
		{
			SetPC(0x327c);
			return 0;
		}
		SetHL(0x5007);
		SetPC(0x324e);
		return 0;
	case 0x32f0:
		if (sw.b(0x51) != 0)
		{
			SetPC(0x32f8);
			return 0;
		}
		SetHL(0x2800);
		SetPC(0x32f3);
		return 0;
	}
	return 1;
}

// RSO2 0x80d851f4: the round the start menu's ROUND SELECT picked (WII_MENU::GetContinueRound of the player who started)
int GetContinueRound() { return gameMenu->GetContinueRound(gameMenu->players); }

// copies the bank's hi score into the game's hi score bytes mem[0x4e88..0x4e8b] (repeated in 0x631 / 0x2ae5)
void HiScoreFromBank()
{
	const int at = 0x34 + GetCurrentBankNumber() * 5;
	for (int i = 0; i < 4; i++) mem[0x4e88 + i] = save[at + 8 + i];
}

// RSO2 0x80d853a0: this PAC-MAN's own hooks; 1 = not one of them
int ExtFuncExtend()
{
	switch (PC())
	{
	case 0x96c:
		A() = 9;
		SetPC(0x96e);
		return 0;
	case 0x889:  // keep the player's work (0x4e0a..0x4e37 -> 0x4e38..), the continue round
	{
		for (int i = 0; i < 0x2e; i++) mem[0x4e38 + i] = mem[0x4e0a + i];
		if (sw.b(0x39) != 0)
		{
			u32 p = (u32)(mem[0x4e74] << 8 | mem[0x4e73]);
			int n = GetContinueRound();
			mem[0x4e13] = (u8)n;
			for (; n > 0 && mem[(u16)p] != 0x14; n--) p++;
			mem[0x4e0b] = (u8)(p >> 8);
			mem[0x4e0a] = (u8)p;
			p = (u32)(mem[0x4e74] << 8 | mem[0x4e73]);
			n = GetContinueRound();
			mem[0x4e41] = (u8)n;
			for (; n > 0 && mem[(u16)p] != 0x14; n--) p++;
			mem[0x4e38] = (u8)p;
			mem[0x4e39] = 0;
		}
		SetHL(0x4e04);
		SetPC(0x894);
		return 0;
	}
	case 0x631:  // IN1 read of the start buttons: the Wii menu's start requests (sw+0x34 / 0x35) or the real IN1 in coin mode
	{
		u8 in1;
		if (sw.b(0x2e) == 0)
		{
			in1 = 0xff;
			if (sw.b(0x34) != 0)
			{
				sw.b(0x34) = 0;
				in1 = 0xdf;
				if (sw.b(0x53) != 0) HiScoreFromBank();
				HiScoreDisp(1);
				SetAch32(0x1e4, 0);
				SetAch32(0x1e8, 0);
				gameState->ChangeState(4);
			}
			if (sw.b(0x35) != 0)
			{
				sw.b(0x35) = 0;
				if (sw.b(0x53) != 0) HiScoreFromBank();
				HiScoreDisp(1);
				SetAch32(0x1e4, 0);
				SetAch32(0x1e8, 0);
				gameState->ChangeState(4);
				in1 &= 0xbf;
			}
		}
		else
		{
			in1 = io[1];
		}
		A() = in1;
		SetPC(0x634);
		return 0;
	}
	case 0x91:
		if (sw.b(0x33) != 0)
		{
			sw.b(0x33) = 0;
			mem[0x4e6e] = 2;
			mem[0x4e9c] |= 2;
		}
		if (sw.b(0x34) != 0 || sw.b(0x35) != 0) mem[0x4e6e] = 2;
		A() = 0;
		SetPC(0x92);
		return 0;
	case 0x1:  // reset: back to the demo
		gameState->ChangeState(sw.b(0x34) == 0 && sw.b(0x35) == 0 ? 2 : 4);
		sw.b(0x2f) = 0;
		swork_SetWork_GameDemo(sw);
		swork_SetWork_1P_PauseStatus(sw, 0, 10);
		swork_SetWork_2P_PauseStatus(sw, 0, 10);
		sw.b(0xa0) = dswA;
		sw.b(0xa2) = dswB;
		sw.b(0x114) = 0;
		sw.b(0x115) = 0;
		sw.b(0x118) = 0;
		sw.b(0x119) = 0;
		A() = 0x3f;
		SetPC(3);
		return 0;
	case 0x68f:
		A() = 0;
		SetPC(0x690);
		if (sw.b(0x39) != 0) A() = (u8)GetContinueRound();
		return 0;
	case 0x649:  // game start
		swork_SetWork_GameStart(sw, -1, -1);
		swork_SetWork_1P_PauseStatus(sw, 1, 10);
		Push(0x26d0);
		A() = mem[0x4e6b];
		SetPC(0x64c);
		return 0;
	case 0x95c:  // game over
	{
		if (sw.b(0x2e) == 0) mem[0x4e6e] = 0;
		sw.b(5) = 0;
		sw.b(4) = 0;
		sw.b(7) = 0;
		sw.b(6) = 0;
		SetSw32(0xc, 10);
		const int at = 0x34 + GetCurrentBankNumber() * 5;
		for (int i = 0; i < 4; i++) save[at + 8 + i] = mem[0x4e88 + i];
		sw.b(0x54) = 1;
		sw.b(0x53) = 1;
		sw.b(0xa3) = 0;
		for (int i = 0; i < 4; i++) mem[0x4e88 + i] = save[0x3c + i];
		if (sw.b(0x2f) != 0)
		{
			ArrivalRoundUpdate();
			SetSw32(0x58, 1);
			if (sw.b(0x54) != 0)
			{
				sw.b(0x54) = 0;
				SetSw32(0x58, 0);
				SetSw32(0x68, 0xd0);
			}
		}
		sw.b(0x2f) = 0;
		sw.b(0x39) = 0;
		SetHL(0x4e04);
		SetPC(0x95f);
		gameState->SetPlayerState(mem[0x4e09], 5);
		return 0;
	}
	case 0x92b:
		if (sw.b(0x2e) == 0) mem[0x4e6e] = 0;
		if (mem[0x4e09] == 0) sw.b(4) = 0;
		else sw.b(5) = 0;
		sw.b(7) = 0;
		sw.b(6) = 0;
		SetSw32(0xc, 10);
		A() = mem[0x4e09];
		SetPC(0x92e);
		gameState->SetPlayerState(mem[0x4e09], 5);
		return 0;
	case 0x91f:
		A() = mem[0x4e70];
		SetPC(0x922);
		return 0;
	case 0x964:  // player change
		A() = mem[0x4e09];
		SetPC(0x967);
		gameState->SetPlayerState(mem[0x4e09] ^ 1, 2);
		if (mem[0x4e42] != 0) gameState->SetPlayerState(mem[0x4e09], 3);
		if ((A() & 1) != 0)
		{
			sw.b(4) = 1;
			sw.b(5) = 0;
			return 0;
		}
		sw.b(4) = 0;
		sw.b(5) = 1;
		return 0;
	case 0x2a77:
	{
		const u8 carry = F() & 1;
		mem[HL()] = A();
		SetPC(0x2a78);
		if (carry != 0) sw.b(0x114 + mem[0x4e09])++;
		return 0;
	}
	case 0x2387:
		if (sw.b(0x34) == 0 && sw.b(0x35) == 0 && sw.b(0x33) == 0) gameState->ChangeState(3);
		sw.b(0x52) = 0;
		SetSw32(0x10, 3);
		gameMenu->EnableMenu();
		SetSw32(0x18, 1);
		A() = 1;
		SetPC(0x2389);
		return 0;
	case 0xa94:  // replaces LD HL,(4E0A): quirk kept, only the low byte is loaded
		SetPC(0xa97);
		SetHL(mem[0x4e0a]);
		if (mem[0x4e13] != 0) return 0;
		sw.b(0x118 + mem[0x4e09])++;
		return 0;
	case 0x972:
		SetSw32(0x10, 3);
		gameMenu->EnableMenu();
		if (sw.b(0x34) == 0 && sw.b(0x35) == 0 && sw.b(0x33) == 0) gameState->ChangeState(3);
		gameState->SetPlayerState(0, 1);
		gameState->SetPlayerState(1, 1);
		SetSw32(0x18, 1);
		A() = 0;
		SetPC(0x973);
		return 0;
	case 0x240c:
		SetPC(0x2418);
		return 0;
	case 0x2bad:
		if (sw.b(0x2e) == 0)
		{
			SetPC(0x2bcc);
			return 0;
		}
		B() = 1;
		SetPC(0x2baf);
		return 0;
	case 0x2ae5:
		if (sw.b(0x53) != 0)
		{
			HiScoreFromBank();
			HiScoreDisp(1);
		}
		std::memset(extendedScore, 0, sizeof(extendedScore));
		A() = 0;
		SetPC(0x2ae6);
		return 0;
	case 0x2aa6:
		sw.b(0x54) = 1;
		SetDE((u16)(DE() - 1));
		SetPC(0x2aa7);
		return 0;
	case 0x2bf3:
		A()++;
		if (save[0x51] != 0 && A() == 0) A() = 0x13;
		SetPC(0x2bf4);
		return 0;
	}
	return 1;
}

// RSO2 0x80d46560: both handlers, every time (Common's 0xa93 leaves PC at 0xa94, which Extend handles in the same call)
void CPU1_ExtFunc()
{
	ExtFuncExtendCommon();
	ExtFuncExtend();
}

// RSO2 0x80d863e8 / 0x80d85208: RST 0 (0xC7) over the patched ROM addresses
void EntryExtfuncExtendCommon()
{
	static const u16 kAt[] = {0x3000, 0x324b, 0x32f0, 0x31e7, 0x31f9, 0x3204, 0x3209, 0x322f, 0x323b, 0x320d, 0x3018, 0x1775,
	                          0x1366, 0x2a60, 0x2a68, 0x1700, 0x3020, 0x0629, 0x05fc, 0x060e, 0x26a2, 0x2a31, 0x2926, 0x0a93,
	                          0x2d59, 0x2e29, 0x01d4, 0x0609, 0x060b, 0x04e8, 0x04fa, 0x2c5e, 0x177a, 0x19b2, 0x19e0};
	for (u16 a : kAt) mem[a] = 0xc7;
}
void EntryExtfuncExtend()
{
	EntryExtfuncExtendCommon();
	static const u16 kAt[] = {0x2bad, 0x0091, 0x0001, 0x0649, 0x092b, 0x095c, 0x096c, 0x0972, 0x2387, 0x068f,
	                          0x0889, 0x2bf3, 0x0631, 0x2ae5, 0x2aa6, 0x240c, 0x091f, 0x2a77, 0x0964, 0x0a94};
	for (u16 a : kAt) mem[a] = 0xc7;
}

// ---- the Z80 bus (RSO2 0x80d46460..0x80d465ac) ----
u8 CPU1_ReadMemory8(u16 a)
{
	if (a == 0x5000)
	{
		if (sw.b(0x2e) != 0) return io[0];
		io[0] |= 0x60;
		return io[0];
	}
	if (a == 0x5040) return io[1];
	if (a == 0x5080) return io[2];
	if (a != 0x50c0) return mem[a];
	return 0;
}
u8 CPU1_ReadM1(u16 a) { return mem[a]; }  // RSO2 0x80d464f0
void CPU1_WriteMemory8(u16 addr, u8 v)
{
	u32 a = addr;
	if (a < 0x4000) return;
	if (((a + 0x8000) & 0xffff) < 0x4000) return;  // 0x8000..0xbfff
	if (a > 0xbfff) a = (a - 0x8000) & 0xffff;
	mem[a] = v;
	if (((a + 0xafc0) & 0xffff) > 0x1f) return;
	wsg->SetRegister(&mem[0x5040]);  // 0x5040..0x505f
}
u8 CPU1_ReadIO8(u16 port) { return z80io[port & 0xff]; }
void CPU1_WriteIO8(u16 port, u8 v)
{
	if ((u8)port != 0) return;
	env.set16(Z80_env::BUS, v);  // the interrupt vector
}

// RSO2 0x80d85f48: pad n's word = translated high word | low word << 4 (0x10 up, 0x20 right, 0x40 down, 0x80 left), word2 the same of
// the 4-way edges; both 0 while CGAMEIO's invalid counter runs
void IO_Convert()
{
	u8 in[3] = {0xff, 0xff, 0xff};
	u32 word2[2];
	for (int p = 0; p < 2; p++)
	{
		const ControllerValue& cv = gameio->Pad(p);
		u32 w = gameio->invalidFrames == 0 ? (u32)(cv.value >> 32) | (u32)cv.value << 4 : 0;
		word2[p] = gameio->invalidFrames == 0 ? (u32)(cv.edgeWay4 >> 32) | (u32)cv.edgeWay4 << 4 : 0;
		if (!gameio->DigitalPadDirectionState(0))
		{
			w |= (u32)((s32)w >> 16);
			word2[p] |= (u32)((s32)word2[p] >> 16);
		}
		w |= (u32)((s32)w >> 16);
		word2[p] |= (u32)((s32)word2[p] >> 16);
		if (w & 0x10) in[p] &= 0xfe;
		if (w & 0x40) in[p] &= 0xf7;
		if (w & 0x80) in[p] &= 0xfd;
		if (w & 0x20) in[p] &= 0xfb;
	}
	io[1] = (in[1] & 0x7f) | (sw.b(0xa2) & 0x80);
	io[0] = in[0];
	io[2] = sw.b(0xa0);
	// A / PLUS (pad 0's edge) between games opens the start menu; its ROUND SELECT goes up to the bank 0 arrival round (save+0x40)
	if (gameMenu->Mode() != 0 && Sw32(0x10) == 0)
	{
		gameio->ROMRAMSkipCheck(sw, sw.b(0x52) != 0, &sw.b(0x51));
		if (Sw32(0x18) == 0 && sw.b(0x2f) == 0 && gameMenu->Mode() == 1 && (word2[0] & 8) != 0)
		{
			OptionsWork_to_System();
			gameMenu->continueRound = ((sw.b(0xae) >> 5) & 1) != 0 ? 0xfe : save[0x40];
			gameMenu->RebuildMenu();
			gameMenu->PushAndChangeMenu(2, true);
			sw.b(0x33) = 1;
		}
	}
}

// RSO2 0x80d47184
void ConvertIO()
{
	gameio->SetPadData();
	IO_Convert();
}

// RSO2 0x80d843b4. The menu texts are the module's MenuStr_*_LOCALE_NEUTRAL strings (pointer words resolved through the module's
// relocations). MenuMM__Init, SetStringConvertTable (PACMAN__MenuStringConvertTable: CMENU::Render passes no table) and MENU_KEYCONV: not
// ported.
void MenuConstruct()
{
	const RsoModule& m = RsoModule::Volume(2);
	auto text = [&](u32 addr) {
		const char* t = m.Pointer(addr);
		return t != nullptr ? t : "";
	};
	WiiMenu::Strings str;
	str.pause = text(0x80dccf40);          // MenuStr_PAUSE_LOCALE_NEUTRAL
	str.cont = text(0x80dcccd0);           // MenuStr_CONTINUE_LOCALE_NEUTRAL
	str.screenAdjust = text(0x80dcddf0);   // MenuStr_NMREMIX_WII_SCREEN_ADJUST_TITLE_LOCALE_NEUTRAL
	str.quit = text(0x80dcccbc);           // MenuStr_QUIT_LOCALE_NEUTRAL
	str.start1P = text(0x80dccbbc);        // MenuStr_STARTMENU_1UP_START_LOCALE_NEUTRAL
	str.start2P = text(0x80dccbd0);        // MenuStr_STARTMENU_2UP_START_LOCALE_NEUTRAL
	str.roundSelect = text(0x80dcde24);    // MenuStr_NMREMIX_WII_STARTMENU_ROUND_SELECT_LOCALE_NEUTRAL
	str.options = text(0x80dccbe4);        // MenuStr_STARTMENU_OPTIONS_LOCALE_NEUTRAL
	str.startScreenAdjust = text(0x80dccc08);   // MenuStr_STARTMENU_SCREEN_ADJUST_LOCALE_NEUTRAL
	str.retMainMenu = text(0x80dccd84);    // MenuStr_STARTMENU_RET_MAINMENU_LOCALE_NEUTRAL
	str.returnToArcade = text(0x80dcde28);   // MenuStr_NMREMIX_WII_STARTMENU_RETURN_TO_ARCADE_LOCALE_NEUTRAL
	str.gameName = text(0x80dcd1cc);       // MenuStr_PAC_MAN_LOCALE_NEUTRAL
	str.soundTest = text(0x80dcccfc);      // MenuStr_SOUND_TEST_LOCALE_NEUTRAL
	optStr.options = str.options;
	optStr.bonus = text(0x80dccc3c);       // MenuStr_BONUS_LOCALE_NEUTRAL
	optStr.none = text(0x80dcd1a4);        // MenuStr_NONE_LOCALE_NEUTRAL
	optStr.life = text(0x80dccc50);        // MenuStr_LIFE_LOCALE_NEUTRAL
	optStr.soundTest = str.soundTest;
	optStr.screenAdjust = str.screenAdjust;
	optStr.deflt = text(0x80dcd12c);       // MenuStr_DEFAULT_LOCALE_NEUTRAL
	optStr.exit = text(0x80dccfcc);        // MenuStr_EXIT_LOCALE_NEUTRAL
	// CommonResourceSet RSO2 0x80d28124: PAD_DECIDE = 0x5800, PAD_CANCEL = 0x8400; the machine's screen: 288 x 224 turned 270 degrees
	gameMenu = std::make_unique<WiiMenu>(sw, *draw, *gameio, draw->ScreenW(), draw->ScreenH(), 270, str, 0x5800u, 0x8400u);
	gameMenu->soundBufferClear = soundBufferClear;
	gameMenu->gameStartInit = GameStartInit;   // GAMEMENU vtable 0x81148538 +0xc
	// SetStringConvertTable(MenuStringConvertTable 0x81148378): CMENU::Render passes no table, nothing ported reads it
	gameMenu->SetPaletStatus(0, 1);   // MENU_PaletOffset (0, InitRender), stride 1 (CDraw +0x180 is never set)
	gameMenu->InitMenu();
	// s_PAC_MAN_80dc9043 (a module literal)
	const u8* title = m.At(0x80dc9043, 8);
	gameMenu->SetStartMenuTitle(title != nullptr ? reinterpret_cast<const char*>(title) : str.gameName, 0x10, 0x10, -1, -1);
	gameMenu->startWait = 0x10;       // GameMenu[5] = GameMenu[6] = 0x10
	gameMenu->quitWait = 0x10;
	OptionsMenuBuild();               // after the SOUND_TEST_NO buffers (static here); LoopSoundBuf = 0xff: not ported
	gameMenu->SetMenuYoffset(0x78, 0x84, 0x80, 0x80, 0x80, 0x80);
	gameMenu->RebuildMenu();
}

// RSO2 0x80d82b34 (the BONUS / LIFE rows' decide: left / right store the cursors into the save)
void SetItemCursorToSaveData(MenuItem*)
{
	save[0x46] = (u8)optionsItem[0]->cursor;
	save[0x47] = (u8)optionsItem[1]->cursor;
}

// RSO2 0x80d82a9c: the SOUND TEST row's decide (the item = the list entry, command = its number): six bytes of the module's
// PACMAN__SoundTestTable (RSO2 0x811483f8) into the game's sound request bytes, sw+0x74 = 1 (the sound plays outside a game)
void OptionsSoundPlay(MenuItem* item)
{
	const u8* t = RsoModule::Volume(2).At(0x811483f8 + item->info.command * 6, 6);
	if (t == nullptr) return;
	for (int i = 0; i < 6; i++) mem[0x4e9c + i * 0x10] = t[i];
	sw.b(0x74) = 1;
}

// RSO2 0x80d82afc (the options menu's own decide: WII_MENU calls it while the cursor is off SOUND TEST and when the menu is left)
void OptionsSoundStop(MenuItem*)
{
	for (int i = 0; i < 6; i++) mem[0x4e9c + i * 0x10] = 0;
	sw.b(0x74) = 1;
}

// RSO2 0x80d81a7c: the options menu, rebuilt every frame (MenuExec). The texts are the module's (MenuStr_*, the value literals at RSO2
// 0x80dc9024.., the SOUND TEST numbers printed with the module's format at RSO2 0x80dc903e). PACMAN's MenuStringConvertTable: not ported (see
// MenuConstruct).
void OptionsMenuBuild()
{
	const RsoModule& m = RsoModule::Volume(2);
	auto lit = [&](u32 addr) {
		const u8* p = m.At(addr, 1);
		return p != nullptr ? reinterpret_cast<const char*>(p) : "";
	};
	const int off = 0, stride = 1;   // MENU_PaletOffset, CDraw +0x180 = 0 (see MenuConstruct)
	CMenu& om = gameMenu->Options();
	const int keep = om.cursor;
	om.DeleteItemAll();
	om.SetText(optStr.options);
	om.info.palette = off + stride * 5;
	om.info.paletteBlink = off + stride * 5;
	MenuItemInfo i = om.info;
	i.palette = off + stride * 0xf;
	i.paletteBlink = off + stride * 3;
	i.paletteCursor = off + stride;
	i.unk0c = 0;
	i.x = 0;
	i.y = 0x40;
	i.decide = SetItemCursorToSaveData;
	i.decideMask = 2;
	i.command = 0x30000;
	optionsItem[0] = new MenuItem(i, optStr.bonus);
	for (const char* t : {lit(0x80dc9024), lit(0x80dc902a), lit(0x80dc9030), optStr.none})   // "10000", "15000", "20000"
	{
		optionsItem[0]->AddItem(new MenuItem(i, t), -1);
		i.command++;
	}
	i.command = 0x30100;
	optionsItem[1] = new MenuItem(i, optStr.life);
	for (u32 a : {0x80dc9036u, 0x80dc9038u, 0x80dc903au, 0x80dc903cu})   // "1", "2", "3", "5"
	{
		optionsItem[1]->AddItem(new MenuItem(i, lit(a)), -1);
		i.command++;
	}
	om.info.decide = OptionsSoundStop;
	i.decide = OptionsSoundPlay;
	i.decideMask = 1;
	i.command = 0x30b00;
	optionsItem[2] = new MenuItem(i, optStr.soundTest);
	for (int n = 0; n < 0x11; n++)
	{
		i.command = (u32)n;
		std::snprintf(soundTestText[n], sizeof(soundTestText[n]), "%s", FormatInt(lit(0x80dc903e), n).c_str());   // sprintf_s(buf, 8, "%02d", n)
		optionsItem[2]->AddItem(new MenuItem(i, soundTestText[n]), -1);
	}
	i.decide = SetItemCursorToSaveData;
	i.decideMask = 2;
	i.command = 0x31200;
	optionsItem[3] = new MenuItem(i, optStr.screenAdjust);
	i.decide = nullptr;
	i.decideMask = 1;
	i.command = 0x39000;
	optionsItem[4] = new MenuItem(i, optStr.deflt);
	i.command = 0x3ff00;
	optionsItem[5] = new MenuItem(i, optStr.exit);
	for (MenuItem* it : optionsItem) om.AddItem(it, -1);
	om.ApplyMenuTemplate(2);
	om.SetItemCursor(keep);
	optionsItem[0]->SetItemCursor(save[0x46]);
	optionsItem[1]->SetItemCursor(save[0x47]);
	optionsItem[2]->SetItemCursor(sw.b(0xad));
}

// RSO2 0x80d847b4. The SCREEN ADJUSTMENT row (0x31200 decided -> PushAndChangeMenu(9)) and the screen adjust copy (sw+0xd8): not ported
// (the screen adjust menus are not ported, so the row does nothing natively).
void MenuExec()
{
	OptionsMenuBuild();
	gameMenu->MenuJob();
	sw.b(0xad) = (u8)optionsItem[2]->cursor;
	// (PAD_DECIDE & options +0x9c) on 0x31200: PushAndChangeMenu(9), not ported
	if (sw.b(0xd9) != 0)
	{
		ExportDataDefaultReset();
		sw.b(0xd9) = 0;
		SetSw32(0x58, 1);
		sw.b(0x54) = 1;
	}
}

// RSO2 0x80d861c8
void VBlankIn_Extend()
{
	if (sw.b(0x54) != 0)
	{
		storage->SaveRequest();   // CDataStorage::SaveRequest(DAT_811fa70c, 1, 0)
		SetSw32(0x58, 0);
		sw.b(0x54) = 0;
	}
	if (Sw32(0x10) == 0)
	{
		gameio->PauseCheck(sw, &primary->muted);
		if (sw.b(0x2f) == 0) gameio->DebugModeCheck(sw);
		gameio->ROMRAMSkipCheck(sw, sw.b(0x52) != 0, &sw.b(0x51));
	}
	if (gameio->ExitCheck(sw)) return;
	if ((sw.b(6) != 0 || sw.b(7) != 0) && sw.b(8) != 0)
	{
		OptionsWork_to_System();
		gameMenu->MenuIn(6, true);
	}
	MenuExec();
	gameMenu->drawHandle = texConfig;
}

// RSO2 0x80d46eec
void VBlankIn()
{
	VBlankIn_Extend();
	gameio->MakePadEG();
	const bool stop = sw.b(0x38) != 0 || sw.b(6) != 0 || sw.b(7) != 0;
	z80.SetFlag(&env, CPUFLAG_DISABLE, stop ? (void*)1 : nullptr);
	if (sw.b(0x38) == 0)
	{
		if (mem[0x5000] & 1) z80.SetFlag(&env, CPUFLAG_IRQ, (void*)1);
		if (Sw32(0x18) != 0) SetSw32(0x18, Sw32(0x18) - 1);
		if (Sw32(0x10) != 0) SetSw32(0x10, Sw32(0x10) - 1);
		if (Sw32(0x14) != 0) SetSw32(0x14, Sw32(0x14) - 1);
		if (((sw.b(0xaf) >> 3) & 1) == 0) sw.b(0x6f) = 0;
		// CDraw::FadeSet(0, 0, 0x66666): not ported
	}
	sw.b(0xae) |= 0x80;
}

// RSO2 0x80d46ed4
void VBlankOut() { machine.vblankCount++; }

// RSO2 0x80d46b8c
void HardwareInit()
{
	std::memset(&mem[0x4000], 0xff, 0x800);
	std::memset(&mem[0x4c00], 0xff, 0x400);
	for (int i = 0; i < 14; i++) mem[0x4ff2 + i] = kInitTable4ff2[i];
	mem[0x5001] = kInitTable4ff2[15];
	mem[0x5000] = 0;
}

// RSO2 0x80d8630c
void HotResetExtend() { sw.b(0x51) = save[0x4c]; }

// RSO2 0x80d470fc
void HotResetMachine()
{
	wsg->Reset();  // WSG vtable +0x10
	swork_HotStart(sw);
	// CConsole (*0x811fa708) +0x32 = 0; the bound PRIMARYSTREAM is muted while +0x31 / +0x32 (the console's own pause) is set: never natively
	primary->muted = false;
	HotResetExtend();
}

// RSO2 0x80d8417c (PACMAN::ExitExtend): sw+0x110 = the high score (BCD mem[0x4e88..0x4e8a], low pair first) as a number, then the save
// goes out at once (SaveRequest + FrameUpdate)
void ExitExtend()
{
	const u8 b0 = mem[0x4e88], b1 = mem[0x4e89], b2 = mem[0x4e8a];
	SetSw32(0x110, (b0 & 0xf) + (b0 >> 4) * 10 + (b1 & 0xf) * 100 + (b1 >> 4) * 1000 + (b2 & 0xf) * 10000 + (b2 >> 4) * 100000);
	storage->SaveRequest();
	storage->FrameUpdate();
}

// RSO2 0x80d46d64: ExitExtend, then frees the work buffers (nothing to free here)
void Exit()
{
	if (storage) ExitExtend();
}

// RSO2 0x80d4726c
void InitRender()
{
	CDraw& d = *draw;
	const u8* rom[8] = {};
	auto fmt = [](int w, int h, int bytes, const u8* layout) {
		CGENFORMAT f;
		f.width = w;
		f.height = h;
		f.bpp = 2;
		f.charBytes = bytes;
		f.layout = layout;
		return f;
	};
	rom[0] = kCgromA;
	texBg = d.AssignTextureAndConvertFromCGen(fmt(8, 8, 0x10, kLayoutBg), 0x100, rom, 0);
	rom[0] = kCgromB;
	texSprite = d.AssignTextureAndConvertFromCGen(fmt(16, 16, 0x40, kLayoutSprite), 0x42, rom, 0);
	rom[0] = kCgromConfig;
	texConfig = d.AssignTextureAndConvertFromCGen(fmt(8, 8, 0x10, kLayoutConfig), 0x100, rom, 0);

	// the colour PROMs: 3 / 3 / 2 bit resistor weights (0x21 / 0x47 / 0x97); colour 0 of a palette stays transparent
	u32 pal[0x80];
	for (u32 i = 0; i < 0x80; i++)
	{
		const u8 b = kPaletRom[kPaletClut[i] & 0xf];
		u32 c = ((((b >> 4) & 1) * 0x47 + ((b >> 5) & 1) * 0x97 + ((b >> 3) & 1) * 0x21) * 0x100 & 0xff00u) |
		        (((b >> 7) * 0x97 + ((b >> 6) & 1) * 0x47) & 0xffu) |
		        ((((b >> 1) & 1) * 0x47 + ((b >> 2) & 1) * 0x97 + (b & 1) * 0x21) * 0x10000 & 0xff0000u);
		if ((i & 3) != 0 && c != 0) c |= 0xff000000u;
		pal[i] = c;
	}
	// MENU_PaletOffset = 0
	for (int p = 0; p < 0x20; p++) d.SetPalet(p, 0, &pal[p * 4], 4);
	// CDraw +0x141208.. = the config texture, 0xa0, 0x2f (the menu font setup): not ported
	for (int p = 0; p < 0x25; p++)
	{
		u32 c[4];
		for (int k = 0; k < 4; k++)
		{
			const u8* s = kMenuPalet + p * 0x10 + k * 4;
			c[k] = (u32)s[0] << 24 | (u32)s[1] << 16 | (u32)s[2] << 8 | s[3];
		}
		d.SetPalet(p + 0x20, 0, c, 4);
	}
}

// RSO2 0x80d474f4
void CallRender()
{
	CDraw& d = *draw;
	const float kScale = 1.0f;  // RSO2 0x80dc8628
	d.Begin(5);
	const bool busy = Sw32(0x14) != 0;
	if (busy) SetSw32(0x14, Sw32(0x14) - 1);
	if (!busy && sw.b(0x2d) == 0)
	{
		const bool notFlip = (mem[0x5003] & 1) == 0;
		const u32 mirror = notFlip ? 0 : 3;
		// the playfield: video RAM columns 2..0x21, 28 rows each (0x4040..0x43bf)
		for (int i = 2, x = 0x10; i < 0x22; i++, x += 8)
		{
			for (int k = 0x1b, off = 0x360, y = 0xd8; k >= 0; k--, off -= 0x20, y -= 8)
			{
				const u8 code = mem[0x403e + i + off];
				const u8 col = mem[0x443e + i + off];
				if (code == 0x40) continue;
				if (kFontPatch[code] == 0) d.DrawChrEx(kScale, kScale, x, y, 20000, texBg, code, col & 0x1f, 0, 0);
				else d.DrawChrEx(kScale, kScale, x, y, 20000, texConfig, kFontPatch[code], col & 0x1f, 0, 0);
			}
		}
		// the two status rows on each side (0x4000.. at x 0x110 / 0x118, 0x43c0.. at x 0 / 8)
		for (int c = 0; c < 2; c++)
		{
			for (int j = 0; j < 0x1c; j++)
			{
				const u16 a = (u16)(0x4000 + c * 0x20 + 2 + j);
				if (mem[a] != 0x40) d.DrawChrEx(kScale, kScale, 0x110 + c * 8, j * 8, 20000, texBg, mem[a], mem[a + 0x400] & 0x1f, 0, 0);
			}
		}
		for (int c = 0; c < 2; c++)
		{
			for (int j = 0; j < 0x1c; j++)
			{
				const u16 a = (u16)(0x43c0 + c * 0x20 + 2 + j);
				if (mem[a] != 0x40) d.DrawChrEx(kScale, kScale, c * 8, j * 8, 20000, texBg, mem[a], mem[a + 0x400] & 0x1f, 0, 0);
			}
		}
		// the sprites; pass 2 (save+0x50) redraws last frame's codes 8 / 9 as 0x40 / 0x41
		const int passes = 1 + (save[0x50] != 0 ? 1 : 0);
		int zoff = 0;
		for (int pass = 0; pass < passes; pass++)
		{
			const u8* atr = pass == 0 ? &mem[0x4ff2] : oldAtr;
			const u8* xy = pass == 0 ? &mem[0x5062] : oldXY;
			for (int i = 0; i < 6; i++)
			{
				const u8 a = atr[0];
				const u8 palette = atr[1];
				atr += 2;
				u32 code = a >> 2;
				if (save[0x50] != 0 && pass != 0)
				{
					if (code - 8 >= 2) continue;
					code = (code & 1) + 0x40;
				}
				int y = notFlip ? xy[0] - 0x1f : xy[0] - 0x20;
				const u8 sx = xy[1];
				xy += 2;  // quirk kept: only advanced for drawn sprites
				if (i < 2) y++;
				d.DrawChrEx(kScale, kScale, 0x110 - sx, y, (i + 10000) - zoff, texSprite, code, palette & 0x1f, (a & 1) | ((a >> 1) & 1) << 1, mirror);
			}
			zoff += 10;
		}
		std::memcpy(oldAtr, &mem[0x4ff2], 12);
		std::memcpy(oldXY, &mem[0x5062], 12);
	}
	// the (c) mark next to the copyright line
	if (sw.b(0x73) != 0 && sw.b(0x23) == 0 && sw.b(0x2b) == 0)
	{
		if (mem[0x425f] == 0x19)
		{
			d.DrawChrEx(kScale, kScale, 0x100, 0x48, 20000, texConfig, 0x80, 1, 0, 0);
			d.DrawChrEx(kScale, kScale, 0x108, 0x48, 20000, texConfig, 0x81, 1, 0, 0);
		}
		else
		{
			sw.b(0x73) = 0;
		}
	}
	d.End();
	d.Begin(4);
	if (sw.b(0x5c) != 0)
	{
		// sw+0x60 > 9: CDraw::PrintfUTF8 "NOW SAVING" at x sw+0x68: not ported
		if (Sw32(0x60) > 0x1d) SetSw32(0x60, 0);
		SetSw32(0x60, Sw32(0x60) + 1);
	}
	if (sw.b(0x77) != 0)
	{
		// the "sharing" texts (PrintfUTF8): not ported
		if (Sw32(0x7c) > 0x27) SetSw32(0x7c, 0);
		SetSw32(0x7c, Sw32(0x7c) + 1);
	}
	d.End();
	// sw+0xae bit 7: ACHIEVEMENT::DebugRender: not ported
	sw.b(0x6f) = 0;
	gameMenu->MenuRender();   // PACMAN::MenuRender RSO2 0x80d848c8
}

// RSO2 0x80d46eb8: CDraw's render (vtable +8) clears the frame and calls the screen spec's CallRender
void Render()
{
	draw->ClearFrame();
	CallRender();
}

// RSO2 0x80d83abc
void InitExtend()
{
	// CConvert8To4Way array, InitMenu_KeySetting(0 / 1): not ported (Wii menu)
	sw.b(0x31) = 0;
	EntryExtfuncExtend();
	copyright[0] = 0;
	// CROSSHATCH / SCREENADJUST construction: not ported
	MenuConstruct();
	sw.b(0xaf) = (u8)(kLocale << 4 | (sw.b(0xaf) & 0xf));
	storage->LoadRequest();
	storage->FrameUpdate();
	storage->ActivateLoadedData();
	if (std::memcmp(&save[0x34], "PPPC0007", 8) != 0) ExportData_Init();
	sw.b(0x53) = 1;
	sw.b(0x51) = save[0x4c];
}

const u8* Need(const RsoModule& m, const char* name, u32 min)
{
	const RsoModule::Export* e = m.Find(name);
	if (e == nullptr || e->size < min)
	{
		std::fprintf(stderr, "gotch::pacman: the module lacks %s\n", name);
		return nullptr;
	}
	return e->data;
}
std::string CString(const RsoModule& m, const char* name)
{
	const RsoModule::Export* e = m.Find(name);
	if (e == nullptr) return {};
	return std::string((const char*)e->data, strnlen((const char*)e->data, e->size));
}
}

bool Setup()
{
	if (isSetup) return true;
	const RsoModule& m = RsoModule::Volume(2);
	if (!m.IsLoaded())
	{
		std::fprintf(stderr, "gotch::pacman: nmwii2.rso not found under the data root\n");
		return false;
	}
	const u8* mainMemory = Need(m, "CPU1_MainMemory__6PACMAN", 0x10000);
	kAsciiToGame = Need(m, "AsciiCodeToGameCodeConvertTable__6PACMAN", 0x80);
	kInitTable4ff2 = Need(m, "InitTable4ff2__6PACMAN", 0x10);
	kMenuPalet = Need(m, "MENUPALET__6PACMAN", 0x250);
	kPaletRom = Need(m, "PALET_ROM__6PACMAN", 0x10);
	kPaletClut = Need(m, "PALET_CLUT__6PACMAN", 0x80);
	kRandomTable = Need(m, "RANDOM_TABLE__6PACMAN", 0x800);
	kCgromA = Need(m, "CGROM_A__6PACMAN", 0x1000);
	kCgromB = Need(m, "CGROM_B__6PACMAN", 0x1000);
	kCgromConfig = Need(m, "CGROM_CONFIG_V", 0x1000);
	kFontPatch = Need(m, "FontPatchTable__6PACMAN", 0x100);
	kTable2c5e = Need(m, "Table2c5e__6PACMAN", 0x78);
	kVersionJ = Need(m, "VersionJStringTable__6PACMAN", 0x6c);
	kDipDefault = Need(m, "IgnoreScreenFlip__6PACMAN", 3);
	const u8* gsDefault = Need(m, "GAME_SETTING_DEFAULT__6PACMAN", 0x10);
	const u8* wsgRom = Need(m, "WSGROM__6PACMAN", 0x100);
	kLayoutBg = m.At(0x80dff250, 8 * 8 * 8 * 4);
	kLayoutSprite = m.At(0x80dffa50, 16 * 16 * 8 * 4);
	kLayoutConfig = m.At(0x80e01a50, 8 * 8 * 8 * 4);
	if (!mainMemory || !kAsciiToGame || !kInitTable4ff2 || !kMenuPalet || !kPaletRom || !kPaletClut || !kRandomTable || !kCgromA || !kCgromB ||
	    !kCgromConfig || !kFontPatch || !kTable2c5e || !kVersionJ || !kDipDefault || !gsDefault || !wsgRom || !kLayoutBg || !kLayoutSprite || !kLayoutConfig)
		return false;
	for (int i = 0; i < 4; i++)
		kGameSettingDefault[i] = (s32)((u32)gsDefault[i * 4] << 24 | (u32)gsDefault[i * 4 + 1] << 16 | (u32)gsDefault[i * 4 + 2] << 8 | gsDefault[i * 4 + 3]);
	// how far RANDOM_ptr may read: the rest of the module file from the table on
	kRandomAvail = 0x800;
	while (kRandomAvail < 0x10000 && m.At(0x80df7640 + kRandomAvail, 0x100) != nullptr) kRandomAvail += 0x100;
	sNamcoLogo = CString(m, "NamcoLogo_String__6PACMAN");
	sCopyright1 = CString(m, "Copyright1_String__6PACMAN");
	sCopyright2 = CString(m, "Copyright2_String__6PACMAN");
	std::memcpy(mem, mainMemory, sizeof(mem));

	// RSO2 0x80d465b0
	env = Z80_env{};
	env.ReadM1 = CPU1_ReadM1;
	env.ReadMemory8 = CPU1_ReadMemory8;
	env.ReadIO8 = CPU1_ReadIO8;
	env.WriteMemory8 = CPU1_WriteMemory8;
	env.WriteIO8 = CPU1_WriteIO8;
	env.ExtFunc = CPU1_ExtFunc;
	z80 = CPU_SPEC{};
	z80.Init = [](void* e) { return Z80_Init((Z80_env*)e); };
	z80.Reset = [](void* e) { return Z80_Reset((Z80_env*)e); };
	z80.Run = [](void* e) { return Z80_Run((Z80_env*)e); };
	z80.SetFlag = [](void* e, int f, void* v) { return Z80_SetFlag((Z80_env*)e, (CPUFLAG)f, v); };
	z80.name = "";
	z80.env = &env;
	// 3.072 MHz: (u64)(2^40 * 3.072f) * 1000 * 1000 (RSO2 0x80dc8608 * 0x80de707c, ints 0x80de6fe0 / 0x80de6fdc)
	z80.clock = (std::uint64_t)(kETimeSecond * 3.072f) * 1000u * 1000u;
	// CreateAudioSpec(&{ETIME 2^40 * 32000.0f (static 0x811fa308, RSO2 0x80d471b4)}, 0, 0x10000, 0x10000): 32000 samples per second
	primary = std::make_unique<PrimaryStream>(32000, 0x2000, 0x10000, 0x10000);
	// CreateScreenSpec: 288 x 224, rotated 270 degrees, InitRender / CallRender / ExitRender
	machine = MACHINE_SPEC{};
	CreateMachineSpecEx(machine, "PACMAN", PeriodFromHz(60.0f), TimeFromSeconds(0.0025252527f), Render, ConvertIO, HardwareInit, VBlankIn, VBlankOut,
	                    Exit, HotResetMachine);
	AddCPU(machine, z80);
	// InitMachine: the services (CDraw + the screen spec's InitRender), then ResetMachine
	draw = std::make_unique<CDraw>(0x120, 0xe0);
	draw->SetAccentTable(m.At(0x80dd27d8, 21 * 12));   // the accented letters of CDraw::PrintfUCS2 (module data)
	storage = std::make_unique<CDataStorage>();
	save = storage->Data();
	gameio = std::make_unique<CGameIO>(m, &machine.running, &machine.unk114, [] { gotch::HotResetMachine(machine); });   // CGAMEIO +0 = the machine
	InitRender();
	ResetMachine(machine);
	storage->magicOffset = 0x28;   // DataStorage +0x34
	storage->sumOffset = 0x2c;     // DataStorage +0x38
	gameio->InitPadData();
	sw = SWork{};
	swork_ColdStart(sw, kLocale);  // UNVERIFIED: the original asks GetGameLocale before SetGameLocale(1)
	sw.b(0xaf) = (u8)((sw.b(0xaf) & 0xf) | 0x10);
	// SetGameLocale(1)
	InitExtend();
	wsg = std::make_unique<SndNamcoWsg3>(*primary, wsgRom);
	gameState = std::make_unique<GameState>();   // PACMAN__GameState
	// ACHIEVEMENT: the counters (+0x1e4..0x200 = 0, +0x1ac = 0xf, +0x1b0 = 1; +0x174 = the title)
	std::memset(achievement, 0, sizeof(achievement));
	SetAch32(0x1ac, 0xf);
	SetAch32(0x1b0, 1);
	isSetup = true;
	return true;
}

bool IsSetup() { return isSetup; }

// RSO2 0x80d46eac (PACMAN::ExecuteExit -> ExecuteExit -> machine +0xf4). The original frees the machine and the host unloads the module:
// the next Start builds it again (Setup, InitExtend = the save loaded from the bridge)
void ExitMachine()
{
	Exit();
	isSetup = false;
}

void StorageFrameUpdate() { if (storage) storage->FrameUpdate(); }   // CDraw::WaitVSync RSO2 0x80d06d9c

int Execute1V() { return gotch::Execute1V(machine); }  // RSO2 0x80d46e94

void HotReset() { HotResetMachine(); }

PrimaryStream* Primary() { return primary.get(); }

bool Playing() { return sw.b(0x2f) != 0; }
bool Paused() { return sw.b(6) != 0 || sw.b(7) != 0; }

void SetSoundBufferClear(void (*fn)()) { soundBufferClear = fn; }

void ExitRequest() { gameio->ExitRequest(sw); }   // the menu's EXIT (UNVERIFIED: the item's path to CGAMEIO::ExitRequest is not traced)

// RSO2 0x80d81854
void GameStartInit(int value)
{
	gameio->ResetGo();
	if (value == 0x18100)
	{
		sw.b(0x39) = 1;
		sw.b(0x34) = 1;
	}
	else if (value == 0x10000)
	{
		sw.b(0x34) = 1;
	}
	else if (value == 0x18200)
	{
		sw.b(0x39) = 1;
		sw.b(0x35) = 1;
	}
	else if (value == 0x10100)
	{
		sw.b(0x35) = 1;
	}
	sw.b(0x6d) = 1;
	save[0x46] = (u8)optionsItem[0]->cursor;
	save[0x47] = (u8)optionsItem[1]->cursor;
	sw.b(0xad) = (u8)optionsItem[2]->cursor;
	SaveDataToGameSetting();
	RebuildDipSwitches();
}

const CDraw& Draw() { return *draw; }
}
}
