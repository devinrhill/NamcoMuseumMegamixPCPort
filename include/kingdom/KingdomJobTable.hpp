#pragma once

#include "common.hpp"

// The master job table (init functions written FUN_ so tools/port_map.py does not count them as ported): every job number (table index << 20) of main.dol's job table (0x804400c8, 86 of 432 slots populated).
// Names are the developers' own (PRJ_JOBNO_* in the Unity port's Constants.cs, prefixes dropped: KINGDOM_G03_PACN_ROLL_MENU ->
// PACNROLL_MENU). Where the DOL and Constants.cs disagree the DOL wins (g00). Grobda (g11) is not in Constants.cs: its names follow the
// other games' slot pattern and are UNVERIFIED. docs/JOBS.md describes what each job does in the port.
namespace kingdom
{
namespace job
{
constexpr uint MASK = 0xfffu;   // PRJ_JOBTBL_MASK
constexpr uint SHIFT = 20u;     // PRJ_JOBTBL_SHIFT
constexpr uint COUNT = 432u;    // PRJ_JOBTBL_nTBL

constexpr uint Index(uint jobNo) { return jobNo >> SHIFT; }
constexpr uint FromIndex(uint index) { return index << SHIFT; }

constexpr uint DEBUG_MENU_ROOT = 0x00000000u;       //   0  FUN_800c25c0  PRJ_JOBNO_DEBUGMENU (the developers' project menu)
// the developers' lab menus the root menu lists (no init in the retail job table: choosing one leaves an empty task tree, as on the Wii)
constexpr uint MH_DEBUG_MENU = 0x01000000u;         //  16  PRJ_JOBNO_MH_DEBUG_MENU (HOSONO LABO)
constexpr uint SM_DEBUG_MENU = 0x02000000u;         //  32  PRJ_JOBNO_SM_DEBUG_MENU (MIURA LABO)
constexpr uint YM_DEBUG_MENU = 0x03000000u;         //  48  PRJ_JOBNO_YM_DEBUG_MENU (MOURI LABO)
constexpr uint KZ_DEBUG_MENU = 0x04000000u;         //  64  PRJ_JOBNO_KZ_DEBUG_MENU (KANAZAWA LABO)
constexpr uint GT_DEBUG_MENU = 0x05000000u;         //  80  PRJ_JOBNO_GT_DEBUG_MENU (GOTOU LABO)
constexpr uint JM_DEBUG_MENU = 0x06000000u;         //  96  PRJ_JOBNO_JM_DEBUG_MENU (MATSUSHITA LOBO)

// Kingdom (the collection itself)
constexpr uint KINGDOM_INIT = 0x08000000u;          // 128  FUN_8000e3bc  creates cKingdomSys, -> MASTER
constexpr uint KINGDOM_TERM = 0x08100000u;          // 129  FUN_8000e3e8  Jbc_SetProcNo(0)
constexpr uint MASTER = 0x08200000u;                // 130  FUN_8000e414  -> BOOT
constexpr uint DEBUG_MENU = 0x08300000u;            // 131  FUN_8000e1f8  cAppDebugTopMenu
constexpr uint BOOT = 0x08400000u;                  // 132  FUN_8009f7a0  cAppBoot
constexpr uint LOAD = 0x08500000u;                  // 133  FUN_8000e420  cAppSaveLoad(true): job-change transition with fade
constexpr uint SAVE = 0x08600000u;                  // 134  FUN_8000e5dc  cAppSaveLoad(false): job-change transition without fade
constexpr uint ATTRACT = 0x08800000u;               // 136  FUN_8019a864  cAppAttract
constexpr uint TITLE = 0x08900000u;                 // 137  FUN_8019a98c  cAppTitle
constexpr uint MENU = 0x08a00000u;                  // 138  FUN_8019ab28  the hub (g03::cAppMenu, castle garden)
constexpr uint STAFFROLL = 0x08b00000u;             // 139  FUN_8019b250  the collection's staff roll (cAppG10StaffRoll)
constexpr uint MENU_CLASSIC = 0x08c00000u;          // 140  FUN_8019aee4  classic menu (the arcade hall, g03::cAppMenu)
constexpr uint RSO_INIT = 0x08d00000u;              // 141  FUN_800c20c0
constexpr uint RSO_EXEC = 0x08e00000u;              // 142  FUN_800c2108
constexpr uint RSO_TERM = 0x08f00000u;              // 143  FUN_800c2230
constexpr uint DEBUG_LOAD = 0x09000000u;            // 144  FUN_8000e79c
constexpr uint DEBUG_SAVE = 0x09100000u;            // 145  FUN_8000e7d0
constexpr uint DEBUG_DELETE = 0x09200000u;          // 146  FUN_8000e81c
constexpr uint DEBUG_WII_MENU = 0x09300000u;        // 147  FUN_8000e814  empty
constexpr uint RSO_DEBUG_MENU = 0x0ae00000u;        // 174  FUN_800c1f98  cAppRSODebugMenu

// g00 dummy game. Constants.cs: 161 DUMMY_MENU, 162 DUMMY_GAME; the DOL has the menu at 162 and the game at 163.
constexpr uint DUMMY_INIT = 0x0a000000u;            // 160  FUN_800c6fc0  -> DUMMY_161
constexpr uint DUMMY_161 = 0x0a100000u;             // 161  (no init in the DOL)
constexpr uint DUMMY_MENU = 0x0a200000u;            // 162  FUN_800c706c  cAppG00DummyMenu
constexpr uint DUMMY_GAME = 0x0a300000u;            // 163  FUN_800c7194  cAppG00DummyGame
constexpr uint DUMMY_TERM = 0x0af00000u;            // 175  FUN_800c7010

// g01 Galaga
constexpr uint GALAGA_INIT = 0x0b000000u;           // 176  FUN_800b0cf4
constexpr uint GALAGA_DEBUG_MENU = 0x0b100000u;     // 177  FUN_800b0c8c
constexpr uint GALAGA_MENU = 0x0b200000u;           // 178  FUN_800b0d98
constexpr uint GALAGA_GAME = 0x0b300000u;           // 179  FUN_800b0ed4
constexpr uint GALAGA_DEBUG_SELECT = 0x0b400000u;   // 180  FUN_800b0ce8  empty
constexpr uint GALAGA_GAMEOVER = 0x0b500000u;       // 181  FUN_800b1250
constexpr uint GALAGA_MATCHING = 0x0b600000u;       // 182  FUN_800b13d0
constexpr uint GALAGA_STAFFROLL = 0x0b700000u;      // 183  FUN_800b150c
constexpr uint GALAGA_NUNCHUK = 0x0b800000u;        // 184  FUN_800b1658
constexpr uint GALAGA_SCREENSHOOT = 0x0bd00000u;    // 189  (no DOL entry)
constexpr uint GALAGA_SETTING = 0x0be00000u;        // 190  (no DOL entry)
constexpr uint GALAGA_TERM = 0x0bf00000u;           // 191  FUN_800b0d40

// g02 Rally-X
constexpr uint RALLYX_INIT = 0x0c000000u;           // 192  FUN_800c6444
constexpr uint RALLYX_DEBUG_MENU = 0x0c100000u;     // 193  FUN_800c6430  empty in the DOL (the port has a native menu)
constexpr uint RALLYX_MENU = 0x0c200000u;           // 194  FUN_800c6504
constexpr uint RALLYX_GAME = 0x0c300000u;           // 195  FUN_800c677c
constexpr uint RALLYX_DEBUG_SELECT = 0x0c400000u;   // 196  FUN_800c6438  empty
constexpr uint RALLYX_TITLE = 0x0c500000u;          // 197  (no DOL entry)
constexpr uint RALLYX_GAMEOVER = 0x0c600000u;       // 198  FUN_800c68c4
constexpr uint RALLYX_MATCHING = 0x0c700000u;       // 199  FUN_800c6a44
constexpr uint RALLYX_MULTI_MENU = 0x0c800000u;     // 200  FUN_800c6640
constexpr uint RALLYX_NUNCHUK = 0x0c900000u;        // 201  FUN_800c6ccc
constexpr uint RALLYX_STAFFROLL = 0x0ce00000u;      // 206  FUN_800c6b80
constexpr uint RALLYX_TERM = 0x0cf00000u;           // 207  FUN_800c64a8

// g03 Pac-N-Roll
constexpr uint PACNROLL_INIT = 0x0d000000u;         // 208  FUN_8012b18c
constexpr uint PACNROLL_DEBUG_MENU = 0x0d100000u;   // 209  FUN_8012b008
constexpr uint PACNROLL_MENU = 0x0d200000u;         // 210  FUN_8012b378
constexpr uint PACNROLL_GAME = 0x0d300000u;         // 211  FUN_8012b4b4
constexpr uint PACNROLL_DEBUG_SELECT = 0x0d400000u; // 212  FUN_8012b174  -> PACNROLL_GAME
constexpr uint PACNROLL_GAMEOVER = 0x0d500000u;     // 213  FUN_8012b698
constexpr uint PACNROLL_EDIT = 0x0d600000u;         // 214  FUN_8012b174  -> PACNROLL_GAME
constexpr uint PACNROLL_EDIT2 = 0x0d700000u;        // 215  FUN_8012b174  -> PACNROLL_GAME
constexpr uint PACNROLL_NUNCHUK = 0x0d800000u;      // 216  FUN_8012b964
constexpr uint PACNROLL_EDIT_ENEMY = 0x0d900000u;   // 217  FUN_8012b174  -> PACNROLL_GAME (name from the debug menu row; not in Constants.cs)
constexpr uint PACNROLL_STAFFROLL = 0x0de00000u;    // 222  FUN_8012b818
constexpr uint PACNROLL_TERM = 0x0df00000u;         // 223  FUN_8012b21c

// g04 Gator Panic (WaniWani)
constexpr uint WANIWANI_INIT = 0x0e000000u;         // 224  FUN_8019a07c
constexpr uint WANIWANI_DEBUG_MENU = 0x0e100000u;   // 225  FUN_80199f64
constexpr uint WANIWANI_GAME = 0x0e300000u;         // 227  FUN_8019a13c
constexpr uint WANIWANI_MATCHING = 0x0e500000u;     // 229  FUN_8019a470
constexpr uint WANIWANI_GAME_MULTI = 0x0e600000u;   // 230  FUN_8019a2a0
constexpr uint WANIWANI_NUNCHUK = 0x0e700000u;      // 231  FUN_8019a5ac
constexpr uint WANIWANI_TERM = 0x0ef00000u;         // 239  FUN_8019a0e0

// g07 Pac-Motos
constexpr uint MOTOS_INIT = 0x11000000u;            // 272  FUN_800a2b40  -> MOTOS_MENU
constexpr uint MOTOS_DEBUG_MENU = 0x11100000u;      // 273  FUN_800a29bc
constexpr uint MOTOS_MENU = 0x11200000u;            // 274  FUN_800a2ba0
constexpr uint MOTOS_GAME = 0x11300000u;            // 275  FUN_800a2e18
constexpr uint MOTOS_DEBUG_SELECT = 0x11400000u;    // 276  FUN_800a2b28  -> MOTOS_GAME
constexpr uint MOTOS_GAMEOVER = 0x11500000u;        // 277  FUN_800a2f9c
constexpr uint MOTOS_MATCHING = 0x11600000u;        // 278  FUN_800a3268
constexpr uint MOTOS_MULTI_MENU = 0x11700000u;      // 279  FUN_800a2cdc
constexpr uint MOTOS_NUNCHUK = 0x11800000u;         // 280  FUN_800a33a4
constexpr uint MOTOS_STAFFROLL = 0x11e00000u;       // 286  FUN_800a311c  (Constants.cs spells it KINGODM_...)
constexpr uint MOTOS_TERM = 0x11f00000u;            // 287  FUN_800a2b70

// g11 Grobda (names UNVERIFIED: not in Constants.cs)
constexpr uint GROBDA_INIT = 0x15000000u;           // 336  FUN_8000f0c8
constexpr uint GROBDA_DEBUG_MENU = 0x15100000u;     // 337  FUN_8000ef44
constexpr uint GROBDA_MENU = 0x15200000u;           // 338  FUN_8000f164
constexpr uint GROBDA_GAME = 0x15300000u;           // 339  FUN_8000f338
constexpr uint GROBDA_DEBUG_SELECT = 0x15400000u;   // 340  FUN_8000f0b0  empty (the debug menu's MODE row)
constexpr uint GROBDA_GAMEOVER = 0x15500000u;       // 341  FUN_8000f380
constexpr uint GROBDA_MATCHING = 0x15600000u;       // 342  FUN_8000fa64
constexpr uint GROBDA_NUNCHUK = 0x15800000u;        // 344  FUN_8000f804
constexpr uint GROBDA_STAFFROLL = 0x15e00000u;      // 350  FUN_8000f618
constexpr uint GROBDA_TERM = 0x15f00000u;           // 351  FUN_8000f10c
}
}
