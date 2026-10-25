#pragma once

#include <list>
#include <memory>
#include <random>
#include <vector>

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g04/cTaskEnemy.hpp"

// Gator Panic Remix (g04) the gator director: kingdom::game::g04::cTaskEnemyManager (0x280 bytes, vtable 0x8044a238; builder 0x800933f4 at
// app +0x288, ctor/Init 0x801ae84c). It owns the four gators (+0x1bc), the 21 patterns of the player count (+0x1d4, cParamEnemyPattern from
// CSV_ENEMY00..20 / CSV_MULTIn_ENEMY00..20, tables 0x8037d0e8..) and the level table (+0x228, cParamEnemyLevel from CSV_ENEMY_LEVEL /
// CSV_MULTIn_LEVEL, 0x8037d0d8). Every 900 frames (15 s) of the play timer it loads a new pattern chosen from the score (0x801af250):
//   score <= L0 (4): pattern rand % P0 (0..3)       score <= L1 (20): P0 + rand % (P1 - P0)
//   not angry and score <= L2 (300), or not angry: P1 + rand % (P2 - P1)
//   angry, first time: the AFTER_EVENT pattern (16) once     angry afterwards: AFTER + 1 + rand % (P3 - AFTER), at most 20
// (L = column 1, P = column 2 of the level rows 0, 1, 2, 4; AFTER = row 3 column 2). rand = libc rand().
// Mode +0x1b4 (0x801af8d4): 0 idle, 1 running, 2 the angry event (every gator state 0x11), 3 game over (state 0x13, angry set).
namespace kingdom::game::g04
{
class cTaskBG;

struct cParamEnemyPattern   // CSV_ENEMYnn (0x5c bytes; parse 0x8019be1c = 0x8019be50 patterns + 0x8019c3a8 commands)
{
	std::vector<sEnemyPattern> patterns;     // +0x20
	std::vector<sEnemyCommand> cmds[4];      // +0x2c (per place A..D)
	bool Load(const char* key);
};

class cTaskEnemyManager : public kingdom::cTaskKingdom
{
public:
	explicit cTaskEnemyManager(cTaskBG* pBG);   // DOL 0x801ae84c
	bool Execute(cJetTask* pParam) override;    // DOL 0x801af048

	void LoadPattern(int idx);                  // DOL 0x801af470
	void SelectLevel(uint score);               // DOL 0x801af250
	void SetAllState(int state);                // DOL 0x801af5d8
	void SetMode(int mode);                     // DOL 0x801af8d4
	void EndAngry();                            // DOL 0x801afed8
	void Reset();                               // DOL 0x801aec9c
	void SetResultLook(int idx);                // DOL 0x801afdec
	void AddRandGroup(u32 mask);                // DOL 0x801af7dc
	uint nFramesLeftInBlock() const { return (m_nFrame - 1u) % 900u; }   // DOL 0x801afeb4
	cTaskEnemy* pEnemy(uint i) { return m_pEnemy[i]; }
	bool bAngry() const { return m_bAngry; }

private:
	void RandGroups();                          // DOL 0x801af634
	void LoadPatterns(int nPlayer);             // DOL 0x801af97c
	void PlaceEnemies();                        // the shared part of 0x801ae84c / 0x801aec9c

	int m_nMode{};                              // +0x1b4
	cTaskBG* m_pBG{};                           // +0x1b8
	cTaskEnemy* m_pEnemy[4]{};                  // +0x1bc
	uint m_nFrame{};                            // +0x1cc (the play timer's frames left)
	std::mt19937 m_Rand;                        // +0x1d0 (cKingdomRand seeded from OSGetTime)
	std::unique_ptr<cParamEnemyPattern> m_pPattern[21];   // +0x1d4
	s32 m_nLevelScore[4]{};                     // +0x228 -> +0x20: column 1 of rows 0, 1, 2, 4
	s32 m_nLevelPattern[4]{};                   //          +0x30: column 2 of rows 0, 1, 2, 4
	s32 m_nAfterEvent{};                        //          +0x40: row 3 column 2
	bool m_bAngry{};                            // +0x234
	bool m_bAfterEvent{};                       // +0x235
	std::list<u32> m_RandGroup;                 // +0x274
};
}
