#include "kingdom/game/g02/cAppG02RallyXGame.hpp"

#include "SoundManager.hpp"

#include <algorithm>
#include <cstdlib>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomModelBase.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cTaskBG.hpp"
#include "kingdom/game/g02/cTaskMap.hpp"
#include "kingdom/game/g02/cParamGame.hpp"
#include "kingdom/game/g02/cTaskConsole.hpp"
#include "kingdom/game/g02/cTaskPause.hpp"
#include "kingdom/game/g02/cTaskRallyXFade.hpp"
#include "kingdom/game/g02/cCamera.hpp"
#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/game/g02/cTaskEffect.hpp"
#include "kingdom/game/g02/cTaskEnemy.hpp"
#include "kingdom/game/g02/cTaskFlagMan.hpp"
#include "kingdom/game/g02/cTaskGimmick.hpp"
#include "kingdom/game/g02/cTaskItem.hpp"
#include "kingdom/game/g02/cTaskSmokeMan.hpp"
#include "kingdom/game/g02/cTaskPlayer.hpp"
#include "kingdom/game/cTaskConsRetry.hpp"
#include "kingdom/KingdomJob.hpp"
#include "GamePad.hpp"
#include "snd/Mixer.hpp"
#include "wii/Anim.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g02
{

namespace
{
// Native glue: the gimmicks (cTaskGimmick.hpp / cTaskItem.hpp) and the flag / smoke managers were ported as separate file sets that reach each
// other through hook tables (the DOL calls the members directly). The map builds its managers in its ctor, so this runs before new cTaskMap.
void InstallMapHooks()
{
	cTaskMap::sHooks& m = cTaskMap::Hooks;
	m.NewFlagMan = [](cTaskMap* map) { cTaskFlagMan* fm = new cTaskFlagMan(); map->ConnectChild(fm); return fm; };
	m.NewSmokeMan = [](cTaskMap* map) { cTaskSmokeMan* sm = new cTaskSmokeMan(); map->ConnectChild(sm); return sm; };
	m.FlagAdd = [](cTaskFlagMan* fm, cTaskItemFlag* f) { fm->Add(f); };
	m.FlagCount = [](cTaskFlagMan* fm) { return (int)fm->nFlag(); };
	m.FlagDrop = [](cTaskFlagMan* fm, int n, const float pTo[4], const float pFrom[4]) { return fm->Drop((u32)n, pTo, pFrom); };
	m.FlagAt = [](cTaskFlagMan* fm, const float pPos[4]) { return fm->IsDroppedAt(pPos); };
	m.FlagGet = [](cTaskFlagMan* fm, int i) { return static_cast<cTaskItemFlag*>(fm->pFlag((u32)i)); };
	m.SmokeAdd = [](cTaskSmokeMan* sm, cTaskGimmickSmoke* s) { sm->Add(s); };
	m.SmokePut = [](cTaskSmokeMan* sm, int owner, int nMax, const int cell[2], int idx) { return sm->Put((u32)owner, (u32)nMax, cell, idx); };
	m.SmokeReset = [](cTaskSmokeMan* sm) { sm->Reset(); };
	m.SmokeFree = [](cTaskSmokeMan* sm, int owner) { return sm->IsFree((u32)owner); };
	cTaskFlagMan::sHooks& f = cTaskFlagMan::Hooks;
	f.Type = [](kingdom::cTaskModel* g) { return static_cast<cTaskGimmick*>(g)->nType(); };
	f.IsTaken = [](kingdom::cTaskModel* g) { return static_cast<cTaskItemFlag*>(g)->IsTaken(); };
	f.Show = [](kingdom::cTaskModel* g) { static_cast<cTaskItemFlag*>(g)->Enable(); };
	f.Drop = [](kingdom::cTaskModel* g, const float* to, const float* from) { return static_cast<cTaskItemFlag*>(g)->Drop(to, from); };
	f.IsDroppedAt = [](kingdom::cTaskModel* g, const float* pos) {
		const cTaskItemFlag* fl = static_cast<cTaskItemFlag*>(g);
		const float* to = fl->vTo();
		return fl->IsFlying() && to[0] == pos[0] && to[1] == pos[1] && to[2] == pos[2];
	};
	cTaskSmokeMan::sHooks& s = cTaskSmokeMan::Hooks;
	s.IsLive = [](kingdom::cTaskModel* g) { return static_cast<cTaskGimmick*>(g)->IsActive(); };
	s.Owner = [](kingdom::cTaskModel* g) { return (u32)static_cast<cTaskGimmick*>(g)->nOwner(); };
	s.Cell = [](kingdom::cTaskModel* g, int out[2]) { const int* c = static_cast<cTaskGimmick*>(g)->pCell(); out[0] = c[0]; out[1] = c[1]; };
	s.Put = [](kingdom::cTaskModel* g, u32 owner, const int cell[2], int arg) { static_cast<cTaskGimmickSmoke*>(g)->Put((int)owner, cell, arg); };
	s.Off = [](kingdom::cTaskModel* g) { static_cast<cTaskGimmick*>(g)->Disable(); };
}
}

// DOL 0x800c677c (job 195)
void cAppG02RallyXGame::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG02RallyXGame());
}

