#pragma once

#include "common.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "CullingStageModel.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/cTaskConsRetry.hpp"
#include "kingdom/game/g03/cFaceList.hpp"
#include "kingdom/game/g03/cGhostCommonParam.hpp"
#include "kingdom/game/g03/cGhostParam.hpp"
#include "kingdom/game/g03/cTaskBg.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "kingdom/game/g03/cTaskBgGimmickPistonMan.hpp"
#include "kingdom/game/g03/cTaskBgGimmickSwitch.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskBox.hpp"
#include "kingdom/game/g03/cTaskCamera.hpp"
#include "kingdom/game/g03/cTaskCameraSet.hpp"
#include "kingdom/game/g03/cTaskCharGolvis.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/cTaskCollision.hpp"
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

namespace kingdom::game::g03
{
class cTaskConsoleMan;
}

namespace kingdom::game::g03
{

class cAppG03PacnRollGame : public cAppKingdom
{
public:
	// set by the hub's job init (138) so that Init keeps the sys's top-menu flag
	static bool s_hubEntry;

	enum EProc
	{
		ePROC_START = 0,
		ePROC_PRE_GAME = 1,
		ePROC_GAME = 2,
		ePROC_MISS = 3,
		ePROC_GOAL = 4,
		ePROC_CLEAR = 5,
		ePROC_RETRY = 6,
		ePROC_GAME_OVER = 7,
		ePROC_EXIT = 8
	};

	class InitGame : public MonoBehaviour
	{
	};

private:
	static constexpr uint DEFAULT_TIME = 599999u;

	uint m_nRound{};

	uint m_nGameTimeCnt{};

	bool m_bStart{};

	bool m_bGame{};

	bool m_bRetry{};

	cTaskStage* m_pTaskStage{};

	cTaskCamera* m_pTaskCamera{};

	cTaskCollision* m_pTaskCollision{};

	cTaskWater* m_pTaskWater{};

	cTaskFallArea* m_pTaskFallArea{};

	cTaskGoal* m_pTaskGoal{};

	cTaskPoint* m_pTaskPoint{};

	cTaskGate* m_pTaskGate{};

	cTaskCharPacman* m_pTaskCharPacman{};

	cTaskItem* m_pTaskItem{};

	cTaskBox* m_pTaskBox{};

	cTaskPacFade* m_pTaskFade{};

	cTaskPause* m_pTaskPause{};

	cTaskConsoleMan* m_pTaskConsole{};

	cTaskConsRetry* m_pTaskConsRetry{};

	cTaskBgGimmickPistonMan* m_pTaskBgGimmickPistonMan{};

	cTaskBgGimmickSwitch* m_pTaskBgGimmickSwitch{};

	std::vector<cTaskBgGimmick*> m_vecTaskBgGimmick;

	cTaskCharGolvis* m_pTaskBoss{};

	cTaskGhostMan* m_pTaskGhostMan{};

	cGhostParam* m_pGhostParam{};

	cGhostCommonParam* m_pGhostCommonParam{};

	bool m_bClear{};

	uint m_nWaitCnt{};

	int m_nIjikeTime{};

	int m_nIjikeCount{};

	std::vector<cTaskKingdom*> m_pvecNakajimaCaliph;

	std::vector<cTaskBg*> m_pTaskBgList;

	std::vector<cTaskBgHit*> m_pTaskBgHitList;

	cTaskCameraSet* m_pTaskCameraSet{};

	std::unordered_map<std::string, cFaceList*> m_HitHash;

	GameObject* m_riverPosObj1{};

	GameObject* m_riverPosObj2{};

	GameObject* m_riverPosObj3{};

	std::vector<CullingStageModel*> m_cullingStageModelList;

public:
	cAppG03PacnRollGame::InitGame* m_initCoObj{};

	bool m_bInitCoPause{};

public:
	uint nRound();   // public for the dev window (the round this stage was built from; the sys's copy changes later)

public:
	cAppG03PacnRollGame();

private:
	IEnumerator* TaskCoroutine(IEnumerator* task);

public:
	void Free() override;

	uint nGameTime();

	uint nGameTimeCnt();

	cTaskStage* pTaskStage();

	cTaskCamera* pTaskCamera();

	cTaskCollision* pTaskCollision();

	cTaskCharPacman* pTaskCharPacman();

	cTaskItem* pTaskItem();

	cTaskGoal* pTaskGoal();

	cTaskPoint* pTaskPoint();

	cTaskPacFade* pTaskFade();

	cTaskPause* pTaskPause();

	cTaskConsoleMan* pTaskConsole();

	cTaskConsRetry* pTaskConsRetry();

	cTaskBgGimmickPistonMan* pTaskBgGimmickPistonMan();

	cTaskBgGimmickSwitch* pTaskBgGimmickSwitch();

	cTaskBgGimmick* pTaskBgGimmick(uint num = 0u);

	uint GetBgGimmickNum();

	cTaskWater* pTaskWater();

	cTaskFallArea* pTaskFallArea();

	cTaskCharGolvis* pTaskBoss();

	cTaskGhostMan* pTaskGhostMan();

	cGhostParam* pGhostParam();

	cGhostCommonParam* pGhostCommonParam();

	bool& IsClear();

	bool& IsStart();

	bool IsGame();

	bool IsIjikeTime();

	int IjikeGetCount();

	cTaskCameraSet* pTaskCameraSet();

	std::unordered_map<std::string, cFaceList*>& pHitHash();

	cFaceList* pHitHash(string key);

private:
	void Init();

public:
	void InitStage();

private:
	void WaitOneFramePlayBGM();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	bool IsBossStage();

private:
	void SaveData();

	void Dispose(string key);

	void SetRetryData();

	cTaskBgGimmick* SetGimmick(ResNode* node0, ResNode* node1);

public:
	void SetEffect(const uint& kind, const Vector4& trans, const Vector4& rotate = Vector4(), cKingdomPtkHnd* hnd = nullptr, float param = 0.0f);

private:
	IEnumerator* MoveNext(IEnumerator* task);

public:
	void IjikeRegist(int nTime = 0);

	bool IsNakajimaPause(cTaskKingdom* pChar);

	bool NakajimaPause(cTaskKingdom* pChar);

	bool NakajimaPlay(cTaskKingdom* pChar);

	uint EnemyGetNum();

	bool EnemyIsActive(uint no);

	uint EnemyGetActiveNum();

	bool EnemyActive(uint no);

	void StopIjikeSnd();
};

}
