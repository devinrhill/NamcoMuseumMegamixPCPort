#pragma once

#include "common.hpp"

// Rally-X Remix (g02) save record + the sys members the stage select needs, ported from main.dol (no C# exists for Rally-X).
//
// cG02RallyXRecord = the global save block 1 of the DOL (DAT_804844a0, 0x250 bytes, big-endian on the Wii): loaded by the sys ctor
// (0x800c54e0: LoadGameData(1, ...); a block that is empty or not version 0x20070824 is reset by 0x800c6024 from CSV_RECORD), saved by
// 0x800c5ff0. Layout: +0 valid, +4 version, +8 "all clear" flag, +0xc + 0x14 * i (i = stage index 0..28) {+0 open, +4 best time (ms),
// +8 best score, +0xc new score flag, +0x10 new time flag}.
//
// NATIVE PLACEMENT (needs from integrator): the DOL keeps this block and the accessors below as plain globals / cG02RallyXSys methods; the
// port's cG02RallyXSys does not have them yet, so the stage select keeps them here. The integrator may move them into cG02RallyXSys.
namespace kingdom::game::g02
{

class cG02RallyXRecord
{
public:
	struct sStage
	{
		u32 nOpen{};        // +0x00 (0x804844ac + 0x14 i)
		u32 nTime{};        // +0x04 best time in ms
		u32 nScore{};       // +0x08 best score
		u32 nNewScore{};    // +0x0c 1 = the score shown as new (0x800c62d8)
		u32 nNewTime{};     // +0x10 1 = the time shown as new (0x800c62fc)
	};
	enum { STAGE_MAX = 29 };

	static cG02RallyXRecord& Get();   // the block; loaded on first use (native: the DOL loads it in the sys ctor 0x800c54e0)

	void Load();                      // DOL 0x800c54e0 (its save part): LoadGameData(1), reset when empty / old
	void Save() const;                // DOL 0x800c5ff0: SaveGameData(1, block, 0x250)
	void Reset();                     // DOL 0x800c6024: defaults from CSV_RECORD (stages 1..24), stage 1 and 25 open

	bool IsAllClear() const { return m_nAllClear != 0; }                         // DOL 0x800c61d8
	bool IsOpen(int idx) const { return Ok(idx) && m_Stage[idx].nOpen != 0; }    // DOL 0x800c61f4
	u32 GetScore(int idx) const { return Ok(idx) ? m_Stage[idx].nScore : 0u; }  // DOL 0x800c62a8
	u32 GetTime(int idx) const { return Ok(idx) ? m_Stage[idx].nTime : 0u; }    // DOL 0x800c62c0
	bool IsNewScore(int idx) const { return Ok(idx) && m_Stage[idx].nNewScore == 1u; }   // DOL 0x800c62d8
	bool IsNewTime(int idx) const { return Ok(idx) && m_Stage[idx].nNewTime == 1u; }     // DOL 0x800c62fc
	int GetFreeStage() const;         // DOL 0x800c6320: the newest open stage (boss stages are skipped)
	void SetOpen(int idx);            // DOL 0x800c63f0
	void SetAllClear();               // DOL 0x800c6410
	bool SetRecord(int idx, u32 score, u32 time);   // DOL 0x800c6218: keeps the better score / time, marks them new
	bool SetTime(int idx, u32 time);  // DOL 0x800c6274

	sStage& Stage(int idx) { return m_Stage[Ok(idx) ? idx : 0]; }

private:
	static bool Ok(int idx) { return idx >= 0 && idx < STAGE_MAX; }

	bool m_bLoaded{};
	u32 m_nValid{};        // +0x00 (DAT_804844a0)
	u32 m_nVersion{};      // +0x04 (DAT_804844a4) 0x20070824
	u32 m_nAllClear{};     // +0x08 (DAT_804844a8)
	sStage m_Stage[STAGE_MAX];   // +0x0c
};

// Members of cG02RallyXSys (r13 - 0x5700) the stage select reads / writes that the port's sys has no accessor for yet (needs from integrator:
// accessors for +0x0c, +0x14 + 0x14 i, +0x1c + 0x14 i, +0x1b8). NATIVE STAND-IN storage until then; Reset() mirrors what the sys ctor
// (0x800c54e0) sets them to and is called by job 192 / 195 (platform/boot.cpp) whenever a new sys is made.
struct cG02RallyXSysExt
{
	static int nPlayer;            // +0x0c number of players (ctor: 1; 0x800c5c60 sets it; 0x800c5c68 reads it)
	static int nCtrlType[4];       // +0x14 + 0x14 i: the control type of player i (ctor: 2; 1 = remote only, 2 = Nunchuk; 0x800c5b78 / 0x800c5b88)
	static int nPadNo[4];          // +0x1c + 0x14 i: the pad of player i (ctor: i; 0x800c5b98)
	static bool bNewStageApear;    // +0x1b8 a new stage was opened (ctor: 0; 0x800c5c70 / 0x800c5c78)
	static void Reset();           // the values the sys ctor 0x800c54e0 sets
	static void AttachMenuArc();   // DOL 0x800c5b24: g02_rallyx/g02menu.lzs
	static void DetachMenuArc();   // DOL 0x800c5b50

	// the sys getters that take an explicit stage index (the DOL passes a pointer to it; null = the sys' own +0x08). cG02RallyXSys has only
	// the "own stage" forms, so the menu uses these.
	static int WorldOf(int idx);          // DOL 0x800c5788: 1..6 -> 1, 7..12 -> 2, 13..18 -> 3, 19..24 -> 4, 25..28 -> 0xb, else 0
	static int StageInWorldOf(int idx);   // DOL 0x800c5808: (idx - 1) % 6 + 1, 0 for 0
	static bool IsBossStageOf(int idx);   // DOL 0x800c5bf8: StageInWorldOf(idx) == 6
	static int StageIdxOf(int world, int stage);   // DOL 0x800c5858 (it stores the result into sys +0x08): world 0 -> 0, 0xb -> 5
};

// cParamDefaultRecord (DOL vtable 0x804385d0, a cJetCSV of CSV_RECORD kept at DAT_80479018 by the sys ctor; its only own function is the
// dtor 0x800c56f0): natively cG02RallyXRecord::Reset reads CSV_RECORD through a local cKingdomCSV.

}
