#pragma once

#include "common.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"
#include "kingdom/game/g07/MOTOS_VS_SETTING.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"
#include "kingdom/game/g07/MO_KEEP_DATA.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"

namespace kingdom::game::g07
{

class cMotosGameSys
{
public:
	static constexpr int MAX_STATIC_VAL_ARRAY_NUM = 11;

	static const uint MOTOS_RAND_TBL[512];

private:
	static int m_nInstance;

	MO_KEEP_DATA* m_KeepData[4]{};

	static bool m_nIdentityCard;

	uint m_nID{};

	uint m_nSyncCounter{};

	uint m_nGameCounter{};

	uint m_nSyncCounterLocal{};

	uint m_nGameCounterLocal{};

	uint m_nJoinFlag{};

	uint m_nMGFlagRecieve{};

	uint m_nMGFlagSend{};

	uint m_nSGFlagRecieve[4]{};

	uint m_nSGFlagSend{};

	uint m_nVSRound{};

	uint m_nGameKind{};

	int m_nGameLevel{};

	int m_nStartMode{};

	bool m_nLoadedData{};

	bool m_nSharingDone{};

	float m_AxisAttack[6]{};

	bool m_nPacketSndRev{};

	MO_CHAR_PARAMS* m_MoEnemyParam[27]{};

	MOTOS_VS_PARAM* m_MoVSParam{};

	MOTOS_VS_SETTING* m_VSSetting{};

	bool m_bVSSharing{};

	float m_PowerLevel2Power[11]{};

	short m_HoverLevel2Time[11]{};

	short m_ChargeLevel2Time[11]{};

	float m_ChargeLevel2Power[11]{};

	float m_ChargeLevel2MyRush[11]{};

	float m_ChargeLevel2Heretic[11]{};

	float m_ChargeLevel2Speed[11]{};

	short m_ChargeLevel2Weight[11]{};

	short m_ChargeLevel2PiyoTime[11]{};

	MO_CHAR_PARAMS* m_MoPlayerParam{};

	int m_deadEnemyCnt{};

	int m_getItemCnt{};

public:
	int get_DeadEnemyCnt();

	int get_GetItemCnt();

	cMotosGameSys();

	void Free();

	uint ID();

	void ID(uint n);

	uint& SyncCounter();

	uint& GameCounter();

	uint& SyncCounterLocal();

	uint& GameCounterLocal();

	MO_KEEP_DATA** KeepData();

	uint RandMax();

	uint GameKind();

	uint TestMasterFlag(uint flag);

	bool bTestMasterFlag(uint flag);

	void OnMasterFlag(uint flag);

	void OffMasterFlag(uint flag);

	void XchgMasterFlag(uint flag);

	uint TestSlaveFlag(int id, uint flag);

	bool bTestSlaveFlag(int id, uint flag);

	void OnSlaveFlag(uint id, uint flag);

	void OffSlaveFlag(uint id, uint flag);

	void XchgSlaveFlag(uint id, uint flag);

	MOTOS_VS_PARAM*& GetVsParam();

	float* pAxisAttack();

	int& nGameLevel();

	uint JoinFlag();

	void JoinFlag(uint n);

	int& nStartMode();

	MOTOS_VS_PARAM* pVSParam();

	MOTOS_VS_SETTING* pVSSetting();

	bool& bVSSharing();

	sbyte& nVSStage(int no = 0);

	sbyte& nVSSharingMap();

	ushort& nVSLimitTime();

	sbyte& nVSLimitNo();

	byte& nVSWinNum(int num);

	byte& nVSColor(int num);

	byte& nVSItemNum();

	byte& nVSRoundWinNum(int num);

	byte& nVSSPCount(int num);

	sbyte& nVSSPLimit();

	float* SysPowerLevel2Power();

	short* SysHoverLevel2Time();

	short* SysChargeLevel2Time();

	float* SysChargeLevel2Power();

	float* SysChargeLevel2MyRush();

	float* SysChargeLevel2Heretic();

	float* SysChargeLevel2Speed();

	short* SysChargeLevel2Weight();

	short* SysChargeLevel2PiyoTime();

	MO_CHAR_PARAMS*& SysMoPlayerParam();

	MO_CHAR_PARAMS** SysMoEnemyParam();

	void ResetDeadEnemyCnt();

	void AddDeadEnemyCnt(int add = 1);

	void ResetGetItemCnt();

	void AddGetItemCnt(int add = 1);

	uint TestSlaveFlagAll(uint flag);

	uint TestSlaveFlagSomebody(uint flag);

private:
	void Init();

	void Term();

public:
	void NeedInit();

	void MultiDataInit();

	uint RandU(uint index);

	float RandF(uint index);

	uint RandRangeU(uint min, uint max, uint index);

	float RandRangeF(float min, float max, uint index);

	bool IsGameSysMyChar(cTaskChar* p);

	bool IsGameSysLanPlay();

	bool IsGameSysMaster();

	void NET_Sync();
};

}
