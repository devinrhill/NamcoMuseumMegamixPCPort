#pragma once

#include "common.hpp"
#include <string>

// Grobda Remix (g11) game system: kingdom::game::g11::cG11GrobdaSys (0x28 bytes, r13 - 0x57d8 = KingdomGlobal::GrobdaSys), made by job 336
// (FUN_8000f0c8: new, ctor FUN_8000e944 -> Init FUN_8000e9e0, then the menu job 338) and deleted by job 351 (FUN_8000f10c, back to the hub).
// Ported from main.dol (no C# exists for Grobda); only what the stage needs so far.
namespace kingdom::game::g11
{
class cG11GrobdaSys
{
public:
	cG11GrobdaSys();                 // DOL 0x8000e944 / Init 0x8000e9e0 (partial)
	~cG11GrobdaSys();
	static cG11GrobdaSys* Instance();   // KingdomGlobal::GrobdaSys (r13 - 0x57d8)
	static void JobInit();           // DOL 0x8000f0c8 (job 336)
	static void JobEnd();            // DOL 0x8000f10c (job 351)

	int nStage() const { return m_nStage; }   // +0x20: index into the stage archive table
	void nStage(int n) { m_nStage = n; }
	int nPlayer() const { return m_nPlayer; }  // **(+4): the first word of the matching block (1..4, set by the debug menu's MODE row)
	void nPlayer(int n) { m_nPlayer = n; }
	int nPad(int player) const { return m_nPad[player]; }       // the matching block's player +0x4c (nCtrl); debug starts = the player's index
	void nPad(int player, int pad) { m_nPad[player] = pad; }
	static const char* StageArc(int n);        // DOL 0x8000ed64: table 0x80476828

	// the score records (DOL +8 best, +0xc + 4 i per player; save block 5 = {be32 1, be32 best}, manager +0x24)
	int nBest() const { return m_nBest; }                        // *(+8)
	void ResetScores();                                          // DOL 0x8000ee28 (per-player records to 0)
	void RecordSingle(int score);                                // DOL 0x8000edbc (best = max, then save: 0x8000ee94 / 0x8000eea0)
	void RecordMulti(int player, int score) { m_nScore[player] = score; }   // DOL 0x8000ee10
	int nScore(int player) const { return m_nScore[player]; }     // DOL cTeam::GetPlayer: the per-player record (+0xc + 4 i)
	// the players' names from the matching data (the DOL copies the whole 0x154 block to sys +4; the port keeps what the menus draw)
	std::string szName(int player) const { return m_szName[player].empty() ? "PLAYER" + std::to_string(player + 1) : m_szName[player]; }
	void szName(int player, const std::string& n) { m_szName[player] = n; }
	bool IsMulti() const { return m_nPlayer != 1; }              // DOL 0x8000ed74 (the matching block's player count != 1)

private:
	void LoadRecord();                                           // DOL 0x8000eee4
	void SaveRecord();                                           // DOL 0x8000eea0
	int m_nBest{};
	int m_nScore[4]{};
	int m_n1c{};                     // +0x1c
	int m_nStage{};                  // +0x20
	int m_nPlayer{1};
	int m_nPad[4]{0, 1, 2, 3};
	std::string m_szName[4];
};
}
