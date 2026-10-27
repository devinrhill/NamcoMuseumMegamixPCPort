#pragma once

#include "common.hpp"

// Gator Panic Remix (g04, "WaniWani") game system: kingdom::game::g04::cG04WaniWaniSys (0x168 bytes, r13 - 0x5618 = KingdomGlobal::WaniWaniSys),
// made by job 224 (FUN_8019a07c: new, ctor 0x80199a7c, then the Nunchuk card job 231) and deleted by job 239 (FUN_8019a0e0, back to the hub).
// Ported from main.dol (no C# exists for Gator Panic).
//   +0x000  1
//   +0x004  the matching block (0x154 bytes, the same layout as Galaga's sGalagaMatching: player count, then 4 x 0x54 player records)
//   +0x158  the high score (game data block 3, 0x80199dfc)
//   +0x15c  the play mode: 1 = alone (0x80091adc "is single" = +0x15c == 1)
//   +0x160 / +0x164 / +0x165  0
namespace kingdom::game::g04
{
struct sWaniPlayer
{
	char szName[0x44];  // +0x00 ("test", DAT_8037c188, until the matching menu fills it)
	s32 n44;            // +0x44 = 6
	s32 nIndex;         // +0x48 = player index
	s32 nWork;          // +0x4c = player index (the player's pad)
	s32 n50;            // +0x50
};
struct sWaniMatching
{
	s32 nPlayer;                // +0x00
	sWaniPlayer player[4];      // +0x04
};
static_assert(sizeof(sWaniPlayer) == 0x54 && sizeof(sWaniMatching) == 0x154);

class cG04WaniWaniSys
{
public:
	cG04WaniWaniSys();                 // DOL 0x80199a7c
	~cG04WaniWaniSys();                // DOL 0x80199b64
	static void JobInit();             // DOL 0x8019a07c (job 224)
	static void JobEnd();              // DOL 0x8019a0e0 (job 239)

	void LoadRecord();                 // DOL 0x80199dfc: the high score from game data block 3 ({flag, version 0x20070723, score})
	bool IsSingle() const { return m_nMode == 1; }        // DOL 0x80091adc
	int& nTimeIndex() { return m_n00; }   // +0x000: the multiplayer play time index (0 / 1 / 2 = 60 / 180 / 300 s, default 1)
	int nPlayers() const { return m_nMode; }              // +0x15c: the player count of the game (1 = alone)
	void SetPlayers(int n) { m_nMode = n; }
	int nPlayer() const { return m_Matching.nPlayer; }
	int nWork(int player) const { return m_Matching.player[player].nWork; }
	sWaniMatching* pMatchingData() { return &m_Matching; }
	int nHighScore() const { return m_nHighScore; }
	void SetHighScore(int n) { if ((u32)m_nHighScore < (u32)n) m_nHighScore = n; }   // DOL 0x80199ed4 (+0x158 = max)
	void SaveRecord();                 // DOL 0x80199db4 (game data block 3)
	int& nWinMask() { return m_n160; }   // +0x160: the winners (bit per player) of the last multiplayer round
	bool& bRetry() { return m_b164; }   // +0x164: "Play again? yes" (the game job starts at the READY, not the menu)
	bool& bNewRecord() { return m_b165; }   // +0x165: the high score was beaten (the result plays m00se_06Hiscore)

private:
	s32 m_n00{1};                      // +0x000
	sWaniMatching m_Matching{};        // +0x004
	s32 m_nHighScore{};                // +0x158
	s32 m_nMode{1};                    // +0x15c
	s32 m_n160{};                      // +0x160
	bool m_b164{};                     // +0x164
	bool m_b165{};                     // +0x165
};
}
