#pragma once

#include "common.hpp"

// Per-game museum statistics of the Unity port (Namco Museum Remix). The Wii game keeps its own records inside museum.bin; until those are
// traced the g03 code writes to this in-memory stand-in (nothing is saved).
struct SGameRecordInfo
{
	int m_totalEatGhost{};
	int m_totalEatItem{};
	int m_maxArrivalRound{};
	int m_clearTime[32]{};
	int m_eatCookieByWorld[8]{};
	int m_highScoreTime[8]{};
	void SetHighScoreTime(int idx, int ms) { if (idx >= 0 && idx < 8 && (m_highScoreTime[idx] == 0 || ms < m_highScoreTime[idx])) m_highScoreTime[idx] = ms; }

	void SetClearTime(int round, int ms)
	{
		if (round >= 0 && round < 32 && (m_clearTime[round] == 0 || ms < m_clearTime[round])) m_clearTime[round] = ms;
	}
	SGameRecordInfo* operator->() { return this; }
};
