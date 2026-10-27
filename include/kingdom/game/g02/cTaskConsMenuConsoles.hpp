#pragma once

#include "common.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/game/menu/cTaskConsMenuTutorialBase.hpp"

// Rally-X Remix (g02) stage select (job 194) consoles, ported from main.dol (no C# exists for Rally-X). All are kingdom::game::cTaskConsMenuBase
// subclasses (one anita vector at +0x1b4; base state +0x1d0 / step +0x1d4) created by g02::cTaskConsMenuMan (0x80108074). Every layout is
// ANITA_MENU_SELECT_01 (crc 0x2d34b4f3, g02menu.lzs) except the Bg (ANITA_MENU_BG). Every SetAnimNo of these consoles passes frame 0.0; the
// DOL's 4th argument is the reset flag (the port's SetAnimNo(no, reset)).
// DOL vtable layout (offsets from the vtable start): +0x48 WaitExec, +0x4c AppearExec, +0x50 PointExec, +0x54 DecideExec, +0x58 DeleteExec,
// +0x5c IsDpdHit, +0x60 RestartAnim, +0x64 WaitState, +0x68 IsWaitState, +0x6c AppearState, +0x70 AppearEndState, +0x74 IsAppearEndState,
// +0x78 PointState, +0x7c DecideState, +0x80 DecideEndState, +0x84 IsDecideEndState, +0x88 DeleteState, +0x8c DeleteEndState,
// +0x90 IsDeleteEndState, +0x94.. class slots. Point / Decide / Delete not listed for a class are the base ones.
namespace kingdom::game::g02
{

// left / right arrow (DOL vtable 0x8043cbe8, 0x1e4 bytes)
class cTaskConsMenuArrow : public kingdom::game::cTaskConsMenuBase
{
public:
	// DOL 0x80105aa4 (man, index 0 left / 1 right, bActive = the man's newest open world (+0x1d0) != 0)
	cTaskConsMenuArrow(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nIndex, bool bActive);
	~cTaskConsMenuArrow() override;      // DOL 0x80105bf8
	bool bActive() const { return m_bActive; }   // +0x1d8 (read by cTaskConsMenuMan::KeySelect)
	bool IsDpdHit(float x, float y) override;    // DOL 0x80105c50

protected:
	void WaitExec() override;            // DOL 0x80105da8
	bool AppearExec() override;          // DOL 0x80105e58
	void PointExec() override;           // DOL 0x80105e00
	bool DecideExec() override;          // DOL 0x80105f18

private:
	static const int s_AnmTbl[2][4];     // DOL 0x803768d0 {appear, wait, point, decide}
	bool m_bActive{};                    // +0x1d8
	int m_nIndex{};                      // +0x1dc
	int m_nHitPat{};                     // +0x1e0 (DOL 0x8047b8e0)
};

// "Back" button (DOL vtable 0x8043ccb8, 0x1dc bytes)
class cTaskConsMenuBack : public kingdom::game::cTaskConsMenuBase
{
public:
	explicit cTaskConsMenuBack(kingdom::game::cTaskConsMenuManBase* const& pConsMan);   // DOL 0x80105fc0
	~cTaskConsMenuBack() override;       // DOL 0x80106004
	void SetData();                      // DOL 0x8010605c
	virtual int GetAnm(int n);           // DOL 0x8010616c (vtable +0x94; table 0x80376918)
	bool IsDpdHit(float x, float y) override;    // DOL 0x801061a8

protected:
	void WaitExec() override;            // DOL 0x801062ec
	bool AppearExec() override;          // DOL 0x80106350
	void PointExec() override;           // DOL 0x8010644c
	bool DecideExec() override;          // DOL 0x801064b0

protected:   // the multiplayer menu's Back (cTaskConsMultiMenuBack) sets the hit pattern itself
	int m_nHitPat{};                     // +0x1d8 (0x1d)
};

// the best score / time board (DOL vtable 0x8043cd90, 0x1ec bytes; 17 anitas)
class cTaskConsMenuBar : public kingdom::game::cTaskConsMenuBase
{
public:
	explicit cTaskConsMenuBar(kingdom::game::cTaskConsMenuManBase* const& pConsMan);   // DOL 0x80106564
	~cTaskConsMenuBar() override;        // DOL 0x80106940
	void SetStage(int nStageIdx);        // DOL 0x80107164 (stage index 1..24: the record shown)
	void SetWorld(int nWorldNo);         // DOL 0x801071d8

protected:
	void WaitExec() override;            // DOL 0x80106f24
	bool AppearExec() override;          // DOL 0x80106f94
	bool DeleteExec() override;          // DOL 0x801070c0

private:
	void SetScoreAnm(int nCol);          // DOL 0x80106998 (column 0 appear, 1 wait, 2 delete)
	void SetTimeAnm(int nCol);           // DOL 0x80106b88