// DOL 0x801236bc
cAppG02RallyXGame::cAppG02RallyXGame()
{
	SetName("cAppG02RallyXGame");
	KingdomGlobal::RallyXSys->AttachStageArc();   // FUN_800c5a1c
	// started from the debug menu (0x0c100000): a multiplayer stage gets its players from the connected pads (0x800c5e90; the single-player
	// counterpart 0x800c5da0 is the debug menu's own business here)
	if (KingdomGlobal::SysCore->GetLastJobNo() == job::RALLYX_DEBUG_MENU && KingdomGlobal::RallyXSys->IsMulti()) cG02RallyXSysExt::DebugMatching();
	kingdom::cKingdomModelBase::ClearAssetCache();
	m_pPause = new cTaskPause();   // +0x2ac
	ConnectChild(m_pPause);
	m_pParam = new cParamGame();   // +0x280
	// the light manager (+0x29c, FUN_80018dd0) with LIGHT_BG / LIGHT_ITEM / LIGHT_MODEL / LIGHT_MII in sets 0..3, the fog manager (+0x2a0) the same
	kingdom::NativeScene::Reset();
	static const char* const LIGHT[4] = {"LIGHT_BG", "LIGHT_ITEM", "LIGHT_MODEL", "LIGHT_MII"};
	for (int i = 0; i < 4; i++)
	{
		std::size_t size = 0;
		const u8* d = wii::ArchiveStore::Find(LIGHT[i], size);
		wii::Brres b;
		wii::ScnAnim a;
		wii::LightResult light;
		if (d != nullptr && b.Load(d, size) && a.Load(b) && a.SampleLight(0, light))
		{
			a.SampleFog(light);
			kingdom::NativeScene::SetLight(i, light);
		}
	}
	ConnectChild(new kingdom::cTaskNativeScene());
	m_pTaskFade = new cTaskRallyXFade();   // +0x2a4
	ConnectChild(m_pTaskFade);
	m_pTaskFade->nDrawPriority(0x8fffffefu);
	m_pTaskFade->Fade(2u, 500);
	// the console reads the cars / sys through hooks (the DOL reads the members directly)
	cTaskConsole::Hooks.CarState = &FillConsCar;
	cTaskConsole::Hooks.SysPlayerNum = []() { return cG02RallyXSysExt::nPlayer; };   // 0x800c5c68
	cTaskConsole::Hooks.SysViewMode = [](int i) { return (unsigned)i < 4u ? cG02RallyXSysExt::nCtrlType[i] : 2; };   // 0x800c5b88
	cTaskConsole::Hooks.StageHighScore = [](int stage) { return (int)cG02RallyXRecord::Get().GetScore(stage); };   // 0x800c62a8
	cTaskPlayer::CameraRotYGoal = [](cCamera* cam, float f) { cam->SetRotYGoal(f); };   // 0x80128f1c
	m_pConsole = new cTaskConsole(this);   // +0x2a8
	ConnectChild(m_pConsole);
	cTaskConsole::sAppState st;
	st.pStageEnd = &m_bStageEnd;
	st.pState = &m_nState;
	st.pStep = &m_nStep;
	st.pScore = &m_nScore;
	st.pPlayTime = &m_fPlayTime;
	st.pCount = &m_fCount;
	st.pRest = &m_nRest;
	st.pParam = m_pParam;
	m_pConsole->SetAppState(st);
	m_pEffect = new cTaskEffect(this);   // DOL 0x801245e4 (0xd50 bytes, app +0x1bc: the cars' particles go through pTaskPtkMan)
	pTaskPtkMan() = m_pEffect;
	ConnectChild(m_pEffect);
	// +0x2b0..+0x2bc = 1 (each ScnRoot camera is driven by its car)
	InstallMapHooks();
	m_pTaskMap = new cTaskMap();   // +0x298 (0x22c bytes; the DOL passes it the console task +0x2a8)
	ConnectChild(m_pTaskMap);
	m_pTaskMap->RegistFlags(m_pConsole);   // FUN_800662c8 per flag (the DOL's map does it while it places them)
	m_pTaskBG = new cTaskBG();   // +0x2c0
	ConnectChild(m_pTaskBG);
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	const bool boss = sys->IsBossStage();   // 0x800c5ba8
	const int nPlayer = nPlayers();
	// the players: colour 6 (Pac-Man), 5 (red) on the boss stage; multiplayer: colour i + 1 (table 0x80378a48 {1, 2, 3, 4}: blue, green, yellow, white)
	int n = 0;
	for (int i = 0; i < nPlayer && n < 7; i++, n++)
	{
		static const int MULTI_COLOUR[4] = {1, 2, 3, 4};
		int colour = sys->IsMulti() ? MULTI_COLOUR[i & 3] : 6;
		if (boss) colour = 5;
		m_pCars[n] = CreatePlayer(this, colour, i, boss);   // 0x80124e54 / 0x80124ac4
		m_pTaskMap->PlaceCar(m_pCars[n], 0, i + 1, false);   // 0x800780e4(map, car, 0, i + 1)
		m_pCars[n]->CreateCamera(&m_nScnLayout);              // 0x80081cec
		ConnectChild(m_pCars[n]);
		m_pConsole->Regist(m_pCars[n], colour, true);        // 0x80065db0
		// FUN_80137768(sys, i, player): the Mii, NOT PORTED
	}
	// the enemies: colour 5 (red), 6 on the boss stage (the red car leads)
	for (int i = 0; i < m_pTaskMap->nEnemy() && n < 7; i++, n++)
	{
		const int colour = boss ? 6 : 5;
		m_pCars[n] = CreateEnemy(this, colour, i, boss);    // 0x801255a0 / 0x801251e8
		m_pTaskMap->PlaceCar(m_pCars[n], 1, i, false);      // 0x800780e4(map, car, 1, i)
		ConnectChild(m_pCars[n]);
		m_pCars[n]->CreateCamera(&m_nScnLayout);
		m_pConsole->Regist(m_pCars[n], colour, false);
	}
	// FUN_80126930(app, table 0x80378a58[players]): the screen layout and every ScnRoot camera's fovy from car 0's camera
	static const s32 LAYOUT[5] = {0, 0, 4, 7, 7};   // DOL 0x80378a58
	m_nScnLayout = LAYOUT[nPlayer < 5 ? nPlayer : 4];
	kingdom::NativeScene::SetLayout(m_nScnLayout);   // cTaskScnRoot +0x1b8: the split screen
	if (m_pCars[0] != nullptr && m_pCars[0]->pCamera() != nullptr)
	{
		cCamera* cam = m_pCars[0]->pCamera();
		for (int i = 0; i < 4; i++) cCamera::SetScnCameraFovy(i, cam->GetFovy(cG02RallyXSysExt::nCtrlType[i]));   // 0x80128e4c(cam, 0x800c5b88(i))
		cam->ModeChange(0);   // 0x80128c54
	}
	m_nNextJob = job::RALLYX_DEBUG_MENU;   // +0x30c
	m_pRetry = new kingdom::game::cTaskConsRetry("COMMON_ANITA_RETRY", "COMMON_ANITA_POINTER", 1u);   // +0x310 (0x97668b20 / 0x1a3f6a03)
	ConnectChild(m_pRetry);
	m_pRetry->nDrawPriority(0x80000008u);
	// the ctor tail: +0x31c = 0; +0x318 = 0 single player, the multiplayer time limit (FUN_800c5978, table 0x80371178) otherwise; +0x32c = CSV_GAME
	// PLAYER_LIFE (FUN_80126a08). NOT PORTED: FUN_80125948 (BGM handle setup)
	m_fCount = 0.0f;
	m_fPlayTime = sys->IsMulti() ? (float)sys->nTimeMs() : 0.0f;
	if (sys->IsMulti()) if (const char* e = std::getenv("MEGAMIX_G02_TIME")) m_fPlayTime = 1000.0f * (float)std::atof(e);   // devtools: a short multiplayer match (seconds)
	m_nRest = m_pParam->m_nLife;
}

