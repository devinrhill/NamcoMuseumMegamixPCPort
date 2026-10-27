// RALLY-X of the arcade collection vol 2 (nmwii2.rso, class RALLYX): a 1:1 port of the machine glue around the Z80 board (see include/gotch/RallyX.hpp,
// and src/gotch/PacMan.cpp which this follows). Module globals as file statics; comments cite them as "sw+0xNN" (RALLYX::THISGAME_sw 0x81203848 + NN),
// "mem[0xNNNN]" (RALLYX::MainMemory 0x80fb6740 + NNNN) and "save+0xNN" (the export data block, CDataStorage 0x81203c24 + 4).
//
// Differences to PACMAN worth knowing: landscape picture (rotation 0); the sound is a WSG3 at mem[0xa100..0xa11f] plus one PCM voice (the bomb,
// written at 0xa180); the ROM patches are all in one hook (RALLYX::CPU1_ExtFuncExtend, called by RST 0 with PC = the patched address); the save block
// is "RLYX0001" at save+0xdc with 9-byte hi score records.
//
// Not ported (marked at the sites): the screen adjust menu (mode 9), CROSSHATCH / SCREENADJUST, CDraw::FadeSet, MenuMM, MENU_KEYCONV.
#include "gotch/RallyX.hpp"

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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

