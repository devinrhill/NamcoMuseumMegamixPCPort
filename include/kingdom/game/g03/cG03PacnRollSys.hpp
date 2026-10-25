#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cSaveLoadMan.hpp"

namespace kingdom::game::g03
{

class cG03PacnRollSys
{
public:
	PostProcessLayer* PostProcess{};   // Unity post-processing layer: not used natively (always null)

public:
	enum ERound
	{
		eROUND_W01S01 = 0,
		eROUND_W01S02 = 1,
		eROUND_W01S03 = 2,
		eROUND_W01S04 = 3,
		eROUND_W01S05 = 4,
		eROUND_W02S01 = 5,
		eROUND_W02S02 = 6,
		eROUND_W02S03 = 7,
		eROUND_W02S04 = 8,
		eROUND_W03S01 = 9,
		eROUND_W03S02 = 10,
		eROUND_W03S03 = 11,
		eROUND_W03S04 = 12,
		eROUND_W03S05 = 13,
		eROUND_W04S01 = 14,
		eROUND_W04S02 = 15,
		eROUND_W04S03 = 16,
		eROUND_W04S04 = 17,
		eROUND_W04S05 = 18,
		eROUND_W04S06 = 19,
		eROUND_W05S01 = 20,
		eROUND_W05S02 = 21,
		eROUND_W05S03 = 22,
		eROUND_W05S04 = 23,
		eROUND_W05S05 = 24,
		eROUND_W05S06 = 25,
		eROUND_GAME_MAX = 26,
		eROUND_X_START = eROUND_GAME_MAX,
		eROUND_WXXS03 = eROUND_GAME_MAX,
		eROUND_WXXS04 = 27,
		eROUND_MAX = 28
	};

	enum ELightSet
	{
		eLIGHT_SET_BG = 0,
		eLIGHT_SET_ITEM = 1,
		eLIGHT_SET_MODEL = 2,
		eLIGHT_SET_GIMMICK = 3
	};

	class sRetryPoint
	{
	public:
		uint nPassIdx{};
	};

	class sRetryItem
	{
	public:
		uint nKind{};

		uint nSFlag{};

		uint nDFlag{};

		Vector4 vInitTrans{};
	};

	class sRetryGate
	{
	public:
		uint nKind{};

		uint nNeedCookieNum{};

		uint nPriorty{};

		Vector4 vTrans{};

		Vector4 vRotate{};
	};

	class sRetryBox
	{
	public:
		uint nKind{};

		uint nTreasure{};

		bool bBreak{};

		Vector4 vTrans{};

		Vector4 vRotate{};
	};

	class sRetryGhost
	{
	public:
		uint nAreaGhostFlag[8]{};

		uint nRouteGhostFlag[6]{};
	};

	class sRetryData
	{
	public:
		uint nLife{};

		uint nMaxHP{};

		uint nGameTimeCnt{};

		int nRetryPointIdx{};

		uint nAteCookie{};

		uint nHaveCookie{};

		std::vector<cG03PacnRollSys::sRetryItem*> vecItem;

		std::vector<cG03PacnRollSys::sRetryPoint*> vecPoint;

		std::vector<cG03PacnRollSys::sRetryGate*> vecGate;

		std::vector<cG03PacnRollSys::sRetryBox*> vecBox;

		std::vector<cG03PacnRollSys::sRetryGhost*> vecGhost;
	};

	static inline const string ARC_DATA_RESIDENT = "g03_pacnroll/arcsv/g03resident";

	static const string arcKeys[28];

private:
	uint m_nRound{};

	bool m_bTopMenuMode{};

	bool m_bAllClear{};

	bool m_bOpenStage{};

	bool m_bRetry{};

	cSaveLoadMan* m_pSaveLoadMan{};

	cG03PacnRollSys::sRetryData* m_RetryData{};

public:
	bool DebugStageAllUnlock{};

	int DeadEnemyCnt{};

	int GetItemCnt{};

	cG03PacnRollSys();

	cG03PacnRollSys(bool game);

	virtual ~cG03PacnRollSys();

	uint& nRound();

	bool& IsRetry();

	cG03PacnRollSys::sRetryData*& pRetryData();

	cSaveLoadMan* pSaveLoadMan();

	void SetTopMenuMode(bool game);

	void ResetDeadEnemyCnt();

	void AddDeadEnemyCnt(int add = 1);

	void ResetGetItemCnt();

	void AddGetItemCnt(int add = 1);

private:
	void Init();

	void Term();

public:
	void Free();

	string pStageArcKey(uint round);

	uint nWorld(uint nWorldStage = std::numeric_limits<uint>::max());

	uint nStage(uint nWorldStage = std::numeric_limits<uint>::max());

	uint nStageMax(uint nWorldNo = std::numeric_limits<uint>::max());

	uint nWorldStage(uint nWorldNo, uint nStageNo);

	void nSetRound(uint nWorldNo, uint nStageNo);

	bool IsBossStage();

	static bool IsBossStage(uint round);

	bool IsTopMenuMode();

	uint nFreeStage();

	bool IsAllClear();

	void bAllClear(bool bFlag);

	bool IsOpenStage();

	void bOpenStage(bool bFlag);

	void InitRetryData();
};

}
