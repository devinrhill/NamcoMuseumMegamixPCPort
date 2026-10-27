#include "platform/devtools_g07edit.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include <cstdio>
#include <cstdlib>
#include "kingdom/game/g07/cMotosGameSys.hpp"

#include <iterator>
#include <limits>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/game/cTaskConsRetry.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"
#include "kingdom/game/g07/cMotosGetTime.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cParamAxisAttack.hpp"
#include "kingdom/game/g07/cParamEnemy.hpp"
#include "kingdom/game/g07/cTaskBG.hpp"
#include "kingdom/game/g07/cTaskCamera.hpp"
#include "kingdom/NativeScene.hpp"
#include "wii/Anim.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharBoss.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "kingdom/game/g07/cTaskConsoleMan.hpp"
#include "kingdom/game/g07/cTaskEffect.hpp"
#include "kingdom/game/g07/cTaskHitMan.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"
#include "kingdom/game/g07/cTaskMotosFade.hpp"
#include "kingdom/game/g07/cTaskPause.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <EBGMKind.hpp>
#include <ESEKind.hpp>
#include <Extentions.hpp>
#include <GameDefine.hpp>
#include <GameStateManager.hpp>
#include <InGamePadManager.hpp>
#include <PadDefine.hpp>
#include <ResultInfo.hpp>
#include <SGameRecordInfo.hpp>
#include <SoundManager.hpp>
#include <SoundSe.hpp>
#include <SoundSeObject.hpp>