void cAppG02RallyXGame::Free()
{
	WiiSnd::Stop(m_SndBgm);
	WiiSnd::Stop(m_SndAlarm);
	delete m_pParam;
	m_pParam = nullptr;
	KingdomGlobal::RallyXSys->DetachStageArc();
	kingdom::NativeScene::Reset();
	SoundManager::get_SE()->ClearListener();   // the cars' listeners (+0x2f0) go with the tree
	cTaskConsole::s_pGameState = nullptr;
	cAppKingdom::Free();
}

int cAppG02RallyXGame::nPlayers() const
{
	const int n = cG02RallyXSysExt::nPlayer;   // sys +0x0c (0x800c5c68)
	return n < 1 ? 1 : (n > 4 ? 4 : n);
}

// DOL 0x80126a10
bool cAppG02RallyXGame::Score(int v, bool bSet)
{
	if (bSet) m_nScore = v;
	else m_nScore += v;
	if (999999 < m_nScore)
	{
		m_nScore = 999999;
		return false;
	}
	return true;
}

// DOL 0x80126a54: the stage ends (lucky flag) unless the player already has every flag
bool cAppG02RallyXGame::UnitStageEnd(bool b, int unitId)
{
	const bool r = m_pCars[0] == nullptr || m_pCars[0]->nFlags() != MapFlagTotal();
	if (r)
	{
		m_bStageEnd = b;
		m_nStageEndId = unitId;
	}
	return r;
}

