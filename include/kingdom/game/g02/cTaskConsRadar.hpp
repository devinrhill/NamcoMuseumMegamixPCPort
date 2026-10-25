#pragma once

#include "common.hpp"
#include "kingdom/game/g02/cTaskRallyAnitaBase.hpp"

namespace kingdom { class cTaskModel; }

// Rally-X Remix (g02) the radar: kingdom::game::g02::cTaskConsRadar (0x32c bytes, vtable 0x803918f8; ctor 0x80060f54, Execute 0x8006b6e8,
// Draw 0x8006b6d4, dtor 0x8006b60c). ANITA_RADAR (0x3a24042c), priority 0x80000002:
//   anita 0 the frame (anim 0x3b; its position = layer 0 of pattern 0x8036b780[player count] = the radar origin +0x228 / +0x22c),
//   1 / 2 the "near" marks (0x3c / 0x43), 3..0x16 the flags (one per registered flag), 0x17..0x1d the cars, 0x1e..0x24 the red cars' flag marks.
// A world point (x, z) is drawn at (origin.x + 6 x, origin.y - 6 z). Cars and flags come from cTaskConsole::Regist / RegistFlag.
// It also drives cTaskConsWarning (console +0x1f4): an enemy within 5 of the player in game state 4 shows the banner and an arrow.
namespace kingdom::game::g02
{
class cTaskConsRadar : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsRadar(cTaskConsole* pConsole);           // DOL 0x80060f54
	bool Execute(cJetTask* pParam) override;                   // DOL 0x8006b6e8
	void Draw(int nStatus, cJetTask* pParam) override;         // DOL 0x8006b6d4

	void AddCar(kingdom::cTaskModel* pCar, int nColour, bool bPlayer);   // the radar half of DOL 0x80065db0
	void AddFlag(kingdom::cTaskModel* pGimmick, int nType);              // the radar half of DOL 0x800662c8

	struct sCar
	{
		kingdom::cTaskModel* pCar{};                           // +0x1d4 + i * 0xc
		int nColour{};                                         // +0x1d8 (1 blue .. 5 red, 6 Pac-Man)
		bool bPlayer{};                                        // +0x1dc
	};
	struct sFlag
	{
		kingdom::cTaskModel* pGimmick{};                       // +0x234 + i * 0xc
		int nType{};                                           // +0x238 (gimmick factory type: 11 flag, 12..31 flag 1..20, 32 lucky, 33 special)
		int nState{};                                          // +0x23c (0 hidden, 1 shown, 2 taken (clip), 3 gone)
	};

	int m_nCar{};                                              // +0x1cc
	int m_nPlayerCar{};                                        // +0x1d0 (the entry registered as the player)
	sCar m_Car[7];                                             // +0x1d4
	float m_fOrgX{};                                           // +0x228
	float m_fOrgY{};                                           // +0x22c
	int m_nFlag{};                                             // +0x230
	sFlag m_Flag[20];                                          // +0x234
	bool m_bDraw{true};                                        // +0x324 (the 4-player games hide the radar unless B is held)
	// +0x328 the console

private:
	void Arrow(int base, int dir, int view);                   // one of the inlined arrow picks: warning anita by base / car direction / view
};
}
