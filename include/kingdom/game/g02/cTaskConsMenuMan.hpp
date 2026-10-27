#pragma once

#include "common.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

// Rally-X Remix (g02) stage select: the console manager (DOL kingdom::game::g02::cTaskConsMenuMan, vtable 0x8043d130, 0x264 bytes, ctor
// 0x80108074), created by cAppG02RallyXMenu's ctor (0x80121bbc) at app +0x2a4. Ported from main.dol (no C# exists for Rally-X).
// The state (+0x1b8) is 0 while the player chooses; Select (0x80109200, from the app's MainExec) turns input into states 2..9, Execute
// (0x80109310) plays them out. Focus numbers (+0x1c0): 1 Back, 2 Rule, 3 Controller, 4 Multi, 5 Map, 6..11 Stage 0..5 (two rows of three),
// 12 / 13 Arrow left / right, 14 = both arrows (the world row of the key navigation). Every console is a child of the main app (the DOL
// connects them to KingSys +0xa40), not of the man.
// Own vtable slots: +0x40 PlaySndSe (0x8005cfd4, the app's +0x64), +0x44 InitStageNo 0x80108674, +0x48 FreeStageCheck 0x8010895c,
// +0x4c AddHitArray 0x80108798.
namespace kingdom::game::g02
{
// the MENU_LIGHT_BG / MENU_LIGHT_MODEL lights and the scene camera of the menu (shared with the multiplayer menu app)
void SetupMenuScene();

class cTaskConsMenuArrow;
class cTaskConsMenuBack;
class cTaskConsMenuBar;
class cTaskConsMenuBg;
class cTaskConsMenuController;
class cTaskConsMenuCurtain;
class cTaskConsMenuMulti;
class cTaskConsMenuRule;
class cTaskConsMenuStage;
class cTaskConsMenuWorld;
class cTaskMenuMap;
class cTaskMenuPacman;

class cTaskConsMenuMan : public kingdom::game::cTaskConsMenuManBase
{
public:
	explicit cTaskConsMenuMan(kingdom::game::cAppGameBaseMenu* pApp);   // DOL 0x80108074
	~cTaskConsMenuMan() override;                                       // DOL 0x801085bc

	bool Execute(cJetTask* pParam) override;   // DOL 0x80109310

	void AllWaitState();        // DOL 0x80108830
	void Reset();               // DOL 0x801092d8: all consoles wait, state 0
	void Select();              // DOL 0x80109200: pointer / keys -> focus and decision (only in state 0)
	void AppearStart();         // DOL 0x80109a2c: every console (and the car) appears; the state stays 0
	bool IsAppearEnd();         // DOL 0x80109b6c
	bool IsBackDecide();        // DOL 0x80109d0c
	bool IsMultiDecide();       // DOL 0x80109d20
	bool IsStageDecide();       // DOL 0x80109d34
	bool IsRuleDecide();        // DOL 0x80109d54
	void SetStage();            // DOL 0x80109d68: sys stage index = stage + world * 6 + 1, 1 player, player 0's control type = the button's
	int nCtrlType();            // DOL 0x80109dcc: the Controller button's type (1 remote only, 2 Nunchuk)

	int nWorldNo() const { return m_nWorldNo; }   // +0x1c8, 0..3
	int nStageNo() const { return m_nStageNo; }   // +0x1cc, 0..5 within the world

private:
	void InitStageNo();         // DOL 0x80108674 (vtable +0x44)
	void FreeStageCheck();      // DOL 0x8010895c (vtable +0x48)
	void AddHitArray();         // DOL 0x80108798 (vtable +0x4c)
	void KeySelect(uint trig);  // DOL 0x80108a4c
	void PointUpdate(uint trig);   // DOL 0x80108f74
	void DecideCheck(uint trig);   // DOL 0x8010912c
	bool IsSelectable(int focus) const;   // inlined in every KeySelect loop
	void SetupScene();          // the light manager +0x1d8 / scene camera +0x1dc of the ctor (natively NativeScene)

	int m_nState{};                           // +0x1b8
	int m_nStep{};                            // +0x1bc
	int m_nSelectNow{};                       // +0x1c0
	int m_nSelectOld{};                       // +0x1c4
	int m_nWorldNo{};                         // +0x1c8
	int m_nStageNo{};                         // +0x1cc
	int m_nWorldMax{};                        // +0x1d0 the world of the newest open stage
	int m_nStageMax{};                        // +0x1d4 the newest open stage in its world
	// +0x1d8 light manager (FUN_80018dd0: MENU_LIGHT_BG set 0, MENU_LIGHT_MODEL set 1), +0x1dc kingdom::game::cTaskSceneCamera (the
	// MENU_LIGHT_MODEL SCN0 on ScnRoot camera 0): natively NativeScene lights / camera (SetupScene)
	cTaskConsMenuBg* m_pBg{};                 // +0x1e0
	cTaskConsMenuCurtain* m_pCurtain{};       // +0x1e4
	cTaskConsMenuBack* m_pBack{};             // +0x1e8
	cTaskConsMenuMulti* m_pMulti{};           // +0x1ec
	cTaskConsMenuRule* m_pRule{};             // +0x1f0
	cTaskConsMenuWorld* m_pWorld{};           // +0x1f4
	cTaskConsMenuBar* m_pBar{};               // +0x1f8
	cTaskConsMenuController* m_pController{}; // +0x1fc
	cTaskMenuMap* m_pMap{};                   // +0x200
	cTaskMenuPacman* m_pPacman{};             // +0x204
	cTaskConsMenuStage* m_pStage[6]{};        // +0x208
	cTaskConsMenuArrow* m_pArrow[2]{};        // +0x220
	kingdom::game::cTaskConsMenuBase* m_pHitArray[15]{};   // +0x228, see AddHitArray
};
}