// DOL 0x80126ac8: the red car crashed on the boss stage -> clear
void cAppG02RallyXGame::UnitRedCrash()
{
	NextState(7);
	WiiSnd::SetVolume(m_SndBgm, 0.5f, 500);   // 0x80126b00: 0x80026394(0.5 (FLOAT_8047c00c), +0x28c, 500)
}

// DOL 0x801272c0: a car clash of the multiplayer match shows the hit mark of the player
void cAppG02RallyXGame::UnitMultiHit(int nIndex)
{
	if (m_pConsole != nullptr) m_pConsole->HitMark(nIndex);
}

// DOL 0x8006cfa0 (via the car's FUN_800881dc)
void cAppG02RallyXGame::FxFlagScore(int count, int special, const Vector4& pos)
{
	if (m_pEffect != nullptr) m_pEffect->FlagScore(count, special != 0, pos);
}

// DOL 0x8006d114
void cAppG02RallyXGame::FxRedFlag(const Vector4& pos)
{
	if (m_pEffect != nullptr) m_pEffect->RedFlag(pos);
}

void cAppG02RallyXGame::MapContact(cTaskUnit* u, int x, int z) { m_pTaskMap->Contact(u, x, z); }
bool cAppG02RallyXGame::MapDropFlag(const float* pos, int dir) { return m_pTaskMap->DropFlag(1, pos, dir); }
// the car's caller 0x800834dc passes nMax = 3
bool cAppG02RallyXGame::MapPutSmoke(int unitId, const float* pos, int index) { return m_pTaskMap->PutSmoke(unitId, 3, pos, index); }
bool cAppG02RallyXGame::MapSmokeFree(int unitId) { return m_pTaskMap->SmokeFree(unitId); }
bool cAppG02RallyXGame::MapNextFlag(int index, float out[4]) const { return m_pTaskMap->NextFlag(index, out); }

int cAppG02RallyXGame::MapFlagTotal() const { return m_pTaskMap != nullptr ? m_pTaskMap->nFlagTotal() : 0; }

void cAppG02RallyXGame::PlayBgm(const char* cue)
{
	WiiSnd::Stop(m_SndBgm);
	WiiSnd::Play(m_SndBgm, cue);
	m_bBgmFading = false;
}

