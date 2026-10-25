#include "platform/devtools.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"

#include <string>

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/eSAVE_VERSION.hpp"
#include "kingdom/sMatchingData.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <GameStateManager.hpp>

namespace kingdom::game::g07
{

const string cG07MotosSys::ARC_NAME_STAGE_TBL[46] = {"g07_motos/g07wxxsxx.lzs", "g07_motos/g07w01s01.lzs", "g07_motos/g07w01s02.lzs", "g07_motos/g07w01s03.lzs", "g07_motos/g07w01s04.lzs", "g07_motos/g07w01s05.lzs", "g07_motos/g07w01s06.lzs", "g07_motos/g07w01s07.lzs", "g07_motos/g07w02s01.lzs", "g07_motos/g07w02s02.lzs", "g07_motos/g07w02s03.lzs", "g07_motos/g07w02s04.lzs", "g07_motos/g07w02s05.lzs", "g07_motos/g07w02s06.lzs", "g07_motos/g07w02s07.lzs", "g07_motos/g07w03s01.lzs", "g07_motos/g07w03s02.lzs", "g07_motos/g07w03s03.lzs", "g07_motos/g07w03s04.lzs", "g07_motos/g07w03s05.lzs", "g07_motos/g07w03s06.lzs", "g07_motos/g07w03s07.lzs", "g07_motos/g07w04s01.lzs", "g07_motos/g07w04s02.lzs", "g07_motos/g07w04s03.lzs", "g07_motos/g07w04s04.lzs", "g07_motos/g07w04s05.lzs", "g07_motos/g07w04s06.lzs", "g07_motos/g07w04s07.lzs", "g07_motos/g07w05s01.lzs", "g07_motos/g07w05s02.lzs", "g07_motos/g07w05s03.lzs", "g07_motos/g07w05s04.lzs", "g07_motos/g07w05s05.lzs", "g07_motos/g07w05s06.lzs", "g07_motos/g07w05s07.lzs", "g07_motos/g07vs01.lzs", "g07_motos/g07vs02.lzs", "g07_motos/g07vs03.lzs", "g07_motos/g07vs04.lzs", "g07_motos/g07vs05.lzs", "g07_motos/g07vs06.lzs", "g07_motos/g07vs07.lzs", "g07_motos/g07vs08.lzs", "g07_motos/g07vs09.lzs", "g07_motos/g07vs10.lzs"};
int cG07MotosSys::m_nInstance = 0;
uint cG07MotosSys::RECORD_TIME_DUMMY = 0u;
int cG07MotosSys::DEFAULT_PLAYER_LIFE = 4;

cG07MotosSys::cG07MotosSys()
{
	Init();
}

cG07MotosSys::~cG07MotosSys()
{
	Free();
}

uint& cG07MotosSys::VsRoundIdx()
{
	return m_nVsRound;
}

uint& cG07MotosSys::VsTimeIdx()
{
	return m_nVsTime;
}

uint& cG07MotosSys::VsItemIdx()
{
	return m_nVsItem;
}

uint cG07MotosSys::GetVsRound()
{
	CustomKingdomUtil::Assert(0 <= m_nVsRound && m_nVsRound < 4, "Index Over");
	return Constants::MULTI_ROUND_LIST[m_nVsRound];
}

uint cG07MotosSys::GetVsTime()
{
	CustomKingdomUtil::Assert(0 <= m_nVsTime && m_nVsTime < 5, "Index Over");
	return Constants::MULTI_TIME_LIST[m_nVsTime];
}

uint cG07MotosSys::GetVsItem()
{
	CustomKingdomUtil::Assert(0 <= m_nVsItem && m_nVsItem < 6, "Index Over");
	return Constants::MULTI_ITEM_LIST[m_nVsItem];
}

bool& cG07MotosSys::IsDebugMuteki()
{
	return m_IsDebugMuteki;
}

bool& cG07MotosSys::bMeteoFall()
{
	return m_bMeteoFall;
}

bool& cG07MotosSys::bEnemyNoMove()
{
	return m_bEnemyNoMove;
}

bool& cG07MotosSys::bMotosCharDead(uint set)
{
	CustomKingdomUtil::Assert(set < 36, "error");
	return m_bMotosCharDead[set];
}

bool cG07MotosSys::IsNewStageApear()
{
	return m_bNewStageApear;
}

bool& cG07MotosSys::bNewStageApear()
{
	return m_bNewStageApear;
}

sMatchingData*& cG07MotosSys::pGetMatchingData()
{
	return m_MatchingData;
}

int& cG07MotosSys::nVsTimeCount()
{
	return m_nVsTimeCount;
}

int& cG07MotosSys::nVsRoundCount()
{
	return m_nVsRoundCount;
}

int& cG07MotosSys::nVsItemCount()
{
	return m_nVsItemCount;
}

int& cG07MotosSys::nForceVsRoundCount()
{
	return m_nForceVsRoundCount;
}

uint& cG07MotosSys::nWinsRound(uint set)
{
	CustomKingdomUtil::Assert(set < 36, "error");
	return m_nWinsRound[set];
}

uint& cG07MotosSys::nLastBossType()
{
	return m_nLastBossType;
}

bool cG07MotosSys::IsPlayerRetry()
{
	return m_bPlayerRetry;
}

bool& cG07MotosSys::bPlayerRetry()
{
	return m_bPlayerRetry;
}

SSaveDataPacMotos& cG07MotosSys::pSaveData()
{
	return m_sSaveData;
}

SSaveDataPacMotos::ScoreData& cG07MotosSys::pScoreData(uint n)
{
	CustomKingdomUtil::Assert(n < 46, "error");
	return m_sSaveData.sScoreDatas[n];
}

SSaveDataPacMotos::ScoreData* cG07MotosSys::pScoreDataArray()
{
	return m_sSaveData.sScoreDatas.data();
}

void cG07MotosSys::Init()
{
	CustomKingdomUtil::Assert(m_nInstance++ == 0, "fatal error");
	KingdomGlobal::KingSys->AttachArcData(ARC_NAME_RESIDENT, true);
	KingdomGlobal::KingSys->AttachArcData(ARC_NAME_ENEMY, true);   // UNVERIFIED where the DOL attaches the shared enemy models   // DOL 0x800a0178 (sys constructor): the shared g07 data (CSV params, ...)
	Constants::Motos_Game_System_Init();
	m_nStageIdx = 0u;
	m_bNewStageApear = false;
	m_bAllClear = false;
	bPlayerRetry() = false;
	std::fill(std::begin(m_bMotosCharDead), std::end(m_bMotosCharDead), false);
	SaveDataLoad();
	m_MatchingData = new sMatchingData();
	m_MatchingData->nNum = 4u;
	for (uint num = 0u; num < 4; num++)
	{
		m_MatchingData->Info[num]->nCtrl = num;
		m_MatchingData->Info[num]->nWork = 0u;
		m_MatchingData->Info[num]->Name = string("motos") + std::to_string(num);
	}
	MultiDataInit();
	m_IsDebugMuteki = false;
	m_bMeteoFall = true;
	m_bEnemyNoMove = false;
	PrevWorldNo = -1;
}

void cG07MotosSys::Term()
{
	CustomKingdomUtil::Assert(--m_nInstance == 0, "fatal error");
	Constants::Motos_Game_System_Term();
}

void cG07MotosSys::Free()
{
	Term();
}

bool cG07MotosSys::SaveDataLoad()
{
	m_sSaveData = GameStateManager::m_pacMotosSave;
	bool flag = false;
	eSAVE_VERSION nSaveVersion = (eSAVE_VERSION)m_sSaveData.nSaveVersion;
	if (nSaveVersion == eSAVE_VERSION::eSAVEDATA_VER_NONE || nSaveVersion != eSAVE_VERSION::eSAVEDATA_VER_0102)
	{
		flag = true;
	}
	if (flag)
	{
		m_sSaveData = SSaveDataPacMotos();
		m_sSaveData.Init();
		SetStageOpen(1u, true);
		for (uint num = 0u; num < 46; num++)
		{
			pScoreData(num).nRecordTime = RECORD_TIME_DUMMY;
		}
		m_sSaveData.nSaveVersion = 3;
		return true;
	}
	for (uint num2 = 1u; num2 < 4; num2++)
	{
		KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nItemNum[num2] = (byte)m_sSaveData.nItemData[num2];
	}
	SetAllClear(m_sSaveData.bAllClear);
	return false;
}

// DOL 0x800a0438
uint cG07MotosSys::SaveDataSave(uint nScore, uint nTime, bool bClear, bool bDataWrite)
{
	uint num = WriteMemoryToSaveData(bDataWrite);
	uint idx = nStageIdx() + 1;
	if (nStageIdx() != 35)
	{
		if (SetStageOpen(idx, bClear, bDataWrite))
		{
			if (bDataWrite)
			{
				m_bNewStageApear = true;
				RegistStageData(idx);
			}
			num = 2u;
		}
	}
	else if (bDataWrite && !m_sSaveData.bAllClear)
	{
		SetAllClear(true);
		m_sSaveData.bAllClear = true;
		m_bNewStageApear = true;
	}
	// The C# opened every stage up to the cleared one here; the DOL (0x800a0438) opens only the next one (above): a stage cleared from
	// the debug menu must not unlock the stages before it (checked in Dolphin, 27th session: clearing w01s05 opens only w01s06).
	if (SetStageHiScore(nStageIdx(), nScore, bDataWrite))
	{
		num = 1u;
	}
	if (SetStageRecordTime(nStageIdx(), nTime, bDataWrite))
	{
		num = 1u;
	}
	if (num != 0 && bDataWrite)
	{
		SetTitleData();
	}
	return num;
}

uint cG07MotosSys::WriteMemoryToSaveData(bool bSave, bool bDataWrite)
{
	uint result = 0u;
	for (uint num = 1u; num < 4; num++)
	{
		if (m_sSaveData.nItemData[num] != KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nItemNum[num])
		{
			if (bDataWrite)
			{
				m_sSaveData.nItemData[num] = KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nItemNum[num];
			}
			result = 2u;
		}
	}
	if (bSave)
	{
		SetTitleData();
	}
	return result;
}

void cG07MotosSys::SetTitleData()
{
	if (devtools::PreviewAll()) return; // devtools: --preview-all never persists (not 1:1)
	GameStateManager::m_pacMotosSave = m_sSaveData;
}

void cG07MotosSys::MultiDataInit()
{
	VsItemIdx() = 2u;
	VsTimeIdx() = 2u;
	VsRoundIdx() = 2u;
	nForceVsRoundCount() = -1;
	std::fill(std::begin(m_nWinsRound), std::end(m_nWinsRound), 0u);
}

bool cG07MotosSys::AttachStageData(uint idx)
{
	if (idx == 0)
	{
		idx = m_nStageIdx;
	}
	CustomKingdomUtil::Assert((long)idx < (long)::Constants::ARRAY_SIZE(ARC_NAME_STAGE_TBL), "idx error");
	if (KingdomGlobal::KingSys->AttachArcData(ARC_NAME_STAGE_TBL[idx]))
	{
		return true;
	}
	return false;
}

bool cG07MotosSys::DetachStageData(uint idx)
{
	if (idx == 0)
	{
		idx = m_nStageIdx;
	}
	CustomKingdomUtil::Assert((long)idx < (long)::Constants::ARRAY_SIZE(ARC_NAME_STAGE_TBL), "idx error");
	if (KingdomGlobal::KingSys->DetachArcData(ARC_NAME_STAGE_TBL[idx]))
	{
		return true;
	}
	return false;
}

bool cG07MotosSys::AttachMenuData()
{
	// archives are real files here (the Unity port had no equivalent); DOL: the menu app's constructor attaches g07menu.lzs
	return KingdomGlobal::KingSys->AttachArcData(ARC_NAME_MENU);
}

bool cG07MotosSys::DetachMenuData()
{
	return KingdomGlobal::KingSys->DetachArcData(ARC_NAME_MENU);
}

bool cG07MotosSys::RegistStageData(uint idx)
{
	if (idx == 0)
	{
		idx = m_nStageIdx;
	}
	CustomKingdomUtil::Assert((long)idx < (long)::Constants::ARRAY_SIZE(ARC_NAME_STAGE_TBL), "idx error");
	if (KingdomGlobal::KingSys->RegistArcDataAS(ARC_NAME_STAGE_TBL[idx]))
	{
		return true;
	}
	return false;
}

uint& cG07MotosSys::nStageIdx()
{
	return m_nStageIdx;
}

uint& cG07MotosSys::nStageIdxTmp()
{
	return m_nStageIdxTmp;
}

bool cG07MotosSys::IsBossStage()
{
	if (KingdomGlobal::MotosSys->nStageIdx() <= 35 && KingdomGlobal::MotosSys->nStage() == 7)
	{
		return true;
	}
	return false;
}

uint cG07MotosSys::nWorld()
{
	switch (m_nStageIdx)
	{
	case 0u:
		return 0u;
	case 1u:
	case 2u:
	case 3u:
	case 4u:
	case 5u:
	case 6u:
	case 7u:
		return 1u;
	case 8u:
	case 9u:
	case 10u:
	case 11u:
	case 12u:
	case 13u:
	case 14u:
		return 2u;
	case 15u:
	case 16u:
	case 17u:
	case 18u:
	case 19u:
	case 20u:
	case 21u:
		return 3u;
	case 22u:
	case 23u:
	case 24u:
	case 25u:
	case 26u:
	case 27u:
	case 28u:
		return 4u;
	case 29u:
	case 30u:
	case 31u:
	case 32u:
	case 33u:
	case 34u:
	case 35u:
		return 5u;
	case 36u:
	case 37u:
		return 1u;
	case 38u:
	case 39u:
		return 2u;
	case 40u:
	case 41u:
		return 3u;
	case 42u:
	case 43u:
		return 4u;
	case 44u:
	case 45u:
		return 5u;
	default:
		CustomKingdomUtil::Assert("fatal error");
		return 0u;
	}
}

uint cG07MotosSys::nStage()
{
	if (m_nStageIdx < 36)
	{
		switch (nWorld())
		{
		case 0u:
			return 0u;
		case 1u:
			return m_nStageIdx - 1 + 1;
		case 2u:
			return m_nStageIdx - 8 + 1;
		case 3u:
			return m_nStageIdx - 15 + 1;
		case 4u:
			return m_nStageIdx - 22 + 1;
		case 5u:
			return m_nStageIdx - 29 + 1;
		default:
			CustomKingdomUtil::Assert("fatal error");
			return 0u;
		}
	}
	return m_nStageIdx - 36 + 1;
}

uint cG07MotosSys::Idx2World(uint nIdx)
{
	switch (nIdx)
	{
	case 0u:
		return 0u;
	case 1u:
	case 2u:
	case 3u:
	case 4u:
	case 5u:
	case 6u:
	case 7u:
		return 1u;
	case 8u:
	case 9u:
	case 10u:
	case 11u:
	case 12u:
	case 13u:
	case 14u:
		return 2u;
	case 15u:
	case 16u:
	case 17u:
	case 18u:
	case 19u:
	case 20u:
	case 21u:
		return 3u;
	case 22u:
	case 23u:
	case 24u:
	case 25u:
	case 26u:
	case 27u:
	case 28u:
		return 4u;
	case 29u:
	case 30u:
	case 31u:
	case 32u:
	case 33u:
	case 34u:
	case 35u:
		return 5u;
	case 36u:
	case 37u:
		return 1u;
	case 38u:
	case 39u:
		return 2u;
	case 40u:
	case 41u:
		return 3u;
	case 42u:
	case 43u:
		return 4u;
	case 44u:
	case 45u:
		return 5u;
	default:
		CustomKingdomUtil::Assert("fatal error");
		return 0u;
	}
}

uint cG07MotosSys::Idx2Stage(uint nIdx)
{
	if (nIdx < 36)
	{
		switch (Idx2World(nIdx))
		{
		case 0u:
			return 0u;
		case 1u:
			return nIdx - 1 + 1;
		case 2u:
			return nIdx - 8 + 1;
		case 3u:
			return nIdx - 15 + 1;
		case 4u:
			return nIdx - 22 + 1;
		case 5u:
			return nIdx - 29 + 1;
		default:
			CustomKingdomUtil::Assert("fatal error");
			return 0u;
		}
	}
	return nIdx - 36 + 1;
}

uint cG07MotosSys::WorldAndStage2Idx(uint w_Idx, uint s_Idx)
{
	uint num = 0u;
	uint num2 = 0u;
	switch (w_Idx)
	{
	case 1u:
		num = 1u;
		break;
	case 2u:
		num = 8u;
		break;
	case 3u:
		num = 15u;
		break;
	case 4u:
		num = 22u;
		break;
	case 5u:
		num = 29u;
		break;
	default:
		CustomKingdomUtil::Assert("(^-^;)");
		break;
	}
	CustomKingdomUtil::Assert(s_Idx != 0, "(^-^;)");
	CustomKingdomUtil::Assert(s_Idx < 8, "(^-^;)");
	num2 = s_Idx - 1;
	return num + num2;
}

uint cG07MotosSys::GetFreeStage()
{
	uint result = 1u;
	for (uint num = 1u; num <= 35 && IsStageOpen(num); num++)
	{
		result = num;
	}
	return result;
}

uint cG07MotosSys::GetStageHiScore(uint wrd_idx, uint stg_idx)
{
	return GetStageHiScore(WorldAndStage2Idx(wrd_idx, stg_idx));
}

uint cG07MotosSys::GetStageHiScore(uint idx)
{
	return pScoreData(idx).nHiScore;
}

uint cG07MotosSys::GetStageRecordTime(uint wrd_idx, uint stg_idx)
{
	return GetStageRecordTime(WorldAndStage2Idx(wrd_idx, stg_idx));
}

uint cG07MotosSys::GetStageRecordTime(uint idx)
{
	return pScoreData(idx).nRecordTime;
}

bool cG07MotosSys::GetStageHiScoreNew(uint wrd_idx, uint stg_idx)
{
	return GetStageHiScoreNew(WorldAndStage2Idx(wrd_idx, stg_idx));
}

bool cG07MotosSys::GetStageHiScoreNew(uint idx)
{
	return pScoreData(idx).bHiScoreNew;
}

bool cG07MotosSys::GetStageRecordTimeNew(uint wrd_idx, uint stg_idx)
{
	return GetStageRecordTimeNew(WorldAndStage2Idx(wrd_idx, stg_idx));
}

bool cG07MotosSys::GetStageRecordTimeNew(uint idx)
{
	return pScoreData(idx).bRecordTimeNew;
}

bool cG07MotosSys::IsStageOpen(uint wrd_idx, uint stg_idx)
{
	return IsStageOpen(WorldAndStage2Idx(wrd_idx, stg_idx));
}

bool cG07MotosSys::IsStageOpen(uint idx)
{
	if (devtools::PreviewAll()) return true; // devtools: --preview-all (not 1:1)
	return pScoreData(idx).bOpen;
}

bool cG07MotosSys::IsAllClear()
{
	return m_bAllClear;
}

bool cG07MotosSys::SetStageHiScore(uint wrd_idx, uint stg_idx, uint set, bool bDataWrite)
{
	return SetStageHiScore(WorldAndStage2Idx(wrd_idx, stg_idx), set, bDataWrite);
}

bool cG07MotosSys::SetStageHiScore(uint idx, uint set, bool bDataWrite)
{
	cKingdomCSV* obj = new cKingdomCSV();
	obj->RegistData("CSV_BASE_SCORE");
	int _out;
	obj->S32((int)(idx + 1), 1, _out);
	if (set > pScoreData(idx).nHiScore)
	{
		if (bDataWrite)
		{
			if ((long)set >= (long)_out)
			{
				pScoreData(idx).bHiScoreNew = true;
			}
			pScoreData(idx).nHiScore = set;
		}
		return true;
	}
	return false;
}

bool cG07MotosSys::SetStageRecordTime(uint wrd_idx, uint stg_idx, uint set, bool bDataWrite)
{
	return SetStageRecordTime(WorldAndStage2Idx(wrd_idx, stg_idx), set, bDataWrite);
}

bool cG07MotosSys::SetStageRecordTime(uint idx, uint set, bool bDataWrite)
{
	cKingdomCSV* obj = new cKingdomCSV();
	obj->RegistData("CSV_BASE_SCORE");
	int _out = 0;
	obj->S32((int)(idx + 1), 2, _out);
	if (set < pScoreData(idx).nRecordTime || pScoreData(idx).nRecordTime == RECORD_TIME_DUMMY)
	{
		if (bDataWrite)
		{
			if ((long)set < (long)_out)
			{
				pScoreData(idx).bRecordTimeNew = true;
			}
			pScoreData(idx).nRecordTime = set;
		}
		return true;
	}
	return false;
}

bool cG07MotosSys::SetStageOpen(uint wrd_idx, uint stg_idx, bool set, bool bDataWrite)
{
	return SetStageOpen(WorldAndStage2Idx(wrd_idx, stg_idx), set, bDataWrite);
}

bool cG07MotosSys::SetStageOpen(uint idx, bool set, bool bDataWrite)
{
	if (pScoreData(idx).bOpen == set)
	{
		return false;
	}
	if (bDataWrite)
	{
		pScoreData(idx).bOpen = set;
	}
	return true;
}

void cG07MotosSys::StageAllOpen()
{
	for (uint num = 1u; num <= 35; num++)
	{
		SetStageOpen(num, true);
	}
	SetAllClear(true);
	pSaveData().bAllClear = true;
}

bool cG07MotosSys::SetAllClear(bool set)
{
	m_bAllClear = set;
	return m_bAllClear;
}

}
