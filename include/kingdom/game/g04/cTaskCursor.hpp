#pragma once

#include "common.hpp"
#include "kingdom/cTaskAnita.hpp"

// Gator Panic Remix (g04) the marker over the hole a player aims at: kingdom::game::g04::cTaskCursor (0x1e0 bytes, vtable 0x80449dd0, a
// cTaskAnita; ctor 0x801a7dd8 with the player, child of the player's Pac-Man, priority 0x80000007). POINTER_01_ANITA, animation
// 0x8037cdc8[player * 2] (aiming) / [player * 2 + 1] (lunging); positions from CSV_GAME_LAYOUT (cParamGameLayout 0xc0 bytes: rows 0..15
// = player 1..4 A..D, rows 16..19 = single A..D; column 1 x, column 2 y, screen pixels from the top left). Execute 0x801a820c (states 1 / 2):
// slides from the previous hole to the new one (+0x1dc += 0.2, max 1); layout x = px - 320, y = 240 - py.
namespace kingdom::game::g04
{
class cTaskCursor : public kingdom::cTaskAnita
{
public:
	explicit cTaskCursor(uint nPlayer);   // DOL 0x801a7dd8
	bool Execute(cJetTask* pParam) override;   // DOL 0x801a820c
	void Show();                          // DOL 0x801a843c
	void SetPlace(int place);             // DOL 0x801a84cc
	void Hide();                          // DOL 0x801a84ec
	void Lunge();                         // DOL 0x801a8550

private:
	int m_nPlace{2};                      // +0x1c8
	int m_nOldPlace{2};                   // +0x1cc
	uint m_nPlayer{};                     // +0x1d0
	int m_nState{};                       // +0x1d4
	s32 m_nX[4][4]{}, m_nY[4][4]{};       // +0x1d8 -> +0x20 / +0x60 (player, place)
	s32 m_nSingleX[4]{}, m_nSingleY[4]{}; // +0xa0 / +0xb0
	float m_fSlide{};                     // +0x1dc
};
}