namespace kingdom::game::g07
{

int cAppG07MotosGame::DRAW_PLAYER_NONE = -1;
uint cAppG07MotosGame::PLAY_TIME_MAX = 5999999u;
uint cAppG07MotosGame::PLAYER_COLOR[4] = {::Constants::JET_RGBA8888(180u, 180u, 255u, 255u), ::Constants::JET_RGBA8888(150u, 255u, 150u, 255u), ::Constants::JET_RGBA8888(230u, 255u, 50u, 255u), ::Constants::JET_RGBA8888(255u, 180u, 230u, 255u)};
uint cAppG07MotosGame::m_nGamePlayTime{};

void cAppG07MotosGame::STEP(int n)
{
	nBefStep() = nStep(0);
	nStep(0) = n;
	nStep(1) = 0;
	nStep(2) = 0;
	nStep(3) = 0;
}

void cAppG07MotosGame::STEP2SYNC(int step)
{
	nBefStep() = nStep(0);
	nStep(3) = step;
	nStep(0) = 16;
	nStep(1) = 0;
	nStep(2) = 0;
}

void cAppG07MotosGame::STEP2EXIT(int exit)
{
	nBefStep() = nStep(0);
	nStep(3) = exit;
	nStep(0) = 15;
	nStep(1) = 0;
	nStep(2) = 0;
}

cAppG07MotosGame::cAppG07MotosGame()
{
	Init();
}

void cAppG07MotosGame::Free()
{
	Term();
	cAppKingdom::Free();
}

uint cAppG07MotosGame::nPlayerParam()
{
	return m_nPlayerParamSet;
}

int cAppG07MotosGame::nMultiBattleResult()
{
	return m_nMultiBattleResult;
}

uint cAppG07MotosGame::GetGamePlayTime()
{
	return m_nGamePlayTime;
}

bool cAppG07MotosGame::IsFirstPlay()
{
	return m_bFirstPlay;
}

int cAppG07MotosGame::nMeteoTimer()
{
	return (int)((long)Constants::get_GAME_COUNTER() - (long)m_nMeteoTimerStart);
}

bool cAppG07MotosGame::bMultiPlay()
{
	return m_bMultiPlay;
}

cTaskMap* cAppG07MotosGame::pMap()
{
	return m_pTaskMap;
}

void cAppG07MotosGame::pSetMap(cTaskMap* set)
{
	m_pTaskMap = set;
}

cTaskChar* cAppG07MotosGame::pChars(uint n)
{
	if ((long)n >= (long)::Constants::ARRAY_SIZE(m_pChars))
	{
		return nullptr;
	}
	return m_pChars[n];
}

cTaskCharPlayer* cAppG07MotosGame::pPlayers(uint n)
{
	if ((long)n >= (long)::Constants::ARRAY_SIZE(m_pPlayers))
	{
		return nullptr;
	}
	return m_pPlayers[n];
}

cTaskCharEnemy* cAppG07MotosGame::pEnemys(uint n)
{
	if ((long)n >= (long)::Constants::ARRAY_SIZE(m_pEnemys))
	{
		return nullptr;
	}
	return m_pEnemys[n];
}

cTaskCharEnemy*& cAppG07MotosGame::pBoss()
{
	return m_pBoss;
}

cTaskHitMan* cAppG07MotosGame::pHitMan()
{
	return m_pHitMan;
}

cTaskMotosFade* cAppG07MotosGame::pTaskFade()
{
	return m_pTaskFade;
}

cTaskConsoleMan* cAppG07MotosGame::pTaskConsoleMan()
{
	return m_pTaskConsoleMan;
}

cTaskEffect* cAppG07MotosGame::pTaskEffect()
{
	return m_pTaskEffect;
}

float& cAppG07MotosGame::fComingSELength()
{
	return m_fComingSELength;
}

bool& cAppG07MotosGame::bGimmick()
{
	return m_bGimmick;
}

uint& cAppG07MotosGame::nGameCounterOLD()
{
	return m_nGameCounterOLD;
}

void cAppG07MotosGame::GameCounterBackup()
{
	m_nGameCounterOLD = Constants::get_GAME_COUNTER();
}

int& cAppG07MotosGame::nScore()
{
	return m_nPoints[0];
}

uint& cAppG07MotosGame::nHiScore()
{
	return KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nHiScore;
}

short& cAppG07MotosGame::nLife()
{
	return KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nLife;
}

uint& cAppG07MotosGame::nLandCount(int n)
{
	return m_nLandCount[n];
}

int& cAppG07MotosGame::nPoints(int n)
{
	return m_nPoints[n];
}

uint& cAppG07MotosGame::nPointCount(int n)
{
	return m_nPointCount[n];
}

sbyte& cAppG07MotosGame::nPRanking(int n)
{
	return m_nPRanking[n];
}

sbyte& cAppG07MotosGame::nTotalPRanking(int n)
{
	return m_nTotalPRanking[n];
}

float& cAppG07MotosGame::fConsSlopeRad()
{
	return m_fConsSlopeRad;
}

float& cAppG07MotosGame::fConsSlopeRadOLD()
{
	return m_fConsSlopeRadOLD;
}

int& cAppG07MotosGame::nSlopeState()
{
	return m_nSlopeState;
}

int cAppG07MotosGame::n1stPointPlayer()
{
	return m_n1stPointPlayer;
}

int cAppG07MotosGame::GetDrawPlayer(uint set)
{
	CustomKingdomUtil::Assert(set < 4, "error");
	return m_naMultiPlayerDrawList[set];
}

bool& cAppG07MotosGame::IsGameStart()
{
	return m_IsGameStart;
}

Vector4& cAppG07MotosGame::vSlopePower()
{
	return m_vSlopePower;
}

cTaskCamera* cAppG07MotosGame::pCamera()
{
	return m_pTaskCamera;
}

Vector4& cAppG07MotosGame::pGravVector()
{
	return m_vGravVector;
}

bool cAppG07MotosGame::IsGameClearPause()
{
	if (Constants::IS_LANPLAY() && KingdomGlobal::pMotosGameSys->TestMasterFlag(3u) != 0)
	{
		return true;
	}
	return false;
}

int& cAppG07MotosGame::nStep(int n)
{
	return m_nStep[n];
}

int& cAppG07MotosGame::nBefStep()
{
	return m_nBefStep;
}

float& cAppG07MotosGame::fSlopeRad()
{
	return m_fSlopeRad;
}

float& cAppG07MotosGame::fSlopeRadOLD()
{
	return m_fSlopeRadOLD;
}

Matrix4x4& cAppG07MotosGame::pSlopeMatrix()
{
	return m_mSlopeMatrix;
}

bool& cAppG07MotosGame::IsMissedReserve()
{
	return m_IsMissedReserve;
}

bool& cAppG07MotosGame::IsClearReserve()
{
	return m_IsClearReserve;
}

void cAppG07MotosGame::Init()
{
	SoundManager::get_BGM()->Stop(0.0f);
	if ((long)KingdomGlobal::MotosSys->nWorld() != (long)KingdomGlobal::MotosSys->PrevWorldNo)
	{
		KingdomGlobal::MotosSys->PrevWorldNo = (int)KingdomGlobal::MotosSys->nWorld();
		// TODO(unity): ResourceCache.Unload(ResourceCache.EType.Temporary);
		// TODO(unity): Resources.UnloadUnusedAssets();
	}
	MotosGetTime = new cMotosGetTime();
	KingdomGlobal::pMotosGameSys->NeedInit();
	Constants::set_SYNC_COUNTER_LOCAL(0u);
	Constants::set_GAME_COUNTER_LOCAL(0u);
	Constants::set_SYNC_COUNTER(0u);
	Constants::set_GAME_COUNTER(0u);
	GameCounterBackup();
	IsGameStart() = false;
	m_IsClearReserve = false;
	m_IsMissedReserve = false;
	m_pTaskConsRetry = nullptr;
	m_fSlopeRad = 0.0f;
	m_fSlopeRadOLD = m_fSlopeRad;
	m_fConsSlopeRad = 0.0f;
	m_fConsSlopeRadOLD = 0.0f;
	m_nSaveFlags = 0u;
	m_nSlopeState = 0;
	m_bFirstPlay = false;
	if (KingdomGlobal::SysCore->GetLastJobNo() == 287309824 || KingdomGlobal::SysCore->GetLastJobNo() == 286261248 || KingdomGlobal::SysCore->GetLastJobNo() == 289406976 || KingdomGlobal::SysCore->GetLastJobNo() == 292552704 || KingdomGlobal::SysCore->GetLastJobNo() != 288358400 || KingdomGlobal::MotosSys->bPlayerRetry())
	{
		KingdomGlobal::MotosSys->bPlayerRetry() = false;
		m_bFirstPlay = true;
		m_nGamePlayTime = 0u;
		KingdomGlobal::MotosSys->nLastBossType() = 0u;
		KingdomGlobal::MotosSys->nForceVsRoundCount() = -1;
		for (int i = 0; i < (int)std::size(m_nPoints); i++)
		{
			m_nPoints[i] = 0;
		}
		for (uint num = 0u; num < 36; num++)
		{
			KingdomGlobal::MotosSys->bMotosCharDead(num) = false;
			KingdomGlobal::MotosSys->nWinsRound(num) = 0u;
		}
		for (uint num2 = 0u; num2 < 4; num2++)
		{
			for (uint num3 = 0u; num3 < 4; num3++)
			{
				KingdomGlobal::pMotosGameSys->KeepData()[num2]->m_nUseItem[num3] = 0;
			}
		}
		KingdomGlobal::MotosSys->SaveDataLoad();
		KingdomGlobal::MotosSys->nStageIdxTmp() = KingdomGlobal::MotosSys->nStageIdx();
		KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nLife = (short)cG07MotosSys::DEFAULT_PLAYER_LIFE;
		KingdomGlobal::pMotosGameSys->ResetDeadEnemyCnt();
		KingdomGlobal::pMotosGameSys->ResetGetItemCnt();
	}
	else
	{
		nScore() = (int)KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nMyScore;
		KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nMyScore = 0u;
	}
	KingdomGlobal::MotosSys->bNewStageApear() = false;
	MultiPlaySettings();
	m_nPLReserve = -1;
	m_n1stPointPlayer = -1;
	m_nMultiBattleResult = 0;
	m_pBoss = nullptr;
	CustomKingdomUtil::Assert(KingdomGlobal::g_pMotosGame == nullptr, "muripo");
	KingdomGlobal::g_pMotosGame = this;
	KingdomGlobal::pMotosGameSys->nGameLevel() = devtools::G07Level(); // devtools: dev window difficulty, 0 = the DOL (not 1:1)
	m_bGimmick = false;
	for (int j = 0; j < (int)std::size(m_naMultiPlayerDrawList); j++)
	{
		m_naMultiPlayerDrawList[j] = DRAW_PLAYER_NONE;
	}
	cParamAxisAttack* obj = new cParamAxisAttack();
	obj->RegistData("CSV_AXIS_ATTACK");
	Constants::set_g_AxisAttack(obj->pAxisAttackLevel((uint)devtools::G07Level())); // devtools: dev window difficulty, 0 = the DOL (not 1:1)
	m_nPlayerParamSet = 0u;
	m_nPlayerNo = 0u;
	for (int k = 0; k < (int)std::size(m_nLandCount); k++)
	{
		m_nLandCount[k] = 0u;
	}
	for (int l = 0; l < (int)std::size(m_nPointCount); l++)
	{
		m_nPointCount[l] = 0u;
	}
	for (int m = 0; m < (int)std::size(m_nStep); m++)
	{
		m_nStep[m] = 0;
	}
	if (KingdomGlobal::MotosSys->IsBossStage() && m_bFirstPlay)
	{
		nStep(0) = 2;
	}
	else
	{
		nStep(0) = 1;
	}
	m_nMeteoTimerStart = 0;
	m_fComingSELength = 5.0f;
	Constants::tspVuMtxUnit(m_mSlopeMatrix);
	::Constants::jetGeoVecUnit(m_vGravVector);
	::Constants::jetGeoVecUnit(m_vSlopePower);
	for (uint num4 = 0u; num4 < 36; num4++)
	{
		m_pChars[num4] = nullptr;
	}
	for (uint num5 = 0u; num5 < 4; num5++)
	{
		m_pPlayers[num5] = nullptr;
	}
	for (uint num6 = 0u; num6 < 32; num6++)
	{
		m_pEnemys[num6] = nullptr;
	}
	for (uint num7 = 0u; num7 < 27; num7++)
	{
		cParamEnemy* cParamEnemy2 = new cParamEnemy();
		(void)(Constants::get_g_MoEnemyParam()[num7]);
		cParamEnemy2->RegistData(devtools::G07LevelCsv("CSV_CHAR_PARAM_NORMAL"), num7); // devtools: dev window difficulty (not 1:1)
		Constants::get_g_MoEnemyParam()[num7] = cParamEnemy2->pMotosParams();
	}
	KingdomGlobal::MotosSys->AttachStageData();
	{
		// the stage's light sets (SCN0 files): 0 background, 1 panels / gimmicks, 2 characters; one native 3D pass draws every model
		kingdom::NativeScene::Reset();
		kingdom::NativeScene::SetPerFrame(true);   // models submitted by the task Draws this frame only (g07 cTaskChar::Draw 0x801e3048, cTaskPanel 0x801fda7c, ...)
		static const char* const kLights[3] = {"SCA_BG", "SCA_PANEL", "SCA_CHAR"};
		for (int i = 0; i < 3; i++)
		{
			std::size_t size = 0;
			const std::uint8_t* d = wii::ArchiveStore::Find(kLights[i], size);
			wii::Brres b;
			wii::ScnAnim sa;
			wii::LightResult light;
			if (d != nullptr && b.Load(d, size) && sa.Load(b) && sa.SampleLight(0, light))
			{
				sa.SampleFog(light);
				kingdom::NativeScene::SetLight(i, light);
			}
		}
		ConnectChild(new kingdom::cTaskNativeScene());
	}
	ConnectChild(m_pTaskCamera = new cTaskCamera(nullptr, "CSV_CAMERA"));
	// DOL 0x800a4824..0x800a493c: the depth shadow light (cTaskScnRoot +0x1f8 = 1; mode 0, target = the camera task's +0x1d0 vector (UNVERIFIED: its
	// first Vector4), +5 up (stage 39: +0.5 first), angles (pi/2, pi, 0), ortho 18 x 18 near 1 far 44 (+0xcd8 flag, taken as multiplayer: 30 / 46), radius 40 / 30)
	{
		kingdom::NativeScene::SetShadow(true);
		kingdom::cKingdomShadow& sh = kingdom::NativeScene::Shadow();
		const Vector4 t = m_pTaskCamera->vPosition();
		float y = t[1];
		if (KingdomGlobal::MotosSys->nStageIdx() == 39u) y = 0.5f;
		y += 5.0f;
		float radius = 40.0f, far = 3.0f + (1.0f + 40.0f);
		if (m_bMultiPlay) { radius = 30.0f; far = 15.0f + (1.0f + 30.0f); }
		sh.SetMode(0);
		sh.SetTarget(t[0], y, t[2]);
		sh.SetAngles(1.5707964f, 3.1415927f, 0.0f);
		sh.SetOrtho(18.0f, 18.0f, 1.0f, far);
		sh.SetRadius(radius);
		sh.SetOrtho(18.0f, 18.0f, 1.0f, 2.0f + far);
	}
	m_pTaskEffect = nullptr;
	m_pTaskEffect = new cTaskEffect();
	CustomKingdomUtil::Assert(m_pTaskEffect != nullptr, "nullpo");
	m_pTaskEffect->SetData();
	KingdomGlobal::KingSys->pAppMain()->pTaskPtkMan() = m_pTaskEffect;
	ConnectChild(m_pTaskEffect);
	ConnectChild(m_pTaskFade = new cTaskMotosFade());
	CustomKingdomUtil::Assert(m_pTaskFade != nullptr, "error");
	m_pTaskFade->nDrawPriority(2415919087u);
	m_pTaskFade->Fade(7u);
	pCamera()->SaveVector();
	if (bMultiPlay())
	{
		nPoints(0) = (int)KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nMyScore;
	}
	ConnectChild(m_pTaskBG = new cTaskBG());
	string path = ResourceDatabase::GetPath("ANT_RETRY");
	m_pTaskConsRetry = new cTaskConsRetry(path, "ANT_CURSOR", 1u);
	ConnectChild(m_pTaskConsRetry);
	ConnectChild(m_pTaskPause = new cTaskPause());
	ConnectChild(m_pTaskMap = new cTaskMap());
	ConnectChild(m_pHitMan = new cTaskHitMan());
	m_pTaskConsoleMan = new cTaskConsoleMan();
	m_pTaskConsoleMan->Create();
	ConnectChild(m_pTaskConsoleMan);
}

void cAppG07MotosGame::Term()
{
	devtools::G07EditItemRestore(KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nItemNum); // devtools: Motos editor test stock back (not 1:1)
	KingdomGlobal::MotosSys->DetachStageData();
	if (m_bMultiPlay && nStep(0) == 5)
	{
		KingdomGlobal::MotosSys->nStageIdx() = KingdomGlobal::MotosSys->nStageIdxTmp();
	}
	SoundManager::get_BGM()->Stop();
	SoundManager::get_SE()->Stop();
	KingdomGlobal::g_pMotosGame = nullptr;
	KingdomGlobal::pMotosGameSys->OffMasterFlag(std::numeric_limits<uint>::max());
	KingdomGlobal::pMotosGameSys->OffSlaveFlag(Constants::MY_ID(), std::numeric_limits<uint>::max());
	if (m_pTaskConsoleMan != nullptr)
	{
		m_pTaskConsoleMan->TermOnce();   // Free() = Term()
	}
	m_pTaskBG->TermOnce();
	m_pTaskMap->TermOnce();
	for (int i = 0; i < (int)std::size(m_pChars); i++)
	{
		if (m_pChars[i] != nullptr)
		{
			m_pChars[i]->TermOnce();
		}
	}
	if (pTaskPtkMan() != nullptr)
	{
		pTaskPtkMan()->TermOnce();
	}
}

uint cAppG07MotosGame::GetMultiBatleStageIdx()
{
	uint num = 0u;
	if (KingdomGlobal::MotosSys->nForceVsRoundCount() == -1 || KingdomGlobal::MotosSys->nForceVsRoundCount() != 10)
	{
		return KingdomGlobal::MotosSys->nStageIdx();
	}
	KingdomGlobal::MotosSys->nStageIdx()++;
	uint num2 = KingdomGlobal::MotosSys->nStageIdx();
	if (num2 > 45)
	{
		num2 = 36u;
	}
	KingdomGlobal::MotosSys->nStageIdx() = num2;
	return num2;
}

void cAppG07MotosGame::MultiPlaySettings()
{
	m_bMultiPlay = false;
	uint multiBatleStageIdx = GetMultiBatleStageIdx();
	if (multiBatleStageIdx < 36 || multiBatleStageIdx > 45)
	{
		return;
	}
	if (KingdomGlobal::MotosSys->nForceVsRoundCount() == -1)
	{
		KingdomGlobal::MotosSys->nForceVsRoundCount() = (int)KingdomGlobal::MotosSys->GetVsRound();
		KingdomGlobal::MotosSys->nVsRoundCount() = KingdomGlobal::MotosSys->nForceVsRoundCount();
		KingdomGlobal::pMotosGameSys->nVSItemNum() = (byte)KingdomGlobal::MotosSys->GetVsItem();
		for (uint num = 0u; num < 4; num++)
		{
			KingdomGlobal::pMotosGameSys->KeepData()[num]->m_nItemNum[1] = KingdomGlobal::pMotosGameSys->nVSItemNum();
		}
	}
	m_bMultiPlay = true;
	KingdomGlobal::MotosSys->nVsTimeCount() = (int)KingdomGlobal::MotosSys->GetVsTime() / 1000;
	KingdomGlobal::pMotosGameSys->nVSLimitTime() = (ushort)(KingdomGlobal::MotosSys->nVsTimeCount() * 60);
	for (int i = 0; i < (int)std::size(m_nPoints); i++)
	{
		m_nPoints[i] = 0;
	}
	for (uint num2 = 0u; num2 < 4; num2++)
	{
		for (uint num3 = 0u; num3 < 4; num3++)
		{
			KingdomGlobal::pMotosGameSys->KeepData()[num2]->m_nUseItem[num3] = 0;
		}
	}
}

void cAppG07MotosGame::SinglePlaySettings()
{
}

void cAppG07MotosGame::BgmStart()
{
	int bgmNo = GetBgmNo();
	if (bgmNo != 0)
	{
		SoundManager::get_BGM()->Play((EBGMKind)bgmNo);
	}
}

int cAppG07MotosGame::GetBgmNo()
{
	if (KingdomGlobal::MotosSys->nStageIdx() == 35 && !m_bMultiPlay)
	{
		if (!bMultiPlay() && m_bFirstPlay)
		{
			return -1;
		}
		return 48;
	}
	if (KingdomGlobal::MotosSys->nStage() == 7 && !m_bMultiPlay)
	{
		return 47;
	}
	if (KingdomGlobal::MotosSys->nStageIdx() == 0 && !m_bMultiPlay)
	{
		return 43;
	}
	switch (KingdomGlobal::MotosSys->nWorld())
	{
	case 1u:
	case 3u:
		return 44;
	case 2u:
	case 4u:
		return 45;
	case 5u:
		return 46;
	default:
		// TODO(unity): Debug.LogError("!?");
		// TODO(unity): Debug.LogError("!?");
		return 0;
	}
}

bool cAppG07MotosGame::Execute(cJetTask* pParam)
{
	{ static int dbg = 0; static uint last = ~0u; static const bool on = std::getenv("MEGAMIX_SCENE_DEBUG") != nullptr; if (on && (++dbg % 60 == 0 || last != (uint)nStep(0)) && (last = (uint)nStep(0), true)) std::fprintf(stderr,"motos: frame %d step %u pause %d fade end %d\n", dbg, nStep(0), (int)IsPause(), (int)m_pTaskFade->IsEnd()); }
	KingdomGlobal::pMotosGameSys->NET_Sync();
	if ((nStep(0) != 6 || KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nLife != 0) && nStep(0) != 2)
	{
		HomeButtonCheck();
	}
	if (!IsPause() && !(devtools::G07EditFrozen() && (nStep(0) == 7 || nStep(0) == 8))) // devtools: the Motos editor freezes READY / GAME (not 1:1)
	{
		switch (nStep(0))
		{
		case 2:
			StepStartDirection();
			break;
		case 3:
			StepClearDirection();
			break;
		case 4:
			StepMultiBattleEnd();
			break;
		case 5:
			StepChampion();
			break;
		case 6:
			StepMissDirection();
			break;
		case 1:
			StepItemSet();
			break;
		case 7:
			StepReady();
			break;
		case 8:
			StepGame();
			break;
		case 10:
			StepContinue();
			break;
		case 16:
			StepSync();
			break;
		default:
			CustomKingdomUtil::Assert("error");
			break;
		}
		if (nStep(0) != 1)
		{
			if (!bMultiPlay())
			{
				CameraPositionMoveSingle();
			}
			else
			{
				CameraPositionMoveMulti();
			}
		}
	}
	MotosGetTime->GetTime();
	KingdomGlobal::pMotosGameSys->OnSlaveFlag(KingdomGlobal::pMotosGameSys->ID(), 1u);
	PointsUpdate();
	GameCounterBackup();
	Constants::set_SYNC_COUNTER_LOCAL(Constants::get_SYNC_COUNTER_LOCAL() + 1u);
	if (!Constants::MOTOS_IS_PAUSE())
	{
		Constants::set_GAME_COUNTER_LOCAL(Constants::get_GAME_COUNTER_LOCAL() + 1u);
	}
	Constants::set_SYNC_COUNTER(Constants::get_SYNC_COUNTER() + 1u);
	if (!Constants::MOTOS_IS_PAUSE())
	{
		Constants::set_GAME_COUNTER(Constants::get_GAME_COUNTER() + 1u);
	}
	return true;
}

void cAppG07MotosGame::HomeButtonCheck()
{
	if (bMultiPlay())
	{
		for (uint num = 0u; num < KingdomGlobal::MotosSys->pGetMatchingData()->nNum; num++)
		{
			if (InGamePadManager::GetKeyDown(InGamePadManager::GetPadNum((int)KingdomGlobal::MotosSys->pGetMatchingData()->Info[num]->nCtrl), PadDefine::EPadKeycode::eKEY_START))
			{
				m_pTaskPause->Start();
				break;
			}
		}
	}
	else if (InGamePadManager::GetKeyDown(InGamePadManager::GetPadNumFirst(), PadDefine::EPadKeycode::eKEY_START))
	{
		m_pTaskPause->Start();
	}
	IsPause() = m_pTaskPause->IsPause();
}

void cAppG07MotosGame::CameraPositionMoveSingle()
{
}

void cAppG07MotosGame::CameraPositionMoveMulti()
{
}

void cAppG07MotosGame::CheckEnemyDead()
{
	for (uint num = 0u; num < 36; num++)
	{
		cTaskChar* cTaskChar2 = m_pChars[num];
		if (cTaskChar2 != nullptr && cTaskChar2->bTestFlag(16384u))
		{
			int set = (int)cTaskChar2->GrobalID();
			KingdomGlobal::MotosSys->bMotosCharDead((uint)set) = true;
		}
	}
}

void cAppG07MotosGame::Draw(int nStatus, cJetTask* pParam)
{
}

void cAppG07MotosGame::SlopeUpdate()
{
	float num = pCamera()->vPosition()[0];
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	v[0] = num;
	Constants::tspVuMtxUnit(m_mSlopeMatrix);
	::Constants::jetGeoVecUnit(m_vGravVector);
	m_vGravVector[1] = -1.0f;
	::Constants::jetGeoVecUnit(m_vSlopePower);
	if (CustomKingdomUtil::IF(m_fSlopeRad))
	{
		::Constants::jetGeoMtxTrans(m_mSlopeMatrix, m_mSlopeMatrix, Extentions::GetUnityPos(v));
		::Constants::jetGeoMtxRotZ(m_mSlopeMatrix, m_mSlopeMatrix, m_fSlopeRad);
		::Constants::jetGeoVecSet(v, 0.0f - num, 0.0f, 0.0f, 1.0f);
		::Constants::jetGeoMtxTrans(m_mSlopeMatrix, m_mSlopeMatrix, Extentions::GetUnityPos(v));
		Matrix4x4 m;
		Constants::tspVuMtxUnit(m);
		Constants::tspVuMtxRotZ(m, m, 0.0f - m_fSlopeRad);
		Constants::tspVuMtxApply(m_vGravVector, m, m_vGravVector);
		m_vSlopePower[0] = 1.0f;
		m_vSlopePower[0] = Constants::Motos_P_vecDot(m_vSlopePower, m_vGravVector);
	}
	m_fSlopeRadOLD = m_fSlopeRad;
}

void cAppG07MotosGame::CameraDirPos(Vector4 vTrans, Vector4 vTarget, float div)
{
	::Constants::jetGeoVecCopy(pCamera()->vTarget(), vTarget);
	Vector4 v;
	::Constants::jetGeoVecCopy(v, pCamera()->vPosition());
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, pCamera()->vTarget());
	Vector4 v3;
	::Constants::jetGeoVecCopy(v3, vTrans);
	v3[1] += 0.5f;
	v2[0] = v[0] - v2[0];
	v2[1] = v[1] - v2[1];
	v2[2] = v[2] - v2[2];
	v2[0] /= div;
	v2[1] /= div;
	v2[2] /= div;
	v[0] += v2[0];
	v[1] += v2[1];
	v[2] += v2[2];
	div = 10.0f;
	pCamera()->vPosition()[0] -= (v[0] - v3[0]) / div;
	pCamera()->vPosition()[1] -= (v[1] - v3[1]) / div;
	pCamera()->vPosition()[2] -= (v[2] - v3[2]) / div;
}

