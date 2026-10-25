#pragma once

#include "WiiSnd.hpp"
#include "common.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/game/g02/cTaskUnit.hpp"

namespace kingdom { class cTaskFade; }
namespace kingdom::game { class cTaskConsRetry; }
namespace kingdom::game::g02 { class cTaskMap; class cTaskConsole; class cTaskPause; class cParamGame; class cTaskEffect; }

// Rally-X Remix (g02) stage = job 195 (0x0c300000): kingdom::game::g02::cAppG02RallyXGame (0x338 bytes; job init FUN_800c677c, ctor FUN_801236bc,
// Execute FUN_80125b1c, Draw FUN_80126908 (applies the next state), dtor FUN_80127448). The app is also the cars' host (cRallyUnitHost: the DOL
// unit reaches it through its parent, +0x14).
// Ported: stage archives, light sets, pause, CSV_GAME, fade, console, map, BG, the cars (players, enemies, red modes), the car cameras, retry and
// the whole state machine (single player). NOT PORTED: multiplayer (the time limit,
// states 8 / 9, the multi console), split screen, BGM / alarm volume fades (WiiSnd has none), the BG reset FUN_8006016c, smoke / flag drop /
// gimmick contact on the map side (Wave 2).
namespace kingdom::game::g02
{
class cAppG02RallyXGame : public kingdom::cAppKingdom, public cRallyUnitHost
{
public:
	static void JobInit();             // DOL 0x800c677c
	cAppG02RallyXGame();               // DOL 0x801236bc
	void Free() override;              // DOL 0x80127448 (partial)
	bool Execute(cJetTask* pParam) override;   // DOL 0x80125b1c
	void Draw(int nStatus, cJetTask* pParam) override;   // DOL 0x80126908 (+0x2e4 = +0x2f0, 0xc bytes: the state change takes effect here)

	// cRallyUnitHost (DOL: the app's members and FUN_801269xx helpers)
	int nUnitGameState() const override { return m_nState; }          // +0x2e4
	bool IsUnitPause() const override { return const_cast<cAppG02RallyXGame*>(this)->IsPause(); }   // +0x1b4
	bool IsUnitStageEnd() const override { return m_bStageEnd; }       // +0x284
	int nUnitStageEndId() const override { return m_nStageEndId; }     // +0x288
	cTaskUnit* pUnit(int i) const override { return (unsigned)i < 7u ? m_pCars[i] : nullptr; }   // DOL 0x80126918
	cTaskMap* pUnitMap() const override { return m_pTaskMap; }         // +0x298
	void UnitAddScore(int add) override { Score(add, false); }          // DOL 0x80126a10(app, add, 0)
	bool UnitStageEnd(bool b, int unitId) override;                     // DOL 0x80126a54
	void UnitRedCrash() override;                                       // DOL 0x80126ac8
	int MapFlagTotal() const override;                                  // DOL 0x80079ad0
	void FxFlagScore(int count, int special, const Vector4& pos) override;
	void FxRedFlag(const Vector4& pos) override;
	void MapContact(cTaskUnit* u, int x, int z) override;              // DOL 0x80079b50
	bool MapDropFlag(const float* pos, int dir) override;              // DOL 0x80079b5c(map, 1, pos, dir)
	bool MapPutSmoke(int unitId, const float* pos, int index) override;   // DOL 0x80079c1c(map, id, 3, pos, index)
	bool MapSmokeFree(int unitId) override;                            // DOL 0x80079fc4
	bool MapNextFlag(int index, float out[4]) const override;          // DOL 0x80079fcc

private:
	bool Score(int v, bool bSet);              // DOL 0x80126a10
	void ResetCars();                          // DOL 0x80125a14
	void StateClear();                         // DOL 0x80126b18 (state 7)
	void NextState(int n) { m_nNextState = n; m_nNextStep = 0; }   // memset(+0x2f0, 0, 0xc); +0x2f0 = n
	void NextStep(int n) { m_nNextStep = n; }                      // memset(+0x2f4, 0, 8); +0x2f4 = n
	int nPlayers() const;                      // sys +0x0c (0x800c5c68)
	void PlayBgm(const char* cue);             // FUN_80129d78(+0x28c, cue, 0, 0)
	void StartPause();

	cParamGame* m_pParam{};              // +0x280 (CSV_GAME)
	bool m_bStageEnd{};                  // +0x284
	s32 m_nStageEndId{};                 // +0x288
	WiiSnd::Handle m_SndBgm;             // +0x28c
	kingdom::cTaskFade* m_pTaskFade{};   // +0x2a4 (cTaskRallyXFade, 0x620 bytes, priority 0x8fffffef, fade in 2 / 500)
	cTaskConsole* m_pConsole{};          // +0x2a8
	cTaskPause* m_pPause{};              // +0x2ac
	s32 m_nCamOn[4]{1, 1, 1, 1};         // +0x2b0 (the ScnRoot camera i is driven by car i)
	cTaskUnit* m_pCars[7]{};             // +0x2c4 (players first, then the enemies)
	s32 m_nState{};                      // +0x2e4
	s32 m_nStep{};                       // +0x2e8
	s32 m_nNextState{};                  // +0x2f0
	s32 m_nNextStep{};                   // +0x2f4
	float m_fMissTime{};                 // +0x2fc
	s32 m_nScore{};                      // +0x300
	s32 m_nNextJob{0xc100000};           // +0x30c
	bool m_bNextFlag{};                  // +0x314
	kingdom::game::cTaskConsRetry* m_pRetry{};   // +0x310
	float m_fPlayTime{};                 // +0x318 (ms)
	float m_fCount{};                    // +0x31c (frames)
	WiiSnd::Handle m_SndAlarm;           // +0x320
	s32 m_nRest{};                       // +0x32c (lives left)
	float m_fWait330{};                  // +0x330
	s32 m_nScnLayout{};                  // cTaskScnRoot +0x1b8 (table 0x80378a58 {0, 0, 4, 7, 7} by the player count)
	kingdom::game::g02::cTaskMap* m_pTaskMap{};   // +0x298
	cTaskEffect* m_pEffect{};            // +0x1bc
	bool m_bBgmFading{};
};
}
