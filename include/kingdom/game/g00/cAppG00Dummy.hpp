#pragma once

#include "common.hpp"
#include "kingdom/cAppKingdom.hpp"

// g00 "dummy game": the developers' placeholder game, drawn entirely with the engine's debug text (0x8013be34). Ported from main.dol (no C#).
// Jobs (table at 0x80389868 order, docs/JOBS.md):
//   160 (0x0a000000, FUN_800c6fc0): new cG00DummySys (1 byte, empty ctor 0x800c6fb8, r13 - 0x56f8 = DAT_80479028), then job 161
//   161 (0x0a100000): no init function in the DOL's job table (nothing runs)
//   162 (0x0a200000, FUN_800c706c / term 0x800c718c): cAppG00DummyMenu
//   163 (0x0a300000, FUN_800c7194 / term 0x800c72b4): cAppG00DummyGame
//   175 (0x0af00000, FUN_800c7010): delete the sys, back to the debug menu (0x08300000)
// The retail debug top menu (0x8000e1f8) has no g00 entry.
namespace kingdom::game::g00
{
class cG00DummySys
{
public:
	static void JobInit();   // DOL 0x800c6fc0 (job 160)
	static void JobEnd();    // DOL 0x800c7010 (job 175)
};

// kingdom::game::g00::cAppG00DummyMenu (0x298 bytes, vtable 0x80449028; Init 0x801915a0, Execute 0x801915b4, Draw 0x80191844, dtor 0x80191b80).
// States +0x284: 0 "FADE IN" (31 frames) -> 1 message pages "[Surprise!]" / "[Halo]" (A next page, B skip, 2 back) -> 2 "NORM SELECT" /
// "1P TEST 1" (A -> 3, 2 -> 5) -> 3 "Do You Begin?" (A or B -> 4, 2 -> 2) -> 4 "FADE OUT" / "GO TO GAME" (31 frames, then job 0x0a200000 =
// this menu again, as in the DOL) ; 5 "FADE OUT" (31 frames, then job 0x0af00000). A "+" follows the pointer.
class cAppG00DummyMenu : public kingdom::cAppKingdom
{
public:
	static void JobInit();   // DOL 0x800c706c (job 162)
	cAppG00DummyMenu();      // DOL 0x801915a0
	bool Execute(cJetTask* pParam) override;            // DOL 0x801915b4
	void Draw(int nStatus, cJetTask* pParam) override;  // DOL 0x80191844

private:
	int m_nState{};   // +0x284
	int m_nPage{};    // +0x288
	int m_nCount{};   // +0x28c
	int m_nPtrX{};    // +0x290 the pointer in screen pixels
	int m_nPtrY{};    // +0x294
};

// kingdom::game::g00::cAppG00DummyGame (0x528 bytes, vtable 0x80438710; Init 0x800c72bc, Execute 0x800c7320, Draw 0x800c7480, dtor 0x800c7d50).
// A shooting toy: 16 targets "\(^o^)/" fly across the screen (spawn 0x800c76e0, move 0x800c7944), B shoots at the pointer (A held + B repeat
// too; 0x800c7ba4: a "\ /" burst, +10 points per target whose 96 x 16 box holds the pointer), hit targets blink "/(^o^)\" for 40 frames.
// The game ends after 1800 frames, 1000 points, 1000 shots or when 1 is held; then a banner and job 0x0af00000.
class cAppG00DummyGame : public kingdom::cAppKingdom
{
public:
	static void JobInit();   // DOL 0x800c7194 (job 163)
	cAppG00DummyGame();      // DOL 0x800c72bc
	bool Execute(cJetTask* pParam) override;            // DOL 0x800c7320
	void Draw(int nStatus, cJetTask* pParam) override;  // DOL 0x800c7480

private:
	struct Target { int nState; float x, y, vx, vy; int nTimer; };   // +0x2a8, 0x18 each
	struct Burst { int bActive; int x, y, nCount; };                 // +0x428, 0x10 each
	void Spawn();       // DOL 0x800c76e0
	void Move();        // DOL 0x800c7944
	void Shoot();       // DOL 0x800c7ba4
	void DrawTargets(); // DOL 0x800c7a6c

	int m_nState{};     // +0x284
	int m_n288{};       // +0x288
	int m_nCount{};     // +0x28c
	int m_nPtrX{};      // +0x290
	int m_nPtrY{};      // +0x294
	int m_nScore{};     // +0x298
	int m_nTime{};      // +0x29c
	int m_nBullet{};    // +0x2a0
	int m_nSpawn{};     // +0x2a4
	Target m_Target[16]{};   // +0x2a8
	Burst m_Burst[16]{};     // +0x428
};
}
