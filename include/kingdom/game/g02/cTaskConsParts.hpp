#pragma once

#include "common.hpp"
#include "kingdom/game/g02/cTaskRallyAnitaBase.hpp"

namespace kingdom { class cTaskModel; }

// Rally-X Remix (g02) the single-player console parts (all kingdom::game::g02::cTaskRallyAnitaBase, built by cTaskConsole Init 0x8006093c).
// Layouts: ANITA_CAR01 (0x9d901d89) and ANITA_RESULT (0x7bc6cf12); the radar / warning use ANITA_RADAR (cTaskConsRadar.hpp), the mud ANITA_MUD.
// "Animate" = anita state bit 2 (the parts register their anitas with bit 2 clear and start them when the game state passes 1).
namespace kingdom::game::g02
{

// kingdom::game::g02::cTaskConsFuel (0x1e8, vtable 0x803915c8; ctor 0x8006210c, Execute 0x8006957c, dtor 0x800694b4): the fuel gauge
// (ANITA_CAR01 anita 0 frame 0xf4 / 0xf5 / 0xf6, 1 the change 0xf2, 2 the level / nitro 0xf2, 3 the smoke frame 0xa7, 4..8 the smoke stock 0xaa)
class cTaskConsFuel : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsFuel(cTaskConsole* pConsole);            // DOL 0x8006210c
	bool Execute(cJetTask* pParam) override;                   // DOL 0x8006957c
	void SetCar(kingdom::cTaskModel* pCar, int nPlayer);       // the single-player half of DOL 0x80065db0 (player-count layer tables)

	int m_nMode{};                                             // +0x1cc
	float m_fFuel{};                                           // +0x1d0
	int m_nLevel{};                                            // +0x1d4 (0..11 = 12 * fuel / 100)
	int m_nSmoke{};                                            // +0x1d8
	kingdom::cTaskModel* m_pCar{};                             // +0x1dc
	int m_nPlayer{};                                           // +0x1e0
	// +0x1e4 the console (base m_pConsole)
};

// kingdom::game::g02::cTaskConsInfo (0x1e0, vtable 0x803917e8; ctor 0x800619d8, Execute 0x8006aa78, dtor 0x8006a9b0): the score (anitas 7..2),
// the flags total (0xc / 0xd) and, on the boss stage, flags left (10 / 11), the rest digit (0x10) and the car state icon (0xe)
class cTaskConsInfo : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsInfo(cTaskConsole* pConsole);            // DOL 0x800619d8
	bool Execute(cJetTask* pParam) override;                   // DOL 0x8006aa78

	int m_nMode{};                                             // +0x1cc
	int m_nScore{};                                            // +0x1d0 (shown)
	int m_nFlags{};                                            // +0x1d4 (shown)
	kingdom::cTaskModel* m_pCar{};                             // +0x1d8 (the player; on the boss stage the last registered enemy)
	// +0x1dc the console
};

// kingdom::game::g02::cTaskConsSignal (0x1d4, vtable 0x80391870; ctor 0x80062530, Execute 0x8006b234, dtor 0x8006b16c): the start lamp
// (ANITA_CAR01 anita 0: 0x87 -> 0x89 -> 0x8b -> 0x8d -> 0x8f -> 0x91 at count 120 / 180 / 240 / 300 / 360, m02se_14Signal1 / m02se_15Signal2)
class cTaskConsSignal : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsSignal(cTaskConsole* pConsole);          // DOL 0x80062530
	bool Execute(cJetTask* pParam) override;                   // DOL 0x8006b234

	int m_nMode{};                                             // +0x1cc
	// +0x1d0 the console
};

// kingdom::game::g02::cTaskConsTime (0x1d4, vtable 0x803916d8; ctor 0x8006272c, Execute 0x80069c08, dtor 0x80069b40): the play time m:ss.cc
// (ANITA_CAR01 digits 0 / 1 minutes, 3 / 4 seconds, 6 / 7 hundredths, 2 / 5 the separators; placed on the layers of pattern 0x40)
class cTaskConsTime : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsTime(cTaskConsole* pConsole);            // DOL 0x8006272c
	bool Execute(cJetTask* pParam) override;                   // DOL 0x80069c08

	int m_nMode{};                                             // +0x1cc
	// +0x1d0 the console
};

