#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/game/g02/cTaskConsMenuConsoles.hpp"
#include "kingdom/game/g02/cTaskMenuCarBase.hpp"
#include "kingdom/game/g02/cTaskMenuMap.hpp"
#include "kingdom/game/menu/cTaskConsMenuTutorialBase.hpp"

// Rally-X Remix (g02) multiplayer menu (job 200) consoles, ported from main.dol (no C# exists for Rally-X). DOL 0x8010b7b4..0x80112b3c.
// All use the layout ANITA_MENU_SELECT_02 (crc 0xb43de549, g02menu.lzs) except the Bg (ANITA_MENU_BG) and the world map's hit area (_01).
// The DOL's per-class slots (offsets from the vtable start): +0x48 WaitExec, +0x4c AppearExec, +0x50 PointExec, +0x54 DecideExec,
// +0x58 DeleteExec, +0x5c IsDpdHit (the audit lists them as "sub+0x1b4 +0x0c.. +0x20").
namespace kingdom::game::g02
{

// one of the six arrows (0 / 1 stage, 2 / 4 flags left / right, 3 / 5 time left / right; DOL vtable 0x8043d6d0, 0x1e4 bytes, ctor 0x8010b7b4)
class cTaskConsMultiMenuArrow : public kingdom::game::cTaskConsMenuBase
{
public:
	cTaskConsMultiMenuArrow(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nIndex, bool bActive);   // DOL 0x8010b7b4
	~cTaskConsMultiMenuArrow() override;                  // DOL 0x8010b914
	bool IsDpdHit(float x, float y) override;             // DOL 0x8010b96c

protected:
	void WaitExec() override;                             // DOL 0x8010bac4
	bool AppearExec() override;                           // DOL 0x8010bb74
	void PointExec() override;                            // DOL 0x8010bb1c
	bool DecideExec() override;                           // DOL 0x8010bc34

private:
	static const int s_AnmTbl[6][4];                      // DOL 0x803772c0 {appear, wait, point, decide}
	static const int s_HitPat[6];                         // DOL 0x80377320
	static const int s_LayPat[6];                         // DOL 0x80377338
	static const int s_LayLayer[6];                       // DOL 0x80377350
	bool m_bActive{};                                     // +0x1d8
	int m_nIndex{};                                       // +0x1dc
	int m_nHitPat{};                                      // +0x1e0
};

// Back (DOL vtable 0x8043d7a0, 0x1dc bytes; ctor 0x8010bcdc = the base ctor, the init 0x8010bd78 follows)
class cTaskConsMultiMenuBack : public cTaskConsMenuBack
{
public:
	explicit cTaskConsMultiMenuBack(kingdom::game::cTaskConsMenuManBase* const& pConsMan);   // DOL 0x8010bcdc
	~cTaskConsMultiMenuBack() override;                   // DOL 0x8010bd20
	void Init();                                          // DOL 0x8010bd78
	int GetAnm(int n) override;                           // DOL 0x8010be88 (table 0x80377430)
};

// the background (DOL vtable 0x8043d880; the init 0x8010bf68 equals cTaskConsMenuBg::SetData)
class cTaskConsMultiMenuBg : public cTaskConsMenuBg
{
public:
	cTaskConsMultiMenuBg(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nStageKey);   // DOL 0x8010becc
	~cTaskConsMultiMenuBg() override;                     // DOL 0x8010bf10
	void Init();                                          // DOL 0x8010bf68
};

// the board behind the settings (DOL vtable 0x8043d960, 0x1d8 bytes)
class cTaskConsMultiMenuBoard : public kingdom::game::cTaskConsMenuBase
{
public:
	explicit cTaskConsMultiMenuBoard(kingdom::game::cTaskConsMenuManBase* const& pConsMan);   // DOL 0x8010c034
	~cTaskConsMultiMenuBoard() override;                  // DOL 0x8010c114

protected:
	bool AppearExec() override;                           // DOL 0x8010c16c
};

// a player's control type button (DOL vtable 0x8043da30, 0x1e8 bytes, ctor 0x8010c234; +0x1e4 = the player)
class cTaskConsMultiMenuController : public cTaskConsMenuController
{
public:
	cTaskConsMultiMenuController(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nType, int nPlayer);   // DOL 0x8010c234
	~cTaskConsMultiMenuController() override;             // DOL 0x8010c288
	void Init();                                          // DOL 0x8010c2e0
	int GetAnm(int n) override;                           // DOL 0x8010c3f4 (table 0x803774e0 [type - 1][player][n])

private:
	static const int s_HitPat[4];                         // DOL 0x803774d0
	static const int s_Anm[2][4][4];                      // DOL 0x803774e0
	int m_nPlayer{};                                      // +0x1e4
};

// the world curtain (DOL vtable 0x8043db30)
class cTaskConsMultiMenuCurtain : public cTaskConsMenuCurtain
{
public:
	cTaskConsMultiMenuCurtain(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nStageKey);   // DOL 0x8010c45c
	~cTaskConsMultiMenuCurtain() override;                // DOL 0x8010c4a0
	void Init();                                          // DOL 0x8010c4f8
	int GetAnm(int n) override;                           // DOL 0x8010c5d4 (table 0x80377598)
};

// the flag count (two digits; DOL vtable 0x8043dc10, 0x1dc bytes, ctor 0x8010c644)
class cTaskConsMultiMenuFlag : public kingdom::game::cTaskConsMenuBase
{
public:
	explicit cTaskConsMultiMenuFlag(kingdom::game::cTaskConsMenuManBase* const& pConsMan);   // DOL 0x8010c644
	~cTaskConsMultiMenuFlag() override;                   // DOL 0x8010c760
	int& nValue() { return m_nValue; }                    // +0x1d8 (the man writes the sys's flag count)

protected:
	void WaitExec() override;                             // DOL 0x8010ca38
	bool AppearExec() override;                           // DOL 0x8010c978
	bool DeleteExec() override;                           // DOL 0x8010ca40

private:
	void SetNumber(int nCol);                             // DOL 0x8010c7b8
	int m_nValue{};                                       // +0x1d8
};

// one player's slot: the number plate, the name, the control type button and the car (DOL vtable 0x8043dda0, 0x210 bytes, ctor 0x8010ecac)
class cTaskConsMultiMenuPlayer : public kingdom::game::cTaskConsMenuBase
{
public:
	// the number plate with the "pointer" mark (DOL vtable 0x8043de70, 0x1e0 bytes, ctor 0x8010e7e8)
	class cTaskPlayerNumber : public kingdom::game::cTaskConsMenuBase
	{
	public:
		cTaskPlayerNumber(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nPlayer);   // DOL 0x8010e7e8
		~cTaskPlayerNumber() override;                    // DOL 0x8010ea50