// DOL 0x80125a14: effects clear (0x8008a814), BG reset (0x8006016c, cTaskBG::ResetPtk), every car reset (vtable +0x48) and back
// on its start (0x800780ec)
void cAppG02RallyXGame::ResetCars()
{
	if (m_pEffect != nullptr) m_pEffect->Clear();
	if (m_pTaskBG != nullptr) m_pTaskBG->ResetPtk();   // UNVERIFIED gate: the DOL calls it between 0x800c6428 / 0x800c5ca0 and the cars
	const int nPlayer = nPlayers();
	int n = 0;
	for (int i = 0; i < nPlayer && n < 7; i++, n++)
		if (m_pCars[n] != nullptr)
		{
			m_pCars[n]->Reset();
			m_pTaskMap->PlaceCar(m_pCars[n], 0, i + 1, true);
		}
	for (int i = 0; i < m_pTaskMap->nEnemy() && n < 7; i++, n++)
		if (m_pCars[n] != nullptr)
		{
			m_pCars[n]->Reset();
			m_pTaskMap->PlaceCar(m_pCars[n], 1, i, true);
		}
}

void cAppG02RallyXGame::StartPause()
{
	m_pPause->Start();   // 0x80147c8c
}

// DOL 0x80126908
void cAppG02RallyXGame::Draw(int nStatus, cJetTask* pParam)
{
	(void)nStatus;
	(void)pParam;
	m_nState = m_nNextState;
	m_nStep = m_nNextStep;
}

