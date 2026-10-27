#pragma once

#include <string>
#include <vector>

#include "common.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g02/cTaskRallyAnitaBase.hpp"

namespace kingdom { class cTaskModel; }

// Rally-X Remix (g02) the multiplayer HUD, ported from main.dol (no C# exists for Rally-X). Built by cTaskConsole (console +0x1bc = 1, init
// 0x8006093c), in this order: the radar, then per player cTaskConsFuel / cTaskConsMud / cTaskConsPBMultiNO, cTaskConsInfoMulti, Signal, Time,
// cTaskConsWinner, cTaskConsDraw and, for 3 players, cTaskConsPBMultiBG (the empty fourth viewport).
// NOT PORTED: nothing of these; the names use the matching's names (cG02RallyXSysExt::szMatchName, "Guest A".. from the matching stand-in).
namespace kingdom::game::g02
{
class cTaskConsole;

// The push-back layers of the split screen (DOL kingdom::cTaskPBAnita, vtable 0x80391a44: a cTaskPushBack with an anita vector; +0x1b8 =
// the camera number): drawn into one camera's viewport (cPBView.hpp). Regist = cTaskConsMud::Regist 0x80123418.
class cTaskConsPBView : public kingdom::cTaskAnita
{
public:
	cTaskConsPBView(cTaskConsole* pConsole, int nCamera);
	bool Execute(cJetTask* pParam) override;            // the freeze test + the anita vector's UpdateFrame (DOL 0x80123320 / 0x80066cdc)
	void Draw(int nStatus, cJetTask* pParam) override;  // DOL 0x801235cc: inside the camera's viewport

protected:
	kingdom::cKingdomAnita* Regist(uint idx, const char* name, int anim);   // DOL 0x80123418
	cTaskConsole* m_pConsole{};
	int m_nCamera{};                                    // +0x1b8
};

// the player's number plate at the start (DOL cTaskConsPBMultiNO, 0x1dc bytes, vtable 0x803910e4, ctor 0x80064afc; Execute 0x80066b5c):
// ANITA_CAR01 anim 0x128 + player; plays during the start signal (app state 2) and hides at its end. Priority 0x8000000.
class cTaskConsPBMultiNO : public cTaskConsPBView
{
public:
	cTaskConsPBMultiNO(cTaskConsole* pConsole, int nPlayer);   // DOL 0x80064afc
	bool Execute(cJetTask* pParam) override;                   // DOL 0x80066b5c
};

// fills the fourth viewport of a 3 player game (DOL cTaskConsPBMultiBG, 0x1dc bytes, vtable 0x8039118c, ctor 0x80065b24, Execute 0x80066cdc):
// ANITA_CAR01 anim 0x117 in camera 3
class cTaskConsPBMultiBG : public cTaskConsPBView
{
public:
	cTaskConsPBMultiBG(cTaskConsole* pConsole, int nCamera);   // DOL 0x80065b24
};

// the flag counts and the nitro marks of the players (DOL cTaskConsInfoMulti, 0x21c bytes, vtable 0x80391760, ctor 0x80064d68, Execute
// 0x80069fa8). ANITA_CAR01, 30 anitas: 0 the frame (anim by the player count), 1..4 / 0x11..0x14 / 5..8 / 9..12 / 13..16 / 0x15..0x18 /
// 0x19..0x1c the per-player parts (car mark, leader mark, plate, flag tens, flag ones, nitro, hit), 0x1d the "hold B" hint.
class cTaskConsInfoMulti : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsInfoMulti(cTaskConsole* pConsole);       // DOL 0x80064d68
	bool Execute(cJetTask* pParam) override;                   // DOL 0x80069fa8
	void SetCar(int nPlayer, kingdom::cTaskModel* pCar) { if ((uint)nPlayer < 4u) m_Player[nPlayer].pCar = pCar; }
	void ShowHit(int nPlayer);                                 // DOL 0x800666e8 (the console's hit mark of a player: UnitMultiHit)

private:
	struct sPlayer
	{
		kingdom::cTaskModel* pCar{};   // +0x1d8 + 0x10 i
		int nFlags{};                  // +0x1dc (the flag count shown)
		int nNitro{};                  // +0x1e0 (the nitro state shown)
		int n1e4{};                    // +0x1e4
	};
	int m_nMode{};                     // +0x1cc
	int m_nPlayerNum{};                // +0x1d0 (sys +0x0c)
	bool m_bHint{};                    // +0x1d4
	sPlayer m_Player[4];               // +0x1d8
};

// the winner (DOL cTaskConsWinner, 0x28c bytes, vtable 0x80391388, ctor 0x8006546c, Execute 0x80067a24, Draw 0x800679e8): ANITA_RESULT, the
// banner, the label of the winning player and his name in his colour. Starts in app state 8. Priority 0x80000005.
class cTaskConsWinner : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsWinner(cTaskConsole* pConsole);          // DOL 0x8006546c
	bool Execute(cJetTask* pParam) override;                   // DOL 0x80067a24
	void Draw(int nStatus, cJetTask* pParam) override;         // DOL 0x800679e8: the base draw, then the font writer's queue
	int m_nMode{};                                             // +0x1cc (2 shown, 4 over: cTaskConsole::IsWinShown / IsWinEnd)

private:
	struct sText
	{
		float x, y;
		std::string text;
		u32 color;
	};
	std::vector<sText> m_Texts;                                // the font writer (+0x1d4) queue
	float m_fTimer{};                                          // +0x1d0
	int m_nWinner{};
};

// a draw (DOL cTaskConsDraw, 0x28c bytes, vtable 0x803912e0, ctor 0x8006579c, Execute 0x800671cc, Draw 0x80067190): the tied players' labels
// and names. Starts in app state 9. Priority 0x80000005.
class cTaskConsDraw : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsDraw(cTaskConsole* pConsole);            // DOL 0x8006579c
	bool Execute(cJetTask* pParam) override;                   // DOL 0x800671cc
	void Draw(int nStatus, cJetTask* pParam) override;         // DOL 0x80067190
	int m_nMode{};                                             // +0x1cc

private:
	struct sText
	{
		float x, y;
		std::string text;
		u32 color;
	};
	std::vector<sText> m_Texts;
	float m_fTimer{};                                          // +0x1d0
};
}