bool cAppG07MotosGame::RegistChars(cTaskChar* pChar)
{
	for (int i = 0; i < 36; i++)
	{
		if (m_pChars[i] == nullptr)
		{
			m_pChars[i] = pChar;
			pChar->GrobalID((uint)i);
			return true;
		}
	}
	return false;
}

bool cAppG07MotosGame::RegistPlayers(cTaskCharPlayer* pChar)
{
	if (pChar != nullptr && RegistChars(pChar))
	{
		if (m_nPLReserve != -1)
		{
			CustomKingdomUtil::Assert(m_pPlayers[m_nPLReserve] != nullptr, "RegistPlayers Error");
			m_pPlayers[m_nPLReserve] = pChar;
			pChar->ID((uint)m_nPLReserve);
			m_nPLReserve = -1;
			return true;
		}
		for (int i = 0; i < 4; i++)
		{
			if (m_pPlayers[i] == nullptr)
			{
				m_pPlayers[i] = pChar;
				pChar->ID((uint)i);
				return true;
			}
		}
	}
	return false;
}

bool cAppG07MotosGame::RegistEnemys(cTaskCharEnemy* pChar)
{
	if (pChar != nullptr && RegistChars(pChar))
	{
		for (int i = 0; i < 32; i++)
		{
			if (m_pEnemys[i] == nullptr)
			{
				m_pEnemys[i] = pChar;
				pChar->ID((uint)i);
				return true;
			}
		}
	}
	return false;
}

