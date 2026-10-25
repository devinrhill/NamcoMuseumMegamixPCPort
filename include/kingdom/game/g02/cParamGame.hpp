#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"

// Rally-X Remix (g02) game parameters: kingdom::game::g02::cParamGame (0x6c bytes, a cJetCSV with its vtable at +0x1c: 0x804412c0), made by the
// app ctor (cAppG02RallyXGame +0x280) from CSV_GAME (0x84635999, g02game.lzs). Ctor DOL 0x80123fbc reads column 1 by ROW INDEX; the comma-only
// lines of the file are not rows here. The waits are seconds in the file, frames here (* FLOAT_8047bff0 = 60).
//   row 0 eCSV_GAME_PLAYER_LIFE (S32)    rows 1, 3..10 the waits (row 2 eCSV_GAME_CLEAR_WAIT is never read)
//   rows 11..14 eCSV_PLAYERn_FONT_COLOR, 15..18 eCSV_PLAYERn_FONT_COLOR_SHADOW: columns 1..4 = R, G, B, A -> 0xRRGGBBAA
namespace kingdom::game::g02
{
class cParamGame : public kingdom::cKingdomCSV
{
public:
	cParamGame();                    // DOL 0x80123fbc

	s32 m_nLife{};                   // +0x20 row 0  PLAYER_LIFE
	float m_fMissWait{};             // +0x24 row 1  MISS_WAIT
	float m_fClearLastWait{};        // +0x28 row 3  CLEAR_LAST_WAIT
	float m_fWinnerLastWait{};       // +0x2c row 4  WINNER_LAST_WAIT
	float m_fDrawLastWait{};         // +0x30 row 5  DRAW_LAST_WAIT
	float m_fLoseLastWait{};         // +0x34 row 6  LOSE_LAST_WAIT
	float m_fClearFlagWait{};        // +0x38 row 7  CLEAR_FLAG_WAIT
	float m_fWinnerFlagWait{};       // +0x3c row 8  WINNER_FLAG_WAIT
	float m_fDrawFlagWait{};         // +0x40 row 9  DRAW_FLAG_WAIT
	float m_fLoseFlagWait{};         // +0x44 row 10 LOSE_FLAG_WAIT
	u32 m_nFontColor[4]{};           // +0x48 rows 11..14
	u32 m_nFontShadow[4]{};          // +0x58 rows 15..18
	bool m_bLoaded{};                // +0x68
};
}