// kingdom::game::g02::cTaskConsStage (0x1d0, vtable 0x80391650; ctor 0x80062afc, Execute 0x80069aa4, dtor 0x800699dc): "world-stage"
class cTaskConsStage : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsStage(cTaskConsole* pConsole);           // DOL 0x80062afc
	bool Execute(cJetTask* pParam) override;                   // DOL 0x80069aa4
	// +0x1cc the console
};

// kingdom::game::g02::cTaskConsMessage (0x1d4, vtable 0x80391540; ctor 0x80062e14, Execute 0x80069380, dtor 0x800692b8): the stage's opening
// message; anita 2 shows the CSV_FLAG column 2 value of the stage (0..3), all hidden for larger values. Priority 0x80000004.
class cTaskConsMessage : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsMessage(cTaskConsole* pConsole);         // DOL 0x80062e14
	bool Execute(cJetTask* pParam) override;                   // DOL 0x80069380

	int m_nMode{};                                             // +0x1cc
	// +0x1d0 the console
};

// kingdom::game::g02::cTaskConsResult (0x1d8, vtable 0x80391410; ctor 0x80064008, Execute 0x80068220, dtor 0x80068158): the stage clear
// (ANITA_RESULT 0 / 3 the banner, 4 the score frame, 5..10 the score digits; m00se_06Hiscore on a new record). Priority 0x80000005.
class cTaskConsResult : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsResult(cTaskConsole* pConsole);          // DOL 0x80064008
	bool Execute(cJetTask* pParam) override;                   // DOL 0x80068220

	int m_nMode{};                                             // +0x1cc (3 = waited, see cTaskConsole::IsResultEnd)
	float m_fTimer{};                                          // +0x1d0
	// +0x1d4 the console (the wait = app +0x280 (cParamGame) +0x38 = CSV_GAME row 7 * 60, read every frame: cTaskConsole::fClearFlagWait)

private:
	void SetScore(const u32* table);                           // the digit block of Execute (tables 0x8036bd70 / 0x8036bd98)
};

// kingdom::game::g02::cTaskConsLose (0x1d8, vtable 0x80391238; ctor 0x800643e4, Execute 0x80066da8, dtor 0x80066ce0): the lost stage
// (ANITA_RESULT 0: 0x3c -> 0x3d -> 0x3e, 1: 0x24 -> 0x25 -> 0x26), starts in game state 10. Priority 0x80000005.
class cTaskConsLose : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsLose(cTaskConsole* pConsole);            // DOL 0x800643e4
	bool Execute(cJetTask* pParam) override;                   // DOL 0x80066da8

	int m_nMode{};                                             // +0x1cc (2 shown, 7 over)
	float m_fTimer{};                                          // +0x1d0
	// +0x1d4 the console (the wait = app +0x280 +0x44 = CSV_GAME row 10 * 60: cTaskConsole::fLoseFlagWait)
};

// kingdom::game::g02::cTaskConsBang (0x1d0, vtable 0x80391060; ctor 0x80064610, Execute 0x80066a74, dtor 0x800669ac): the crash banner
// (ANITA_CAR01 anita 0, anim 0x118 when shown by cTaskConsole::ShowBang). Its Execute is the base's freeze test + UpdateFrame written out
// (it reads the pause / stage-end bytes from the Execute parameter = the app). Priority 0x80000005.
class cTaskConsBang : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsBang(cTaskConsole* pConsole);            // DOL 0x80064610
	bool Execute(cJetTask* pParam) override;                   // DOL 0x80066a74
	void Show();                                               // the body of DOL 0x8006662c
	void Hide();                                               // the body of DOL 0x8006669c / 0x800667c4
	// +0x1cc the console
};

// kingdom::game::g02::cTaskConsWarning (0x1d4, vtable 0x80390fd8; ctor 0x80064814, Execute 0x800668cc, dtor 0x80066804): an enemy near the
// player (ANITA_RADAR anita 0 the banner 0x57, 1..4 the arrows 0x5b / 0x59 / 0x58 / 0x5a); driven by cTaskConsRadar. Priority 0x80000005.
class cTaskConsWarning : public cTaskRallyAnitaBase
{
public:
	explicit cTaskConsWarning(cTaskConsole* pConsole);         // DOL 0x80064814
	bool Execute(cJetTask* pParam) override;                   // DOL 0x800668cc
	void HideAll();                                            // anitas 0..4 hidden
	using cTaskRallyAnitaBase::Show;

	// +0x1cc the console
	bool m_bActive{};                                          // +0x1d0
};
}