void cAppG07MotosGame::PointsUpdate()
{
	if (nStep(0) != 8 || kingdom::Constants::IS_PAUSE() || Constants::IS_CLEARPAUSE())
	{
		return;
	}
	for (int i = 0; i < 36; i++)
	{
		cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)i);
		if (cTaskChar2 == nullptr)
		{
			continue;
		}
		int nTargetID = cTaskChar2->pGivePoints()->m_nTargetID;
		if (nTargetID != -1 && CustomKingdomUtil::IF(cTaskChar2->pGivePoints()->m_nGameCounter) && cTaskChar2->pGivePoints()->m_nGameCounter != nPointCount(i))
		{
			if (!CustomKingdomUtil::IF(nTargetID & 0x80))
			{
				nPoints(nTargetID) += cTaskChar2->pGivePoints()->m_nAddPoints;
			}
			nPointCount(i) = cTaskChar2->pGivePoints()->m_nGameCounter;
			if (cTaskChar2->bTestFlag(2u))
			{
				nPoints((int)cTaskChar2->ID()) -= cTaskChar2->pGivePoints()->m_nSubPoints;
			}
		}
	}
	if (m_bMultiPlay)
	{
		LandPoints();
		for (int j = 0; j < 4; j++)
		{
			cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pPlayers((uint)j);
			if (cTaskChar2 != nullptr)
			{
				if (nPoints(j) > 999)
				{
					nPoints(j) = 999;
				}
				else if (nPoints(j) < 0)
				{
					nPoints(j) = 0;
				}
			}
		}
		for (int k = 0; k < 4; k++)
		{
			cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pPlayers((uint)k);
			if (cTaskChar2 != nullptr && Constants::IS_JOIN(k))
			{
				nPRanking(k) = (sbyte)k;
			}
			else
			{
				nPRanking(k) = -1;
			}
		}
		for (int l = 0; l < 3; l++)
		{
			for (int m = l + 1; m < 4; m++)
			{
				if (nPRanking(l) >= 0 && nPRanking(m) >= 0 && (nPoints(nPRanking(l)) < nPoints(nPRanking(m)) || (nPoints(nPRanking(l)) == nPoints(nPRanking(m)) && nPRanking(l) > nPRanking(m))))
				{
					int num = nPRanking(l);
					nPRanking(l) = nPRanking(m);
					nPRanking(m) = (sbyte)num;
				}
			}
		}
		return;
	}
	for (int n = 0; n < 4; n++)
	{
		cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pPlayers((uint)n);
		if (cTaskChar2 != nullptr)
		{
			if (nPoints(n) > 999999)
			{
				nPoints(n) = 999999;
			}
			else if (nPoints(n) < 0)
			{
				nPoints(n) = 0;
			}
		}
	}
}