// DOL 0x80125b1c. Its state switch (+0x2e4) jumps through the table at 0x804410ec, right after cTaskRallyAnitaBase's secondary vtable, so
// vtable_audit read the case labels as that class's slots: 0x80125c18 0x80125ca0 0x80125e50 0x80126028 0x8012613c 0x80126488 0x801265b8 0x801266ec
// 0x801266f8 0x80126704 0x80126710 0x80126860 0x8012682c (all inside this function)
bool cAppG02RallyXGame::Execute(cJetTask* pParam)
{
	(void)pParam;
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	const int nPlayer = nPlayers();
	const int nCars = std::min(7, nPlayer + m_pTaskMap->nEnemy());
	for (int i = 0; i < nPlayer; i++)
		if ((GamePad::Get(i).trig & 0x500u) != 0 && (unsigned)(m_nState - 1) < 6u && !m_pRetry->IsVisible()   /* +0x1c8 == 0 */) StartPause();
	IsPause() = m_pPause->IsPause();   // +0x1b4 = pause +0x1d4
	if (IsPause() || m_bStageEnd || (GamePad::Get(0).hold & 0x400u) != 0) return true;
	switch (m_nState)
	{
	case 0:   // fade in
		if (m_nStep == 0)
		{
			m_pTaskFade->Fade(2u, 500);
			ResetCars();
			NextStep(1);
		}
		else if (m_nStep == 1 && m_pTaskFade->IsEnd()) NextState(1);
		break;
	case 1:   // the opening turn of the cameras
		if (m_nStep == 0)
		{
			cTaskConsole::PlaySE("m02se_18Start");
			NextStep(1);
		}
		else if (m_nStep == 1)
		{
			bool done = true;
			for (int i = 0; i < nPlayer; i++)
				if (m_pCars[i] != nullptr && m_pCars[i]->pCamera() != nullptr && 0.0872664600610733f < m_pCars[i]->pCamera()->fRotYNow()) done = false;   // FLOAT_8047bff8
			if (done) NextStep(2);
		}
		else if (m_nStep == 2)
		{
			NextState(2);
			m_fCount = 0.0f;
			for (int i = 0; i < nCars; i++)
				if (m_pCars[i] != nullptr) m_pCars[i]->StartEngine();   // 0x8008868c
			for (int i = 0; i < nPlayer; i++)
				if (m_pCars[i] != nullptr) m_pCars[i]->CameraStart(2, 0);   // 0x80081dd0
		}
		break;
	case 2:   // the start signal
		if (m_pConsole->IsSignalGo() && cTaskUnit::Up(300.0f, 1.0f, m_fCount))   // 0x800c541c; FLOAT_8047bffc
		{
			NextState(4);
			for (int i = 0; i < nPlayer; i++)
				if (m_pCars[i] != nullptr) m_pCars[i]->CameraStart(0, 0);
			if (!sys->IsBossStage())
			{
				int w = sys->nWorld();
				if (w == 0xb) w = sys->nStageInWorld();
				static const char* const BGM[4] = {"m02bgm_World1", "m02bgm_World2", "m02bgm_World3", "m02bgm_World4"};
				if (w >= 1 && w <= 4) PlayBgm(BGM[w - 1]);
			}
			else PlayBgm("m02bgm_RedCar");
			WiiSnd::SetVolume(m_SndBgm, 1.0f, 500);   // 0x80126020: 0x80026394(1.0 (FLOAT_8047c000), +0x28c, 500)
		}
		break;
	case 3:   // retry after a miss
		if (m_nStep == 0)
		{
			ResetCars();
			for (int i = 0; i < nCars; i++)
				if (m_pCars[i] != nullptr) m_pCars[i]->StartEngine();
			for (int i = 0; i < nPlayer; i++)
				if (m_pCars[i] != nullptr) m_pCars[i]->CameraStart(0, 1);
			m_pTaskMap->ResetGimmicks();   // 0x80079fbc
			m_pConsole->HideBang2();       // 0x800667c4
			NextStep(1);
		}
		else if (m_nStep == 1 && m_pTaskFade->IsEnd()) NextState(4);
		break;
	case 4:   // play
	{
		cTaskUnit::Up(99999.0f, 1.0f, m_fCount);   // 0x800c541c
		if (!sys->IsMulti())
		{
			cTaskUnit::Up(3599999.0f, 16.0f, m_fPlayTime);   // 0x800c53c0 (FLOAT_8047bfb8, FLOAT_8047c004 ms a frame)
		}
		else
		{
			// multiplayer: the time counts down; the alarm plays in the last 10 s (FLOAT_8047c008), until the time is up
			const float before = m_fPlayTime;
			cTaskUnit::Down(0.0f, -16.0f, m_fPlayTime);   // 0x800c5450(0, -16, +0x318)
			if (10000.0f < before || m_fPlayTime <= 0.0f)
				WiiSnd::Stop(m_SndAlarm);
			else if (m_SndAlarm.voice < 0 || !snd::IsPlaying(m_SndAlarm.voice))
				WiiSnd::Play(m_SndAlarm, "m02se_13Alarm");
		}
		if (sys->IsBossStage() && m_pCars[1] != nullptr)
		{
			// the alarm while the red car is within 3 flags of the total
			const int total = MapFlagTotal(), got = m_pCars[1]->nFlags();
			if (got < total - 3 || total - 1 < got) WiiSnd::Stop(m_SndAlarm);   // 0x80026318(+0x320, 0)
			else if (m_SndAlarm.voice < 0 || !snd::IsPlaying(m_SndAlarm.voice)) WiiSnd::Play(m_SndAlarm, "m02se_13Alarm");   // 0x80026424
		}
		for (int i = 0; i < nPlayer; i++)
			if (m_pCars[i] != nullptr && m_pCars[i]->IsStop()) NextState(5);   // the crash (+0x1d8)
		if (sys->IsMulti())
		{
			if ((int)m_fPlayTime == 0)
			{
				// the time is up: the winners are the players with the most flags (the mask of sys +0x10 via 0x800c5898); a tie -> state 9
				int best = 0;
				bool tie = false;
				sys->ClearWinners();   // 0x800c5890
				sys->SetWinner(0);
				for (int i = 1; i < nPlayer; i++)
				{
					const int a = m_pCars[best] != nullptr ? m_pCars[best]->nFlags() : 0;
					const int b = m_pCars[i] != nullptr ? m_pCars[i]->nFlags() : 0;
					if (a == b)
					{
						tie = true;
						sys->SetWinner(i);
					}
					else if (a < b)
					{
						tie = false;
						sys->ClearWinners();
						sys->SetWinner(i);
						best = i;
					}
				}
				NextState(tie ? 9 : 8);
				WiiSnd::SetVolume(m_SndBgm, 0.5f, 500);
			}
		}
		else
		{
			if (!sys->IsBossStage())
			{
				if (m_pCars[0] != nullptr && m_pCars[0]->nFlags() == MapFlagTotal())
				{
					NextState(7);
					WiiSnd::SetVolume(m_SndBgm, 0.5f, 500);   // 0x80126334: 0x80026394(0.5, +0x28c, 500)
				}
			}
			else if (m_pCars[1] != nullptr && m_pCars[1]->nFlags() == MapFlagTotal())
			{
				NextState(10);
				WiiSnd::SetVolume(m_SndBgm, 0.5f, 500);   // 0x80126380
				m_pCars[1]->Clear();   // 0x80088a70
			}
		}
		break;
	}
	case 5:   // miss
		if (m_nStep == 0)
		{
			m_fMissTime = 0.0f;
			m_pConsole->ShowBang();   // 0x8006662c
			NextStep(1);
		}
		else if (m_nStep == 1)
		{
			if (cTaskUnit::Inc1(m_pParam->m_fMissWait, m_fMissTime))   // CSV_GAME +0x24
			{
				m_pTaskFade->Fade(0x10004u, 500);
				NextStep(2);
			}
		}
		else if (m_nStep == 2 && m_pTaskFade->IsEnd())
		{
			m_pTaskFade->Fade(4u, 500);
			if (m_nRest < 1) NextState(6);
			else
			{
				m_nRest--;
				NextState(3);
			}
		}
		break;
	case 6:   // game over
		if (m_nStep == 0)
		{
			for (int i = 0; i < nPlayer; i++)
				if (m_pCars[i] != nullptr) m_pCars[i]->CameraStart(0, 1);
			m_pTaskMap->ResetGimmicks();
			m_pConsole->HideBang();   // 0x8006669c
			NextStep(1);
		}
		else if (m_nStep == 1 && m_nNextJob == job::RALLYX_DEBUG_MENU)
		{
			m_pRetry->Start();   // 0x80149b88
			if (m_pRetry->IsYes())
			{
				m_nNextJob = job::RALLYX_GAME;   // retry the stage
				m_pTaskFade->Fade(0x10002u, 500);
				NextState(0xc);
			}
			if (m_pRetry->IsNo())
			{
				m_nNextJob = job::RALLYX_GAMEOVER;   // game over
				m_pTaskFade->Fade(0x10002u, 500);
				NextState(0xc);
			}
		}
		break;
	case 7: StateClear(); break;
	case 8:   // the winner (0x80126f70)
	case 9:   // a draw (0x80127118)
		StateMultiResult(m_nState == 8);
		break;
	case 10:   // the red car got every flag
		if (m_nStep == 0)
		{
			if (m_pConsole->IsLoseShown())
			{
				cTaskConsole::PlaySE("m02se_23draw");
				NextStep(1);
				m_fWait330 = 0.0f;
			}
		}
		else if (m_nStep == 1 && m_pConsole->IsLoseEnd() &&
		         (cTaskUnit::Inc1(m_pParam->m_fLoseLastWait, m_fWait330) || (GamePad::Get(0).trig & 0x860u) != 0))   // CSV_GAME +0x34
		{
			m_nNextJob = job::RALLYX_MENU;
			m_pTaskFade->Fade(0x10002u, 500);
			NextState(0xc);
		}
		break;
	case 0xc:   // leave
		if (!m_bBgmFading)
		{
			WiiSnd::FadeOut(m_SndBgm, 500);   // 0x80025560(global +0xa54, 500)
			m_bBgmFading = true;
		}
		if (m_pTaskFade->IsEnd()) Kingdom_SetJobNo((uint)m_nNextJob, m_bNextFlag, false);
		break;
	default: break;
	}
	if (m_nState != 0xc && GamePad::Get(0).hold == 0xffffffffu) NextState(0xc);
	for (int i = 0; i < 4; i++)
		if (m_nCamOn[i] == 1 && m_pCars[i] != nullptr && m_pCars[i]->pCamera() != nullptr) m_pCars[i]->pCamera()->Apply(i);   // 0x80129c94
	return true;
}