	int m_nWorldNo{};                    // +0x1d8
	int m_nStageIdx{-1};                 // +0x1dc
	s32 m_nScore{};                      // +0x1e0 (the DOL tests it signed)
	u32 m_nTime{};                       // +0x1e4 ms
	bool m_bNewScore{};                  // +0x1e8
	bool m_bNewTime{};                   // +0x1e9
};

// the background layout (DOL vtable 0x8043ce60, 0x1dc bytes)
class cTaskConsMenuBg : public kingdom::game::cTaskConsMenuBase
{
public:
	cTaskConsMenuBg(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nWorldNo);   // DOL 0x8010723c
	~cTaskConsMenuBg() override;         // DOL 0x80107290
	void SetData();                      // DOL 0x801072e8
	virtual int GetAnm(int n);           // DOL 0x801073ac (vtable +0x94; table 0x80376b10)
	int& nWorldNo() { return m_nWorldNo; }   // +0x1d8 (the man writes it)

protected:
	bool AppearExec() override;          // DOL 0x801073e8

private:
	int m_nWorldNo{};                    // +0x1d8
};

// the control type button (1 = remote only, 2 = Nunchuk; DOL vtable 0x8043cf38, 0x1e4 bytes). Its own Execute adds states 9..12 (switching).
class cTaskConsMenuController : public kingdom::game::cTaskConsMenuBase
{
public:
	cTaskConsMenuController(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nType);   // DOL 0x80107458
	~cTaskConsMenuController() override; // DOL 0x801074ac
	void SetData();                      // DOL 0x80107504
	bool Execute(cJetTask* pParam) override;   // DOL 0x80107aac
	bool IsDpdHit(float x, float y) override;  // DOL 0x80107654
	int nType() const { return m_nType; }      // +0x1dc (1 / 2; DOL 0x80109dcc reads it through the man)