void cAppG07MotosGame::LandPoints()
{
	for (int i = 0; i < 4; i++)
	{
		bool flag = false;
		cTaskChar* cTaskChar2 = pPlayers((uint)i);
		if (cTaskChar2 == nullptr)
		{
			continue;
		}
		if (!cTaskChar2->bTestFlag(279008u))
		{
			Vector4 v;
			::Constants::jetGeoVecCopy(v, cTaskChar2->vMTrans());
			if (!flag && KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(v[0], v[1], v[2] - KingdomGlobal::g_pMotosGame->pMap()->fPanelSize()))
			{
				flag = true;
			}
			if (!flag && KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(v[0], v[1], v[2] + KingdomGlobal::g_pMotosGame->pMap()->fPanelSize()))
			{
				flag = true;
			}
			if (!flag && KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(v[0] + KingdomGlobal::g_pMotosGame->pMap()->fPanelSize(), v[1], v[2]))
			{
				flag = true;
			}
			if (!flag && KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(v[0] - KingdomGlobal::g_pMotosGame->pMap()->fPanelSize(), v[1], v[2]))
			{
				flag = true;
			}
		}
		if (flag)
		{
			if (!CustomKingdomUtil::IF(nLandCount((int)cTaskChar2->ID())))
			{
				nLandCount((int)cTaskChar2->ID()) = Constants::get_GAME_COUNTER();
			}
			else if ((long)nLandCount((int)cTaskChar2->ID()) + (long)Constants::get_MO_VS_LAND_POINT_FRAME() <= Constants::get_GAME_COUNTER())
			{
				nPoints((int)cTaskChar2->ID()) += Constants::get_MO_VS_LAND_POINT_POINT();
				nLandCount((int)cTaskChar2->ID()) = 0u;
			}
		}
		else
		{
			nLandCount((int)cTaskChar2->ID()) = 0u;
		}
	}
}

void cAppG07MotosGame::StepItemSet()
{
	int padNum = InGamePadManager::GetPadNum((int)KingdomGlobal::MotosSys->pGetMatchingData()->Info[m_nPlayerParamSet]->nCtrl);
	if (!m_bMultiPlay) devtools::G07EditItemStock(KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nItemNum); // devtools: Motos editor test stock 99 (not 1:1)
	if (nStep(3)++ == 0)
	{
		m_nPlayerParamSet = 0u;
		Constants::IS_BOSS_STAGE();
	}
	if (!m_pTaskFade->IsEnd())
	{
		return;
	}
	int num = nStep(1);
	int num2 = -1;
	if (nStep(1) == num)
	{
		switch (nStep(1))
		{
		case 0:
			num2 = 1;
			break;
		case 1:
			num2 = 2;
			break;
		case 2:
			num2 = 3;
			break;
		}
	}
	if (nStep(1) != 4)
	{
		if (InGamePadManager::GetEnterKeyDown(padNum) || devtools::G07SkipItemSet()) // devtools: the Motos editor's "Skip the item settings" (not 1:1)
		{
			nStep(1) = 3;
			nStep(2) = 0;
			KingdomGlobal::g_pMotosGame->pPlayers(m_nPlayerParamSet)->UseItem();
			KingdomGlobal::g_pMotosGame->pPlayers(m_nPlayerParamSet)->Create(m_nPlayerParamSet);
			KingdomGlobal::g_pMotosGame->pPlayers(0u)->SetAnimation(1u);
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE033_ITEM_);
			if (m_bMultiPlay)
			{
				KingdomGlobal::pMotosGameSys->KeepData()[m_nPlayerParamSet]->m_nItemNum[1] -= (byte)(KingdomGlobal::pMotosGameSys->KeepData()[m_nPlayerParamSet]->m_nUseItem[1] + KingdomGlobal::pMotosGameSys->KeepData()[m_nPlayerParamSet]->m_nUseItem[2] + KingdomGlobal::pMotosGameSys->KeepData()[m_nPlayerParamSet]->m_nUseItem[3]);
				nStep(1) = 0;
			}
			bool flag = false;
			m_nPlayerParamSet++;
			if (m_bMultiPlay)
			{
				if (m_nPlayerParamSet == KingdomGlobal::MotosSys->pGetMatchingData()->nNum)
				{
					flag = true;
				}
			}
			else if (m_nPlayerParamSet == 1)
			{
				flag = true;
			}
			if (flag)
			{
				STEP2SYNC(1);
			}
		}
		else if (InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_DOWN) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_DOWN))
		{
			nStep(1)++;
			nStep(1) %= 3;
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE032_ITEM_SELECT);
		}
		else if (InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_UP) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_UP))
		{
			if (--nStep(1) < 0)
			{
				nStep(1) = 2;
			}
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE032_ITEM_SELECT);
		}
	}
	if (nStep(1) >= 3 || (num2 == -1 && !Constants::IS_LANPLAY()))
	{
		return;
	}
	// the last player has confirmed (STEP2SYNC above): m_nPlayerParamSet == player count. The C# indexed KeepData() with it anyway and Unity
	// swallowed the IndexOutOfRangeException for that frame; natively it read past the 4-entry array (segfault after 4 players)
	if (m_nPlayerParamSet >= 4u || (m_bMultiPlay && m_nPlayerParamSet >= KingdomGlobal::MotosSys->pGetMatchingData()->nNum))
	{
		return;
	}
	if (!m_bMultiPlay)
	{
		{
			byte* ptr = &KingdomGlobal::pMotosGameSys->KeepData()[m_nPlayerParamSet]->m_nUseItem[num2];
			{
				{
					byte* ptr2 = &KingdomGlobal::pMotosGameSys->KeepData()[m_nPlayerParamSet]->m_nItemNum[num2];
					{
						int num3 = *ptr;
						(void)(*ptr2);
						if (nullptr != ptr && nullptr != ptr2)
						{
							if ((InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_RIGHT) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_RIGHT)) && *ptr < 5 && num3 < *ptr2)
							{
								(*ptr)++;
								SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE032_ITEM_SELECT);
							}
							if ((InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LEFT) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_LEFT)) && *ptr > 0)
							{
								(*ptr)--;
								SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE032_ITEM_SELECT);
							}
						}
					}
				}
			}
		}
		return;
	}
	{
		byte* ptr3 = &KingdomGlobal::pMotosGameSys->KeepData()[m_nPlayerParamSet]->m_nUseItem[num2];
		{
			{
				byte* ptr4 = &KingdomGlobal::pMotosGameSys->KeepData()[m_nPlayerParamSet]->m_nItemNum[1];
				{
					int num4 = KingdomGlobal::pMotosGameSys->KeepData()[m_nPlayerParamSet]->m_nUseItem[1] + KingdomGlobal::pMotosGameSys->KeepData()[m_nPlayerParamSet]->m_nUseItem[2] + KingdomGlobal::pMotosGameSys->KeepData()[m_nPlayerParamSet]->m_nUseItem[3];
					(void)(*ptr4);
					if (nullptr != ptr3 && nullptr != ptr4)
					{
						if ((InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_RIGHT) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_RIGHT)) && *ptr3 < 5 && num4 < *ptr4)
						{
							(*ptr3)++;
							SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE032_ITEM_SELECT);
						}
						if ((InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LEFT) || InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_LEFT)) && *ptr3 > 0)
						{
							(*ptr3)--;
							SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE032_ITEM_SELECT);
						}
					}
				}
			}
		}
	}
}

