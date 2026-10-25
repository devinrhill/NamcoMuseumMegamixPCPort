#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/game/menu/cTaskConsMenuTutorialBase.hpp"

// Grobda Remix (g11) menu 338 consoles, ported 1:1 from main.dol (no C# exists for Grobda). Layouts of g11menu.lzs: GROBDA_BIN_ANT_MENU_SELECT01
// (crc 0x6ec78825, single player) / SELECT02 (0xf7ced99f, multiplayer), GROBDA_BIN_ANT_MENU_BG (0x9d87f296), GROBDA_BIN_ANT_TUTORIAL (0x90cbdb1f).
// The buttons share one body (DOL: four classes Back 0x802043d8 / Rule 0x80206e24 / Start 0x80207298 / Multi 0x80205e60): anim 4 columns
// {appear, wait, point, decide}, hit pattern; Back / Start / Multi add 5 to the anim numbers of the multiplayer layout (+0x1d8 = 1).
// Multiplayer branches (the four cTaskConsMenuPlayer panels with Mii icons, cTaskConsMenuMulti's matching) are NOT PORTED.
namespace kingdom::game::g11
{

class cTaskGrobdaMenuButton : public kingdom::game::cTaskConsMenuBase
{
public:
	cTaskGrobdaMenuButton(kingdom::game::cTaskConsMenuManBase* const& pConsMan, bool bMulti, int hitPat, const int anm[4], uint priority);
	bool IsDpdHit(float x, float y) override;   // DOL 0x802044e0 (pattern hitPat of anita 0)
protected:
	void WaitExec() override;
	bool AppearExec() override;
	void PointExec() override;
	bool DecideExec() override;
private:
	int m_nHitPat;
	int m_nAnm[4];   // appear, wait, point, decide
};

class cTaskConsMenuBack : public cTaskGrobdaMenuButton
{
public:
	cTaskConsMenuBack(kingdom::game::cTaskConsMenuManBase* const& pConsMan, bool bMulti);   // DOL 0x802043d8 (hit 0x1a)
};
class cTaskConsMenuRule : public cTaskGrobdaMenuButton
{
public:
	cTaskConsMenuRule(kingdom::game::cTaskConsMenuManBase* const& pConsMan, bool bMulti);   // DOL 0x80206e24 (hit 0x19)
};
class cTaskConsMenuStart : public cTaskGrobdaMenuButton
{
public:
	cTaskConsMenuStart(kingdom::game::cTaskConsMenuManBase* const& pConsMan, bool bMulti);  // DOL 0x80207298 (hit 0x1b)
};
class cTaskConsMenuMulti : public cTaskGrobdaMenuButton
{
public:
	cTaskConsMenuMulti(kingdom::game::cTaskConsMenuManBase* const& pConsMan, bool bMulti);  // DOL 0x80205e60 (hit 0x1c)
};

// kingdom::game::g11::cTaskConsMenuBg (0x1d8, ctor 0x802048bc): GROBDA_BIN_ANT_MENU_BG, anim 0 (single) / 1 (multi), priority 0x10001
class cTaskConsMenuBg : public kingdom::game::cTaskConsMenuBase
{
public:
	cTaskConsMenuBg(kingdom::game::cTaskConsMenuManBase* const& pConsMan, bool bMulti);
};

// kingdom::game::g11::cTaskConsMenuHighScore (0x1dc, ctor 0x802049ac, anim table 0x80204aac, WaitExec 0x80204cac, AppearExec 0x80204d24): layer 0 =
// the frame (anim 0x28 appear / 0x29 wait / 0x2a), layers 1..6 = the digits of the saved best score (anim 3 * digit + 0 / 1 / 2)
class cTaskConsMenuHighScore : public kingdom::game::cTaskConsMenuBase
{
public:
	explicit cTaskConsMenuHighScore(kingdom::game::cTaskConsMenuManBase* const& pConsMan);
protected:
	void WaitExec() override;
	bool AppearExec() override;
private:
	int AnimNo(uint layer, int kind) const;   // DOL 0x80204aac
	int m_nHigh{};                            // +0x1d8 = *(sys +8)
};

// kingdom::game::g11::cTaskConsMenuTutorial (0x2f0, ctor 0x8020777c): the HOW TO PLAY pages on BIN_COMMON_ANT_TUTORIAL (crc 0xd70b5cff)
class cTaskConsMenuTutorial : public kingdom::game::menu::cTaskConsMenuTutorialBase
{
public:
	explicit cTaskConsMenuTutorial(kingdom::cTaskCursor* pCursor);
};

// kingdom::game::g11::cTaskGrobdaMenuChar (0x1cc, ctor 0x802079d0, Execute 0x80207e18): player n's tank (m11m_menu_c0n) with its pilot
// (m11m_menu_p0n) on the node "cannon", placed on the node play_dp_chr_0n of m11s_menu01_s
class cTaskGrobdaMenuChar : public kingdom::cTaskModel
{
public:
	explicit cTaskGrobdaMenuChar(int player);
	bool Execute(cJetTask* pParam) override;
};

// kingdom::game::g11::cTaskConsMenuMan (0x218, vtable 0x8044fed8; ctor 0x80204e94, Execute 0x802053e0, Select 0x802057cc, Appear 0x80205b6c)
// state (+0x1b8): 0 choosing, 1 appearing, 2 Back / 3 Rule / 4 Multi / 5 Start decided. Focus (+0x1c0): 1 Back, 2 Rule, 3 Multi, 4 Start.
class cTaskConsMenuMan : public kingdom::game::cTaskConsMenuManBase
{
public:
	explicit cTaskConsMenuMan(kingdom::game::cAppGameBaseMenu* pApp);   // DOL 0x80204e94
	bool Execute(cJetTask* pParam) override;   // DOL 0x802053e0
	void Select();                             // DOL 0x802057cc
	void AppearStart();                        // DOL 0x80205b6c (every console appears, state 1)
	void Reset();                              // DOL 0x802056c8 (every console waits, state 0, step 0)
	void PointCurrent();                       // DOL 0x802057ac
	bool IsIdle() const { return m_nState == 0; }   // DOL 0x80205b08
	bool IsBackDecide();                       // DOL 0x80205b54
	bool IsStartDecide();                      // DOL 0x80205b2c
	bool IsRuleDecide();                       // DOL 0x80205b18
	bool IsMultiDecide();                      // DOL 0x80205b40
private:
	bool IsAppearEnd();                        // DOL 0x80205c40
	void AllWait();
	int m_nState{};                            // +0x1b8
	int m_nStep{};                             // +0x1bc
	int m_nSelectNow{};                        // +0x1c0
	int m_nSelectOld{};                        // +0x1c4
	cTaskGrobdaMenuChar* m_pChar[4]{};         // +0x1cc
	cTaskConsMenuBg* m_pBg{};                  // +0x1dc
	cTaskConsMenuStart* m_pStart{};            // +0x1e0
	cTaskConsMenuMulti* m_pMulti{};            // +0x1e4
	cTaskConsMenuBack* m_pBack{};              // +0x1e8
	cTaskConsMenuRule* m_pRule{};              // +0x1ec
	cTaskConsMenuHighScore* m_pHigh{};         // +0x1f0 (single player only)
	kingdom::game::cTaskConsMenuBase* m_pItem[5]{};   // +0x204: 0 none, 1 Back, 2 Rule, 3 Multi, 4 Start
};
}
