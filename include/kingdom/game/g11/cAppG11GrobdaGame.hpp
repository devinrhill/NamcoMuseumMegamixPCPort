#pragma once

#include "common.hpp"
#include "kingdom/cAppKingdom.hpp"

// Grobda Remix (g11) stage = job 339 (0x15300000): kingdom::game::g11::cAppG11GrobdaGame (0x2a0 bytes; job init FUN_8000f338, ctor FUN_8009f8f0,
// Execute FUN_8009fdf4, dtor FUN_8009fd3c). Ported: the stage archive, the effect task (+0x298, 0xd50, cTaskGrobdaEffect), cTaskGrobdaStage. Execute (pause, record, end job).
namespace kingdom::game::g11
{
class cTaskGrobdaStage;
class cTaskGrobdaPause;
class cTaskGrobdaConsoleManager;

class cAppG11GrobdaGame : public kingdom::cAppKingdom
{
public:
	static void JobInit();             // DOL 0x8000f338
	cAppG11GrobdaGame();               // DOL 0x8009f8f0 (partial)
	void Free() override;              // DOL 0x8009fd3c (partial)
	bool Execute(cJetTask* pParam) override;   // DOL 0x8009fdf4
	int nScore(int i) const { return m_nScore[i]; }   // +0x280 + 4 i
	void AddScore(int player, int pts, cTaskGrobdaConsoleManager* console);   // DOL 0x8009ffcc
	void StartPlayWatch() { StopWatchReset(0); StopWatchStart(0); }   // DOL 0x8009ff68 

private:
	int m_nStage{};                    // +0x290 (the sys' +0x20)
	cTaskGrobdaStage* m_pStage{};      // +0x294
	cTaskGrobdaPause* m_pPause{};      // +0x29c
	int m_nScore[4]{};                 // +0x280 (score records written back by Execute; the console that fills them is NOT PORTED)
};
}