void cAppG07MotosGame::StepMissDirection()
{
	switch (nStep(1))
	{
	case 0:
		nStep(1)++;
		if ((long)KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nLife == 0)
		{
			SoundManager::get_BGM()->Stop(1.0f);
			m_pTaskConsRetry->Start();
			m_pTaskConsRetry->nDrawPriority(m_pTaskConsoleMan->nPriority());
			if (!m_bMultiPlay)
			{
				SGameRecordInfo& record2 = GameStateManager::GetRecord(GameDefine::EGameTitle::ePACMOTOS);
				record2.m_totalEatGhost += KingdomGlobal::pMotosGameSys->get_DeadEnemyCnt();
				KingdomGlobal::pMotosGameSys->ResetDeadEnemyCnt();
				record2.m_totalEatItem += KingdomGlobal::pMotosGameSys->get_GetItemCnt();
				KingdomGlobal::pMotosGameSys->ResetGetItemCnt();
			}
		}
		break;
	case 1:
		if ((long)KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nLife != 0)
		{
			if (nStep(3)++ == 0)
			{
				SoundManager::get_BGM()->Stop(1.0f);
				m_pTaskFade->Fade(65543u);
			}
			if (m_pTaskFade->IsEnd())
			{
				KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nMyScore = (uint)nScore();
				kingdom::Constants::Kingdom_SetJobNo(288358400u);
			}
		}
		else if (m_pTaskConsRetry->IsYes())
		{
			m_nGamePlayTime = 0u;
			if (nStep(3)++ == 0)
			{
				m_pTaskFade->Fade(65543u);
			}
			if (m_pTaskFade->IsEnd())
			{
				kingdom::Constants::Kingdom_SetJobNo(288358400u);
				KingdomGlobal::MotosSys->bPlayerRetry() = true;
				KingdomGlobal::MotosSys->SaveDataLoad();
				KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nLife = (short)cG07MotosSys::DEFAULT_PLAYER_LIFE;
				KingdomGlobal::MotosSys->WriteMemoryToSaveData(true);
			}
		}
		else if (m_pTaskConsRetry->IsNo())
		{
			if (nStep(3)++ == 0)
			{
				m_pTaskFade->Fade(65543u);
			}
			if (m_pTaskFade->IsEnd())
			{
				SGameRecordInfo& record = GameStateManager::GetRecord(GameDefine::EGameTitle::ePACMOTOS);
				record.m_totalEatGhost += KingdomGlobal::pMotosGameSys->get_DeadEnemyCnt();
				record.m_totalEatItem += KingdomGlobal::pMotosGameSys->get_GetItemCnt();
				GameStateManager::GetResult()->AddScore(GameDefine::scGAMEOVER_TIME, (int)KingdomGlobal::MotosSys->nWorld(), (int)KingdomGlobal::MotosSys->nStage());
				KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nLife = (short)cG07MotosSys::DEFAULT_PLAYER_LIFE;
				kingdom::Constants::Kingdom_SetJobNo(290455552u);
				KingdomGlobal::MotosSys->SaveDataLoad();
			}
		}
		break;
	}
}

void cAppG07MotosGame::StepReady()
{
	int padNumFirst = InGamePadManager::GetPadNumFirst();
	if (IsPause())
	{
		return;
	}
	switch (nStep(1))
	{
	case 0:
		nStep(3) = 0;
		pCamera()->LoadVector();
		nStep(1)++;
		break;
	case 1:
		if (!Constants::IS_LANPLAY() && InGamePadManager::GetEnterKeyDown(padNumFirst))
		{
			nStep(1) = 3;
		}
		else if (++nStep(3) > 10)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE015_JINGLE1);
			nStep(2) = 253;
			nStep(1)++;
		}
		break;
	case 2:
		if (!Constants::IS_LANPLAY() && InGamePadManager::GetEnterKeyDown(padNumFirst))
		{
			SoundManager::get_SE()->Stop((ESEKind)nStep(2));
			nStep(1)++;
		}
		else if (++nStep(3) > 185)
		{
			nStep(1)++;
		}
		break;
	case 3:
		STEP2SYNC(2);
		break;
	}
}

void cAppG07MotosGame::StepGame()
{
	if (KingdomGlobal::pMotosGameSys->bTestMasterFlag(1u) || (Constants::IS_LANPLAY() && KingdomGlobal::pMotosGameSys->bTestMasterFlag(2u)))
	{
		KingdomGlobal::pMotosGameSys->OffMasterFlag(32u);
		STEP(3);
		return;
	}
	if (!Constants::MOTOS_IS_PAUSE() && !InGamePadManager::get_Disable())
	{
		m_nGamePlayTime += MotosGetTime->GetMilliSeconds();
		if (m_nGamePlayTime > PLAY_TIME_MAX)
		{
			m_nGamePlayTime = PLAY_TIME_MAX;
		}
	}
	SlopeUpdate();
	if (fComingSELength() < 5.0f)
	{
		uint num = 271u;
		if (++nStep(1) >= 30 || num < 0 || !SoundManager::get_SE()->IsPlayinSEKind((ESEKind)num))
		{
			nStep(1) = 0;
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE034_GIGA_PETA, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
		}
	}
	fComingSELength() = 5.0f;
	if (!m_bMultiPlay)
	{
		cTaskCharEnemy* cTaskCharEnemy2 = m_pEnemys[0];
		bool flag = true;
		bool flag2 = true;
		int num2 = 0;
		int num3 = 0;
		while (cTaskCharEnemy2 != nullptr)
		{
			if (cTaskCharEnemy2->bTestFlag(4u))
			{
				if (!cTaskCharEnemy2->bTestFlag(8u))
				{
					uint num4 = cTaskCharEnemy2->nKind();
					if (num4 - 14 > 1 && num4 - 22 > 1 && num4 != 25)
					{
						if (!cTaskCharEnemy2->bTestFlag(16384u))
						{
							flag = false;
						}
						if (!cTaskCharEnemy2->bTestFlag(16448u))
						{
							flag2 = false;
						}
					}
				}
				else
				{
					cTaskCharBoss* obj = static_cast<cTaskCharBoss*>(cTaskCharEnemy2);
					if (!cTaskCharEnemy2->bTestFlag(16384u))
					{
						flag = false;
					}
					if (obj->nHp() <= 0)
					{
						if (!IsClearReserve())
						{
							flag2 = true;
							KingdomGlobal::pMotosGameSys->AddDeadEnemyCnt();
						}
					}
					else
					{
						flag2 = false;
					}
				}
			}
			if (cTaskCharEnemy2->bTestFlag(16384u))
			{
				num3 |= 1 << num2;
			}
			cTaskCharEnemy2 = m_pEnemys[++num2];
		}
		if (!IsMissedReserve())
		{
			IsClearReserve() = flag2;
		}
		if (((Constants::get_GAME_COUNTER() > 10) & flag) && num2 != 0)
		{
			KingdomGlobal::pMotosGameSys->OnMasterFlag(1u);
			return;
		}
		if (pPlayers(0u)->bTestFlag(64u))
		{
			IsMissedReserve() = true;
		}
		if (!pPlayers(0u)->bTestFlag(16384u) || IsClearReserve())
		{
			return;
		}
		if (KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nLife != 0 && --KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nLife > 0)
		{
			if (flag && num2 != 0)
			{
				KingdomGlobal::pMotosGameSys->OnMasterFlag(1u);
				return;
			}
			KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nEnemyTaikiBits |= num3;
			STEP(6);
		}
		else
		{
			STEP(6);
		}
		return;
	}
	if (KingdomGlobal::MotosSys->nVsTimeCount() > 0 && !CustomKingdomUtil::IF(Constants::get_GAME_COUNTER() % 60))
	{
		KingdomGlobal::MotosSys->nVsTimeCount()--;
		if (KingdomGlobal::MotosSys->nVsTimeCount() <= 10)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE050_COUNTDOWN, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
		}
	}
	if (KingdomGlobal::MotosSys->nVsTimeCount() == 0)
	{
		STEP(4);
		SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE050_COUNTDOWN);
	}
}

