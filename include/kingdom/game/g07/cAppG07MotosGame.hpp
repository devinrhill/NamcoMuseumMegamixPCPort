#pragma once

#include "common.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/cTaskConsRetry.hpp"
#include "kingdom/game/g07/cMotosGetTime.hpp"
#include "kingdom/game/g07/cTaskBG.hpp"
#include "kingdom/game/g07/cTaskCamera.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "kingdom/game/g07/cTaskConsoleMan.hpp"
#include "kingdom/game/g07/cTaskEffect.hpp"
#include "kingdom/game/g07/cTaskHitMan.hpp"
#include "kingdom/game/g07/cTaskMotosFade.hpp"
#include "kingdom/game/g07/cTaskPause.hpp"

namespace kingdom::game::g07
{
class cTaskMap;
}

namespace kingdom::game::g07
{

class cAppG07MotosGame : public cAppKingdom
{
public:
	enum ELightSet
	{
		eLIGHT_SET_BG = 0,
		eLIGHT_SET_PANEL = 1,
		eLIGHT_SET_CHAR = 2,
		eLIGHT_SET_BOSS = 3
	};

	enum ETaskPri : uint
	{
		eTask_Pri_Default = 65536u,
		eTask_Pri_Console = 2147483648u,
		eTask_Pri_Fade = 2415919103u,
		eTask_Pri_Term = 4294901760u
	};

	static int DRAW_PLAYER_NONE;

	static uint PLAY_TIME_MAX;

	static uint PLAYER_COLOR[4];

private:
	cTaskCamera* m_pTaskCamera{};

	cTaskMap* m_pTaskMap{};

	cTaskChar* m_pChars[36]{};

	cTaskCharPlayer* m_pPlayers[4]{};

	cTaskCharEnemy* m_pEnemys[32]{};

	cTaskCharEnemy* m_pBoss{};

	cTaskHitMan* m_pHitMan{};

	cTaskEffect* m_pTaskEffect{};

	cTaskMotosFade* m_pTaskFade{};

	cTaskConsoleMan* m_pTaskConsoleMan{};

	cTaskConsRetry* m_pTaskConsRetry{};

	cTaskPause* m_pTaskPause{};

	cTaskBG* m_pTaskBG{};

	int m_nPLReserve{};

	int m_n1stPointPlayer{};

	int m_naMultiPlayerDrawList[4]{};

	int m_nMultiBattleResult{};

	static uint m_nGamePlayTime;

	float m_fSlopeRad{};

	float m_fSlopeRadOLD{};

	int m_nSlopeState{};

	Vector4 m_vSlopePower{};

	Vector4 m_vGravVector{};

	Matrix4x4 m_mSlopeMatrix{};

	bool m_bGimmick{};

	uint m_nPlayerNo{};

	bool m_bMultiPlay{};

	uint m_nPlayerParamSet{};

	float m_fConsSlopeRad{};

	float m_fConsSlopeRadOLD{};

	int m_nStep[4]{};

	int m_nBefStep{};

	uint m_nGameCounterOLD{};

	float m_fComingSELength{};

	bool m_IsGameStart{};

	bool m_IsMissedReserve{};

	bool m_IsClearReserve{};

	bool m_bFirstPlay{};

	uint m_nSaveFlags{};

	uint m_nLandCount[4]{};

	uint m_nPointCount[36]{};

	int m_nPoints[4]{};

	sbyte m_nPRanking[4]{};

	sbyte m_nTotalPRanking[4]{};

	int m_nMeteoTimerStart{};

public:
	static constexpr int READY_TIME = 185;

	static constexpr float HIT_REACTION_POW = 1.0f;

	static constexpr float HIT_REACTION_ITEM_POW = 3.0f;

	cMotosGetTime* MotosGetTime{};

	void STEP(int n);

	void STEP2SYNC(int step);

	void STEP2EXIT(int exit);

	cAppG07MotosGame();

	void Free() override;

	uint nPlayerParam();

	int nMultiBattleResult();

	uint GetGamePlayTime();

	bool IsFirstPlay();

	int nMeteoTimer();

	bool bMultiPlay();

	cTaskMap* pMap();

	void pSetMap(cTaskMap* set);

	cTaskChar* pChars(uint n);

	cTaskCharPlayer* pPlayers(uint n);

	cTaskCharEnemy* pEnemys(uint n);

	cTaskCharEnemy*& pBoss();

	cTaskHitMan* pHitMan();

	cTaskMotosFade* pTaskFade();

	cTaskConsoleMan* pTaskConsoleMan();

	cTaskEffect* pTaskEffect();

	float& fComingSELength();

	bool& bGimmick();

	uint& nGameCounterOLD();

	void GameCounterBackup();

	int& nScore();

	uint& nHiScore();

	short& nLife();

	uint& nLandCount(int n);

	int& nPoints(int n);

	uint& nPointCount(int n);

	sbyte& nPRanking(int n);

	sbyte& nTotalPRanking(int n);

	float& fConsSlopeRad();

	float& fConsSlopeRadOLD();

	int& nSlopeState();

	int n1stPointPlayer();

	int GetDrawPlayer(uint set);

	bool& IsGameStart();

	Vector4& vSlopePower();

	cTaskCamera* pCamera();

	Vector4& pGravVector();

	bool IsGameClearPause();

	int& nStep(int n);

	int& nBefStep();

	float& fSlopeRad();

	float& fSlopeRadOLD();

	Matrix4x4& pSlopeMatrix();

	bool& IsMissedReserve();

	bool& IsClearReserve();

private:
	void Init();

	void Term();

public:
	uint GetMultiBatleStageIdx();

	void MultiPlaySettings();

	void SinglePlaySettings();

	void BgmStart();

	int GetBgmNo();

	bool Execute(cJetTask* pParam) override;

	void HomeButtonCheck();

	void CameraPositionMoveSingle();

	void CameraPositionMoveMulti();

	void CheckEnemyDead();

	void Draw(int nStatus, cJetTask* pParam) override;

	void SlopeUpdate();

	void CameraDirPos(Vector4 vTrans, Vector4 vTarget, float div = 20.0f);

	bool RegistChars(cTaskChar* pChar);

	bool RegistPlayers(cTaskCharPlayer* pChar);

	bool RegistEnemys(cTaskCharEnemy* pChar);

	void PointsUpdate();

	void LandPoints();

	void StepItemSet();

	void StepMissDirection();

	void StepReady();

	void StepGame();

	void StepStartDirection();

	void StepClearDirection();

	void CheckMultiBattleWinnerOfRound();

	void CheckMultiBattleWinnerOfGame();

	void StepMultiBattleEnd();

	void StepChampion();

	void StepContinue();

	void StepSync();

	float GrobalID2HitPower(int nGrobalID, int nParts, int nOtherID, int refID = 0);

	float GrobalID2ChargePower(uint nGrobalID, int nParts, uint nOtherID, int refID = 0, int nCharge = 0);

	float Power2Heretic(float power);

	uint Weight2Heretic(float weight);

	int VSRestTimer();
};

}
