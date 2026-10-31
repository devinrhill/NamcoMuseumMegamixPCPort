#include "kingdom/game/g03/cPacnRollLoopSndMan.hpp"
#include "kingdom/game/g03/cTaskLight.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "platform/devtools.hpp"
#include "platform/devtools_g03gray.hpp"
#include "GamePad.hpp"
#include "wii/Anim.hpp"
#include "wii/Brres.hpp"
#include "wii/Archive.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cKingdomModelBase.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "ReadFileUtil.hpp"
#include "kingdom/game/g03/cTaskWind.hpp"
#include "kingdom/game/g03/cTaskEffect.hpp"
#include "kingdom/game/g03/cTaskShadowCamera.hpp"
#include "kingdom/game/g03/sSaveDataWind.hpp"
#include "ResMdl.hpp"

#include "CSharpCompat.hpp"
#include "cJetTask.hpp"
#include "CullingStageModel.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/game/cTaskConsRetry.hpp"
#include "kingdom/game/g03/cFaceList.hpp"
#include "kingdom/game/g03/cGhostCommonParam.hpp"
#include "kingdom/game/g03/cGhostParam.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBg.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "kingdom/game/g03/cTaskBgGimmickAddinSnd.hpp"
#include "kingdom/game/g03/cTaskBgGimmickArrow.hpp"
#include "kingdom/game/g03/cTaskBgGimmickElevator.hpp"
#include "kingdom/game/g03/cTaskBgGimmickFall.hpp"
#include "kingdom/game/g03/cTaskBgGimmickFallSw.hpp"
#include "kingdom/game/g03/cTaskBgGimmickFloorEvent.hpp"
#include "kingdom/game/g03/cTaskBgGimmickGondola.hpp"
#include "kingdom/game/g03/cTaskBgGimmickIronBall.hpp"
#include "kingdom/game/g03/cTaskBgGimmickPillar.hpp"
#include "kingdom/game/g03/cTaskBgGimmickPistonMan.hpp"
#include "kingdom/game/g03/cTaskBgGimmickSeesaw.hpp"
#include "kingdom/game/g03/cTaskBgGimmickSwitch.hpp"
#include "kingdom/game/g03/cTaskBgGimmickTruck.hpp"
#include "kingdom/game/g03/cTaskBgGimmickWall.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskBox.hpp"
#include "kingdom/game/g03/cTaskCamera.hpp"
#include "kingdom/game/g03/cTaskCameraSet.hpp"
#include "kingdom/game/g03/cTaskCharGolvis.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/cTaskCollision.hpp"
#include "kingdom/game/g03/cTaskConsoleMan.hpp"
#include "kingdom/game/g03/cTaskFallArea.hpp"
#include "kingdom/game/g03/cTaskGate.hpp"
#include "kingdom/game/g03/cTaskGhostMan.hpp"
#include "kingdom/game/g03/cTaskGoal.hpp"
#include "kingdom/game/g03/cTaskItem.hpp"
#include "kingdom/game/g03/cTaskPacFade.hpp"
#include "kingdom/game/g03/cTaskPause.hpp"
#include "kingdom/game/g03/cTaskPoint.hpp"
#include "kingdom/game/g03/cTaskStage.hpp"
#include "kingdom/game/g03/cTaskWater.hpp"
#include "KingdomGlobal.hpp"
#include "ResNode.hpp"
#include <AudioListenerManager.hpp>
#include <Constants.hpp>
#include <EBGMKind.hpp>
#include <ESEKind.hpp>
#include <GameDefine.hpp>
#include "kingdom/MslRand.hpp"
#include <GameStateManager.hpp>
#include <InGamePadManager.hpp>
#include <PadDefine.hpp>
#include <ReleaseObjectManager.hpp>
#include <SGameRecordInfo.hpp>
#include <SoundManager.hpp>
#include <SoundSe.hpp>
#include <SoundSeObject.hpp>
#include <SSaveDataPacnRoll.hpp>