void cAppG07MotosGame::StepStartDirection()
{
	int pad1No = InGamePadManager::GetPad1No();
	cTaskCharBoss* cTaskCharBoss2 = static_cast<cTaskCharBoss*>(m_pBoss);
	if (nStep(1) == 0 && (InGamePadManager::GetEnterKeyDown(pad1No) || cTaskCharBoss2->IsApearDemoEnd()))
	{
		STEP(1);
	}
}

void cAppG07MotosGame::StepClearDirection()
{
	int pad1No = InGamePadManager::GetPad1No();
	if (Constants::IS_LANPLAY())
	{
		return;
	}
	switch (nStep(1))
	{
	case 0:
		SoundManager::get_BGM()->Stop(1.0f);
		SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE047_BOSS_FALL);
		SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE007_BOSS_DEATH);
		nStep(1)++;
		nStep(2) = 0;
		break;
	case 1:
		if (nStep(2)++ == 60)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE016_GAMECLEAR);
			nStep(1)++;
			m_nSaveFlags = KingdomGlobal::MotosSys->SaveDataSave((uint)KingdomGlobal::g_pMotosGame->nPoints(0), m_nGamePlayTime, true, false);
		}
		break;
	case 2:
		if (!SoundManager::get_SE()->IsPlayinSEKind(ESEKind::MOTOS_SND_SE016_GAMECLEAR) || (InGamePadManager::GetEnterKeyDown(pad1No) && m_nSaveFlags != 1))
		{
			SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE016_GAMECLEAR);
			if (m_nSaveFlags == 1)
			{
				SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE06_HISCORE);
				nStep(1)++;
			}
			else
			{
				nStep(1)++;
				nStep(1)++;
				m_pTaskFade->Fade(EFadeKind::eFADE_TYPE_OUT_G07);
			}
		}
		break;
	case 3:
		if (!SoundManager::get_SE()->IsPlayinSEKind(ESEKind::RESIDENT_SND_SE06_HISCORE))
		{
			nStep(1)++;
			m_pTaskFade->Fade(EFadeKind::eFADE_TYPE_OUT_G07);
		}
		break;
	case 4:
		{
			if (!m_pTaskFade->IsEnd())
			{
				break;
			}
			if (!m_bMultiPlay && !devtools::G07EditItemRestore(KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nItemNum)) // devtools: test stock back, nothing used (not 1:1)
			{
				for (uint num = 1u; num < 4; num++)
				{
					byte b = KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nUseItem[num];
					KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nItemNum[num] -= b;
				}
			}
			if (!m_bMultiPlay)
			{
				SGameRecordInfo& record = GameStateManager::GetRecord(GameDefine::EGameTitle::ePACMOTOS);
				int num2 = (int)KingdomGlobal::MotosSys->nStageIdx();
				int nGamePlayTime = (int)m_nGamePlayTime;
				record.SetClearTime(num2 - 1, nGamePlayTime);
				record.m_maxArrivalRound = Mathf::Max(record->m_maxArrivalRound, num2 + 1);
				record.m_totalEatGhost += KingdomGlobal::pMotosGameSys->get_DeadEnemyCnt();
				KingdomGlobal::pMotosGameSys->ResetDeadEnemyCnt();
				record.m_totalEatItem += KingdomGlobal::pMotosGameSys->get_GetItemCnt();
				KingdomGlobal::pMotosGameSys->ResetGetItemCnt();
				ResultInfo* result = GameStateManager::GetResult();
				for (int i = 1; i < (int)result->m_use5Item.size(); i++)
				{
					if (5 <= KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nUseItem[i])
					{
						result->m_use5Item[i] = true;
					}
				}
				GameStateManager::GetResult()->AddScore(nGamePlayTime, (int)KingdomGlobal::MotosSys->nWorld(), (int)KingdomGlobal::MotosSys->nStage());
			}
			bool save = ((m_nSaveFlags != 0) ? true : false);
			if (KingdomGlobal::MotosSys->nStageIdx() == 35 && !KingdomGlobal::MotosSys->IsAllClear())
			{
				kingdom::Constants::Kingdom_SetJobNo(299892736u);
			}
			else
			{
				kingdom::Constants::Kingdom_SetJobNo(287309824u, save);
			}
			KingdomGlobal::MotosSys->SaveDataSave((uint)KingdomGlobal::g_pMotosGame->nPoints(0), m_nGamePlayTime, true);
			if (KingdomGlobal::MotosSys->IsAllClear())
			{
				int num3 = 0;
				for (uint num4 = 0u; num4 < 36; num4++)
				{
					num3 += (int)KingdomGlobal::MotosSys->pScoreData(num4).nRecordTime;
				}
				GameStateManager::GetRecord(GameDefine::EGameTitle::ePACMOTOS).SetHighScoreTime(0, num3);
			}
			break;
		}
	}
}

void cAppG07MotosGame::CheckMultiBattleWinnerOfRound()
{
	int num = -1;
	int num2 = 0;
	for (int i = 0; i < 4; i++)
	{
		if (nPoints(i) > num)
		{
			num = nPoints(i);
		}
	}
	for (int j = 0; j < 4; j++)
	{
		if (nPoints(j) == num)
		{
			num2++;
			m_n1stPointPlayer = j;
		}
	}
	if (num2 > 1)
	{
		m_nMultiBattleResult = 2;
	}
	if (m_nMultiBattleResult == 2)
	{
		for (uint num3 = 0u; num3 < 4; num3++)
		{
			if (nPoints((int)num3) != num)
			{
				continue;
			}
			for (uint num4 = 0u; num4 < 4; num4++)
			{
				if (m_naMultiPlayerDrawList[num4] == DRAW_PLAYER_NONE)
				{
					m_naMultiPlayerDrawList[num4] = (int)num3;
					break;
				}
			}
		}
	}
	else
	{
		KingdomGlobal::MotosSys->nWinsRound((uint)m_n1stPointPlayer)++;
	}
}

void cAppG07MotosGame::CheckMultiBattleWinnerOfGame()
{
	int num = -1;
	int num2 = 0;
	for (int i = 0; i < 4; i++)
	{
		bool flag = false;
		int num3 = (int)KingdomGlobal::MotosSys->nWinsRound((uint)i);
		if (num3 > num)
		{
			num2 = 1;
			m_n1stPointPlayer = i;
			flag = true;
		}
		else if (num3 == num)
		{
			num2++;
			flag = true;
		}
		if (flag)
		{
			num = num3;
		}
	}
	if (num2 > 1)
	{
		m_nMultiBattleResult = 2;
	}
	else
	{
		m_nMultiBattleResult = 1;
	}
	if (m_nMultiBattleResult != 2)
	{
		return;
	}
	for (int j = 0; j < (int)std::size(m_naMultiPlayerDrawList); j++)
	{
		m_naMultiPlayerDrawList[j] = DRAW_PLAYER_NONE;
	}
	for (uint num4 = 0u; num4 < 4; num4++)
	{
		if (KingdomGlobal::MotosSys->nWinsRound(num4) != (uint)num)
		{
			continue;
		}
		for (uint num5 = 0u; num5 < 4; num5++)
		{
			if (m_naMultiPlayerDrawList[num5] == DRAW_PLAYER_NONE)
			{
				m_naMultiPlayerDrawList[num5] = (int)num4;
				break;
			}
		}
	}
}

void cAppG07MotosGame::StepMultiBattleEnd()
{
	switch (nStep(1))
	{
	case 0:
		pTaskEffect()->AllDetach();
		SoundManager::get_BGM()->Stop(1.0f);
		m_IsGameStart = false;
		m_nMultiBattleResult = 1;
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE016_GAMECLEAR);
		nStep(1)++;
		nStep(2) = 0;
		if (SoundManager::get_SE()->IsPlayinSEKind(ESEKind::MOTOS_SND_SE009_CHARGE))
		{
			SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE009_CHARGE);
		}
		if (SoundManager::get_SE()->IsPlayinSEKind(ESEKind::MOTOS_SND_SE010_CHARGE_MAX))
		{
			SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE010_CHARGE_MAX);
		}
		break;
	case 1:
		if (nStep(2)++ == 160)
		{
			CheckMultiBattleWinnerOfRound();
			KingdomGlobal::MotosSys->nVsRoundCount()--;
			if (KingdomGlobal::MotosSys->nVsRoundCount() == 0)
			{
				STEP(5);
				break;
			}
			m_pTaskFade->Fade(EFadeKind::eFADE_TYPE_OUT_G07);
			nStep(1)++;
			nStep(2) = 0;
		}
		break;
	case 2:
		if (m_pTaskFade->IsEnd())
		{
			kingdom::Constants::Kingdom_SetJobNo(288358400u);
		}
		break;
	}
}