	virtual bool NextExec();             // DOL 0x8010791c (vtable +0x94)
	virtual bool PrevExec();             // DOL 0x801079e4 (+0x98)
	virtual int GetAnm(int n);           // DOL 0x801075e8 (+0x9c; table 0x80376b48)
	virtual void NextState();            // DOL 0x80107cd0 (+0xa0) state 11
	virtual void NextEndState();         // DOL 0x80107c6c (+0xa4) state 12
	virtual bool IsNextEndState();       // DOL 0x80107cbc (+0xa8)
	virtual void PrevState();            // DOL 0x80107ca8 (+0xac) state 9
	virtual void PrevEndState();         // DOL 0x80107c80 (+0xb0) state 10
	virtual bool IsPrevEndState();       // DOL 0x80107c94 (+0xb4)

protected:
	void WaitExec() override;            // DOL 0x80107854
	bool AppearExec() override;          // DOL 0x80107798
	void PointExec() override;           // DOL 0x801078b8

protected:   // cTaskConsMultiMenuController
	int m_nType{};                       // +0x1dc
	int m_nHitPat{};                     // +0x1e0 (0x16)
};

// the world curtain (DOL vtable 0x8043d030, 0x1dc bytes)
class cTaskConsMenuCurtain : public kingdom::game::cTaskConsMenuBase
{
public:
	cTaskConsMenuCurtain(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nWorldNo);   // DOL 0x80107cec
	~cTaskConsMenuCurtain() override;    // DOL 0x80107d40
	void SetData();                      // DOL 0x80107f28
	void SetWorld(int nWorldNo);         // DOL 0x80108004
	virtual int GetAnm(int n);           // DOL 0x80107d98 (vtable +0x94; table 0x80376b98)

protected:
	void WaitExec() override;            // DOL 0x80107e00
	bool AppearExec() override;          // DOL 0x80107e64

protected:   // cTaskConsMultiMenuCurtain
	int m_nWorldNo{};                    // +0x1d8
};

// the "multiplayer" button of the single player select (DOL vtable 0x8043d1b0, 0x1dc bytes); it leads to the matching job 199
class cTaskConsMenuMulti : public kingdom::game::cTaskConsMenuBase
{
public:
	explicit cTaskConsMenuMulti(kingdom::game::cTaskConsMenuManBase* const& pConsMan);   // DOL 0x80109dd8
	~cTaskConsMenuMulti() override;      // DOL 0x80109e1c
	void SetData();                      // DOL 0x80109e74
	virtual int GetAnm(int n);           // DOL 0x80109f84 (vtable +0x94; table 0x80376d80)
	bool IsDpdHit(float x, float y) override;    // DOL 0x80109fc0

protected:
	void WaitExec() override;            // DOL 0x8010a104
	bool AppearExec() override;          // DOL 0x8010a168
	void PointExec() override;           // DOL 0x8010a264
	bool DecideExec() override;          // DOL 0x8010a2c8

private:
	int m_nHitPat{};                     // +0x1d8 (0x1f)
};

// "How to play" button (DOL vtable 0x8043d288, 0x1dc bytes)
class cTaskConsMenuRule : public kingdom::game::cTaskConsMenuBase
{
public:
	explicit cTaskConsMenuRule(kingdom::game::cTaskConsMenuManBase* const& pConsMan);   // DOL 0x8010a37c
	~cTaskConsMenuRule() override;       // DOL 0x8010a4a4
	virtual int GetAnm(int n);           // DOL 0x8010a4fc (vtable +0x94; table 0x80376db8)
	virtual void RegistData();           // DOL 0x8010a3c0 (vtable +0x98)
	bool IsDpdHit(float x, float y) override;    // DOL 0x8010a538

protected:
	void WaitExec() override;            // DOL 0x8010a67c
	bool AppearExec() override;          // DOL 0x8010a6e0
	void PointExec() override;           // DOL 0x8010a7dc
	bool DecideExec() override;          // DOL 0x8010a840

protected:   // cTaskConsMultiMenuRule
	int m_nHitPat{};                     // +0x1d8 (0x18)
};

// one of the six stage buttons of a world (DOL vtable 0x8043d360, 0x1e8 bytes)
class cTaskConsMenuStage : public kingdom::game::cTaskConsMenuBase
{
public:
	cTaskConsMenuStage(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nWorldNo, int nStageNo);   // DOL 0x8010a8f4
	~cTaskConsMenuStage() override;      // DOL 0x8010aaac
	void LoadStageData(int nWorldNo);    // DOL 0x8010b050
	bool IsOpen() const { return m_bOpen; }      // +0x1d9
	int nCurState() { return nState(); }         // +0x1d0 (the man reads it: 4 = pointed)
	bool IsDpdHit(float x, float y) override;    // DOL 0x8010ab04

protected:
	void WaitExec() override;            // DOL 0x8010ac5c
	bool AppearExec() override;          // DOL 0x8010ad8c
	void PointExec() override;           // DOL 0x8010acf4
	bool DecideExec() override;          // DOL 0x8010ae90
	bool DeleteExec() override;          // DOL 0x8010af70

private:
	int Anm(int nCol) const;             // table 0x80376df0 [closed][world][stage][col] (inlined in every *Exec)
	static const int s_AnmTbl[2][4][6][5];   // DOL 0x80376df0
	static const int s_LayPat[6];        // DOL 0x803771b0
	bool m_bVisible{true};               // +0x1d8 (only the boss stage hides when closed)
	bool m_bOpen{true};                  // +0x1d9
	int m_nWorldNo{};                    // +0x1dc
	int m_nStageNo{};                    // +0x1e0
	int m_nHitPat{};                     // +0x1e4 (DAT_80477b28 = 0x14)
};

// the world name (DOL vtable 0x8043d600, 0x1dc bytes)
class cTaskConsMenuWorld : public kingdom::game::cTaskConsMenuBase
{
public:
	explicit cTaskConsMenuWorld(kingdom::game::cTaskConsMenuManBase* const& pConsMan);   // DOL 0x8010b480
	~cTaskConsMenuWorld() override;      // DOL 0x8010b5a8
	int& nWorldNo() { return m_nWorldNo; }   // +0x1d8 (the man writes it)

protected:
	void WaitExec() override;            // DOL 0x8010b600
	bool AppearExec() override;          // DOL 0x8010b658
	bool DeleteExec() override;          // DOL 0x8010b70c

private:
	int m_nWorldNo{};                    // +0x1d8
};

// HOW TO PLAY pages for the remote-only control type 1 (DOL vtable 0x8043d560, 0x2f0 bytes): 7 pages of MENU_TEXT_TUTORIAL rows 1..7
class cTaskConsMenuMotionTutorial : public kingdom::game::menu::cTaskConsMenuTutorialBase
{
public:
	explicit cTaskConsMenuMotionTutorial(kingdom::cTaskCursor* pCursor);   // DOL 0x8010b0f8
	~cTaskConsMenuMotionTutorial() override;                               // DOL 0x8010b308
};

// HOW TO PLAY pages for the Nunchuk control type 2 (DOL vtable 0x8043d4c0, 0x2f0 bytes): 5 pages of MENU_TEXT_TUTORIAL rows 9..13
class cTaskConsMenuCtlTutorial : public kingdom::game::menu::cTaskConsMenuTutorialBase
{
public:
	explicit cTaskConsMenuCtlTutorial(kingdom::cTaskCursor* pCursor);      // DOL 0x8010b14c
	~cTaskConsMenuCtlTutorial() override;                                  // DOL 0x8010b1a0
};

}