namespace kingdom::game::g03
{

// the stage archive: g03_pacnroll/g03wWWsSS.lzs (the Unity key of pStageArcKey is an asset path). Rounds 26 / 27 (eROUND_WXXS03 / WXXS04) are the
// "wXX" stages (DOL file table 0x80374c88.., last two entries): nWorld / nStage give 0 for them
static string StageArcPath(uint round)
{
	char arc[64];
	if (round >= 26u) std::snprintf(arc, sizeof arc, "g03_pacnroll/g03wXXs%02u.lzs", round - 23u);
	else std::snprintf(arc, sizeof arc, "g03_pacnroll/g03w%02us%02u.lzs", KingdomGlobal::PacnRollSys->nWorld(round) + 1, KingdomGlobal::PacnRollSys->nStage(round) + 1);
	return arc;
}

uint cAppG03PacnRollGame::nRound()
{
	return m_nRound;
}

bool cAppG03PacnRollGame::s_hubEntry = false;

cAppG03PacnRollGame::cAppG03PacnRollGame()
{
	KingdomGlobal::g_pRollGame = this;
	// the C# started Init() as a Unity coroutine (InitGame component); native: it runs to completion here (the coroutine only spread the loading over frames)
	Init();
}

IEnumerator* cAppG03PacnRollGame::TaskCoroutine(IEnumerator* task)
{
	// TODO(unity): coroutine; original C# body:
	// 	IEnumerator ie;
	// 	do
	// 	{
	// 		ie = MoveNext(task);
	// 		yield return ie;
	// 	}
	// 	while (!(ie.Current is bool) || (bool)ie.Current);
	return nullptr;
}

void cAppG03PacnRollGame::Free()
{
	Term();
	cAppKingdom::Free();
}

uint cAppG03PacnRollGame::nGameTime()
{
	return (uint)((float)m_nGameTimeCnt * 1000.0f / 60.0f);
}

uint cAppG03PacnRollGame::nGameTimeCnt()
{
	return m_nGameTimeCnt;
}

cTaskStage* cAppG03PacnRollGame::pTaskStage()
{
	return m_pTaskStage;
}

cTaskCamera* cAppG03PacnRollGame::pTaskCamera()
{
	return m_pTaskCamera;
}

cTaskCollision* cAppG03PacnRollGame::pTaskCollision()
{
	return m_pTaskCollision;
}

cTaskCharPacman* cAppG03PacnRollGame::pTaskCharPacman()
{
	return m_pTaskCharPacman;
}

cTaskItem* cAppG03PacnRollGame::pTaskItem()
{
	return m_pTaskItem;
}

cTaskGoal* cAppG03PacnRollGame::pTaskGoal()
{
	return m_pTaskGoal;
}

cTaskPoint* cAppG03PacnRollGame::pTaskPoint()
{
	return m_pTaskPoint;
}

cTaskPacFade* cAppG03PacnRollGame::pTaskFade()
{
	return m_pTaskFade;
}

cTaskPause* cAppG03PacnRollGame::pTaskPause()
{
	return m_pTaskPause;
}

cTaskConsoleMan* cAppG03PacnRollGame::pTaskConsole()
{
	return m_pTaskConsole;
}

cTaskConsRetry* cAppG03PacnRollGame::pTaskConsRetry()
{
	return m_pTaskConsRetry;
}

cTaskBgGimmickPistonMan* cAppG03PacnRollGame::pTaskBgGimmickPistonMan()
{
	return m_pTaskBgGimmickPistonMan;
}

cTaskBgGimmickSwitch* cAppG03PacnRollGame::pTaskBgGimmickSwitch()
{
	return m_pTaskBgGimmickSwitch;
}

cTaskBgGimmick* cAppG03PacnRollGame::pTaskBgGimmick(uint num)
{
	return m_vecTaskBgGimmick[(int)num];
}

uint cAppG03PacnRollGame::GetBgGimmickNum()
{
	return (uint)(int)m_vecTaskBgGimmick.size();
}

cTaskWater* cAppG03PacnRollGame::pTaskWater()
{
	return m_pTaskWater;
}

cTaskFallArea* cAppG03PacnRollGame::pTaskFallArea()
{
	return m_pTaskFallArea;
}

cTaskCharGolvis* cAppG03PacnRollGame::pTaskBoss()
{
	return m_pTaskBoss;
}

cTaskGhostMan* cAppG03PacnRollGame::pTaskGhostMan()
{
	return m_pTaskGhostMan;
}

cGhostParam* cAppG03PacnRollGame::pGhostParam()
{
	return m_pGhostParam;
}

cGhostCommonParam* cAppG03PacnRollGame::pGhostCommonParam()
{
	return m_pGhostCommonParam;
}

bool& cAppG03PacnRollGame::IsClear()
{
	return m_bClear;
}

bool& cAppG03PacnRollGame::IsStart()
{
	return m_bStart;
}

bool cAppG03PacnRollGame::IsGame()
{
	return m_bGame;
}

bool cAppG03PacnRollGame::IsIjikeTime()
{
	if (!CustomKingdomUtil::IF(m_nIjikeCount))
	{
		return false;
	}
	return true;
}

int cAppG03PacnRollGame::IjikeGetCount()
{
	return m_nIjikeCount;
}

cTaskCameraSet* cAppG03PacnRollGame::pTaskCameraSet()
{
	return m_pTaskCameraSet;
}

std::unordered_map<std::string, cFaceList*>& cAppG03PacnRollGame::pHitHash()
{
	return m_HitHash;
}

cFaceList* cAppG03PacnRollGame::pHitHash(string key)
{
	if (m_HitHash.contains(key))
	{
		return m_HitHash[key];
	}
	return nullptr;
}

// DOL 0x800b3508 [audited: per AUDIT_G03_G07 row] (the C# coroutine body, synchronous; sys +0xa54 audio / screen setup and cPacnRollLoopSndMan (sound, E1) are not ported)
void cAppG03PacnRollGame::Init()
{
	// the DOL's Init (0x800b3508) does not touch the top-menu flag (sys + 4): the hub (job 138, 0x8019ab28) sets it before the app is built
	if (!KingdomGlobal::PacnRollSys->IsTopMenuMode() || !s_hubEntry) KingdomGlobal::PacnRollSys->SetTopMenuMode(true);
	// UNVERIFIED / not ported: rounds 24 / 25 turn on linear fog + the post-process layer, world 3 changes the shadow strength (Unity render settings)
	m_pTaskStage = nullptr;
	m_pTaskCharPacman = nullptr;
	m_pTaskCamera = nullptr;
	m_pTaskCollision = nullptr;
	m_pTaskItem = nullptr;
	m_pTaskPoint = nullptr;
	m_pTaskBox = nullptr;
	m_pTaskGoal = nullptr;
	m_pTaskGate = nullptr;
	m_pTaskConsole = nullptr;
	m_pTaskBoss = nullptr;
	m_pTaskFade = nullptr;
	m_pTaskBgGimmickSwitch = nullptr;
	m_pGhostParam = nullptr;
	m_pGhostCommonParam = nullptr;
	m_pTaskWater = nullptr;
	m_pTaskFallArea = nullptr;
	m_pTaskBgGimmickPistonMan = nullptr;
	m_bStart = false;
	m_bGame = false;
	m_bRetry = false;
	nModeState() = 0u;
	m_nRound = KingdomGlobal::PacnRollSys->nRound();
	m_bClear = false;
	m_nGameTimeCnt = 0u;
	m_nWaitCnt = 0u;
	m_nIjikeTime = 300;
	m_nIjikeCount = 0;
	if (KingdomGlobal::PacnRollSys->nRound() < 26 && KingdomGlobal::PacnRollSys->nRound() >= 0)
	{
		cKingdomCSV obj;
		if (obj.RegistData("IjikeTime.csv")) obj.S32((int)(KingdomGlobal::PacnRollSys->nRound() + 1), 0, m_nIjikeTime);
	}
	KingdomGlobal::PacnRollSys->bOpenStage(false);
	if (KingdomGlobal::PacnRollSys->IsRetry())
	{
		m_nGameTimeCnt = KingdomGlobal::PacnRollSys->pRetryData()->nGameTimeCnt;
	}
	// DOL 0x800b3508 has no else branch: ResetDeadEnemyCnt / ResetGetItemCnt are the C#'s Steam stat counters (not in the DOL)
	devtools::G03ExploreAttach(); // devtools: --g03-explore (not 1:1)
	wii::ArchiveStore::Attach(StageArcPath(KingdomGlobal::PacnRollSys->nRound()));
	kingdom::cKingdomModelBase::ClearAssetCache();
	kingdom::NativeScene::Reset();
	// the DOL's scene root draws only what the task Draws submitted this frame (cTaskModel::Draw 0x8009a650, cTaskModelEx::Draw 0x80187270,
	// the push-backs 0x8009abe0 -> 0x8009c3f4); the hub (cAppMenu) runs this Init too
	kingdom::NativeScene::SetPerFrame(true);
	{
		// the stage's light sets (SCN0 files): 0 background, 1 items, 2 characters (Pac-Man, ghosts), 3 gimmicks / gates / goal
		static const char* const kLights[4] = {"BIN_BG_SCA", "BIN_ITEM_SCA", "BIN_MODEL_SCA", "BIN_GIMMICK_SCA"};
		for (int i = 0; i < 4; i++)
		{
			std::size_t size = 0;
			const std::uint8_t* d = wii::ArchiveStore::Find(kLights[i], size);
			wii::Brres b;
			wii::ScnAnim sa;
			wii::LightResult light;
			// the whole light set (ambient light + up to 4 point / directional / spot lights), as g3d loads it; SampleLight kept only the
			// first light as a directional one with a white ambient (bugs.txt w5s2: wrong stage lighting)
			if (d != nullptr && b.Load(d, size) && sa.Load(b) && sa.SampleLightSet(0, light))
			{
				sa.SampleFog(light);
				kingdom::NativeScene::SetLight(i, light);
			}
		}
	}
	ConnectChild(new cPacnRollLoopSndMan());   // DOL 0x800b3a00 (KingSys +0x60)
	ConnectChild(m_pTaskPause = new cTaskPause());
	ConnectChild(new kingdom::cTaskNativeScene());   // replaces the C# cTaskScnRoot: draws every submitted model once per frame
	m_pTaskStage = new cTaskStage();
	CustomKingdomUtil::Assert(m_pTaskStage != nullptr, "nullpo");
	ConnectChild(m_pTaskStage);
	m_pTaskCollision = new cTaskCollision();
	CustomKingdomUtil::Assert(m_pTaskCollision != nullptr, "nullpo");
	ConnectChild(m_pTaskCollision);
	m_pGhostParam = new cGhostParam("BIN_CSV_GHOST_PARAM");
	m_pGhostCommonParam = new cGhostCommonParam("BIN_CSV_GHOST_COMMON_PARAM");
	InitStage();
	ConnectChild(m_pTaskCamera);
}

// DOL 0x800b3c2c [audited: per AUDIT_G03_G07 row] (the stage tasks, in the DOL's order)
void cAppG03PacnRollGame::InitStage()
{
	CustomKingdomUtil::Assert(m_pTaskStage != nullptr, "nullpo");
	kingdom::NativeScene::SetShadow(true);   // DOL 0x800b3c50 [audited: described by the 43rd-session pass]: cTaskScnRoot +0x1f8 = 1, then the shadow camera task
	cTaskShadowCamera* pTaskShadowCamera = new cTaskShadowCamera();
	m_pTaskCamera = new cTaskCamera();
	m_pTaskCamera->nDrawPriority(65540u);
	m_pTaskCamera->Setup("BIN_CAMERA_SCA");
	m_pTaskFade = new cTaskPacFade();
	m_pTaskStage->ConnectChild(m_pTaskFade);
	m_pTaskFade->nDrawPriority(2415919087u);
	// DOL 0x800b3c2c: the one fade-in, slot +0x48 (Fade) with kind 2 (black) or, on a retry (sys +7), kind 5 (g03), 500 ms; the converted code first
	// faded OUT to black for 1 ms and faded in at the end of Init
	m_pTaskFade->Fade(KingdomGlobal::PacnRollSys->IsRetry() ? EFadeKind::eFADE_TYPE_IN_G03 : EFadeKind::eFADE_TYPE_IN_BLACK, 500);
	m_pTaskWater = new cTaskWater();
	m_pTaskStage->ConnectChild(m_pTaskWater);
	m_pTaskFallArea = new cTaskFallArea();
	m_pTaskStage->ConnectChild(m_pTaskFallArea);
	Dispose("BIN_SET_MDL");
	m_pTaskPoint = new cTaskPoint();
	m_pTaskCharPacman = new cTaskCharPacman();
	m_pTaskCharPacman->nDrawPriority(65539u);
	m_pTaskStage->ConnectChild(m_pTaskCharPacman);
	m_pTaskStage->ConnectChild(m_pTaskPoint);
	m_pTaskItem = new cTaskItem();
	m_pTaskStage->ConnectChild(m_pTaskItem);
	m_pTaskBox = new cTaskBox();
	m_pTaskStage->ConnectChild(m_pTaskBox);
	m_pTaskGate = new cTaskGate();
	m_pTaskStage->ConnectChild(m_pTaskGate);
	m_pTaskGoal = new cTaskGoal();
	m_pTaskStage->ConnectChild(m_pTaskGoal);
	{
		sKingdomData* sKingdomData2 = KingdomGlobal::KingSys->SearchDataEx("BIN_WIND_SET_DAT");
		if (sKingdomData2 != nullptr && 0 < sKingdomData2->nSize)
		{
			std::vector<sSaveDataWind*> outParams;
			int outParamNum;
			int _discard1;
			ReadFileUtil::Convert<sSaveDataWind>(sKingdomData2->pBin, outParams, outParamNum, _discard1);
			for (int num = 0; num < outParamNum; num++)
			{
				cTaskWind* cTaskWind2 = new cTaskWind(*outParams[num]);
				ConnectChild(cTaskWind2);
			}
		}
	}
	m_pTaskStage->ConnectChild(m_pTaskGhostMan = new cTaskGhostMan());
	m_pTaskBoss = cTaskCharGolvis::Create(m_pTaskStage, nRound());
	// DOL 0x800b5864 [audited: described by the 43rd-session pass] (light sets 0..3): a light whose SCN0 user data has "follow" stays at Pac-Man + its authored position
	for (int nSet = 0; nSet < 4; nSet++)
	{
		const wii::LightResult* pLight = kingdom::NativeScene::Light(nSet);
		for (int nRig = 0; pLight != nullptr && nRig < pLight->numRig; nRig++)
		{
			if (pLight->rig[nRig].follow)
			{
				m_pTaskStage->ConnectChild(new cTaskLight(nSet, nRig, &m_pTaskCharPacman->vTrans()));
			}
		}
	}
	m_pTaskStage->ConnectChild(pTaskPtkMan());
	if (!KingdomGlobal::PacnRollSys->IsTopMenuMode())
	{
		m_pTaskConsole = new cTaskConsoleMan(this);
		m_pTaskConsole->RegistPac(m_pTaskCharPacman);
		m_pTaskConsole->RegistBoss(m_pTaskBoss);
		m_pTaskConsole->RegistGate(m_pTaskGate);
		m_pTaskConsole->RegistCheckPoint(m_pTaskPoint);
	}
	m_pTaskStage->ConnectChild(m_pTaskConsole);
	SoundManager::get_BGM()->Stop(0.0f);
	WaitOneFramePlayBGM();
	m_pTaskConsRetry = new cTaskConsRetry(ResourceDatabase::GetPath("BIN_COMMON_ANT_RETRY"), ResourceDatabase::GetPath("BIN_COMMON_ANT_CURSOR"), 1u);
	m_pTaskStage->ConnectChild(m_pTaskConsRetry);
	m_pTaskConsRetry->nDrawPriority(2147483649u);
	m_pTaskCameraSet = new cTaskCameraSet();
	m_pTaskStage->ConnectChild(m_pTaskCameraSet);
	m_pTaskCameraSet->LoadData(KingdomGlobal::PacnRollSys->nWorld(), KingdomGlobal::PacnRollSys->nStage());
	m_pTaskStage->ConnectChild(pTaskShadowCamera);
	// DOL 0x800b3bb8 [audited: described by the 43rd-session pass]: the sound listener: interior size 120.1, unit distance 10.2, max volume distance 15.5 (0x801378d0 / d4 / d8)
	SoundManager::get_SE()->SetListenerParam(120.1f, 15.5f, 10.2f);
}

// C# WaitOneFramePlayBGM (coroutine: one frame later; here at once, the mixer starts it asynchronously)
void cAppG03PacnRollGame::WaitOneFramePlayBGM()
{
	EBGMKind kind;
	switch (KingdomGlobal::PacnRollSys->nRound())
	{
	case 8u:
	case 13u:
	case 19u:
	case 24u:
		kind = EBGMKind::PACN_ROLL_SND_BGM07_BOSS1;
		break;
	case 25u:
		kind = EBGMKind::PACN_ROLL_SND_BGM08_BOSS2;
		break;
	default:
		switch (KingdomGlobal::PacnRollSys->nWorld())
		{
		case 1u: kind = EBGMKind::PACN_ROLL_SND_BGM03_WORLD2; break;
		case 2u: kind = EBGMKind::PACN_ROLL_SND_BGM04_WORLD3; break;
		case 3u: kind = EBGMKind::PACN_ROLL_SND_BGM05_WORLD4; break;
		case 4u: kind = EBGMKind::PACN_ROLL_SND_BGM06_WORLD5; break;
		default: kind = EBGMKind::PACN_ROLL_SND_BGM02_WORLD1; break;
		}
		break;
	}
	SoundManager::get_BGM()->Play(kind, 0.5f, true, false, true, 0.1f);
}

// DOL 0x8001398c [audited: described by the 43rd-session pass] -> 0x800b475c (3D actor, light holders, DetachArcData, ScnRoot shadow flag off, ghost params, retry flag sys +7 / InitRetryData unless round 25)
void cAppG03PacnRollGame::Term()
{
	kingdom::NativeScene::Reset();   // the stage's models go away with it
	m_cullingStageModelList.clear();
	SoundManager::get_BGM()->Stop();
	SoundManager::get_SE()->Stop();
	if (nullptr != m_riverPosObj1)
	{
		ReleaseObjectManager::Regist(m_riverPosObj1);
	}
	if (nullptr != m_riverPosObj2)
	{
		ReleaseObjectManager::Regist(m_riverPosObj2);
	}
	if (nullptr != m_riverPosObj3)
	{
		ReleaseObjectManager::Regist(m_riverPosObj3);
	}
	// native: pStageArcKey is the Unity asset key ("g03_pacnroll/arcsv/g03w01s01"), which never matched the attached file, so every stage's archive
	// stayed attached and the next stage / the hub found the old one's entries first (gates of the last stage in the hub)
	KingdomGlobal::KingSys->DetachArcData(StageArcPath(KingdomGlobal::PacnRollSys->nRound()));
	m_pGhostParam = nullptr;
	m_pGhostCommonParam = nullptr;
	if (m_bRetry)
	{
		KingdomGlobal::PacnRollSys->IsRetry() = true;
	}
	else
	{
		KingdomGlobal::PacnRollSys->IsRetry() = false;
		if (KingdomGlobal::PacnRollSys->nRound() != 25)
		{
			KingdomGlobal::PacnRollSys->InitRetryData();
		}
	}
	m_HitHash.clear();
	m_pTaskGoal->TermOnce();
	m_pTaskPoint->TermOnce();
	if (m_pTaskGate != nullptr)
	{
		m_pTaskGate->TermOnce();
	}
	m_pTaskCharPacman->TermOnce();
	m_pTaskItem->TermOnce();
	m_pTaskBox->TermOnce();
	if (m_pTaskBgGimmickPistonMan != nullptr)
	{
		m_pTaskBgGimmickPistonMan->TermOnce();
	}
	if (m_pTaskBgGimmickSwitch != nullptr)
	{
		m_pTaskBgGimmickSwitch->TermOnce();
	}
	for (cTaskBgGimmick* item : m_vecTaskBgGimmick)
	{
		if (item != nullptr)
		{
			item->TermOnce();
		}
	}
	if (m_pTaskBoss != nullptr)
	{
		m_pTaskBoss->TermOnce();
	}
	if (m_pTaskGhostMan != nullptr)
	{
		m_pTaskGhostMan->TermOnce();
	}
	for (cTaskBg* pTaskBg : m_pTaskBgList)
	{
		if (pTaskBg != nullptr)
		{
			pTaskBg->TermOnce();
		}
	}
	for (cTaskBgHit* pTaskBgHit : m_pTaskBgHitList)
	{
		if (pTaskBgHit != nullptr)
		{
			pTaskBgHit->TermOnce();
		}
	}
	pTaskPtkMan()->TermOnce();
	if (m_pTaskConsole != nullptr)
	{
		m_pTaskConsole->TermOnce();
	}
	AudioListenerManager::SetStalkReset();
	AudioListenerManager::SetEnableRangeCheck(false);
	SoundManager::get_SE()->ClearListener();   // the app's Sound3DListener goes with it
	KingdomGlobal::PacnRollSys->SetTopMenuMode(false);
	// TODO(unity): RenderSettings.fog = false;
	if (nullptr != KingdomGlobal::PacnRollSys->PostProcess)
	{
		// FIXME(cs2cpp): member enabled on <unknown PostProcessLayer>
		// TODO(unity): KingdomGlobal.PacnRollSys.PostProcess.enabled = false;
	}
}

// DOL 0x800b4aa4 [audited: per AUDIT_G03_G07 row] (modes 0..8 at +0x34, previous mode +0x3c, sub step +0x40; every transition, pacman callee 0x8016efb8 IsMiss / 0x8016efc4 IsMissFinish / 0x8016efd0 IsGameOver and the job numbers checked)
bool cAppG03PacnRollGame::Execute(cJetTask* pParam)
{
	if (m_initCoObj != nullptr)
	{
		return true;
	}
	{
		static int dbgFrame = 0;
		static const bool dbg = std::getenv("MEGAMIX_SCENE_DEBUG") != nullptr;
		if (dbg && m_pTaskCharPacman != nullptr && ++dbgFrame % 30 == 0)
			std::fprintf(stderr, "pacman: frame %d mode %u pos (%.2f %.2f %.2f) speed %.3f ate %u/%u\n", dbgFrame, nModeState(0), m_pTaskCharPacman->vTrans().x, m_pTaskCharPacman->vTrans().y, m_pTaskCharPacman->vTrans().z, m_pTaskCharPacman->CalcSpeed(), m_pTaskItem ? m_pTaskItem->GetAteCookie() : 0u, m_pTaskItem ? m_pTaskItem->GetCookieMax() : 0u);
	}
	int pad1No = InGamePadManager::GetPad1No();
	// DOL 0x800b4aa4: get_pad1(0)->trig & 0x500 (MINUS | HOME) and the retry dialog hidden -> pause
	if ((GamePad::Get(0).trig & (GamePad::PAD_MINUS | GamePad::PAD_HOME)) != 0 && !m_pTaskConsRetry->IsVisible())
	{
		if (!IsPause())
		{
			cPacnRollLoopSndMan::PauseAll();   // DOL 0x800b4b28 [audited: described by the 43rd-session pass] (the 0x8012a8a8 call before it discards its result)
		}
		m_pTaskPause->Start();
	}
	IsPause() = m_pTaskPause->IsPause();
	pTaskCollision()->ClearEntry();
	if (IsPause())
	{
		return true;
	}
	if (GamePad::Get(0).hold & GamePad::PAD_HOME)   // DOL 0x800b4b24 [audited: described by the 43rd-session pass]: hold & 0x400 (the Home Button Menu is open) freezes the game logic
	{
		return true;
	}
	uint n = 0u;
	if (nModeState(n) == 0)
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch1_0u:
			nModeStep(n) = 1u;
			goto _switch1_1u;
		case 1u:
		_switch1_1u:
			if (m_pTaskFade->IsEnd())
			{
				if (!KingdomGlobal::PacnRollSys->IsRetry() && !devtools::G03SkipTitle())   // devtools: --g03-skip-title takes the retry path (no stage name card; not 1:1)
				{
					NextMode(n, 1u);
					m_pTaskConsole->TitleStart();
				}
				else
				{
					NextMode(n, 2u);
					m_pTaskConsole->TitleStart(false);
				}
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch1_2u;
		case 2u:
		_switch1_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (1 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch2_0u:
			nModeStep(n) = 1u;
			goto _switch2_1u;
		case 1u:
		_switch2_1u:
			if (m_pTaskConsole->IsTitle())
			{
				NextMode(n, 2u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch2_2u;
		case 2u:
		_switch2_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (2 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch3_0u:
			nModeStep(n) = 1u;
			m_bStart = true;
			m_bGame = true;
			m_pTaskConsole->StartStart();
			if (KingdomGlobal::PacnRollSys->nWorld() == 1 && KingdomGlobal::PacnRollSys->nStage() == 0)
			{
				// DOL 0x800b4d8c [audited: described by the 43rd-session pass]: ONE river sound at (158, -206, -115) through the app's 3D actor (+0x3ac), handle +0x388 (the C# placed three
				// Unity objects at other points instead)
				Vector4 v = Vector4::zero;
				::Constants::jetGeoVecSet(v, 158.0f, -206.0f, -115.0f, 1.0f);
				SoundManager::get_SE()->PlaySE3D(ESEKind::PACN_ROLL_SND_SE043_RIVER, v);
			}
			AudioListenerManager::SetEnableRangeCheck(true);
			goto _switch3_1u;
		case 1u:
		_switch3_1u:
			m_nGameTimeCnt++;
			if (devtools::G03KillFrame() >= 0 && (int)m_nGameTimeCnt == devtools::G03KillFrame()) { pTaskCharPacman()->DamageProcess(9999u, Vector4::zero, 0.0f, 0); } // devtools: --g03-kill (not 1:1)
			if (pTaskCharPacman()->IsMiss())
			{
				if (!pTaskCharPacman()->IsGameOver())
				{
					NextMode(n, 3u);
				}
				else
				{
					NextMode(n, 7u);
				}
			}
			else if (pTaskGoal()->IsGoal())
			{
				NextMode(n, 4u);
				for (int i = 0; i < (int)m_cullingStageModelList.size(); i++)
				{
					m_cullingStageModelList[i]->SetCulling(true);
				}
			}
			else if (IsClear())
			{
				NextMode(n, 5u);
			}
			if (m_nIjikeCount != 0)
			{
				if (m_nIjikeCount == 60)
				{
					SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE032_IJIKE);
					if (cPacnRollLoopSndMan::CanPlay()) cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE033_IJIKE_OWARI, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY));
				}
				if (!CustomKingdomUtil::IF(--m_nIjikeCount))
				{
					SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE033_IJIKE_OWARI);
					if (m_pTaskCharPacman != nullptr)
					{
						m_pTaskCharPacman->IjikeEnd();
					}
					if (m_pTaskBoss != nullptr)
					{
						m_pTaskBoss->IjikeEnd();
					}
					if (m_pTaskGhostMan != nullptr)
					{
						m_pTaskGhostMan->IjikeEnd();
					}
				}
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch3_2u;
		case 2u:
		_switch3_2u:
			nModeStep(n) = 0u;
			m_bGame = false;
			SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE032_IJIKE);
			SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE033_IJIKE_OWARI);
			break;
		}
	}
	else if (3 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch4_0u:
			nModeStep(n) = 1u;
			SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE032_IJIKE);
			SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE033_IJIKE_OWARI);
			goto _switch4_1u;
		case 1u:
		_switch4_1u:
			if (pTaskCharPacman()->IsMissFinish())
			{
				if (pTaskCharPacman()->IsGameOver())
				{
					NextMode(n, 6u);
				}
				else
				{
					NextMode(n, 8u);
				}
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch4_2u;
		case 2u:
		_switch4_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (6 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch5_0u:
			{
				nModeStep(n) = 1u;
				m_pTaskConsRetry->Start();
				// DOL 0x800b5128 [audited: described by the 43rd-session pass]: only the loop-sound manager's stop (0x800ba454), no stat records (the C#'s GameStateManager totals / Steam counters are not in the DOL)
				cPacnRollLoopSndMan::StopAll();
				goto _switch5_1u;
			}
		case 1u:
		_switch5_1u:
			if (m_pTaskConsRetry->IsYes() && !m_pTaskConsRetry->IsVisible())
			{
				NextMode(n, 8u);
			}
			else if (m_pTaskConsRetry->IsNo() && !m_pTaskConsRetry->IsVisible())
			{
				NextMode(n, 8u);   // DOL 0x800b512c [audited: described by the 43rd-session pass]: result 2 (no) leaves the same way as 1; the C#'s AddScore(GAMEOVER_TIME) is not in the DOL
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch5_2u;
		case 2u:
		_switch5_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (7 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch6_0u:
			nModeStep(n) = 1u;
			goto _switch6_1u;
		case 1u:
		_switch6_1u:
			NextMode(n, 8u);
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch6_2u;
		case 2u:
		_switch6_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (4 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch7_0u:
			nModeStep(n) = 1u;
			SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE032_IJIKE);
			SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE033_IJIKE_OWARI);
			SoundManager::get_BGM()->Play(EBGMKind::PACN_ROLL_SND_BGM11_GOAL);
			IsClear() = true;
			StopWatchReset(0u);
			StopWatchStart(0u);
			m_nWaitCnt = 0u;
			goto _switch7_1u;
		case 1u:
		_switch7_1u:
			if (pTaskCharPacman()->IsGoal() && pTaskCamera()->nType() != 1)
			{
				pTaskCamera()->SetCameraType(1u);
			}
			m_nWaitCnt++;
			if (240 < m_nWaitCnt || (pTaskCharPacman()->IsGoal() && InGamePadManager::GetEnterKeyDown(pad1No)))
			{
				NextMode(n, 5u);
			}
			if (!SoundManager::get_BGM()->IsPlaying())
			{
				SoundManager::get_BGM()->Play(EBGMKind::PACN_ROLL_SND_BGM11_GOAL);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch7_2u;
		case 2u:
		_switch7_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (5 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch8_0u:
			nModeStep(n) = 1u;
			SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE032_IJIKE);
			SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE033_IJIKE_OWARI);
			m_pTaskConsole->ClearStart();
			if (KingdomGlobal::PacnRollSys->IsBossStage())
			{
				SoundManager::get_BGM()->Play(EBGMKind::PACN_ROLL_SND_BGM11_GOAL);
			}
			if (pTaskCamera()->nType() != 1)
			{
				pTaskCamera()->SetCameraType(1u);
			}
			m_nWaitCnt = 0u;
			goto _switch8_1u;
		case 1u:
		_switch8_1u:
			m_nWaitCnt++;
			if (240 < m_nWaitCnt || InGamePadManager::GetEnterKeyDown(pad1No))
			{
				NextMode(n, 8u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch8_2u;
		case 2u:
		_switch8_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (8 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch9_0u:
			nModeStep(n) = 1u;
			m_pTaskFade->Fade(EFadeKind::eFADE_TYPE_OUT_G03);   // DOL 0x800b55c0 [audited: described by the 43rd-session pass]: only the fade (500 ms); the BGM runs on under it and stops with the app (Term)
			goto _switch9_1u;
		case 1u:
		_switch9_1u:
			if (m_pTaskFade->IsEnd())
			{
				if (pTaskCharPacman()->IsMiss())
				{
					if (!pTaskCharPacman()->IsGameOver())
					{
						SetRetryData();
						m_bRetry = true;
						kingdom::Constants::Kingdom_SetJobNo(221249536u);
					}
					else
					{
						if (m_pTaskConsRetry->IsYes())
						{
							if (KingdomGlobal::PacnRollSys->nRound() == 25)
							{
								KingdomGlobal::PacnRollSys->nRound() = 24u;
							}
							KingdomGlobal::PacnRollSys->pSaveLoadMan()->nLife() = KingdomGlobal::PacnRollSys->pSaveLoadMan()->nDefaultLife();
							m_bRetry = false;
							kingdom::Constants::Kingdom_SetJobNo(221249536u);
						}
						else
						{
							KingdomGlobal::PacnRollSys->pSaveLoadMan()->nLife() = KingdomGlobal::PacnRollSys->pSaveLoadMan()->nDefaultLife();
							kingdom::Constants::Kingdom_SetJobNo(223346688u);
						}
						// (the C# saved here; the DOL (0x800b55c0..) does not)
					}
				}
				else
				{
					bool flag = KingdomGlobal::PacnRollSys->IsAllClear();
					bool flag2 = false;
					SaveData();
					if (m_nRound == 25 && KingdomGlobal::PacnRollSys->IsAllClear() && !flag && KingdomGlobal::PacnRollSys->IsAllClear())
					{
						flag2 = true;
					}
					if (flag2)
					{
						kingdom::Constants::Kingdom_SetJobNo(232783872u);
					}
					else
					{
						kingdom::Constants::Kingdom_SetJobNo(220200960u, true);
					}
				}
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch9_2u;
		case 2u:
		_switch9_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	// DOL 0x800b57dc [audited: described by the 43rd-session pass]: the sound listener (app +0x2cc) follows the camera: look-at (camera +0x1d0 eye, +0x1f0 target, up (0, 1, 0)) -> 0x80137824
	SoundManager::get_SE()->SetListener(m_pTaskCamera->vPos(), m_pTaskCamera->vTarget(), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	return true;
}

// DOL 0x800b5860 [audited: blr] (blr)
void cAppG03PacnRollGame::Draw(int nStatus, cJetTask* pParam)
{
}

bool cAppG03PacnRollGame::IsBossStage()
{
	switch (nRound())
	{
	case 8u:
	case 13u:
	case 19u:
	case 24u:
	case 25u:
		return true;
	default:
		return false;
	}
}

// DOL 0x800b5b90 [audited: per AUDIT_G03_G07 row] (time = 1000 * frames / 60 FLOAT_8047b07c / b080; round 25 sets sys bAllClear 0x8012a994 + the save's +0x14; round < 25 opens the next stage 0x8012a9a4)
void cAppG03PacnRollGame::SaveData()
{
	SSaveDataPacnRoll::sStageRecord _out = SSaveDataPacnRoll::sStageRecord();
	KingdomGlobal::PacnRollSys->pSaveLoadMan()->GetStageRecord(_out, m_nRound);
	uint num = m_pTaskGhostMan->DeadGhostNum();
	_out.bFree = true;
	if (!IsBossStage())
	{
		if (_out.nTime > nGameTime())
		{
			_out.nTime = nGameTime();
		}
		if (!_out.bTimeNew && _out.nTime < 599999)
		{
			_out.bTimeNew = true;
		}
		if (_out.nGhost < num)
		{
			_out.nGhost = num;
		}
		if (!_out.bGhostNew && _out.nGhost >= m_pTaskGhostMan->AllGhostNum())
		{
			_out.bGhostNew = true;
		}
		if (_out.nCookie < m_pTaskItem->GetAteCookie())
		{
			_out.nCookie = m_pTaskItem->GetAteCookie();
		}
		if (!_out.bCookieNew && _out.nCookie >= _out.nCookieMax)
		{
			_out.bCookieNew = true;
		}
	}
	else
	{
		_out.nCookie = 0u;
		_out.nGhost = 0u;
		_out.bTimeNew = false;
		_out.bGhostNew = false;
		_out.bCookieNew = false;
	}
	_out.bEatPizza = pTaskItem()->IsEatPizza();
	KingdomGlobal::PacnRollSys->pSaveLoadMan()->SetStageRecord(_out, m_nRound);
	if (m_nRound < 25)
	{
		KingdomGlobal::PacnRollSys->pSaveLoadMan()->GetStageRecord(_out, m_nRound + 1);
		if (!_out.bFree)
		{
			KingdomGlobal::PacnRollSys->bOpenStage(true);
			_out.bFree = true;
			KingdomGlobal::PacnRollSys->pSaveLoadMan()->SetStageRecord(_out, m_nRound + 1);
		}
	}
	else
	{
		KingdomGlobal::PacnRollSys->bAllClear(true);
		KingdomGlobal::PacnRollSys->pSaveLoadMan()->ValidateAllClear();
	}
	KingdomGlobal::PacnRollSys->pSaveLoadMan()->nLife() = pTaskCharPacman()->nLife();
	KingdomGlobal::PacnRollSys->pSaveLoadMan()->nMaxHP() = pTaskCharPacman()->nMaxHP();
	for (uint num3 = 0u; num3 < m_nRound; num3++)
	{
		KingdomGlobal::PacnRollSys->pSaveLoadMan()->GetStageRecord(_out, num3);
		_out.bFree = true;
		KingdomGlobal::PacnRollSys->pSaveLoadMan()->SetStageRecord(_out, num3);
	}
	KingdomGlobal::PacnRollSys->pSaveLoadMan()->SaveData();
	// DOL 0x800b5b90 ends here: the C#'s GameStateManager records (clear time, max round, eaten ghosts / items, result AddScore, all-clear total) are not in the DOL
}

// DOL 0x800b5e6c [audited: per AUDIT_G03_G07 row] (placement model nodes effect / bg / hit / obj in that order); builds the stage from the placement model: "effect" (particles), "bg" (drawn models), "hit" (collision models), "obj" (gimmicks)
void cAppG03PacnRollGame::Dispose(string key)
{
	pTaskPtkMan() = new cTaskEffect();
	// the placement model: only its node tree is used
	if (devtools::G03Explore()) { ((cTaskEffect*)pTaskPtkMan())->Regist(); devtools::G03ExploreStage(m_pTaskStage, m_pTaskBgList, m_pTaskBgHitList); return; } // devtools: --g03-explore (not 1:1)
	if (devtools::G03GrayOnly()) { ((cTaskEffect*)pTaskPtkMan())->Regist(); devtools::G03GrayStage(m_pTaskStage, m_pTaskBgHitList); return; } // devtools: graybox stage only (not 1:1)
	ResMdl* resMdl = new ResMdl(kingdom::LoadModelObject(key));
	ResNode* resNode0 = resMdl->GetResNode("effect");
	if (resNode0->IsValid())
	{
		((cTaskEffect*)pTaskPtkMan())->Regist(resNode0);
	}
	else
	{
		((cTaskEffect*)pTaskPtkMan())->Regist();
	}
	resNode0 = resMdl->GetResNode("bg");
	int childIdx = 0;
	ResNode* resNode1;
	if (resNode0->IsValid())
	{
		resNode1 = resNode0->GetChildNode(childIdx);
		while (resNode1->IsValid())
		{
			cTaskBg* taskBg = new cTaskBg();
			taskBg->coRegist(resNode1, resNode1);
			taskBg->Initialize();
			taskBg->nDrawPriority(65536u);
			taskBg->Model->AttachShadow();
			m_pTaskStage->ConnectChild(taskBg);
			m_pTaskBgList.push_back(taskBg);
			childIdx++;
			resNode1 = resNode0->GetChildNode(childIdx);
		}
	}
	resNode0 = resMdl->GetResNode("hit");
	if (devtools::G03BgCollision()) devtools::G03BgCollisionStage(m_pTaskStage, m_pTaskBgList, m_pTaskBgHitList); // devtools: --g03-bg-collision (not 1:1)
	else if (resNode0->IsValid())
	{
		childIdx = 0;
		resNode1 = resNode0->GetChildNode(childIdx);
		while (resNode1->IsValid())
		{
			cTaskBgHit* cTaskBgHit2 = new cTaskBgHit();
			cTaskBgHit2->nDrawPriority(65536u);
			m_pTaskStage->ConnectChild(cTaskBgHit2);
			m_pTaskBgHitList.push_back(cTaskBgHit2);
			string outStr;
			CustomKingdomUtil::snprintf(outStr, -1, "%s.brres", resNode1->GetName());
			cTaskBgHit2->RegistHitData(outStr);
			childIdx++;
			resNode1 = resNode0->GetChildNode(childIdx);
		}
	}
	devtools::G03GrayStage(m_pTaskStage, m_pTaskBgHitList); // devtools: the editor's graybox primitives (not 1:1)
	resNode0 = resMdl->GetResNode("obj");
	if (!resNode0->IsValid())
	{
		return;
	}
	childIdx = 0;
	resNode1 = resNode0->GetChildNode(childIdx);
	while (resNode1->IsValid())
	{
		int node1Idx = 0;
		ResNode* resNode2 = resNode1->GetChildNode(node1Idx);
		while (resNode2->IsValid())
		{
			cTaskBgGimmick* taskGimmick = SetGimmick(resNode1, resNode2);
			if (taskGimmick != nullptr)
			{
				taskGimmick->Model->AttachShadow();
				taskGimmick->nDrawPriority(65537u);
				m_pTaskStage->ConnectChild(taskGimmick);
				m_vecTaskBgGimmick.push_back(taskGimmick);
			}
			node1Idx++;
			resNode2 = resNode1->GetChildNode(node1Idx);
		}
		childIdx++;
		resNode1 = resNode0->GetChildNode(childIdx);
	}
}

// DOL: the retry block of Execute mode 8 (0x800b563c..0x800b5668): InitRetryData, then point / item / gate / box / ghost man unless a boss stage, then Pac-Man, then sys +0x14 = game time
void cAppG03PacnRollGame::SetRetryData()
{
	KingdomGlobal::PacnRollSys->InitRetryData();
	if (!IsBossStage())
	{
		m_pTaskPoint->SetRetryData();
		m_pTaskItem->SetRetryData();
		if (m_pTaskGate != nullptr)
		{
			m_pTaskGate->SetRetryData();
		}
		m_pTaskBox->SetRetryData();
		m_pTaskGhostMan->SetRetryData();
	}
	m_pTaskCharPacman->SetRetryData();
	KingdomGlobal::PacnRollSys->pRetryData()->nGameTimeCnt = m_nGameTimeCnt;
}

// DOL 0x800b6700 [audited: per AUDIT_G03_G07 row] (strstr chain over the node name: _si _je _ya _ka _sw _fa|_do _wa|_hi|_ro _wo _el _ca _bo01 _bo02 _di01 _di02 _ws _wm _wl _ne _hu _pi _me, same order and constructors)
cTaskBgGimmick* cAppG03PacnRollGame::SetGimmick(ResNode* node0, ResNode* node1)
{
	CustomKingdomUtil::Assert(node0 != nullptr, "nullpo");
	CustomKingdomUtil::Assert(node1 != nullptr, "nullpo");
	if (CustomKingdomUtil::strstr(node0->GetName(), "_si"))
	{
		return new cTaskBgGimmickSeesaw(node0, node1);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_je"))
	{
		return new cTaskBgGimmickGondola(node0, node1);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_ya"))
	{
		return new cTaskBgGimmickArrow(node0, node1);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_ka"))
	{
		return new cTaskBgGimmickWall(node0, node1);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_sw"))
	{
		return m_pTaskBgGimmickSwitch = new cTaskBgGimmickSwitch(node0, node1);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_fa") || CustomKingdomUtil::strstr(node0->GetName(), "_do"))
	{
		return new cTaskBgGimmickFall(node0, node1);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_wa") || CustomKingdomUtil::strstr(node0->GetName(), "_hi") || CustomKingdomUtil::strstr(node0->GetName(), "_ro"))
	{
		return new cTaskBgGimmickFallSw(node0, node1);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_wo"))
	{
		return new cTaskBgGimmickPillar(node0, node1);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_el"))
	{
		return new cTaskBgGimmickElevator(node0, node1);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_ca"))
	{
		return new cTaskBgGimmickTruck(node0, node1);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_bo01"))
	{
		return new cTaskBgGimmickIronBall(node0, node1, Constants::eFACE_KIND_EX::eFACE_KIND_EX_IRON_BALL_01);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_bo02"))
	{
		return new cTaskBgGimmickIronBall(node0, node1, Constants::eFACE_KIND_EX::eFACE_KIND_EX_IRON_BALL_02);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_di01"))
	{
		return new cTaskBgGimmickFloorEvent(node0, node1, Constants::eFACE_KIND_EX::eFACE_KIND_EX_BOUND_GIMMICK_01);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_di02"))
	{
		return new cTaskBgGimmickFloorEvent(node0, node1, Constants::eFACE_KIND_EX::eFACE_KIND_EX_BOUND_GIMMICK_02);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_ws"))
	{
		return new cTaskBgGimmickAddinSnd(node0, node1, ESEKind::PACN_ROLL_SND_SE110_GEAR_01);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_wm"))
	{
		return new cTaskBgGimmickAddinSnd(node0, node1, ESEKind::PACN_ROLL_SND_SE111_GEAR_02);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_wl"))
	{
		return new cTaskBgGimmickAddinSnd(node0, node1, ESEKind::PACN_ROLL_SND_SE112_GEAR_03);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_ne"))
	{
		return new cTaskBgGimmickAddinSnd(node0, node1, ESEKind::PACN_ROLL_SND_SE113_SPRING);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_hu"))
	{
		return new cTaskBgGimmickAddinSnd(node0, node1, ESEKind::PACN_ROLL_SND_SE114_PENDULUM);
	}
	if (CustomKingdomUtil::strstr(node0->GetName(), "_pi"))
	{
		if (m_pTaskBgGimmickPistonMan == nullptr)
		{
			m_pTaskBgGimmickPistonMan = new cTaskBgGimmickPistonMan();
			CustomKingdomUtil::Assert(m_pTaskBgGimmickPistonMan != nullptr, "nullpo");
			m_pTaskStage->ConnectChild(m_pTaskBgGimmickPistonMan);
		}
		cTaskBgGimmick* cTaskBgGimmick2 = new cTaskBgGimmick(node0, node1);
		CustomKingdomUtil::Assert(cTaskBgGimmick2 != nullptr, "nullpo");
		m_pTaskBgGimmickPistonMan->RegistGimmick(cTaskBgGimmick2);
		return cTaskBgGimmick2;
	}
	CustomKingdomUtil::strstr(node0->GetName(), "_me");
	return new cTaskBgGimmick(node0, node1);
}

// DOL 0x800b737c [audited: per AUDIT_G03_G07 row] (38 kinds -> particle slot, onetime / depend / clipping flags, rotation or not, fog 0 / -1: all rows compared equal; kinds 31 / 32 = two splashes 4.0 / 2.0 wide)
void cAppG03PacnRollGame::SetEffect(const uint& kind, const Vector4& trans, const Vector4& rotate, cKingdomPtkHnd* hnd, float param)
{
	CustomKingdomUtil::Assert(38 > kind, "error");
	CustomKingdomUtil::Assert("nullpo");
	cKingdomPtkHnd* cKingdomPtkHnd2 = new cKingdomPtkHnd();
	cKingdomPtkHnd* cKingdomPtkHnd3 = ((hnd == nullptr) ? cKingdomPtkHnd2 : hnd);
	switch (kind)
	{
	case 0u:
		cKingdomPtkHnd3->Generate(1u, false, true, false, 0.0f, trans, rotate, nullptr, 0, 0);
		break;
	case 1u:
		cKingdomPtkHnd3->Generate(2u, true, false, false, 0.0f, trans, rotate, nullptr, 0, 0);
		break;
	case 2u:
		cKingdomPtkHnd3->Generate(3u, true, false, false, 0.0f, trans, rotate, nullptr, 0, 0);
		break;
	case 3u:
		cKingdomPtkHnd3->Generate(4u, true, true, false, 0.0f, trans, rotate, nullptr, 0, 0);
		break;
	case 4u:
		cKingdomPtkHnd3->Generate(5u, true, true, true, 0.0f, trans);
		break;
	case 5u:
		cKingdomPtkHnd3->Generate(6u, true, true, true, 0.0f, trans);
		break;
	case 6u:
		cKingdomPtkHnd3->Generate(7u, true, false, false, 0.0f, trans, rotate, nullptr, 0, 0);
		break;
	case 7u:
		cKingdomPtkHnd3->Generate(8u, true, true, true, 0.0f, trans);
		break;
	case 8u:
		cKingdomPtkHnd3->Generate(9u, true, true, true, 0.0f, trans, nullptr, nullptr, 0, 0);
		break;
	case 9u:
		cKingdomPtkHnd3->Generate(10u, true, true, true, 0.0f, trans, nullptr, nullptr, 0, 0);
		break;
	case 10u:
		cKingdomPtkHnd3->Generate(11u, true, false, false, 0.0f, trans);
		break;
	case 11u:
		cKingdomPtkHnd3->Generate(12u, true, true, true, 0.0f, trans);
		break;
	case 12u:
		cKingdomPtkHnd3->Generate(13u, true, true, true, 0.0f, trans);
		break;
	case 13u:
		cKingdomPtkHnd3->Generate(14u, false, true, false, 0.0f, trans);
		break;
	case 14u:
		cKingdomPtkHnd3->Generate(15u, true, true, false, 0.0f, trans);
		break;
	case 15u:
		cKingdomPtkHnd3->Generate(16u, false, false, false, 0.0f, trans);
		break;
	case 16u:
		cKingdomPtkHnd3->Generate(17u, false, true, false, 0.0f, trans);
		break;
	case 17u:
		cKingdomPtkHnd3->Generate(18u, false, true, false, 0.0f, trans);
		break;
	case 18u:
		cKingdomPtkHnd3->Generate(19u, true, false, false, 0.0f, trans, rotate, nullptr, 0, 0);
		break;
	case 19u:
		cKingdomPtkHnd3->Generate(20u, false, false, false, 0.0f, trans, rotate, nullptr, 0, 0);
		break;
	case 20u:
		cKingdomPtkHnd3->Generate(21u, true, true, false, 0.0f, trans);
		break;
	case 21u:
		cKingdomPtkHnd3->Generate(17u, true, true, false, 0.0f, trans, rotate);
		break;
	case 22u:
		cKingdomPtkHnd3->Generate(23u, true, true, false, 0.0f, trans, rotate);
		break;
	case 23u:
		cKingdomPtkHnd3->Generate(24u, true, true, false, 0.0f, trans);
		break;
	case 24u:
		cKingdomPtkHnd3->Generate(25u, false, false, true, 0.0f, trans);
		break;
	case 25u:
		cKingdomPtkHnd3->Generate(26u, false, true, true, 0.0f, trans);
		break;
	case 26u:
		cKingdomPtkHnd3->Generate(27u, false, false, true, 0.0f, trans);
		break;
	case 27u:
		cKingdomPtkHnd3->Generate(28u, false, false, true, 0.0f, trans);
		break;
	case 28u:
		cKingdomPtkHnd3->Generate(29u, false, false, true, 0.0f, trans);
		break;
	case 29u:
		cKingdomPtkHnd3->Generate(30u, true, true, true, 0.0f, trans);
		break;
	case 30u:
		cKingdomPtkHnd3->Generate(31u, true, true, true, 0.0f, trans);
		break;
	case 31u:
	case 32u:
		{
			Vector4 v = Vector4();
			uint num2;
			float num;
			if (kind == 31)
			{
				num = 8.0f;
				num2 = 2u;
				num = 4.0f;
			}
			else
			{
				num2 = 2u;
				num = 2.0f;
			}
			for (uint num3 = 0u; num3 < num2; num3++)
			{
				v[0] = (0.0f - num) / 2.0f + num / (float)num2 * (float)num3;
				v[1] = 0.0f;
				v[2] = (float)kingdom::MslRandRange(0, 300) / 100.0f - 1.5f;   // DOL 0x800b737c: rand() % 300 (PtkSim::Rand15)
				v[3] = 1.0f;
				::Constants::jetGeoVecRotY(v, v, param);
				::Constants::jetGeoVecAdd(v, v, trans);
				float fDelay = (float)kingdom::MslRandRange(0, 45);   // rand() % 45
				cKingdomPtkHnd3->Generate(24u, true, false, false, fDelay, v, nullptr, nullptr, 0, 0);
			}
			break;
		}
	case 33u:
		cKingdomPtkHnd3->Generate(32u, false, false, true, 0.0f, trans, rotate, nullptr, 0, 0);
		break;
	case 34u:
		cKingdomPtkHnd3->Generate(33u, true, false, false, 0.0f, trans, rotate, nullptr, 0, 0);
		break;
	case 35u:
		cKingdomPtkHnd3->Generate(34u, true, true, true, 0.0f, trans);
		break;
	case 36u:
		cKingdomPtkHnd3->Generate(35u, false, true, false, 0.0f, trans);
		break;
	case 37u:
		cKingdomPtkHnd3->Generate(36u, false, true, false, 0.0f, trans);
		break;
	}
}

IEnumerator* cAppG03PacnRollGame::MoveNext(IEnumerator* task)
{
	// TODO(unity): coroutine; original C# body:
	// 	if (m_bInitCoPause)
	// 	{
	// 		yield return null;
	// 	}
	// 	else if (task.MoveNext())
	// 	{
	// 		object current = task.Current;
	// 		if (current == null)
	// 		{
	// 			yield return null;
	// 			yield break;
	// 		}
	// 		Type type = current.GetType();
	// 		if (current is IEnumerator)
	// 		{
	// 			IEnumerator ie;
	// 			do
	// 			{
	// 				ie = MoveNext((IEnumerator)current);
	// 				yield return ie;
	// 			}
	// 			while (!(ie.Current is bool) || (bool)ie.Current);
	// 		}
	// 		else if (type == typeof(Coroutine))
	// 		{
	// 			yield return current;
	// 		}
	// 		else
	// 		{
	// 			yield return current;
	// 		}
	// 	}
	// 	else
	// 	{
	// 		yield return false;
	// 	}
	return nullptr;
}

// DOL 0x800b7f44 [audited: class row done in the 43rd session; function not individually re-read]
void cAppG03PacnRollGame::IjikeRegist(int nTime)
{
	if (nTime != 0)
	{
		m_nIjikeCount = nTime;
	}
	else
	{
		m_nIjikeCount = m_nIjikeTime;
	}
	SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE032_IJIKE);
	SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE033_IJIKE_OWARI);
	if (cPacnRollLoopSndMan::CanPlay()) cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE032_IJIKE, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY));
	if (m_pTaskCharPacman != nullptr)
	{
		m_pTaskCharPacman->IjikeStart();
	}
	if (m_pTaskBoss != nullptr)
	{
		m_pTaskBoss->IjikeStart();
	}
	if (m_pTaskGhostMan != nullptr)
	{
		m_pTaskGhostMan->IjikeStart();
	}
}

// DOL 0x800b805c [audited: described by the 43rd-session pass] (the first paused task keeps running, every other one is paused)
bool cAppG03PacnRollGame::IsNakajimaPause(cTaskKingdom* pChar)
{
	if ((int)m_pvecNakajimaCaliph.size() == 0)
	{
		return false;
	}
	if (pChar == m_pvecNakajimaCaliph[0])
	{
		return false;
	}
	return true;
}

// DOL 0x800b808c [audited: class row done in the 43rd session; function not individually re-read] (push_back)
bool cAppG03PacnRollGame::NakajimaPause(cTaskKingdom* pChar)
{
	m_pvecNakajimaCaliph.push_back(pChar);
	return true;
}

// DOL 0x800b8384 [audited: described by the 43rd-session pass] (erases the first match)
bool cAppG03PacnRollGame::NakajimaPlay(cTaskKingdom* pChar)
{
	for (cTaskKingdom* item : m_pvecNakajimaCaliph)
	{
		if (item == pChar)
		{
			CSharpCompat::Remove(m_pvecNakajimaCaliph, item);
			return true;
		}
	}
	return false;
}

// DOL 0x800b841c [audited: class row done in the 43rd session; function not individually re-read]
uint cAppG03PacnRollGame::EnemyGetNum()
{
	return m_pTaskGhostMan->GetGhostNum();
}

bool cAppG03PacnRollGame::EnemyIsActive(uint no)
{
	return m_pTaskGhostMan->GhostIsActive(no);
}

// DOL 0x800b8424 [audited: class row done in the 43rd session; function not individually re-read]
uint cAppG03PacnRollGame::EnemyGetActiveNum()
{
	uint num = 0u;
	uint num2 = EnemyGetNum();
	for (uint num3 = 0u; num3 < num2; num3++)
	{
		if (EnemyIsActive(num3))
		{
			num++;
		}
	}
	return num;
}

// DOL 0x800b84a0 [audited: class row done in the 43rd session; function not individually re-read]
bool cAppG03PacnRollGame::EnemyActive(uint no)
{
	if (!EnemyIsActive(no))
	{
		return m_pTaskGhostMan->GhostRevive(no);
	}
	return true;
}

// DOL 0x800b84f8 [audited: class row done in the 43rd session; function not individually re-read]
void cAppG03PacnRollGame::StopIjikeSnd()
{
	SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE033_IJIKE_OWARI);
	SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE032_IJIKE);
	m_nIjikeCount = 0;
}

}