	protected:
		void WaitExec() override;                         // DOL 0x8010ec08
		bool AppearExec() override;                       // DOL 0x8010eaa8

	private:
		int m_nPlayer{};                                  // +0x1d8
		bool m_bActive{};                                 // +0x1dc (sys +0x10 bit)
	};

	cTaskConsMultiMenuPlayer(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nPlayer, const string& name);   // DOL 0x8010ecac
	~cTaskConsMultiMenuPlayer() override;                 // DOL 0x8010f0f0
	void WaitState() override;                            // DOL 0x8010f148
	bool Execute(cJetTask* pParam) override;              // DOL 0x8010f440
	void Draw(int nStatus, cJetTask* pParam) override;    // DOL 0x8010f364
	void DrawName() const;                                // the font writer's text (the DOL's cTaskFontWriter child, priority 0x80000002)
	cTaskConsMultiMenuController* pController() { return m_pController; }   // +0x1e4
	cTaskPlayerNumber* pNumber() { return m_pNumber; }    // +0x1e0

protected:
	void WaitExec() override;                             // DOL 0x8010f30c
	bool AppearExec() override;                           // DOL 0x8010f1bc

private:
	static const int s_AnmTbl[4][2];                      // DOL 0x803777f8 {appear, wait}
	static const int s_LayLayer[4];                       // DOL 0x80377868
	int m_nPlayer{};                                      // +0x1d8
	string m_Name;                                        // +0x1dc (the matching info)
	cTaskPlayerNumber* m_pNumber{};                       // +0x1e0
	cTaskConsMultiMenuController* m_pController{};        // +0x1e4
	cTaskMenuPacman* m_pCar{};                            // +0x1e8 (cTaskMultiMenuCar)
	Vector4 m_vPos{};                                     // +0x1f0 the name's centre (layout pixels, y up)
	bool m_bShowName{};                                   // +0x200 -> font writer +0x268
};

// Rule (DOL vtable 0x8043df40; the RegistData 0x8010f528 replaces the base one)
class cTaskConsMultiMenuRule : public cTaskConsMenuRule
{
public:
	explicit cTaskConsMultiMenuRule(kingdom::game::cTaskConsMenuManBase* const& pConsMan);   // DOL 0x8010f4e4
	~cTaskConsMultiMenuRule() override;                   // DOL 0x8010f648
	void RegistData() override;                           // DOL 0x8010f528
	int GetAnm(int n) override;                           // DOL 0x8010f60c (table 0x803779c8)
};

// the "single player" button (DOL vtable 0x8043e020, 0x1d8 bytes)
class cTaskConsMultiMenuSingle : public kingdom::game::cTaskConsMenuBase
{
public:
	explicit cTaskConsMultiMenuSingle(kingdom::game::cTaskConsMenuManBase* const& pConsMan);   // DOL 0x8010f6a8
	~cTaskConsMultiMenuSingle() override;                 // DOL 0x8010f7b4
	bool IsDpdHit(float x, float y) override;             // DOL 0x8010f80c

protected:
	void WaitExec() override;                             // DOL 0x8010f950
	bool AppearExec() override;                           // DOL 0x8010f988
	void PointExec() override;                            // DOL 0x8010fa48
	bool DecideExec() override;                           // DOL 0x8010fa80
};

// the stage name between the stage arrows (DOL vtable 0x8043e0f0, 0x1dc bytes)
class cTaskConsMultiMenuStage : public kingdom::game::cTaskConsMenuBase
{
public:
	explicit cTaskConsMultiMenuStage(kingdom::game::cTaskConsMenuManBase* const& pConsMan);   // DOL 0x8010fb14
	~cTaskConsMultiMenuStage() override;                  // DOL 0x8010fc3c
	int& nStageKey() { return m_nStageKey; }              // +0x1d8 (0..3)

protected:
	void WaitExec() override;                             // DOL 0x8010fc94
	bool AppearExec() override;                           // DOL 0x8010fcec
	bool DeleteExec() override;                           // DOL 0x8010fda0

private:
	int m_nStageKey{};                                    // +0x1d8
};

// START (DOL vtable 0x8043e1c0, 0x1d8 bytes)
class cTaskConsMultiMenuStart : public kingdom::game::cTaskConsMenuBase
{
public:
	explicit cTaskConsMultiMenuStart(kingdom::game::cTaskConsMenuManBase* const& pConsMan);   // DOL 0x8010fe48
	~cTaskConsMultiMenuStart() override;                  // DOL 0x8010ff54
	bool IsDpdHit(float x, float y) override;             // DOL 0x8010ffac

protected:
	void WaitExec() override;                             // DOL 0x801100f0
	bool AppearExec() override;                           // DOL 0x80110128
	void PointExec() override;                            // DOL 0x801101e8
	bool DecideExec() override;                           // DOL 0x80110220
};

// the time (minutes : seconds; DOL vtable 0x8043e290, 0x1dc bytes, ctor 0x801102b4)
class cTaskConsMultiMenuTime : public kingdom::game::cTaskConsMenuBase
{
public:
	explicit cTaskConsMultiMenuTime(kingdom::game::cTaskConsMenuManBase* const& pConsMan);   // DOL 0x801102b4
	~cTaskConsMultiMenuTime() override;                   // DOL 0x80110470
	int& nValue() { return m_nValue; }                    // +0x1d8 (the man writes the sys's time in ms)

protected:
	void WaitExec() override;                             // DOL 0x80110730
	bool AppearExec() override;                           // DOL 0x80110658
	bool DeleteExec() override;                           // DOL 0x80110738

private:
	void SetNumber(int nCol);                             // DOL 0x801104c8
	int m_nValue{};                                       // +0x1d8
};

// HOW TO PLAY pages (DOL vtables 0x8043e490 / 0x8043e3f0, 0x2f0 bytes): 6 pages each of MENU_TEXT_TUTORIAL, see the .cpp
class cTaskConsMultiMenuMotionTutorial : public kingdom::game::menu::cTaskConsMenuTutorialBase
{
public:
	explicit cTaskConsMultiMenuMotionTutorial(kingdom::cTaskCursor* pCursor);   // DOL 0x801107ec
	~cTaskConsMultiMenuMotionTutorial() override;                               // DOL 0x801109fc
};

class cTaskConsMultiMenuCtlTutorial : public kingdom::game::menu::cTaskConsMenuTutorialBase
{
public:
	explicit cTaskConsMultiMenuCtlTutorial(kingdom::cTaskCursor* pCursor);      // DOL 0x80110840
	~cTaskConsMultiMenuCtlTutorial() override;                                  // DOL 0x80110894
};

// Pac-Man's car on a player's slot: the colour body, no Mii head (NOT PORTED: Mii, out of scope). DOL vtable 0x8043e530, 0x228 bytes, ctor 0x80111050.
class cTaskMultiMenuCar : public cTaskMenuPacman
{
public:
	explicit cTaskMultiMenuCar(int nPlayer);              // DOL 0x80111050 (priority 0x10006 + player)
	~cTaskMultiMenuCar() override;                        // DOL 0x801111a4
	void Init() override;                                 // DOL 0x80111274

private:
	int m_nPlayer{};                                      // +0x214
};

// the world map of the chosen stage (4 of them; DOL vtable 0x8043e898, 0x230 bytes, ctor 0x80112830)
class cTaskMultiMenuMap : public cTaskMenuMap
{
public:
	cTaskMultiMenuMap(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nStageKey);   // DOL 0x80112830
	~cTaskMultiMenuMap() override;                        // DOL 0x80112adc
	void RegistData() override;                           // DOL 0x80112898
};

}