void cAppG07MotosGame::StepChampion()
{
	switch (nStep(1))
	{
	case 0:
		{
			CheckMultiBattleWinnerOfGame();
			if (m_nMultiBattleResult != 2)
			{
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE049_JINGLE3);
			}
			for (uint num = 0u; num < 4; num++)
			{
				for (uint num2 = 0u; num2 < 4; num2++)
				{
					KingdomGlobal::pMotosGameSys->KeepData()[num]->m_nItemNum[num2] = 0;
				}
			}
			nStep(1)++;
			break;
		}
	case 1:
		{
			nStep(2)++;
			if (nStep(2) > 360)
			{
				nStep(2) = 360;
			}
			bool flag = !SoundManager::get_SE()->IsPlayinSEKind(ESEKind::MOTOS_SND_SE049_JINGLE3) || m_nMultiBattleResult == 2;
			if (!((nStep(2) > 60) & flag))
			{
				break;
			}
			for (uint num3 = 0u; num3 < KingdomGlobal::MotosSys->pGetMatchingData()->nNum; num3++)
			{
				int padNum = InGamePadManager::GetPadNum((int)KingdomGlobal::MotosSys->pGetMatchingData()->Info[num3]->nCtrl);
				if (nStep(2) == 360 || InGamePadManager::GetEnterKeyDown(padNum))
				{
					m_pTaskFade->Fade(EFadeKind::eFADE_TYPE_OUT_G07);
					nStep(1)++;
					break;
				}
			}
			break;
		}
	case 2:
		if (m_pTaskFade->IsEnd())
		{
			kingdom::Constants::Kingdom_SetJobNo(292552704u);
		}
		break;
	}
}

void cAppG07MotosGame::StepContinue()
{
}

void cAppG07MotosGame::StepSync()
{
	switch (nStep(3))
	{
	case 0:
		KingdomGlobal::pMotosGameSys->OnSlaveFlag(Constants::MY_ID(), 8u);
		if (KingdomGlobal::pMotosGameSys->TestSlaveFlagAll(8u) != 0)
		{
			if (!Constants::IS_LANPLAY())
			{
				STEP2EXIT(0);
			}
			else
			{
				STEP(13);
			}
		}
		break;
	case 1:
		STEP(7);
		break;
	case 2:
		KingdomGlobal::pMotosGameSys->OnSlaveFlag(Constants::MY_ID(), 4u);
		BgmStart();
		IsGameStart() = true;
		m_nMeteoTimerStart = (int)Constants::get_GAME_COUNTER();
		STEP(8);
		break;
	case 3:
		KingdomGlobal::pMotosGameSys->OnSlaveFlag(Constants::MY_ID(), 256u);
		if (KingdomGlobal::pMotosGameSys->TestSlaveFlagAll(256u) != 0)
		{
			STEP(9);
		}
		break;
	case 4:
		KingdomGlobal::pMotosGameSys->OffSlaveFlag(Constants::MY_ID(), 256u);
		if (KingdomGlobal::pMotosGameSys->TestSlaveFlagSomebody(256u) == 0)
		{
			STEP(8);
		}
		break;
	case 8:
		KingdomGlobal::pMotosGameSys->OnSlaveFlag(Constants::MY_ID(), 128u);
		if (KingdomGlobal::pMotosGameSys->TestSlaveFlagAll(128u) != 0)
		{
			STEP2EXIT(6);
		}
		break;
	case 5:
		KingdomGlobal::pMotosGameSys->OnSlaveFlag(Constants::MY_ID(), 32u);
		if (KingdomGlobal::pMotosGameSys->TestSlaveFlagAll(32u) != 0)
		{
			if (KingdomGlobal::pMotosGameSys->TestMasterFlag(2u) != 0)
			{
				STEP2EXIT(4);
			}
			else
			{
				STEP(14);
			}
		}
		break;
	case 6:
		KingdomGlobal::pMotosGameSys->OnSlaveFlag(Constants::MY_ID(), 64u);
		if (KingdomGlobal::pMotosGameSys->TestSlaveFlagAll(64u) != 0)
		{
			STEP2EXIT(5);
		}
		break;
	case 7:
		break;
	}
}

float cAppG07MotosGame::GrobalID2HitPower(int nGrobalID, int nParts, int nOtherID, int refID)
{
	cTaskChar* cTaskChar2 = pChars((uint)nGrobalID);
	if (cTaskChar2 != nullptr)
	{
		if (refID != 0 && !cTaskChar2->bTestFlag(16u))
		{
			Constants::MO_CHAR_REFLECT* mO_CHAR_REFLECT = cTaskChar2->SearchRefHistory(refID);
			uint num = (mO_CHAR_REFLECT != nullptr) ? (uint)mO_CHAR_REFLECT->m_nCharFlag : cTaskChar2->TestFlag(std::numeric_limits<uint>::max());   // C# `a?.m_nCharFlag ?? TestFlag(..)`
			if (CustomKingdomUtil::IF(num & 0x400) && !CustomKingdomUtil::IF(num & 0x1000))
			{
				if (mO_CHAR_REFLECT != nullptr)
				{
					int num2 = mO_CHAR_REFLECT->m_nIndex - 1;
					if (num2 < 0)
					{
						num2 += 60;
					}
					mO_CHAR_REFLECT = cTaskChar2->RefHistory(num2);
				}
				else
				{
					mO_CHAR_REFLECT = cTaskChar2->RefHistoryOffset(1);
				}
				if (mO_CHAR_REFLECT != nullptr && mO_CHAR_REFLECT->m_nRefID != 0)
				{
					(void)(mO_CHAR_REFLECT->m_nOtherID);
					return mO_CHAR_REFLECT->m_fPower * 0.8f;
				}
			}
			return cTaskChar2->GetHitPower(nParts, nOtherID);
		}
		return cTaskChar2->GetHitPower(nParts, nOtherID);
	}
	cTaskChar* cTaskChar3 = pChars((uint)nOtherID);
	if (nGrobalID == 129)
	{
		if (cTaskChar3 != nullptr)
		{
			if (cTaskChar3->bTestFlag(16u))
			{
				return 3.0f;
			}
			if (cTaskChar3->bTestFlag(4u) && cTaskChar3->nKind() == 14)
			{
				return 1.5f;
			}
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE022_EXE_REACION);
		}
		return 1.0f;
	}
	return 0.0f;
}

float cAppG07MotosGame::GrobalID2ChargePower(uint nGrobalID, int nParts, uint nOtherID, int refID, int nCharge)
{
	if (!CustomKingdomUtil::IF(nGrobalID & 0x80))
	{
		cTaskChar* cTaskChar2 = pChars(nGrobalID);
		if (cTaskChar2 != nullptr)
		{
			if (nCharge != 0 && cTaskChar2->bTestFlag(2u))
			{
				cTaskCharPlayer* cTaskCharPlayer2 = static_cast<cTaskCharPlayer*>(cTaskChar2);
				return Constants::get_g_ChargeLevel2Power()[cTaskCharPlayer2->nChargePowLevelOLD()];
			}
			return pChars(nGrobalID)->GetChargePower(nParts, (int)nOtherID);
		}
		return 0.0f;
	}
	return 0.5f;
}

float cAppG07MotosGame::Power2Heretic(float power)
{
	return 0.5f * power;
}

uint cAppG07MotosGame::Weight2Heretic(float weight)
{
	Constants::MOTOS_NO_SOURCE("Weight2Heretic");
	return 0u;
}

int cAppG07MotosGame::VSRestTimer()
{
	int num = KingdomGlobal::pMotosGameSys->nVSLimitTime();
	int gAME_COUNTER = (int)Constants::get_GAME_COUNTER();
	if (num != 0)
	{
		return num - gAME_COUNTER;
	}
	return 0;
}

}
