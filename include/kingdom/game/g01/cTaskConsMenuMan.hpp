#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/g01/cObject.hpp"

// Galaga stage select: the console manager (DOL kingdom::game::g01::cTaskConsMenuMan, 0x2a0 bytes, vtable 0x80437ab0 (cObject part) /
// 0x80437ac4 (task part at +0x50)). Created by cAppG01GalagaMenu (0x800aea18) as new cTaskConsMenuMan(app, bMulti app +0x2a0) at app +0x2bc.
// The state (cObjectBase +0) is 0 while the player chooses; Select (0x800be0fc, called from the app's MainExec) turns input into states
// 2..8, Execute (0x800bcad4) plays them out on the consoles. Focus numbers (+0x220): 1 Back, 2 Map, 3 Rule, 4 Multi, 5..7 Stage, 8 / 9 Arrow,
// 10 = both arrows (the world row of the key navigation).
namespace kingdom::game::g01
{
class cTaskConsMenuArrow;
class cTaskConsMenuBack;
class cTaskConsMenuBar;
class cTaskConsMenuBg;
class cTaskConsMenuMap;
class cTaskConsMenuMulti;
class cTaskConsMenuRule;
class cTaskConsMenuStage;
class cTaskConsMenuWorld;
class cTaskConsMultiMenuPlayer;

// The task part's vtable (+0x50 of the DOL object) keeps the engine's cJetTask / cTaskKingdom slots (0x80009dc8 DeleteExecute, 0x80009dfc /
// 0x800077f8 nDrawPriority, 0x80009df4 / 0x80009dec AutoRelease, 0x8005cfd4: the app's +0x1b4 vtable +0x64 = the shared menu console call):
// natively cTaskConsMenuManBase's.
class cTaskConsMenuMan : public cObject, public kingdom::game::cTaskConsMenuManBase
{
public:
	cTaskConsMenuMan(kingdom::game::cAppGameBaseMenu* pApp, bool bMulti);   // DOL 0x800bc114
	~cTaskConsMenuMan() override;                                           // DOL 0x800bc774

	bool Execute(cJetTask* pParam) override;   // DOL 0x800bcad4 (task vtable slot 4 through the -0x50 thunk 0x800be38c)

	void Reset();               // DOL 0x800bd984: all consoles wait, state 0
	void Select();              // DOL 0x800be0fc: pointer / keys -> focus and decision (only in state 0)
	void AppearStart();         // DOL 0x800be1d4: state 1, every console appears
	bool IsRuleDecide();        // DOL 0x800be328
	bool IsStageDecide();       // DOL 0x800be33c
	bool IsMultiDecide();       // DOL 0x800be35c
	bool IsBackDecide();        // DOL 0x800be370
	void SaveStageRecord();     // DOL 0x800be384 (vtable slot 18): the Bar's numbers back into the current stage record

	int nWorldNo() const { return m_nWorldNo; }   // +0x218, 0..2
	int nStageNo() const { return m_nStageNo; }   // +0x210, 0..2 within the world

private:
	void InitStageNo();         // DOL 0x800bc838
	void AllWaitState();        // DOL 0x800bc988
	void FreeStageCheck();      // DOL 0x800bda14 (vtable slot 15)
	void AddHitArray();         // DOL 0x800bd9bc (vtable slot 16)
	void KeySelect(uint trig);  // DOL 0x800bdac8
	void PointUpdate(uint trig);   // DOL 0x800bde70
	void DecideCheck(uint trig);   // DOL 0x800be028
	bool IsSelectable(int focus) const;   // inlined 9 times in KeySelect
	void ChangeWorld(int step);   // Execute states 7 (next, +1) and 8 (previous, -1): the shared middle part
	bool AllBoardAppearEnd();     // the IsAppearEndState chain of states 7 / 8 step 2 (Bar or players, then the stages)
	bool AllBoardDeleteEnd();     // the IsDeleteEndState chain of states 7 / 8 step 1

	s32 m_nPlayer{};                          // +0x208 GalagaSys->nPlayer()
	bool m_bMulti{};                          // +0x20c
	s32 m_nStageNo{};                         // +0x210
	s32 m_nStageMax{};                        // +0x214 the newest open stage (in its world)
	s32 m_nWorldNo{};                         // +0x218
	s32 m_nWorldMax{};                        // +0x21c the world of the newest open stage
	s32 m_nSelectNow{};                       // +0x220
	s32 m_nSelectOld{};                       // +0x224
	cTaskConsMenuMulti* m_pMulti{};           // +0x228
	cTaskConsMenuStage* m_pStage[3]{};        // +0x22c
	cTaskConsMenuBack* m_pBack{};             // +0x238
	cTaskConsMenuWorld* m_pWorld{};           // +0x23c
	cTaskConsMenuArrow* m_pArrow[2]{};        // +0x240
	cTaskConsMenuRule* m_pRule{};             // +0x248
	cTaskConsMenuBg* m_pBg{};                 // +0x24c
	cTaskConsMenuBar* m_pBar{};               // +0x250 single player
	cTaskConsMultiMenuPlayer* m_pPlayer[4]{}; // +0x254 multiplayer, one per player
	cTaskConsMenuMap* m_pMap{};               // +0x264
	kingdom::game::cTaskConsMenuBase* m_pHitArray[11]{};   // +0x268, see AddHitArray
};
}
