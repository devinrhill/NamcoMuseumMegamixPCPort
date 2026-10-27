#pragma once

#include "common.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

// Rally-X Remix (g02) multiplayer menu console manager (DOL kingdom::game::g02::cTaskConsMultiMenuMan, vtable 0x8043dd20, 0x2ac bytes, ctor
// 0x8010cadc), created by cAppG02RallyXMultiMenu's ctor (0x80122254) at app +0x2b4. Ported from main.dol (no C# exists for Rally-X).
// The state arrays (+0x1b8 / +0x1c8) hold the main state of pad 0 [0] and one "switch my control type" state per other player [1..3] (14 + player).
// Main states: 0 choosing, 2 Back, 3 Single, 4 Start (button), 5 Rule, 6 Start (map), 7 / 8 flag count left / right, 9 / 10 time left / right,
// 0xb unused, 0xc / 0xd stage left / right, 0xe player 0's control type.
// Focus numbers (+0x1ec): 1 Back, 2 Rule, 3 Map, 4 player 0's control button, 8 Start, 9 Single, 10 / 11 stage arrows, 13 / 15 flag arrows,
// 14 / 16 time arrows; the rows 0xc / 0x11 / 0x12 stand for "the arrow pair" of the key navigation.
// Own vtable slots: +0x40 PlaySndSe (0x8005cfd4), +0x44 InitStageNo 0x8010d05c, +0x48 an empty function 0x8010d25c, +0x4c AddHitArray 0x8010d0c0.
namespace kingdom::game::g02
{
class cTaskConsMultiMenuArrow;
class cTaskConsMultiMenuBack;
class cTaskConsMultiMenuBg;
class cTaskConsMultiMenuBoard;
class cTaskConsMultiMenuCurtain;
class cTaskConsMultiMenuFlag;
class cTaskConsMultiMenuPlayer;
class cTaskConsMultiMenuRule;
class cTaskConsMultiMenuSingle;
class cTaskConsMultiMenuStage;
class cTaskConsMultiMenuStart;
class cTaskConsMultiMenuTime;
class cTaskMultiMenuMap;

class cTaskConsMultiMenuMan : public kingdom::game::cTaskConsMenuManBase
{
public:
	explicit cTaskConsMultiMenuMan(kingdom::game::cAppGameBaseMenu* pApp);   // DOL 0x8010cadc
	~cTaskConsMultiMenuMan() override;                                       // DOL 0x8010cff4

	bool Execute(cJetTask* pParam) override;   // DOL 0x8010daac

	void AllWaitState();        // DOL 0x8010d164
	void Reset();               // DOL 0x8010d99c: all consoles wait, state 0, step 0
	void Select();              // DOL 0x8010d86c
	void AppearStart();         // DOL 0x8010e3d8
	bool IsAppearEnd();         // DOL 0x8010e534
	bool IsBackDecide();        // DOL 0x8010e6f0
	bool IsStartDecide();       // DOL 0x8010e704
	bool IsSingleDecide();      // DOL 0x8010e718
	bool IsRuleDecide();        // DOL 0x8010e72c
	void ApplySettings();       // DOL 0x8010e740: stage / flags / time / control types into the sys
	int nCtrlType();            // DOL 0x8010e7d8: player 0's control type (picks the HOW TO PLAY pages)

private:
	void InitStageNo();         // DOL 0x8010d05c (vtable +0x44)
	void AddHitArray();         // DOL 0x8010d0c0 (vtable +0x4c)
	void KeySelect(uint trig);  // DOL 0x8010d260
	void PointUpdate(uint trig);   // DOL 0x8010d388
	void DecideCheck(uint trig);   // DOL 0x8010d748

	int m_nState[4]{};                        // +0x1b8
	int m_nStep[4]{};                         // +0x1c8
	int m_nPlayer{};                          // +0x1d8
	int m_nTimeIdx{};                         // +0x1dc
	int m_nFlagIdx{};                         // +0x1e0
	int m_nStageKey{};                        // +0x1e4 (0..3)
	int m_nMaxKey{};                          // +0x1e8
	int m_nSelectNow[4]{};                    // +0x1ec
	int m_nSelectOld[4]{};                    // +0x1fc
	cTaskConsMultiMenuBg* m_pBg{};            // +0x20c
	cTaskConsMultiMenuBack* m_pBack{};        // +0x210
	cTaskConsMultiMenuStart* m_pStart{};      // +0x214
	cTaskConsMultiMenuRule* m_pRule{};        // +0x218
	cTaskConsMultiMenuCurtain* m_pCurtain{};  // +0x21c
	cTaskConsMultiMenuSingle* m_pSingle{};    // +0x220
	cTaskConsMultiMenuArrow* m_pArrow[6]{};   // +0x224
	cTaskConsMultiMenuStage* m_pStage{};      // +0x23c
	cTaskConsMultiMenuBoard* m_pBoard{};      // +0x240
	cTaskConsMultiMenuTime* m_pTime{};        // +0x244
	cTaskConsMultiMenuFlag* m_pFlag{};        // +0x248
	cTaskConsMultiMenuPlayer* m_pPlayer[4]{}; // +0x24c
	cTaskMultiMenuMap* m_pMap{};              // +0x25c
	kingdom::game::cTaskConsMenuBase* m_pHitArray[19]{};   // +0x260, see AddHitArray
};
}
