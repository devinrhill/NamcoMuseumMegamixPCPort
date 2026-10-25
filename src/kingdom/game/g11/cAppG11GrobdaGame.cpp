#include "kingdom/game/g11/cAppG11GrobdaGame.hpp"

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomModelBase.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"
#include "kingdom/game/g11/cTaskGrobdaEffect.hpp"
#include "kingdom/game/g11/cTaskGrobdaConsole.hpp"
#include "kingdom/game/g11/cTaskGrobdaStage.hpp"
#include "kingdom/game/g11/cTaskGrobdaPause.hpp"
#include "GamePad.hpp"
#include "kingdom/KingdomJob.hpp"

namespace kingdom::game::g11
{

// DOL 0x8000f338 (job 339)
void cAppG11GrobdaGame::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG11GrobdaGame());
}

// DOL 0x8009f8f0 (partial, see the header)
cAppG11GrobdaGame::cAppG11GrobdaGame()
{
	SetName("cAppG11GrobdaGame");
	m_nStage = KingdomGlobal::GrobdaSys->nStage();
	KingdomGlobal::KingSys->AttachArcData(cG11GrobdaSys::StageArc(m_nStage), true);
	kingdom::cKingdomModelBase::ClearAssetCache();
	// cTaskScnRoot (0x200, 0x200, 0x80, 0x10) at +0x1b8: the native scene stands in
	pTaskPtkMan() = new cTaskGrobdaEffect();   // +0x298 (also the global +0xa40 -> +0x1bc; the stage connects it as its child)
	ConnectChild(pTaskPtkMan());
	m_pStage = new cTaskGrobdaStage();   // +0x294
	ConnectChild(m_pStage);
	m_pPause = new cTaskGrobdaPause();   // +0x29c
	ConnectChild(m_pPause);
}

void cAppG11GrobdaGame::Free()
{
	KingdomGlobal::KingSys->DetachArcData(cG11GrobdaSys::StageArc(m_nStage));
	kingdom::NativeScene::Reset();
	cAppKingdom::Free();
}

// DOL 0x8009ffcc: the score of the player, clamped to 0 .. 999999 (99999 in multiplayer), then the console's Score
void cAppG11GrobdaGame::AddScore(int player, int pts, cTaskGrobdaConsoleManager* console)
{
	if (player < 0 || player > 3) return;
	m_nScore[player] += pts;
	const int cap = KingdomGlobal::GrobdaSys->IsMulti() ? 99999 : 999999;
	if (m_nScore[player] < 0) m_nScore[player] = 0;
	else if (m_nScore[player] > cap) m_nScore[player] = cap;
	if (console != nullptr && console->pScore(player) != nullptr) console->pScore(player)->SetScore(m_nScore[player]);   // 0x80130ccc
}

// DOL 0x8009fdf4
bool cAppG11GrobdaGame::Execute(cJetTask* pParam)
{
	(void)pParam;
	const int nPlayer = KingdomGlobal::GrobdaSys->nPlayer();   // UNVERIFIED: the DOL loops the pad count of the sys
	bool* paused = &IsPause();   // app +0x1b4
	for (int i = 0; i < nPlayer; i++)
		if ((GamePad::Get(i).trig & 0x500u) != 0 && !*paused)
		{
			m_pPause->Start();   // FUN_80147c8c
			break;
		}
	*paused = m_pPause->IsPause();   // app +0x1b4 = pause +0x1d4
	m_pStage->SetPaused(*paused);
	if (!*paused && (GamePad::Get(0).hold & 0x400u) == 0)   // HOME held = HBM open
	{
		if (!StopWatchState(0)) StopWatchStart(0);
		if (m_pStage->IsEnded())
		{
			cG11GrobdaSys* sys = KingdomGlobal::GrobdaSys;
			if (!sys->IsMulti())
				sys->RecordSingle(m_nScore[0]);   // FUN_8000edbc(sys, app +0x280)
			else
				for (int i = 0; i < nPlayer; i++) sys->RecordMulti(i, m_nScore[i]);   // FUN_8000ee10
			Kingdom_SetJobNo(0x15200000u, true, false);
		}
	}
	else if (StopWatchState(0)) StopWatchStop(0);
	return true;
}

}