// DOL 0x80126f70 (bWin) / 0x80127118 (draw): step 0 the cars' nitro goes off (0x80088ba0), step 1 the console's result is shown (m02se_22win /
// m02se_23draw), step 2 after CSV_GAME +0x2c / +0x30 frames or A / B / Z: back to the multiplayer menu (job 200)
void cAppG02RallyXGame::StateMultiResult(bool bWin)
{
	if (m_nStep == 0)
	{
		for (int i = 0; i < nPlayers(); i++)
			if (m_pCars[i] != nullptr) m_pCars[i]->ResetNitro();   // 0x80088ba0: +0x2a4 = 0
		NextStep(1);
	}
	if (m_nStep == 1)
	{
		if (bWin ? m_pConsole->IsWinShown() : m_pConsole->IsDrawShown())
		{
			cTaskConsole::PlaySE(bWin ? "m02se_22win" : "m02se_23draw");
			NextStep(2);
			m_fWait330 = 0.0f;
		}
	}
	else if (m_nStep == 2 && (bWin ? m_pConsole->IsWinEnd() : m_pConsole->IsDrawEnd()))
	{
		if (cTaskUnit::Inc1(bWin ? m_pParam->m_fWinnerLastWait : m_pParam->m_fDrawLastWait, m_fWait330) || (GamePad::Get(0).trig & 0x860u) != 0)
		{
			m_nNextJob = job::RALLYX_MULTI_MENU;
			m_pTaskFade->Fade(0x10002u, 500);
			NextState(0xc);
		}
	}
}

