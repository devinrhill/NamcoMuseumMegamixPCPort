#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

// Galaga (g01) stage select: the record bar of the selected stage (DOL vtable 0x804378e0, 0x1fc bytes; single player only).
// 25 anitas, all ANT_MENU: 0 = frame, 1..23 = digits / marks placed on the layers of pattern 4 of their own layout, 24 = rank.
// Ported from main.dol only.
namespace kingdom::game::g01
{

class cTaskConsMenuBar : public cTaskConsMenuBase
{
protected:
	// animation column of the 5-entry tables (columns 2 and 3 are -1 in every table)
	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_DELETE = 4,
		eANM_MAX = 5
	};

	static const int s_LayTbl[23];            // DOL 0x8036fd90: layer of pattern 4 that anita 1 + i sits on
	static const int s_NumberAnmTbl[10][5];   // DOL 0x8036fe50: digit 0..9
	static const int s_MarkAnmTbl[5];         // DOL 0x8036ff18: anitas 20..23
	static const int s_RankAnmTbl[8][5];      // DOL 0x8036ff30: anita 24, row = +0x1f0
	static const int s_Anita14AnmTbl[5];      // DOL 0x8036ffd0: anita 14
	static const int s_Anita10AnmTbl[5];      // DOL 0x8036ffe4: anita 10
	static const int s_Anita15AnmTbl[5];      // DOL 0x8036fff8: anita 15
	static const int s_FrameAnmTbl[5];        // DOL 0x8036fdec: anita 0

private:
	int m_nWorldNo{-1};   // +0x1d8
	int m_nStageNo{-1};   // +0x1dc
	uint m_u1e0{};        // +0x1e0 record +0x10, 6 digits on anitas 1..6
	uint m_u1e4{};        // +0x1e4 record +0x00, 3 digits on anitas 11..13
	uint m_u1e8{};        // +0x1e8 ScoreNorma +0x00 (nMaxKill1), 3 digits on anitas 7..9
	uint m_u1ec{};        // +0x1ec record +0x08, 4 digits on anitas 16..19 (at least 2 shown)
	int m_n1f0{};         // +0x1f0 record +0x0c, row of s_RankAnmTbl
	bool m_b1f4{};        // +0x1f4 record +0x1c == 1: anita 20 shown
	bool m_b1f5{};        // +0x1f5 record +0x14 == 1: anita 21 shown
	bool m_b1f6{};        // +0x1f6 record +0x18 == 1: anita 22 shown
	bool m_b1f7{};        // +0x1f7 record +0x20 == 1: anita 23 shown
	int m_n1f8{};         // +0x1f8 = 0 in the ctor, not read by this class. UNVERIFIED use

	// DOL 0x800bb328: anims (column nAnm) + visibility of anitas 1..23
	void SetNumberAnm(int nAnm);

public:
	// DOL 0x800bb090
	cTaskConsMenuBar(cTaskConsMenuManBase* const& pConsMan);

	// DOL 0x800bb2d0 (thunk 0x800bbf30)
	virtual ~cTaskConsMenuBar();

	// DOL 0x800bbdc0: copy the record of (world, stage) when it changed (the manager calls it with man+0x218, man+0x210)
	void SetStage(int nWorldNo, int nStageNo);

	// DOL 0x800bbedc: write the bar's numbers back into the current stage's record (called by 0x800be370)
	void SetCurrentStageRecord();

protected:
	// DOL 0x800bbad4
	void WaitExec() override;

	// DOL 0x800bbb8c
	bool AppearExec() override;

	// DOL 0x800bbd28
	bool DeleteExec() override;
};

}
