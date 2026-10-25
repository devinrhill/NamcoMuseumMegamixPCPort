#pragma once

#include "common.hpp"
#include <SSaveDataPacMotos.hpp>
#include "kingdom/sMatchingData.hpp"

namespace kingdom::game::g07
{

class cG07MotosSys
{
public:
	enum ESave
	{
		eSAVE_NONE = 0,
		eSAVE_SCORE = 1,
		eSAVE_ATHER = 2
	};

	static inline const string ARC_NAME_RESIDENT = "g07_motos/g07resident.lzs";

	static inline const string ARC_NAME_ENEMY = "g07_motos/g07enemy.lzs";

	static inline const string ARC_NAME_MENU = "g07_motos/g07menu.lzs";

	static const string ARC_NAME_STAGE_TBL[46];

private:
	static int m_nInstance;

	uint m_nStageIdx{};

	uint m_nStageIdxTmp{};

	bool m_IsDebugMuteki{};

	bool m_bMeteoFall{};

	uint m_nLastBossType{};

	sMatchingData* m_MatchingData{};

	uint m_nVsItem{};

	uint m_nVsRound{};

	uint m_nVsTime{};

	int m_nVsTimeCount{};

	int m_nVsRoundCount{};

	int m_nVsItemCount{};

	int m_nForceVsRoundCount{};

	int m_nForceVsItemCount{};

	uint m_nWinsRound[36]{};

	bool m_bEnemyNoMove{};

	bool m_bMotosCharDead[36]{};

	bool m_bNewStageApear{};

	bool m_bAllClear{};

	bool m_bPlayerRetry{};

	SSaveDataPacMotos m_sSaveData{};

public:
	static uint RECORD_TIME_DUMMY;

	static int DEFAULT_PLAYER_LIFE;

	int PrevWorldNo = -1;

	bool DebugStageAllUnlock{};

	cG07MotosSys();

	virtual ~cG07MotosSys();

	uint& VsRoundIdx();

	uint& VsTimeIdx();

	uint& VsItemIdx();

	uint GetVsRound();

	uint GetVsTime();

	uint GetVsItem();

	bool& IsDebugMuteki();

	bool& bMeteoFall();

	bool& bEnemyNoMove();

	bool& bMotosCharDead(uint set);

	bool IsNewStageApear();

	bool& bNewStageApear();

	sMatchingData*& pGetMatchingData();

	int& nVsTimeCount();

	int& nVsRoundCount();

	int& nVsItemCount();

	int& nForceVsRoundCount();

	uint& nWinsRound(uint set);

	uint& nLastBossType();

	bool IsPlayerRetry();

	bool& bPlayerRetry();

	SSaveDataPacMotos& pSaveData();

	SSaveDataPacMotos::ScoreData& pScoreData(uint n);

	SSaveDataPacMotos::ScoreData* pScoreDataArray();

private:
	void Init();

	void Term();

public:
	void Free();

	bool SaveDataLoad();

	uint SaveDataSave(uint nScore, uint nTime, bool bClear, bool bDataWrite = true);

	uint WriteMemoryToSaveData(bool bSave = false, bool bDataWrite = true);

	void SetTitleData();

	void MultiDataInit();

	bool AttachStageData(uint idx = 0u);

	bool DetachStageData(uint idx = 0u);

	bool AttachMenuData();

	bool DetachMenuData();

	bool RegistStageData(uint idx = 0u);

	uint& nStageIdx();

	uint& nStageIdxTmp();

	bool IsBossStage();

	uint nWorld();

	uint nStage();

	uint Idx2World(uint nIdx);

	uint Idx2Stage(uint nIdx);

	uint WorldAndStage2Idx(uint w_Idx, uint s_Idx);

	uint GetFreeStage();

	uint GetStageHiScore(uint wrd_idx, uint stg_idx);

	uint GetStageHiScore(uint idx);

	uint GetStageRecordTime(uint wrd_idx, uint stg_idx);

	uint GetStageRecordTime(uint idx);

	bool GetStageHiScoreNew(uint wrd_idx, uint stg_idx);

	bool GetStageHiScoreNew(uint idx);

	bool GetStageRecordTimeNew(uint wrd_idx, uint stg_idx);

	bool GetStageRecordTimeNew(uint idx);

	bool IsStageOpen(uint wrd_idx, uint stg_idx);

	bool IsStageOpen(uint idx);

	bool IsAllClear();

	bool SetStageHiScore(uint wrd_idx, uint stg_idx, uint set, bool bDataWrite = true);

	bool SetStageHiScore(uint idx, uint set, bool bDataWrite = true);

	bool SetStageRecordTime(uint wrd_idx, uint stg_idx, uint set, bool bDataWrite = true);

	bool SetStageRecordTime(uint idx, uint set, bool bDataWrite = true);

	bool SetStageOpen(uint wrd_idx, uint stg_idx, bool set, bool bDataWrite = true);

	bool SetStageOpen(uint idx, bool set, bool bDataWrite = true);

	void StageAllOpen();

	bool SetAllClear(bool set);
};

}