// DOL 0x80126b18 (state 7: stage clear)
void cAppG02RallyXGame::StateClear()
{
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	if (m_nStep == 0)
	{
		cTaskConsole::PlaySE("m02se_17Clear");
		NextStep(2);
		if (m_pCars[0] != nullptr) m_pCars[0]->Clear();   // 0x80088a70
		if (m_SndAlarm.voice >= 0 && snd::IsPlaying(m_SndAlarm.voice)) WiiSnd::Stop(m_SndAlarm);
		m_fWait330 = 0.0f;
	}
	else if (m_nStep == 2)
	{
		if (m_pConsole->IsResultEnd()) NextStep(3);   // 0x80066584
	}
	else if (m_nStep == 3) NextStep(4);
	else if (m_nStep == 4)
	{
		// the fuel bonus: 1 fuel -> 50 points a frame; A / B / 2 (0x860) adds the rest at once
		for (int i = 0; i < 7; i++)
		{
			cTaskUnit* car = m_pCars[i];
			if (car == nullptr || car->nUnitId() != 0x10) continue;   // vtable +0x44: player 0
			float& fuel = car->fFuelRef();
			if ((GamePad::Get(0).trig & 0x860u) == 0)
			{
				fuel -= 1.0f;   // FLOAT_8047c000
				Score(50, false);
				if (fuel <= 0.0f)
				{
					fuel = 0.0f;
					NextStep(5);
					m_fWait330 = 0.0f;
				}
			}
			else
			{
				const u32 whole = (u32)fuel;
				Score((int)whole * 50, false);
				if ((float)whole < fuel) Score(50, false);
				fuel = 0.0f;
				NextStep(5);
				m_fWait330 = 0.0f;
			}
		}
	}
	else if (m_nStep == 5 &&
	         (cTaskUnit::Inc1(m_pParam->m_fClearLastWait, m_fWait330) || (GamePad::Get(0).trig & 0x860u) != 0))   // CSV_GAME +0x28
	{
		const int stage = sys->nStageIdx();   // sys +0x08
		cG02RallyXRecord& rec = cG02RallyXRecord::Get();
		if (!sys->IsBossStage()) rec.SetRecord(stage, (u32)m_nScore, (u32)m_fPlayTime);   // 0x800c6218
		else rec.SetTime(stage, (u32)m_fPlayTime);                                       // 0x800c6274
		m_nNextJob = job::RALLYX_MENU;
		m_bNextFlag = true;   // +0x314
		if (!sys->IsBossStage())
		{
			int next = stage + 1;
			if (cG02RallyXSysExt::IsBossStageOf(next)) next = stage + 2;   // 0x800c5bf8
			if ((unsigned)next < 0x18u)
			{
				if (rec.GetScore(next) == 0)
				{
					rec.SetOpen(next);   // 0x800c63f0
					cG02RallyXSysExt::bNewStageApear = true;   // 0x800c5c70
				}
			}
			else
			{
				if (!rec.IsAllClear())   // 0x800c61d8
				{
					m_nNextJob = job::RALLYX_STAFFROLL;   // the staff roll, once
					m_bNextFlag = false;
				}
				rec.SetOpen(6);
				rec.SetOpen(0xc);
				rec.SetOpen(0x12);
				rec.SetOpen(0x18);
				rec.SetAllClear();   // 0x800c6410
				cG02RallyXSysExt::bNewStageApear = true;
			}
		}
		rec.Save();   // 0x800c5ff0
		m_pTaskFade->Fade(0x10002u, 500);
		NextState(0xc);
		NextStep(3);
	}
}

}
