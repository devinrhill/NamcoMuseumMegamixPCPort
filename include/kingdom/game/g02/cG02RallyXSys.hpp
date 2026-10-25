#pragma once

#include "common.hpp"

// Rally-X Remix (g02) game system (DOL: an unnamed 0x1bc-byte object at r13 - 0x5700, KingdomGlobal::RallyXSys), made by job 192
// (FUN_800c6444: new, memset +0x64 0x154, ctor FUN_800c54e0, then job 194) and deleted by job 207 (FUN_800c64a8: dtor FUN_800c5690, back to the
// hub 0x08a00000). Ported from main.dol (no C# exists for Rally-X); only what the stage needs so far.
namespace kingdom::game::g02
{
class cG02RallyXSys
{
public:
	cG02RallyXSys();                 // DOL 0x800c54e0
	~cG02RallyXSys();                // DOL 0x800c5690
	static void JobInit();           // DOL 0x800c6444 (job 192)
	static void JobEnd();            // DOL 0x800c64a8 (job 207)

	int nStageIdx() const { return m_nStageIdx; }   // +0x08 (1..30; 0 = the multiplayer stage g02wxxsxx)
	void nStageIdx(int n) { m_nStageIdx = n; }
	bool IsBossStage() const;        // DOL 0x800c5ba8: the 6th stage of a world
	int nWorld() const;              // DOL 0x800c5788: 1..4, 0xb = the multiplayer stages 25..28, 0 otherwise
	int nStageInWorld() const;       // DOL 0x800c5808: 1..6 (0 for stage index 0)
	bool IsMulti() const { return nWorld() == 0xb; }   // DOL 0x800c5ca0
	void AttachStageArc() const;     // DOL 0x800c5a1c: g02game.lzs + the stage archive (table 0x803710e0)
	void DetachStageArc() const;

	static const char* const STAGE_ARC[31];   // DOL 0x803710e0

private:
	s32 m_n00{2};                    // +0x00
	s32 m_n04{1};                    // +0x04
	s32 m_nStageIdx{1};              // +0x08
	s32 m_n0c{1};                    // +0x0c
	s32 m_n10{};                     // +0x10
	s32 m_nPlayerWork[4][5]{};       // +0x14: per player {2, 6, index, 6, rand % 6} (FUN_800c54e0; meanings not traced)
	u8 m_Matching[0x154]{};          // +0x64 (cleared by the job init)
	bool m_b1b8{};                   // +0x1b8
};
}