namespace gotch
{
namespace rallyx
{
namespace
{
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using s32 = std::int32_t;

// ---- module data (read from the player's nmwii2.rso at Setup) ----
const u8* kHiscoreInit = nullptr;   // HISCORE_INITDATA__6RALLYX (RSO2 0x81147468, 8 bytes)
const u8* kAsciiToGame = nullptr;   // AsciiCodeToGameCodeConvertTable__6RALLYX (0x81147470)
const u8* kSoundTest = nullptr;     // SoundTestTable__6RALLYX (0x811474f0, 6 bytes per entry)
const u8* kLeftToDip = nullptr;     // LeftToDipSWTable__6RALLYX (0x811475c8; MenuStringConvertTable[0x80 + n] reads it)
const u8* kMenuPalet = nullptr;     // MENUPALET__6RALLYX (0x80fc6740, 0x25 palettes x 4 big-endian u32)
const u8* kPaletRom = nullptr;      // PALET_ROM__6RALLYX (0x80fc7b90)
const u8* kPaletClut = nullptr;     // PALET_CLUT__6RALLYX (0x80fc7a90)
const u8* kCgromA = nullptr;        // CGROM_A__6RALLYX (0x80fc6990)
const u8* kCgromC = nullptr;        // CGROM_C__6RALLYX (0x80fc7990)
const u8* kCgromConfig = nullptr;   // CGROM_CONFIG_V
const u8* kLayoutBg = nullptr;      // unexported CGen layouts: background / config (0x80fc7ec0), sprites (0x80fc86c0), radar (0x80fc7cc0)
const u8* kLayoutSprite = nullptr;
const u8* kLayoutRadar = nullptr;
const u8* kPcmBomb = nullptr;       // PCM_BOMB__6RALLYX (0x80fca6c0..0x80ffea18)
const u32 kPcmBombSize = 0x80ffea18u - 0x80fca6c0u;
std::string sCopyright1, sCopyright2;   // Copyright1_String / Copyright2_String (one character each)

// ---- the module's globals ----
u8 mem[0x10000];             // RALLYX::MainMemory 0x80fb6740
Z80_env env;                 // RALLYX::Z80_ENV 0x81203a70
CPU_SPEC z80;                // RALLYX::Z80 0x81203ab8
MACHINE_SPEC machine;        // RALLYX::Machine 0x81203b00
SWork sw;                    // RALLYX::THISGAME_sw 0x81203848
u8 io[3];                    // RALLYX::IO 0x81203d40: IN0 (0xa000), IN1 (0xa080), DSW (0xa100)
u8 z80io[0x100];             // RALLYX::Z80IO 0x81203c40
std::unique_ptr<GameState> gameState;    // RALLYX::GameState
std::unique_ptr<CDataStorage> storage;   // *0x81203c24
u8* save = nullptr;          // *(CDataStorage + 4): the export data (save block)
u8 copyright[8];             // RALLYX::Copyright (written only)
std::unique_ptr<CDraw> draw;
std::unique_ptr<PrimaryStream> primary;
std::unique_ptr<SndNamcoWsg3> wsg;       // RALLYX::WSG
std::unique_ptr<SndPcm> pcm;             // RALLYX::PCM
bool pcmLatch = false;                   // RALLYX::@LOCAL@WriteMemory8 (0x81203d44)
int texBg = -1, texSprite = -1, texRadar = -1, texConfig = -1;   // wdh_cgrom_bg / _sprite / _radar / _config
std::unique_ptr<CGameIO> gameio;
std::unique_ptr<WiiMenu> gameMenu;       // RALLYX::GameMenu (GAMEMENU = WII_MENU with the overrides below)
void (*soundBufferClear)() = nullptr;
// RALLYX::OptionsMenuItem 0x8120a7c8..: BONUS, LIFE, SOUND TEST, SCREEN ADJUSTMENT, DEFAULT, EXIT (rebuilt every frame by OptionsMenuBuild)
MenuItem* optionsItem[6] = {};
char soundTestText[0x40][8] = {};   // RALLYX::SOUND_TEST_NO_Buffer
struct OptionStrings
{
	const char* options = "";
	const char* bonus = "";
	const char* life = "";
	const char* soundTest = "";
	const char* screenAdjust = "";
	const char* deflt = "";
	const char* exit = "";
	const char* extend2[3] = {"", "", ""};   // MenuStr_RALLYX_OPTIONS_EXTEND_LEFT2_{1,2,3}of3
	const char* extend3[3] = {"", "", ""};   // ..._LEFT3_...
} optStr;
bool soundTestSmoke = false;   // RALLYX::SoundTest_Smoke 0x8120a6b9
bool isSetup = false;
const int kLocale = 1;
const int kMenuPaletOffset = 0x41;   // RALLYX::MENU_PaletOffset

// ---- helpers over the original layouts ----
u8& A() { return env.r8(Z80_env::A); }
u8 B() { return env.r8(Z80_env::B); }
u16 HL() { return env.r16(Z80_env::HL); }
void SetHL(u16 v) { env.set16(Z80_env::HL, v); }
u16 DE() { return env.r16(Z80_env::DE); }
u16 SP() { return env.r16(Z80_env::SP); }
u16 PC() { return env.r16(Z80_env::PC); }
void SetPC(u16 v) { env.set16(Z80_env::PC, v); }
// a push of a return address straight into mem
void Push(u16 v)
{
	const u16 sp = SP();
	mem[(u16)(sp - 1)] = (u8)(v >> 8);
	mem[(u16)(sp - 2)] = (u8)v;
	env.set16(Z80_env::SP, (u16)(sp - 2));
}
s32 Sw32(int off) { return sw.i32(off); }
void SetSw32(int off, s32 v) { sw.set32(off, v); }
int Bank() { return sw.b(0xa3); }   // RALLYX::GetCurrentBankNumber 0x80d73db4 (DAT_812038eb)

[[noreturn]] void Spin(const char* where)
{
	// the original spins forever on a PC that has no hook
	std::fprintf(stderr, "gotch::rallyx: %s: no hook for PC 0x%04x (the original hangs here)\n", where, (unsigned)PC());
	std::abort();
}

// the hi score record of the current bank (save+0xe4 + bank * 9, 8 BCD bytes) and the game's copy (mem[0x8060..0x8067])
void HiScorePush()   // RSO2 0x80d74fb8 (inlined in the hooks)
{
	for (int i = 0; i < 8; i++) save[Bank() * 9 + 0xe4 + i] = mem[0x8060 + i];
	SetSw32(0x58, 1);
	sw.b(0x54) = 1;
	sw.b(0x53) = 1;
}
void HiScorePop()    // RSO2 0x80d750d0 (inlined in the 0x558 hook)
{
	if (sw.b(0x53) == 0) return;
	for (int i = 0; i < 8; i++) mem[0x8060 + i] = save[Bank() * 9 + 0xe4 + i];
}

// RSO2 0x80d73a1c
void ExportDataDefaultReset()
{
	save[0xf6] = 9;
	save[0xf7] = (u8)((save[0xf7] & 0x9f) | 0x10);
	sw.b(0xad) = 0;
	save[0xf7] |= 0x80;
	sw.b(0x51) = (u8)(save[0xf7] >> 7);
	save[0xf7] |= 8;
	sw.b(0x54) = 1;
	SetSw32(0x58, 1);
}

// RSO2 0x80d73b30
void ExportData_Init()
{
	std::memcpy(&save[0xdc], "RLYX0001", 8);
	ExportDataDefaultReset();
	// the screen adjust ints save+0xfc..0x118 = 0 (the CDraw zoom / aspect copies: not ported)
	for (int off : {0xfc, 0x100, 0x110, 0x10c, 0x118, 0x114, 0x108, 0x104}) std::memset(&save[off], 0, 4);
	for (int i = 0; i < 8; i++)
	{
		save[0xe4 + i] = kHiscoreInit[i];
		save[0xed + i] = kHiscoreInit[i];
	}
	save[0xec] = 0;
	save[0xf5] = 0;
	save[0xf7] = (u8)(((storage->Valid() ? 1 : 0) << 3) | (save[0xf7] & 0xf7));
	save[0xf8] = 0;
	save[0xf9] = 0;
	if (sw.b(0x2f) == 0)
	{
		sw.b(0xae) &= 0x83;
		sw.b(0xaf) = (u8)(kLocale << 4 | (sw.b(0xaf) & 7) | 4);
	}
}

// RSO2 0x80d73d44 (not called from RALLYX's own code; the lives / bonus switches live in save+0xf6)
void SaveDataToGameSetting()
{
	SetSw32(0x80, save[0xf6] >> 6);
	SetSw32(0x84, (save[0xf6] >> 3) & 7);
	SetSw32(0x8c, (save[0xf6] >> 2) & 1);
	SetSw32(0x88, (save[0xf6] >> 1) & 1);
	if (sw.b(0xa3) != 0)
	{
		SetSw32(0x80, 1);
		SetSw32(0x8c, 1);
		SetSw32(0x84, 1);
		SetSw32(0x88, 0);
	}
}

// RSO2 0x80d75458: the DIP switches from the options in save+0xf6
void ReflectSaveDataToItemDipSW()
{
	const u8 b = save[0xf6];
	sw.b(0xa2) = (u8)((b >> 2) & 1);
	io[1] = (u8)((io[1] & 0xfe) | sw.b(0xa2));
	sw.b(0xa0) = (u8)((((((b >> 6) + 1) * 2) | 0xe1) & 199) | kLeftToDip[(b >> 3) & 1]);
	io[2] = sw.b(0xa0);
}

// RSO2 0x80d73f18
void CopyrightStringSet()
{
	mem[0x7000] = 0x73;
	mem[0x7001] = 1;
	mem[0x7002] = kAsciiToGame[(u8)(sCopyright1.empty() ? 0 : sCopyright1[0])];
	mem[0x70f] = 4;
	mem[0x710] = 0x87;
	mem[0x712] = 0;
	mem[0x713] = 0x70;
	mem[0x7080] = 0x73;
	mem[0x7081] = 1;
	mem[0x7082] = kAsciiToGame[(u8)(sCopyright2.empty() ? 0 : sCopyright2[0])];
}

// RSO2 0x80d74e88: RST 0 (0xC7) over the patched ROM addresses, the "CHALLENGING STAGE NO." text and its tail
void EntryExtfuncExtend()
{
	static const u16 kAt[] = {0x0000, 0x0006, 0x397c, 0x39b9, 0x0741, 0x0743, 0x19ef, 0x08b4, 0x1aee, 0x1a8e, 0x06a8, 0x14da, 0x14e7, 0x14ee,
	                          0x06d4, 0x06fe, 0x0724, 0x1ed5, 0x1578, 0x0558, 0x1d86, 0x08a0, 0x08a1, 0x07c1, 0x057b, 0x175a, 0x3966, 0x18a4,
	                          0x0377, 0x16ef, 0x19b2, 0x0714, 0x27ba};
	for (u16 a : kAt) mem[a] = 0xc7;
	mem[0x3e30] = 0x28;
	mem[0x3e31] = 0x84;
	mem[0x3e32] = 0x16;
	const char* text = "CHALLENGING@STAGE@NO.%";   // s_CHALLENGING_STAGE_NO___80dc8e9c (the module's literal, '@' = space in the game's font)
	for (int i = 0; text[i] != 0; i++) mem[0x3e33 + i] = (u8)text[i];
	mem[0x8cb] = 0x30;
	mem[0x8cc] = 0x3e;
}

// RSO2 0x80d751f0
int GetContinueRound() { return gameMenu->GetContinueRound(gameMenu->players) + 1; }

// ---- the Z80 bus (RSO2 0x80d5f60c..0x80d5f750) ----
u8 CPU1_ReadMemory8(u16 a)
{
	if (a == 0xa000) return io[0];
	if (a == 0xa080) return io[1];
	if (a == 0xa100) return io[2];
	return mem[a];
}
u8 CPU1_ReadM1(u16 a) { return mem[a]; }
void CPU1_WriteMemory8(u16 a, u8 v)
{
	if (a < 0x4000) return;
	mem[a] = v;
	if ((u16)(a + 0x5f00) < 0x20) wsg->SetRegister(&mem[0xa100]);
	if (a == 0xa180)
	{
		const bool now = v != 0;
		if (now != pcmLatch && pcmLatch) pcm->Play(0);   // the falling edge plays the bomb
		pcmLatch = now;
	}
}
u8 CPU1_ReadIO8(u16 port) { return z80io[port & 0xff]; }
void CPU1_WriteIO8(u16 port, u8 v)
{
	if ((u8)port != 0) return;
	env.set16(Z80_env::BUS, v);   // the interrupt vector
}

// RSO2 0x80d769a0. Pad words come from the 4-way pair (ControllerValue +0x20): 0x10 up, 0x20 right, 0x40 down, 0x80 left, 0xf000 = the button
void IO_Convert()
{
	u8 in[2] = {0xff, 0xfe};
	for (int p = 0; p < 2; p++)
	{
		const ControllerValue& cv = gameio->Pad(p);
		u32 w = gameio->invalidFrames == 0 ? (u32)(cv.way4 >> 32) | (u32)cv.way4 << 4 : 0;
		if (!gameio->DigitalPadDirectionState(0)) w |= (u32)((s32)w >> 16);
		w |= (u32)((s32)w >> 16);
		if (w & 0x10) in[p] &= 0xdf;
		if (w & 0x40) in[p] &= 0xef;
		if (w & 0x80) in[p] &= 0xfb;
		if (w & 0x20) in[p] &= 0xf7;
		if (w & 0xf000) in[p] &= 0xfd;
	}
	io[0] = in[0];
	io[1] = (u8)((in[1] & 0xfe) | (sw.b(0xa2) & 1));
	io[2] = sw.b(0xa0);
}

// RSO2 0x80d60004
void ConvertIO()
{
	gameio->SetPadData();
	IO_Convert();
}

// ---- the hooks (RSO2 0x80d75594): RST 0 over the patched ROM addresses; the PC is the patched address ----
void CPU1_ExtFuncExtend()
{
	const u8 a0 = A();
	const u8 fillByte = mem[0x7080];
	const ControllerValue& cv0 = gameio->Pad(0);
	const u32 edge0 = gameio->invalidFrames == 0 ? (u32)(cv0.edge >> 32) : 0;   // **(gameio + 4) + 0x30
	switch (PC())
	{
	case 0x14da:
		A() = sw.b(0x2e) == 0 ? 0xff : CPU1_ReadMemory8(0xa000);
		SetPC(0x14dd);
		break;
	case 0x6fe:
		if (sw.b(0x2e) == 0)
		{
			SetPC(0x707);
		}
		else
		{
			SetHL(0x1f5a);
			SetPC(0x701);
		}
		break;
	case 0x558:
		A() = (u8)(A() << 1);
		SetPC(0x559);
		HiScorePop();
		break;
	case 6:
		machine.skipFrames = 0;
		if ((sw.b(0xa0) & 1) == 0)
		{
			machine.skipFrames = 0x30c;
		}
		else
		{
			gameState->SetPlayerState(0, 1);
			gameState->SetPlayerState(1, 1);
			gameState->ChangeState(3);
		}
		sw.b(0x52) = 0;
		if (sw.b(0x2e) == 0) SetSw32(0x18, 3);
		SetPC(0x3a);
		break;
	case 0:
		machine.skipFrames = 600;
		gameState->ChangeState(2);
		sw.b(0x51) = 1;
		SetSw32(0x18, 3);
		SetPC(3);
		break;
	case 0x377:
		SetHL(0x824c);
		SetPC(0x37a);
		if (((sw.b(0xae) >> 6) & 1) != 0) SetPC(0x3bd);
		break;
	case 0x6a8:
		gameState->SetPlayerState(0, 1);
		gameState->SetPlayerState(1, 1);
		gameState->ChangeState(3);
		if (sw.b(0x2e) == 0) mem[0x8024] = 0;
		sw.b(0x2f) = 0;
		sw.b(0x39) = 0;
		HiScorePush();
		sw.b(0xa3) = 0;
		for (int i = 0; i < 8; i++) mem[0x8060 + i] = save[0xe4 + i];
		sw.b(5) = 0;
		sw.b(4) = 0;
		SetPC(0x57b);
		break;
	case 0x57b:
		if (Sw32(0x8c) == 0)
		{
			A() = mem[0x81a9];
			mem[0xa183] = mem[0x81a9];
		}
		else
		{
			mem[0xa183] = 1;
		}
		SetPC(0x584);
		mem[0x81a8] = mem[0xa183];
		mem[0x81a9] = mem[0xa183];
		break;
	case 0x6d4:
		if (sw.b(0x2e) == 0)
		{
			SetHL(0x1f42);
			SetPC(0x6d7);
		}
		else
		{
			SetPC(0x1c29);
			Push(0x06d7);
		}
		break;
	case 0x743:
		gameState->SetPlayerState(0, 2);
		gameState->SetField1c(1);
		gameState->ChangeState(4);
		sw.b(4) = 1;
		sw.b(6) = 0;
		SetSw32(0xc, 10);
		SetHL(0x8024);
		SetPC(0x746);
		break;
	case 0x724:
		if (sw.b(0x2e) == 0)
		{
			SetPC(0x717);
			if (sw.b(0x34) != 0)
			{
				if (sw.b(0x6d) == 0)
				{
					ReflectSaveDataToItemDipSW();
					sw.b(0x6d) = 1;
					env.r8(Z80_env::B) = CPU1_ReadMemory8(0xa100);
					SetPC(0x4e7);
				}
				else
				{
					sw.b(0x34) = 0;
					sw.b(0x6d) = 0;
					SetPC(0x743);
				}
				if (sw.b(0x39) != 0) mem[0x81b0] = (u8)GetContinueRound();
			}
			if (sw.b(0x35) != 0)
			{
				if (sw.b(0x6d) == 0)
				{
					swork_SetWork_1P_PauseStatus(sw, 1, 10);
					swork_SetWork_2P_PauseStatus(sw, 0, 10);
					ReflectSaveDataToItemDipSW();
					sw.b(0x6d) = 1;
					env.r8(Z80_env::B) = CPU1_ReadMemory8(0xa100);
					SetPC(0x4e7);
				}
				else
				{
					sw.b(0x35) = 0;
					sw.b(0x6d) = 0;
					SetPC(0x732);
				}
				if (sw.b(0x39) != 0)
				{
					mem[0x81b0] = (u8)GetContinueRound();
					mem[0x81b1] = (u8)GetContinueRound();
				}
			}
		}
		else
		{
			A() = CPU1_ReadMemory8(0xa000);
			SetPC(0x727);
		}
		break;
	case 0x714:
	{
		// the "COPYRIGHT" characters: fill mem[0x8f44..] with mem[0x7080] and copy mem[0x7082..] to mem[0x8744..], mem[0x7081] bytes each
		const u32 n = mem[0x7081];
		for (u32 i = 0; i < n; i++) mem[(u16)(0x8f44 + i)] = fillByte;
		for (u32 i = 0; i < n; i++) mem[(u16)(0x8744 + i)] = mem[(u16)(0x7082 + i)];
		SetPC(0x1b15);
		Push(0x0717);
		break;
	}
	case 0x741:
		gameState->SetPlayerState(0, 2);
		gameState->SetPlayerState(1, 3);
		gameState->SetField1c(2);
		gameState->ChangeState(4);
		sw.b(4) = 1;
		sw.b(7) = 0;
		sw.b(6) = 0;
		SetSw32(0xc, 10);
		SetPC(0x74c);
		break;
	case 0x8a1:
		if (sw.b(0x2f) != 0)
		{
			const u8 p = CPU1_ReadMemory8(0x81aa);
			const u8 q = CPU1_ReadMemory8(0x81ab);
			if ((p & 2) != 0)
			{
				sw.b(4) = 1;
				sw.b(5) = 0;
				gameState->SetPlayerState(0, 2);
				if ((q & 2) != 0) gameState->SetPlayerState(1, 3);
			}
			if ((p & 1) != 0)
			{
				sw.b(4) = 0;
				sw.b(5) = 1;
				gameState->SetPlayerState(1, 2);
				if ((q & 1) != 0) gameState->SetPlayerState(0, 3);
			}
		}
		A() = CPU1_ReadMemory8(0x81a8);
		SetPC(0x8a4);
		break;
	case 0x7c1:
		SetPC(0x7c2);
		if (((save[0xf7] >> 4) & 1) != 0 && mem[HL()] == 0) mem[HL()] = 2;
		// ArrivalRoundUpdate (RSO2 0x80d73dc4, inlined): the furthest round of the bank, 2 less than the game's round counters
		if (sw.b(0x2f) != 0 && ((sw.b(0xae) >> 5) & 1) == 0)
		{
			u8 r = mem[0x81b0];
			if (r < mem[0x81b1]) r = mem[0x81b1];
			u8& best = save[0xdc + Bank() * 9 + 0x10];
			if (best < (u8)(r - 2))
			{
				best = (u8)(r - 2);
				sw.b(0x54) = 1;
			}
		}
		A() = 0;
		break;
	case 0x8a0:
	{
		SetPC(0x8a1);
		const u8 q = (u8)(A() / 10);
		if (q != 0) mem[(u16)(HL() - 1)] = q;
		mem[HL()] = (u8)(A() - q * 10);
		break;
	}
	case 0x8b4:
		if (sw.b(0x2f) != 0)
		{
			gameState->ChangeState(4);
			sw.b(0x26) = 1;
		}
		A() = mem[HL()];
		SetPC(0x8b5);
		break;
	case 0x1a8e:
		SetPC(0x1a92);
		HiScorePush();
		if ((mem[HL()] & 1) != 0) SetPC(0x1ad8);
		break;
	case 0x175a:
		A() = Sw32(0x8c) != 0 ? 1 : 0;
		SetPC(0x175b);
		break;
	case 0x1578:
		SetHL(0x89f6);
		SetPC(sw.b(0x29) == 0 ? 0x157b : 0x157d);
		break;
	case 0x14ee:
		if (sw.b(0x2e) == 0)
		{
			A() = 0xf;
			if (sw.b(0x2f) == 0 && mem[0x8024] == 0 && Sw32(0x18) == 0 && (edge0 & 8) != 0)
			{
				A() = 0xc;
				mem[0x8024] = 1;
				gameMenu->EnableMenu();
				sw.b(0x21) = 1;
				sw.b(0x20) = 0;
				u32 round = save[0xec];
				if (((sw.b(0xae) >> 5) & 1) != 0) round = 0xfe;
				gameMenu->continueRound = (int)round;
				gameMenu->RebuildMenu();
				gameMenu->PushAndChangeMenu(2, true);
			}
		}
		else
		{
			A() &= 0xf;
		}
		SetPC(0x14f0);
		break;
	case 0x14e7:
		A() = sw.b(0x2e) == 0 ? 0xff : CPU1_ReadMemory8(0xa080);
		SetPC(0x14ea);
		break;
	case 0x16ef:
	case 0x19b2:
		SetPC((u16)(PC() + 3));
		env.set16(Z80_env::HL, (u16)(mem[0x8989] << 8 | mem[0x8988]));
		if (((sw.b(0xae) >> 4) & 1) != 0)
		{
			if (mem[0x81b1] != 1) mem[0x81b2] |= 1;
			mem[HL()]++;
		}
		break;
	case 0x18a4:
		A() = mem[0x8250];
		if (((sw.b(0xae) >> 2) & 1) != 0) A() = (u8)(mem[0x8250] * 10);
		SetPC(0x18a7);
		break;
	case 0x19ef:
		gameState->SetPlayerState(0, 5);
		gameState->ChangeState(5);
		sw.b(0x26) = 0;
		env.set16(Z80_env::IX, 0x8014);
		SetPC(0x19f3);
		break;
	case 0x27ba:
		SetPC(0x27bc);
		A() = 0;
		if (soundTestSmoke) SetPC(0x27c0);
		break;
	case 0x1d86:
		SetPC(0x1d9a);
		sw.b(0x54) = 1;
		break;
	case 0x1aee:
		if ((A() & 1) == 0)
		{
			sw.b(4) = 0;
			gameState->SetPlayerState(0, 5);
		}
		if ((a0 & 2) == 0)
		{
			sw.b(5) = 0;
			gameState->SetPlayerState(1, 5);
		}
		gameState->ChangeState(5);
		mem[HL()] = A();
		SetPC(0x1aef);
		break;
	case 0x1ed5:
		SetPC(0x1efc);
		if (sw.b(0x2e) != 0)
		{
			SetHL(0x1f64);
			SetPC(0x1ed8);
		}
		break;
	case 0x397c:
		if (sw.b(0x50) == 0)
		{
			A() = mem[DE()];
			SetPC(0x397d);
		}
		else
		{
			SetPC(0x3980);
		}
		break;
	case 0x3966:
		mem[0xa183] = 1;
		SetPC(0x39f9);
		break;
	case 0x39b9:
		A() = (u8)DE();
		SetPC(0x39ba);
		break;
	default:
		Spin("CPU1_ExtFuncExtend");
	}
}

// RSO2 0x80d5f754 (a thunk to the hook above)
void CPU1_ExtFunc() { CPU1_ExtFuncExtend(); }

// ---- the options menu (RSO2 0x80d72964) ----
// RSO2 0x80d75404 / 0x80d72714 (the BONUS, LIFE and SOUND TEST rows' left / right decide): the cursors into save+0xf6 and sw+0xad
void ReflectItemCursorToSaveData(MenuItem*)
{
	const u8 b = save[0xf6];
	const u8 bonus = (u8)(optionsItem[0]->cursor << 6);
	save[0xf6] = (u8)(bonus | (b & 0x3f));
	save[0xf6] = (u8)(((optionsItem[1]->cursor << 3) & 0x38) | bonus | (b & 7));
	sw.b(0xad) = (u8)optionsItem[2]->cursor;
}

// RSO2 0x80d74c70: the SOUND TEST row's decide (the item = the list entry, command = its number): six bytes of SoundTestTable into the game's
// sound request bytes, then the game's sound state is cleared
void OptionsSoundPlay(MenuItem* item)
{
	const u8* t = kSoundTest + item->info.command * 6;
	if (t[0] != 0xff) mem[0x89f4] = t[0];
	u8 c = t[1];
	if (c != 0xff)
	{
		c = t[2];
		mem[0x89f5] = t[1];
	}
	if (c != 0xff) mem[0x89f6] = c;   // quirk kept: nothing is stored when t[1] is 0xff
	if (t[3] != 0xff) mem[0x89f7] = t[3];
	if (t[4] != 0xff) mem[0x81b0] = t[4];
	soundTestSmoke = t[5] != 0;
	mem[0x898a] = 0xb0;
	mem[0x898b] = 0x81;
	for (int a : {0xa115, 0xa11a, 0xa11f, 0x8a28, 0x8a34, 0x8a48, 0x8a54, 0x8a68, 0x8a74, 0x8a88, 0x8a94, 0x8aa8, 0x8ab4, 0x8ac8, 0x8ad4, 0x8ae8,
	              0x8af4, 0x8b08, 0x8b14, 0x8b54})
		mem[a] = 0;
	sw.b(0x74) = 1;
}

// RSO2 0x80d74dd8
void OptionsSoundStop(MenuItem*)
{
	for (int a : {0x89f4, 0x89f5, 0x89f6, 0x89f7, 0xa115, 0xa11a, 0xa11f, 0x8a54, 0x8a68, 0x8a74, 0x8a88, 0x8b54, 0x8aa8, 0x8ab4, 0x8ae8, 0x8af4,
	              0x8a94, 0x8a28, 0x8a34, 0x8a48, 0x8ac8, 0x8ad4})
		mem[a] = 0;
	pcm->Stop(0);
	sw.b(0x74) = 1;
}

// RSO2 0x80d72964: rebuilt every frame (MenuExec). The texts are the module's (MenuStr_*, the literals at RSO2 0x80dc8ef0..)
void OptionsMenuBuild()
{
	const RsoModule& m = RsoModule::Volume(2);
	auto lit = [&](u32 addr) {
		const u8* p = m.At(addr, 1);
		return p != nullptr ? reinterpret_cast<const char*>(p) : "";
	};
	const int off = kMenuPaletOffset, stride = 1;
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
	i.decide = ReflectItemCursorToSaveData;
	i.decideMask = 2;
	i.command = 0x30000;
	optionsItem[0] = new MenuItem(i, optStr.bonus);
	// the BONUS choices depend on the LIFE setting (LEFT3 when it is 1)
	const char* const* extend = ((save[0xf6] >> 3) & 7) == 1 ? optStr.extend3 : optStr.extend2;
	i.command = 0x10;
	for (int k = 0; k < 3; k++)
	{
		optionsItem[0]->AddItem(new MenuItem(i, extend[k]), -1);
		i.command++;
	}
	i.command = 0x30100;
	optionsItem[1] = new MenuItem(i, optStr.life);
	optionsItem[1]->AddItem(new MenuItem(i, lit(0x80dc8ef0)), -1);   // "2"
	i.command++;
	optionsItem[1]->AddItem(new MenuItem(i, lit(0x80dc8ef2)), -1);   // "3"
	om.info.decide = OptionsSoundStop;
	i.decide = OptionsSoundPlay;
	i.decideMask = 1;
	i.command = 0x30b00;
	optionsItem[2] = new MenuItem(i, optStr.soundTest);
	for (int n = 0; n < 0xe; n++)
	{
		i.command = (u32)n;
		std::snprintf(soundTestText[n], sizeof(soundTestText[n]), "%s", FormatInt(lit(0x80dc8ef4), n).c_str());   // sprintf_s(buf, 8, "%02d", n)
		optionsItem[2]->AddItem(new MenuItem(i, soundTestText[n]), -1);
	}
	i.decide = ReflectItemCursorToSaveData;
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
	optionsItem[0]->SetItemCursor(save[0xf6] >> 6);
	optionsItem[1]->SetItemCursor((save[0xf6] >> 3) & 7);
	optionsItem[2]->SetItemCursor(sw.b(0xad));
	gameMenu->SetStartPauseScreenSpec(0x1c, 0x1c, 0);   // CMENU::SetScreenSpec(start / pause, 0x1c, 0x1c, rotation 0)
	gameMenu->SetMenuYoffset(0x68, 0x5c, 0x80, 0x80, 0x80, 0x80);
}

// RSO2 0x80d72770 (GAMEMENU::GameStartInit): the start flags the hook at 0x724 consumes
void GameStartInit(int value)
{
	if (value == 0x18100 || value == 0x18200) sw.b(0x39) = 1;
	if (value == 0x10100 || value == 0x18200)
		sw.b(0x35) = 1;
	else if (value == 0x10000 || value == 0x18100)
		sw.b(0x34) = 1;
}

// RSO2 0x80d72888 (GAMEMENU::MenuExitExtend) = HardwareInit's video RAM clear
void HardwareInit();
void MenuExitExtend() { HardwareInit(); }

// RSO2 0x80d7472c / 0x80d73f88 (the part of InitExtend that builds GAMEMENU). Texts: the module's MenuStr_*_LOCALE_NEUTRAL strings.
void MenuConstruct()
{
	const RsoModule& m = RsoModule::Volume(2);
	auto text = [&](u32 addr) {
		const char* t = m.Pointer(addr);
		return t != nullptr ? t : "";
	};
	WiiMenu::Strings str;
	str.pause = text(0x80dccf40);
	str.cont = text(0x80dcccd0);
	str.screenAdjust = text(0x80dcddf0);
	str.quit = text(0x80dcccbc);
	str.start1P = text(0x80dccbbc);
	str.start2P = text(0x80dccbd0);
	str.roundSelect = text(0x80dcde24);
	str.options = text(0x80dccbe4);
	str.startScreenAdjust = text(0x80dccc08);
	str.retMainMenu = text(0x80dccd84);
	str.returnToArcade = text(0x80dcde28);
	str.gameName = text(0x80dcd1e4);       // MenuStr_RALLYX_LOCALE_NEUTRAL
	str.soundTest = text(0x80dcccfc);
	optStr.options = str.options;
	optStr.bonus = text(0x80dccc3c);
	optStr.life = text(0x80dccc50);
	optStr.soundTest = str.soundTest;
	optStr.screenAdjust = str.screenAdjust;
	optStr.deflt = text(0x80dcd12c);
	optStr.exit = text(0x80dccfcc);
	for (int k = 0; k < 3; k++)
	{
		optStr.extend2[k] = text(0x80dcd574 + (u32)k * 4);   // MenuStr_RALLYX_OPTIONS_EXTEND_LEFT2_{k+1}of3_LOCALE_NEUTRAL
		optStr.extend3[k] = text(0x80dcd5a4 + (u32)k * 4);   // ..._LEFT3_...
	}
	// the machine's screen: 288 x 224, rotation 0 (CommonResourceSet RSO2 0x80d28124: PAD_DECIDE = 0x5800, PAD_CANCEL = 0x8400)
	gameMenu = std::make_unique<WiiMenu>(sw, *draw, *gameio, draw->ScreenW(), draw->ScreenH(), 0, str, 0x5800u, 0x8400u);
	gameMenu->soundBufferClear = soundBufferClear;
	gameMenu->gameStartInit = GameStartInit;
	gameMenu->menuExitExtend = MenuExitExtend;
	gameMenu->startBoxW8 = 0x1c;   // AutoCalcBlackBoxSize(2, ...): 0x1c x 0x1c
	gameMenu->startBoxH8 = 0x1c;
	gameMenu->SetPaletStatus(kMenuPaletOffset, 1);
	gameMenu->InitMenu();
	const u8* title = m.At(0x80dc8ef9, 8);   // s_RALLY_X_80dc8ef9
	gameMenu->SetStartMenuTitle(title != nullptr ? reinterpret_cast<const char*>(title) : str.gameName, -1, -1, -1, 3);
	gameMenu->startWait = 0x10;
	gameMenu->quitWait = 0x10;
	OptionsMenuBuild();
}

// RSO2 0x80d74ac4
void MenuExec()
{
	OptionsMenuBuild();
	gameMenu->MenuJob();
	sw.b(0xad) = (u8)optionsItem[2]->cursor;
	// (PAD_DECIDE & options +0x9c) on 0x31200: PushAndChangeMenu(9), not ported (the screen adjust menu)
	if (sw.b(0xd9) != 0)
	{
		// the DEFAULT row
		save[0xf6] = 9;
		save[0xf7] = (u8)((save[0xf7] & 0x9f) | 0x10);
		sw.b(0xad) = 0;
		save[0xf7] |= 0x80;
		sw.b(0x51) = (u8)(save[0xf7] >> 7);
		save[0xf7] |= 8;
		sw.b(0xd9) = 0;
		SetSw32(0x58, 1);
		sw.b(0x54) = 1;
	}
	// sw+0xd8: the screen adjust values copied into the save (never set: not ported)
}

// RSO2 0x80d75220
void VBlankIn_Extend()
{
	if (sw.b(0x54) != 0)
	{
		storage->SaveRequest();
		SetSw32(0x58, 0);
		sw.b(0x54) = 0;
	}
	u32 edge0 = 0, edge1 = 0;
	if (gameio->invalidFrames == 0)
	{
		edge0 = (u32)(gameio->Pad(0).edge >> 32);
		edge1 = (u32)(gameio->Pad(1).edge >> 32);
	}
	if (Sw32(0x10) == 0)
	{
		if (sw.b(0x2f) == 0) gameio->DebugModeCheck(sw);
		gameio->ROMRAMSkipCheck(sw, sw.b(0x52) != 0, &sw.b(0x51));
	}
	if (gameState->GetState() == 4 && gameMenu->Mode() == 1)
	{
		if (sw.b(4) != 0 && (edge0 & 8) != 0)
		{
			sw.b(6) = 1;
			gameMenu->MenuIn(6, true);
		}
		if (sw.b(5) != 0 && (edge1 & 8) != 0)
		{
			sw.b(7) = 1;
			gameMenu->MenuIn(6, true);
		}
	}
	if (!gameio->ExitCheck(sw))
	{
		MenuExec();
		gameMenu->drawHandle = texConfig;
	}
}

// RSO2 0x80d5fd74
void VBlankIn()
{
	VBlankIn_Extend();
	gameio->MakePadEG();
	const bool stop = sw.b(0x38) != 0 || sw.b(6) != 0 || sw.b(7) != 0;
	z80.SetFlag(&env, CPUFLAG_DISABLE, stop ? (void*)1 : nullptr);
	if (sw.b(0x38) == 0)
	{
		if ((mem[0xa181] & 1) != 0)
		{
			if (((env.status() >> 2) & 3) == 0) env.set16(Z80_env::BUS, 0x30);
			env.setStatus((u16)(env.status() | 0x80));
		}
		if (Sw32(0x18) != 0) SetSw32(0x18, Sw32(0x18) - 1);
		if (Sw32(0x10) != 0) SetSw32(0x10, Sw32(0x10) - 1);
		if (Sw32(0x14) != 0) SetSw32(0x14, Sw32(0x14) - 1);
		if (((sw.b(0xaf) >> 3) & 1) == 0) sw.b(0x6f) = 0;
		// CDraw::FadeSet(0, 0, 0x66666): not ported
	}
}

// RSO2 0x80d5fd5c
void VBlankOut() { machine.vblankCount++; }

// RSO2 0x80d5fbc8
void HardwareInit()
{
	std::memset(&mem[0x8000], 0xff, 0x2000);
	mem[0xa181] = 0;
}

// RSO2 0x80d5ff44
void HotResetMachine()
{
	swork_HotStart(sw);
	for (int i = 0; i < 0x20; i++)
	{
		const u16 a = (u16)(0xa100 + i);
		mem[a] = 0;
		wsg->SetRegister(&mem[0xa100]);
	}
	// the PCM latch at 0xa180 is never reached (the loop covers 0xa100..0xa11f)
}

// RSO2 0x80d745a8: the save goes out at once
void ExitExtend()
{
	storage->SaveRequest();
	storage->FrameUpdate();
}

// RSO2 0x80d5fc84
void Exit()
{
	if (soundBufferClear != nullptr) soundBufferClear();   // CWiiSound::BufferClear(console +0xc0)
	if (storage) ExitExtend();
}

// RSO2 0x80d60138
void InitRender()
{
	CDraw& d = *draw;
	d.SetDepthTest(true);   // GX keeps the depth buffer between the batches: sprites (Begin 5) and tiles (Begin 4) are separate ones
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
	rom[0] = kCgromA;
	texSprite = d.AssignTextureAndConvertFromCGen(fmt(16, 16, 0x40, kLayoutSprite), 0x40, rom, 0);
	rom[0] = kCgromC;
	texRadar = d.AssignTextureAndConvertFromCGen(fmt(4, 4, 0x10, kLayoutRadar), 8, rom, 0);
	rom[0] = kCgromConfig;
	texConfig = d.AssignTextureAndConvertFromCGen(fmt(8, 8, 0x10, kLayoutBg), 0x100, rom, 0);

	// the colour PROMs: 3 / 3 / 2 bit resistor weights (0x21 / 0x47 / 0x97). A colour is opaque unless it is black on pen 0 of its palette.
	auto colour = [](u8 b) {
		return ((((b >> 4) & 1) * 0x47 + ((b >> 5) & 1) * 0x97 + ((b >> 3) & 1) * 0x21) * 0x100 & 0xff00u) | (((b >> 7) * 0x97 + ((b >> 6) & 1) * 0x47) & 0xffu) |
		       ((((b >> 1) & 1) * 0x47 + ((b >> 2) & 1) * 0x97 + (b & 1) * 0x21) * 0x10000 & 0xff0000u);
	};
	u32 pal[0x104];
	for (u32 i = 0; i < 0x100; i++)
	{
		u32 c = colour(kPaletRom[kPaletClut[i] & 0xf]);
		if ((i & 3) != 0) c |= 0xff000000u;
		pal[i] = c | (c != 0 ? 0xff000000u : 0);
	}
	for (u32 i = 0; i < 4; i++)
	{
		const u32 c = colour(kPaletRom[0x10 + i]);
		pal[0x100 + i] = c | (c != 0 ? 0xff000000u : 0);
	}
	for (int p = 0; p < 0x40; p++) d.SetPalet(p, 0, &pal[p * 4], 4);
	d.SetPalet(0x40, 0, &pal[0x100], 4);   // the radar dots
	// CDraw +0x141208.. = the config texture, 0xa0, 0x50 (the menu font setup): not ported
	for (int p = 0; p < 0x25; p++)
	{
		u32 c[4];
		for (int k = 0; k < 4; k++)
		{
			const u8* s = kMenuPalet + p * 0x10 + k * 4;
			c[k] = (u32)s[0] << 24 | (u32)s[1] << 16 | (u32)s[2] << 8 | s[3];
		}
		d.SetPalet(p + kMenuPaletOffset, 0, c, 4);
	}
}

// RSO2 0x80d604ec
void CallRender()
{
	CDraw& d = *draw;
	const float kScale = 1.0f;   // RSO2 0x80dc8a64
	d.Begin(5);
	const bool flip = (mem[0xa183] & 1) != 0;   // the screen is flipped (cocktail 2P)
	// the six sprites: attribute / x at mem[0x8014 + 2i], y / palette at mem[0x8814 + 2i]
	for (int i = 0; i < 6; i++)
	{
		const u8 attr = mem[0x8014 + i * 2];
		const u8 sx = mem[0x8015 + i * 2];
		const u8 sy = mem[0x8814 + i * 2];
		const u8 col = mem[0x8815 + i * 2];
		int x, y;
		u32 mirror;
		if (flip)
		{
			mirror = 3;
			x = sx + (col & 0x80) * 2 - 4;
			y = 0xe0 - sy;
		}
		else
		{
			mirror = 0;
			y = 0xe1 - sy;
			x = sx + (col & 0x80) * 2;
		}
		if (x > 0x1f0) x -= 0x200;
		d.DrawChrEx(kScale, kScale, x, y, 5000, texSprite, (attr >> 2) & 0x3f, col & 0x3f, (attr & 1) | ((attr >> 1) & 1) << 1, mirror);
	}
	// the twelve radar dots: x mem[0x8034 + i] (+ bit 0 of mem[0xa004 + i] as the 9th bit), y mem[0x8834 + i], pattern mem[0xa004 + i] bits 1..3
	for (int i = 0; i < 12; i++)
	{
		const u32 t = mem[0xa004 + i];
		int x;
		u32 mirror;
		if (flip)
		{
			mirror = 3;
			x = mem[0x8034 + i] + (int)(~t & 1) * 0x100 - 3;
		}
		else
		{
			mirror = 0;
			x = mem[0x8034 + i] + (int)(~t & 1) * 0x100;
		}
		d.DrawChrEx(kScale, kScale, x, 0xed - mem[0x8834 + i], 2000, texRadar, ((t >> 1) & 7) ^ 7, 0x40, 0, mirror);
	}
	sw.b(0x73) = 0;
	d.End();
	d.Begin(4);
	// the scrolling 32 x 32 playfield (mem[0x8400..] codes, mem[0x8c00..] attributes); the scroll registers mem[0xa130] / mem[0xa140]
	{
		const u32 scrollX = mem[0xa130];
		u32 yy = ~(u32)mem[0xa140];
		for (int row = 0; row < 0x20; row++, yy += 8)
		{
			const u32 ty = (yy - 0xf) & 0xff;
			u32 xx = ~scrollX;
			for (int col = 0; col < 0x20; col++, xx += 8)
			{
				const u8 code = mem[0x8400 + row * 0x20 + col];
				const u8 at = mem[0x8c00 + row * 0x20 + col];
				int x = (int)((xx + 4) & 0xff);
				if (x > 0xf8) x -= 0x100;
				int y = (int)ty;
				if (y > 0xf8) y -= 0x100;
				d.DrawChrEx(kScale, kScale, x, y, (at & 0x20) != 0 ? 4000 : 7000, texBg, code, at & 0x3f, ((at & 0x40) == 0 ? 1 : 0) | ((at >> 7) << 1), 0);
			}
		}
	}
	// the two fixed side columns: tiles 4..7 of each row at x 0xe0.., tiles 0..3 at x 0x100..
	for (int row = 0; row < 0x20; row++)
	{
		for (int k = 0; k < 4; k++)
		{
			const int a = 0x8004 + row * 0x20 + k;
			const u8 at = mem[a + 0x800];
			d.DrawChrEx(kScale, kScale, 0xe0 + k * 8, -0x10 + row * 8, (at & 0x20) != 0 ? 3000 : 6000, texBg, mem[a], at & 0x3f,
			            ((at & 0x40) == 0 ? 1 : 0) | ((at >> 7) << 1), 0);
		}
	}
	for (int row = 0; row < 0x20; row++)
	{
		for (int k = 0; k < 4; k++)
		{
			const int a = 0x8000 + row * 0x20 + k;
			const u8 at = mem[a + 0x800];
			d.DrawChrEx(kScale, kScale, 0x100 + k * 8, -0x10 + row * 8, (at & 0x20) != 0 ? 3000 : 6000, texBg, mem[a], at & 0x3f,
			            ((at & 0x40) == 0 ? 1 : 0) | ((at >> 7) << 1), 0);
		}
	}
	d.End();
	// CallRender_Extend (RSO2 0x80d754d8): CROSSHATCH / SCREENADJUST when sw+0x2d (not ported), then the menu
	gameMenu->MenuRender();
	sw.b(0x6f) = 0;
}

// RSO2 0x80d5fd40: CDraw's render (vtable +8) clears the frame and calls the screen spec's CallRender
void Render()
{
	draw->ClearFrame();
	CallRender();
}

// RSO2 0x80d74e88 .. 0x80d73f88: the extension's start-up (InitExtend)
void InitExtend()
{
	sw.b(0x31) = 0;   // DAT_81203879
	EntryExtfuncExtend();
	copyright[0] = 0;
	MenuConstruct();
	sw.b(0xaf) = (u8)(kLocale << 4 | (sw.b(0xaf) & 0xf));
	storage->LoadRequest();
	storage->FrameUpdate();
	storage->ActivateLoadedData();
	if (std::memcmp(&save[0xdc], "RLYX0001", 8) != 0) ExportData_Init();
	sw.b(0x53) = 1;
	sw.b(0x51) = (u8)(save[0xf7] >> 7);
	// the zoom / aspect / offsets of the screen adjust (CDraw +0x54..0x60): not ported
	CopyrightStringSet();
}

const u8* Need(const RsoModule& m, const char* name, u32 min)
{
	const RsoModule::Export* e = m.Find(name);
	if (e == nullptr || e->size < min)
	{
		std::fprintf(stderr, "gotch::rallyx: the module lacks %s\n", name);
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
		std::fprintf(stderr, "gotch::rallyx: nmwii2.rso not found under the data root\n");
		return false;
	}
	const u8* mainMemory = Need(m, "MainMemory__6RALLYX", 0x10000);
	kHiscoreInit = Need(m, "HISCORE_INITDATA__6RALLYX", 8);
	kAsciiToGame = Need(m, "AsciiCodeToGameCodeConvertTable__6RALLYX", 0x80);
	kSoundTest = Need(m, "SoundTestTable__6RALLYX", 0x54);
	kLeftToDip = Need(m, "LeftToDipSWTable__6RALLYX", 2);
	kMenuPalet = Need(m, "MENUPALET__6RALLYX", 0x250);
	kPaletRom = Need(m, "PALET_ROM__6RALLYX", 0x14);
	kPaletClut = Need(m, "PALET_CLUT__6RALLYX", 0x100);
	kCgromA = Need(m, "CGROM_A__6RALLYX", 0x1000);
	kCgromC = Need(m, "CGROM_C__6RALLYX", 0x100);
	kCgromConfig = Need(m, "CGROM_CONFIG_V", 0x1000);
	const u8* wsgRom = Need(m, "WSGROM__6RALLYX", 0x100);
	kPcmBomb = Need(m, "PCM_BOMB__6RALLYX", kPcmBombSize);
	kLayoutBg = m.At(0x80fc7ec0, 8 * 8 * 8 * 4);
	kLayoutSprite = m.At(0x80fc86c0, 16 * 16 * 8 * 4);
	kLayoutRadar = m.At(0x80fc7cc0, 4 * 4 * 8 * 4);
	if (!mainMemory || !kHiscoreInit || !kAsciiToGame || !kSoundTest || !kLeftToDip || !kMenuPalet || !kPaletRom || !kPaletClut || !kCgromA || !kCgromC ||
	    !kCgromConfig || !wsgRom || !kPcmBomb || !kLayoutBg || !kLayoutSprite || !kLayoutRadar)
		return false;
	sCopyright1 = CString(m, "Copyright1_String__6RALLYX");
	sCopyright2 = CString(m, "Copyright2_String__6RALLYX");
	std::memcpy(mem, mainMemory, sizeof(mem));
	std::memset(io, 0, sizeof(io));
	std::memset(z80io, 0, sizeof(z80io));
	pcmLatch = false;
	soundTestSmoke = false;

	// RSO2 0x80d5f758
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
	// 3.072 MHz: (u64)(2^40 * 3.072f) * 1000 * 1000 (RSO2 0x80dc8a44 * 0x80fb66d8, ints 0x80fb66c8 / 0x80fb66c4)
	z80.clock = (std::uint64_t)(kETimeSecond * 3.072f) * 1000u * 1000u;
	// CreateAudioSpec: 32000 samples per second (RSO2 0x80fb66cc), the same spec as PACMAN
	primary = std::make_unique<PrimaryStream>(32000, 0x2000, 0x10000, 0x10000);
	// CreateScreenSpec: 288 x 224, rotation 0 (RSO2 0x80dc8a54 = 0.0); frame 60 Hz (0x80fb66d0), vblank 0.0025252527 s (0x80fb66d4)
	machine = MACHINE_SPEC{};
	CreateMachineSpecEx(machine, "RALLYX", PeriodFromHz(60.0f), TimeFromSeconds(0.0025252527f), Render, ConvertIO, HardwareInit, VBlankIn, VBlankOut, Exit,
	                    HotResetMachine);
	AddCPU(machine, z80);
	// InitMachine: the services (CDraw + the screen spec's InitRender), then ResetMachine
	draw = std::make_unique<CDraw>(0x120, 0xe0);
	draw->SetAccentTable(m.At(0x80dd27d8, 21 * 12));
	storage = std::make_unique<CDataStorage>();
	save = storage->Data();
	gameio = std::make_unique<CGameIO>(m, &machine.running, &machine.unk114, [] { gotch::HotResetMachine(machine); });
	InitRender();
	ResetMachine(machine);
	storage->magicOffset = 0x28;   // DataStorage +0x34
	storage->sumOffset = 0x2c;     // DataStorage +0x38
	gameio->InitPadData();
	sw = SWork{};
	swork_ColdStart(sw, kLocale);
	wsg = std::make_unique<SndNamcoWsg3>(*primary, wsgRom);
	pcm = std::make_unique<SndPcm>(*primary, 1);
	pcm->SetPCM(0, 0xac44, 0x10000, kPcmBomb, kPcmBombSize, 0);   // 44100 Hz
	gameState = std::make_unique<GameState>();
	InitExtend();
	isSetup = true;
	return true;
}

bool IsSetup() { return isSetup; }

// RSO2 0x80d5fc84 (ExecuteExit -> Exit). The original frees the machine and the host unloads the module: the next Start builds it again
void ExitMachine()
{
	Exit();
	isSetup = false;
}

void StorageFrameUpdate() { if (storage) storage->FrameUpdate(); }
int Execute1V() { return gotch::Execute1V(machine); }
void HotReset() { HotResetMachine(); }
PrimaryStream* Primary() { return primary.get(); }
void SetSoundBufferClear(void (*fn)()) { soundBufferClear = fn; }
void ExitRequest() { gameio->ExitRequest(sw); }
const CDraw& Draw() { return *draw; }
}
}
