#include "kingdom/game/g04/cAppG04WaniWaniGame.hpp"

#include <cstdio>
#include <cstdlib>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "platform/devtools.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomModelBase.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/game/cTaskConsRetry.hpp"
#include "kingdom/game/g04/cG04WaniWaniSys.hpp"
#include "kingdom/game/g04/cTaskBG.hpp"
#include "kingdom/game/g04/cTaskCamera.hpp"
#include "kingdom/game/g04/cTaskCollision.hpp"
#include "kingdom/game/g04/cTaskEffect.hpp"
#include "kingdom/game/g04/cTaskEnemyManager.hpp"
#include "kingdom/game/g04/cTaskGameConsole.hpp"
#include "kingdom/game/g04/cTaskPacmanBase.hpp"
#include "kingdom/game/g04/cTaskTortoise.hpp"
#include "wii/Anim.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"

namespace kingdom::game::g04
{

cAppG04WaniWaniGame* cAppG04WaniWaniGame::s_pApp = nullptr;

// DOL 0x80091d60 (CSV_GAME, 0x84635999)
void cParamGame::Load()
{
	kingdom::cKingdomCSV csv;
	if (!csv.RegistData("CSV_GAME")) return;
	nAngryTime = csv.S32(0, 1, 30);
	nAngryCount = csv.S32(1, 1, 55);
	nEventTime = csv.S32(2, 1, 2);
	vEventEnemyPos = Vector4(csv.F32(3, 1, 0.0f), csv.F32(3, 2, 0.0f), csv.F32(3, 3, 0.0f), 1.0f);
	for (int set = 0; set < 3; set++)
		for (int i = 0; i < 4; i++)
		{
			u32 c = 0;
			for (int col = 1; col <= 4; col++) c = (c << 8) | ((u32)csv.S32(4 + set * 4 + i, col, 0) & 0xffu);
			nNameColor[set][i] = c;
		}
	for (int i = 0; i < 3; i++) nNameSize[i] = csv.S32(0x10 + i, 1, 0);
	nPlayTime = csv.S32(0x13, 1, 120);
	for (int i = 0; i < 3; i++)
	{
		nMultiTime[i] = csv.S32(0x14 + i * 2, 1, 0);
		nMultiCount[i] = csv.S32(0x15 + i * 2, 1, 0);
	}
}

// DOL 0x80091af8 + 0x80094a58
cTaskPause::cTaskPause()
{
	RegistData("COMMON_ANITA_PAUSE", "COMMON_ANITA_POINTER", nullptr);   // 0x6e1c482e / 0x1a3f6a03 (multiplayer: the matching data)
	nMenuJob() = 0x0e300000u;
}

cTaskPause::~cTaskPause()
{
	Free();
}

// DOL 0x8019a13c (job 227): new 0x330, the six-entry array +0x2bc (0x8019a27c), Init 0x80090fb8, RegistMainTask
void cAppG04WaniWaniGame::JobInit()
{
	cAppG04WaniWaniGame* app = new cAppG04WaniWaniGame();
	app->Init();
	KingdomGlobal::SysCore->RegistMainTask(app);
}

cAppG04WaniWaniGame::cAppG04WaniWaniGame()
{
	SetName("g04::cAppG04WaniWaniGame");
	s_pApp = this;
}

cAppG04WaniWaniGame::~cAppG04WaniWaniGame()
{
	if (s_pApp == this) s_pApp = nullptr;
}

bool cAppG04WaniWaniGame::IsHome()
{
	return (GamePad::Get(0).hold & GamePad::PAD_HOME) != 0;
}

uint cAppG04WaniWaniGame::TimerFrames() const
{
	return m_pTimer != nullptr ? (uint)m_pTimer->nFrames() : 0u;
}

uint cAppG04WaniWaniGame::Score() const
{
	return m_pConsole != nullptr ? m_pConsole->Hits() : 0u;
}

void cAppG04WaniWaniGame::TortoiseReact(int state)
{
	if (m_pTortoise != nullptr) m_pTortoise->SetState(state);
}

void cAppG04WaniWaniGame::CameraShake(int mode)
{
	if (m_pTaskCamera != nullptr) m_pTaskCamera->SetMode(mode);
}

// DOL 0x80093ae0
void cAppG04WaniWaniGame::BuildEffect()
{
	m_pEffect = new cTaskEffect();
	m_pEffect->nDrawPriority(0x10003u);
	pTaskPtkMan() = m_pEffect;   // KingSys +0xa40 -> +0x1bc
	ConnectChild(m_pEffect);
	m_pFlare = new cTaskEffectRensFlear(m_pTaskBG->RenseNode());   // +0x2ac at rense_set (0x8019d238)
	ConnectChild(m_pFlare);
	ConnectChild(new cTaskEffectBird(m_pTaskBG->BirdNode()));   // the birds at bird_set (0x8019d1f4)
}

// DOL 0x80093260
void cAppG04WaniWaniGame::BuildTortoise()
{
	m_pTortoise = new cTaskTortoise(m_pTaskBG->KameNode());
	ConnectChild(m_pTortoise);
}

// DOL 0x800933f4
void cAppG04WaniWaniGame::BuildEnemyManager()
{
	m_pEnemyManager = new cTaskEnemyManager(m_pTaskBG);
	ConnectChild(m_pEnemyManager);
}

// DOL 0x80093624
void cAppG04WaniWaniGame::BuildPacmanManager()
{
	m_pPacmanManager = new cTaskPacmanManager(m_pTaskBG);
	m_pPacmanManager->SetPlayers((uint)KingdomGlobal::WaniWaniSys->nPlayers());   // 0x801b6550
	// multiplayer: each Pac-Man's Mii listener = the Mii icon manager +0x1b4: NOT PORTED
	ConnectChild(m_pPacmanManager);
}

// DOL 0x80093920
void cAppG04WaniWaniGame::BuildCollision()
{
	m_pCollision = new cTaskCollision(m_pEnemyManager, m_pPacmanManager);
	ConnectChild(m_pCollision);
}

// DOL 0x800937bc (+0x1c4 = the Pac-Man manager; priority 0x10002)
void cAppG04WaniWaniGame::BuildConsole()
{
	m_pConsole = new cTaskGameConsole();
	m_pConsole->nDrawPriority(0x10002u);
	ConnectChild(m_pConsole);
}

// DOL 0x800940b8
void cAppG04WaniWaniGame::BuildTimer()
{
	m_pTimer = new cTaskConsTimer();
	ConnectChild(m_pTimer);
}

// DOL 0x800943cc (multiplayer: by the chosen play time)
void cAppG04WaniWaniGame::SetAngry()
{
	if (KingdomGlobal::WaniWaniSys->nPlayers() == 1)
	{
		m_nAngryTime = m_Param.nAngryTime;
		m_nAngryCount = (uint)m_Param.nAngryCount;
	}
	else
	{
		const int f = m_pTimer->nFrames();
		const int k = f == 0xe10 ? 0 : (f == 0x2a30 ? 1 : (f == 18000 ? 2 : -1));
		if (k >= 0)
		{
			m_nAngryTime = m_Param.nMultiTime[k];
			m_nAngryCount = (uint)m_Param.nMultiCount[k];
		}
	}
}

// DOL 0x80090fb8
void cAppG04WaniWaniGame::Init()
{
	m_bNewRecord = false;
	m_bFromRetry = false;
	KingdomGlobal::WaniWaniSys->LoadRecord();   // 0x80199dfc
	kingdom::cKingdomModelBase::ClearAssetCache();
	// the scene root +0x1b8 (cTaskScnRoot 0x80, 0x80, 0x80, 0x10; priority 0x7fffffff): natively cTaskNativeScene, registered drawing
	kingdom::NativeScene::Reset();
	ConnectChild(new kingdom::cTaskNativeScene());
	m_pTaskCamera = new cTaskCamera();   // +0x2a4 (0x8019d314)
	ConnectChild(m_pTaskCamera);
	// the light manager +0x2b8 (0x80018dd0 / 0x800195c4) and the fog manager +0x2b4 (0x8001688c / 0x80016ac4): sets 0..5
	static const char* const SCA[6] = {"BG_SCA", "CHA_SCA", "CHA_A_SCA", "CHA_B_SCA", "CHA_C_SCA", "CHA_D_SCA"};
	for (int i = 0; i < 6; i++)
	{
		std::size_t size = 0;
		const u8* d = wii::ArchiveStore::Find(SCA[i], size);
		wii::Brres b;
		wii::ScnAnim a;
		wii::LightResult light;
		if (d != nullptr && b.Load(d, size) && a.Load(b) && a.SampleLight(0, light))
		{
			a.SampleFog(light);
			kingdom::NativeScene::SetLight(i, light);
		}
	}
	// KingSys +0xa54 FUN_80025560(500): the sound fade (natively nothing)
	m_pTaskFade = new kingdom::cTaskFade();   // +0x290 (0x80091964)
	ConnectChild(m_pTaskFade);
	m_pTaskBG = new cTaskBG();                // +0x284 (0x800931c0)
	m_pTaskBG->nDrawPriority(0x10005u);
	ConnectChild(m_pTaskBG);
	BuildEffect();          // 0x80093ae0
	// 0x80094250 cTaskMiiIconManager (+0x2b0): multiplayer, NOT PORTED
	BuildTortoise();        // 0x80093260
	BuildEnemyManager();    // 0x800933f4
	BuildPacmanManager();   // 0x80093624
	BuildCollision();       // 0x80093920
	BuildConsole();         // 0x800937bc
	BuildTimer();           // 0x800940b8
	m_pPause = new cTaskPause();   // +0x2a8 (multiplayer: with the matching data)
	ConnectChild(m_pPause);
	for (uint i = 0; i < 4; i++) m_pPacmanManager->pPacman(i)->SetEnemyManager(m_pEnemyManager);   // 0x80091d28
	const int n = KingdomGlobal::WaniWaniSys->nPlayers();
	for (int i = 0; i < n && i < 4; i++) m_pPacmanManager->pPacman((uint)i)->SetScoreListener(m_pConsole->pListener());   // 0x80091d40 (+0x1d8 +0x1c8; more than one player: the multiplayer board +0x1e8)
	m_pPacmanManager->SetPlayers((uint)n);
	m_Param.Load();   // cParamGame (0x80091d60) +0x314
	// the scene root's camera 0: the near / far planes and the projection of the cParamCameraSetting (0x800242f0 .. 0x800244dc) are the
	// camera task's (natively NativeScene::SetCamera)
	m_nFlow = 0;
	if (n == 1)
	{
		m_nAngryTime = m_Param.nAngryTime;
		m_nAngryCount = (uint)m_Param.nAngryCount;
		KingdomGlobal::WaniWaniSys->bNewRecord() = false;
		nModeState(m_nFlow) = KingdomGlobal::WaniWaniSys->bRetry() ? 4u : 0u;
	}
	KingdomGlobal::WaniWaniSys->bRetry() = false;   // 0x80199f5c-like (+0x164 = 0)
	m_nNextPlayers = n;
}

// DOL 0x80094480 (partial)
void cAppG04WaniWaniGame::Free()
{
	kingdom::NativeScene::Reset();
	cAppKingdom::Free();
}

// the start of 0x8009219c: any player's MINUS / HOME trigger opens the pause outside the fades / exit (states 0, 1, 8, 9)
void cAppG04WaniWaniGame::PauseCheck()
{
	const uint st = nModeState(m_nFlow);
	const int n = KingdomGlobal::WaniWaniSys->nPlayers();
	for (int i = 0; i < n && i < 4; i++)
		if ((GamePad::Get(i).trig & (GamePad::PAD_MINUS | GamePad::PAD_HOME)) != 0)
		{
			if (st - 8u < 2u || st < 2u) m_bPause = false;
			else
			{
				m_pPause->Start();   // 0x80147c8c
				m_bPause = true;
			}
		}
	if (st - 8u < 2u || st < 2u) m_bPause = false;
	else m_bPause = m_pPause->IsPause();   // +0x1d4
}

// DOL 0x8009219c
bool cAppG04WaniWaniGame::Execute(cJetTask* pParam)
{
	(void)pParam;
	PauseCheck();
	if (m_bPause || IsHome()) return true;
	const uint n = m_nFlow;
	if (std::getenv("MEGAMIX_G04DBG") && nModeState(n) != m_nDbgMode) { std::fprintf(stderr, "g04 app mode %u -> %u\n", m_nDbgMode, nModeState(n)); m_nDbgMode = nModeState(n); }
	const bool single = KingdomGlobal::WaniWaniSys->nPlayers() == 1;
	static int s_dbgFrame = 0;
	if (std::getenv("MEGAMIX_G04DBG") && ++s_dbgFrame % 60 == 0 && m_pConsole != nullptr)
	{
		std::fprintf(stderr, "g04 f%d timer %d hits %u score %u gators", s_dbgFrame, m_pTimer->nFrames(), m_pConsole->Hits(), m_pConsole->pScore()->Score());
		for (uint i = 0; i < 4; i++) std::fprintf(stderr, " %d", m_pEnemyManager->pEnemy(i)->nState());
		std::fprintf(stderr, " pac %d\n", m_pPacmanManager->pPacman(0)->nState());
	}
	auto change = [&](uint s) { NextMode(n, s); };
	const uint st = nModeState(n);
	nOldState(n) = st;
	// jump table 0x803940b8 (bctr 0x800922b8; it follows cAnimeBase's vtable, so vtable_audit lists its words as cAnimeBase slots):
	// case 0 0x800922bc, 1 0x8009243c, 2 0x800925f8, 3 0x80092874, 4 0x80092374, 5 0x800926a0, 6 0x80092a80, 7 0x80092b40,
	// 8 0x80092cb0, 9 0x80093044, 10 0x80092f98, 11 0x80092e58
	switch (st)
	{
	case 0:   // fade in
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			m_pEnemyManager->SetAllState(1);    // 0x801af5d8(+0x288, 1)
			m_pPacmanManager->SetAllState(1);   // 0x801b64e0(+0x298, 1)
			m_pTaskFade->Fade(2u, 500);
		}
		if (nModeStep(n) == 1)
		{
			if (m_pTaskFade->IsEnd()) change(1);
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
		}
		nModeStep(n) = 0;
		break;
	case 1:   // the menu
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			// 0x800219d4(1): the pointer on (natively always)
			m_pTortoise->SetState(0);   // 0x801b7c7c(+0x280, 0)
			if (m_SndMenu.voice < 0 || !snd::IsPlaying(m_SndMenu.voice)) WiiSnd::Play(m_SndMenu, "m04bgm_Select");   // 0x80026424 / 0x80199ee8
			m_pEnemyManager->SetAllState(1);
			m_pConsole->SetMode(3);
		}
		if (nModeStep(n) == 1)
		{
			if (m_pConsole->bMenuDecided())
			{
				if (m_pConsole->MenuIsSel(0))   // back: the hub
				{
					m_nNextJob = 0x0ef00000u;
					change(9);
				}
				else if (m_pConsole->MenuIsSel(2))   // start
					change(2);
				else if (m_pConsole->MenuIsSel(1))   // how to play
					m_pConsole->SetMode(2);
				else if (m_pConsole->MenuIsSel(3))   // players: the matching (alone) / the Nunchuk card
				{
					if (single) m_nNextJob = 0x0e500000u;
					else
					{
						m_nNextPlayers = 1;
						m_nNextJob = 0x0e700000u;
					}
					change(9);
				}
			}
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
		}
		nModeStep(n) = 0;
		break;
	case 2:   // the menu closes
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			m_pConsole->StartGame();   // 0x801b1c20
		}
		if (nModeStep(n) == 1)
		{
			if (m_pConsole->MenuClosed()) change(5);   // 0x801b1c0c (the DOL also sets 5 unconditionally first)
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
		}
		nModeStep(n) = 0;   // 0x800219d4(0): the pointer off
		break;
	case 3:   // the game
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			m_nCounter = 0;
			m_pEnemyManager->SetMode(1);        // 0x801af8d4(+0x288, 1)
			m_pEnemyManager->SetAllState(5);    // 0x801af5d8(+0x288, 5)
			m_pConsole->SetMode(4);
			m_pTimer->SetMode(2);               // 0x801a7be4
			m_pFlare->Start();                  // 0x801a9064(+0x2ac)
			m_pTortoise->SetState(3);
			SetAngry();                         // 0x800943cc
		}
		if (nModeStep(n) == 1)
		{
			if (m_pTimer->nFrames() < 0) change(6);
			if ((int)(m_nAngryTime * 0x3c) == m_pTimer->nFrames())
			{
				if (m_nAngryCount <= m_pConsole->Hits())
				{
					if (!m_pEnemyManager->bAngry())   // +0x234 (the event once)
					{
						WiiSnd::FadeOut(m_SndGame, 0);   // 0x80026318(+0x2c8, 0)
						WiiSnd::Play(m_SndGame, "m04bgm_Game2");
						WiiSnd::Handle h;
						WiiSnd::Play(h, "m04se_02okotta");
						WiiSnd::Detach(h);
					}
					m_pTimer->bRun() = false;           // +0x1c8
					m_pEnemyManager->SetMode(3);        // the big gator (state 0x13)
					m_pPacmanManager->SetAllState(6);
					m_pTaskCamera->SetMode(2);          // the zoom
				}
				if ((uint)(m_Param.nEventTime * 0x3c) <= m_nCounter)
				{
					m_pTimer->bRun() = true;
					m_pTaskCamera->SetMode(0);
					m_pEnemyManager->EndAngry();         // 0x801afed8
					m_pPacmanManager->SetAllState(2);
				}
				m_nCounter++;
			}
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
		}
		nModeStep(n) = 0;
		WiiSnd::FadeOut(m_SndGame, 400);   // 0x80026394(0, +0x2c8, 400)
		break;
	case 4:   // from the retry: fade in, then READY
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			m_bFromRetry = true;
			m_pEnemyManager->SetAllState(1);
			m_pTaskFade->Fade(2u, 500);
			m_pPacmanManager->SetAllState(1);
			m_pConsole->SetMode(1);
		}
		if (nModeStep(n) == 1)
		{
			if (m_pTaskFade->IsEnd()) change(5);
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
		}
		nModeStep(n) = 0;
		break;
	case 5:   // READY / GO
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			WiiSnd::FadeOut(m_SndGame, 0);
			WiiSnd::FadeOut(m_SndMenu, 0);
			WiiSnd::Play(m_SndStart, "m04se_12start");
			if (!m_bFromRetry) m_pConsole->SetMode(1);
			m_pTimer->SetMode(1);
			m_pTimer->SetTime(single ? (uint)(m_Param.nPlayTime * 0x3c) : (uint)(m_pConsole->MultiTime() * 0x3c));   // 0x801a7bec
			if (devtools::G04Time() > 0) m_pTimer->SetTime((uint)(devtools::G04Time() * 0x3c));   // devtools: --g04-time
			m_nCounter = 0;
			KingdomGlobal::WaniWaniSys->bNewRecord() = false;   // +0x165
		}
		if (nModeStep(n) == 1)
		{
			if (!(m_SndStart.voice >= 0 && snd::IsPlaying(m_SndStart.voice)))   // 0x80026424(+0x2d4)
			{
				if (m_nCounter == 0)
				{
					if (!(m_SndEat.voice >= 0 && snd::IsPlaying(m_SndEat.voice))) WiiSnd::Play(m_SndEat, "m04se_01tabetyau");
					m_pTortoise->SetState(1);
					m_pConsole->pReadly()->Ready();   // 0x801a461c(+0x294 +0x1dc)
				}
				if (m_nCounter == 0x78)
				{
					WiiSnd::Play(m_SndGame, "m04bgm_Game1");
					m_pConsole->pReadly()->SetMode(2);   // GO
					m_pPacmanManager->SetAllState(2);
				}
				else if (m_nCounter == 0xb4)
					change(3);
				m_nCounter++;
			}
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
		}
		nModeStep(n) = 0;
		break;
	case 6:   // time up
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			m_pFlare->Stop();                   // 0x801a90c0(+0x2ac)
			WiiSnd::Play(m_SndResult, "m04se_15result");
			m_pEnemyManager->SetAllState(0);
			m_pEnemyManager->SetMode(0);
			m_pPacmanManager->SetAllState(7);
		}
		if (nModeStep(n) == 1)
		{
			if (!(m_SndResult.voice >= 0 && snd::IsPlaying(m_SndResult.voice))) change(7);
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
		}
		nModeStep(n) = 0;
		break;
	case 7:   // the result
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			const int rank = m_pConsole->ResultRank();   // 0x801b1afc
			m_pEnemyManager->SetResultLook(rank);         // 0x801afdec
			m_pEnemyManager->SetMode(2);                  // every gator 0x11: gator A comes out
			m_pTortoise->SetState(10);
			m_pPacmanManager->Result((uint)rank);         // 0x801b6668
			if (single && (uint)KingdomGlobal::WaniWaniSys->nHighScore() <= m_pConsole->pScore()->Score())
				KingdomGlobal::WaniWaniSys->bNewRecord() = true;   // +0x165
		}
		if (nModeStep(n) == 1)
		{
			if (m_pTortoise->bTurned())   // +0x2d1
			{
				m_pConsole->SetMode(5);
				m_pTortoise->Result(m_pConsole->pResult()->nRank());   // 0x801b86b8(+0x280, result +0x1cc)
				m_nCounter = 0;
			}
			if (m_pConsole->bResultDone()) change(single ? 8u : 0xbu);   // +0x209
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
		}
		nModeStep(n) = 0;
		break;
	case 8:   // Play again?
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			m_pConsole->pRetry()->Start();   // 0x80149b88(+0x294 +0x1ec)
			WiiSnd::FadeOut(m_SndGame, 1000);
		}
		if (nModeStep(n) == 1)
		{
			kingdom::game::cTaskConsRetry* r = m_pConsole->pRetry();
			if (r->IsYes() && !r->IsVisible())   // +0x1d0 == 1 && +0x1c8 == 0
			{
				m_nNextJob = 0x0e300000u;
				KingdomGlobal::WaniWaniSys->bRetry() = true;
				change(single ? 9u : 1u);
			}
			else if (r->IsNo() && !r->IsVisible())
			{
				KingdomGlobal::WaniWaniSys->bRetry() = false;
				m_nNextJob = 0x0e300000u;
				change(single ? 9u : 1u);
			}
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
		}
		nModeStep(n) = 0;
		if (single && (uint)KingdomGlobal::WaniWaniSys->nHighScore() <= m_pConsole->pScore()->Score())
		{
			m_bNewRecord = true;
			KingdomGlobal::WaniWaniSys->SetHighScore((int)m_pConsole->pScore()->Score());
		}
		break;
	case 9:   // fade out, then the next job
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			// m_nNextJob != matching: KingSys +0xa54 FUN_80025560(500) (the sound fade)
			WiiSnd::FadeOut(m_SndMenu, 500);
			WiiSnd::FadeOut(m_SndGame, 500);
			m_pTaskFade->Fade(0x10002u, 500);
		}
		if (nModeStep(n) == 1)
		{
			if (m_pTaskFade->IsEnd())
			{
				KingdomGlobal::WaniWaniSys->SaveRecord();   // 0x80199db4
				KingdomGlobal::WaniWaniSys->SetPlayers(m_nNextPlayers);   // +0x15c = +0x308
				Kingdom_SetJobNo(m_nNextJob, m_bNewRecord, false);       // 0x8000db5c(+0x318, +0x304, 0)
			}
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
		}
		nModeStep(n) = 0;
		break;
	case 10:   // multiplayer: the gators dive, the Pac-Mans' round restarts (-> the menu, 1)
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			m_pEnemyManager->SetAllState(0x17);
			m_pPacmanManager->SetAllState(0xb);
		}
		if (nModeStep(n) == 1)
		{
			if (m_pEnemyManager->pEnemy(0)->nState() != 0x17) change(1);   // +0x1bc -> +0x1d0
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
		}
		nModeStep(n) = 0;
		m_pEnemyManager->Reset();     // 0x801aec9c
		m_pPacmanManager->Restart();  // 0x801b63b0
		break;
	case 0xb:   // multiplayer: the result board leaves (a second's wait), everything resets
		if (nModeStep(n) == 0)
		{
			nModeStep(n) = 1;
			m_pConsole->SetMode(8);
			// the Mii icon manager's per-player call (+0x2b0 vtable +0x68): NOT PORTED
			m_pTimer->SetMode(3);
		}
		if (nModeStep(n) == 1)
		{
			if (++m_nCounter > 0x3b) change(10);
			if (nOldState(n) == nModeState(n)) break;
			nModeStep(n) = 2;
		}
		nModeStep(n) = 0;
		m_pTortoise->Reset();     // 0x801b7810
		m_pConsole->Restart();    // 0x801b13b4
		m_pTimer->Reset();        // 0x801a70ac
		break;
	default:
		break;
	}
	return true;
}

}
