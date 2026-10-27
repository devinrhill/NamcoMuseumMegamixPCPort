#pragma once

#include "common.hpp"
#include "DObjParam.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g07/MAP_HIT_RESULT.hpp"
#include "kingdom/game/g07/cMotosMapoo.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include "kingdom/game/g07/cTaskCharItem.hpp"
#include "kingdom/game/g07/cTaskGimmickBase.hpp"
#include "kingdom/game/g07/cTaskPanelMan.hpp"

namespace kingdom::game::g07
{

class cTaskMap : public cTaskKingdom
{
public:
	using ENEMY_TASK_KIND_TBL = std::function<cTaskCharEnemy*()>;

	using ITEM_TASK_KIND_TBL = std::function<cTaskCharItem*()>;

	using GIMMICK_TASK_KIND_TBL = std::function<cTaskGimmickBase*()>;

	static const cTaskMap::ENEMY_TASK_KIND_TBL s_enemy_task_tbl[27];

	static const cTaskMap::ITEM_TASK_KIND_TBL s_item_task_tbl[4];

	static const cTaskMap::GIMMICK_TASK_KIND_TBL s_gimmick_task_tbl[4];

private:
	cTaskPanelMan* m_pTaskPanelMan{};

	cTaskGimmickBase* m_apGimmicks[256]{};

	cTaskGimmickBase* m_pGimmicks[32]{};

	cTaskCharEnemy* m_pStackEnemy[16]{};

	uint m_nFormationNum{};

public:
	cMotosMapoo* Mapoo{};

	cTaskMap();

	cTaskCharEnemy*& ppStackEnemy(uint set);

	cTaskPanelMan* pTaskPanelMan();

	MAP_HIT_RESULT* HitResult(int n);

	cTaskGimmickBase* GetGimmickArray(uint num);

	cTaskGimmickBase* GetGimmickArray(uint x, uint z);

	void SetGimmickArray(uint num, cTaskGimmickBase* p);

	cTaskGimmickBase* pGimmicks(uint num);

	void pGimmicks(uint num, cTaskGimmickBase* p);

private:
	void Init();

public:
	void Term();

private:
	void SetupPlayer();

	static bool SetupPlayerCB(DObjParam::sObj* obj, cJetTask* args);

	void SetupEnemy();

	static bool SetupEnemyFormCB(DObjParam::sObj* obj, cJetTask* args);

	void SetFormation();

	static bool SetupEnemyCB(DObjParam::sObj* obj, cJetTask* args);

	void SetupItem();

	static bool SetupItemCB(DObjParam::sObj* obj, cJetTask* args);

	void SetupGimmick();

	static bool SetupGimmickCB(DObjParam::sObj* obj, cJetTask* args);

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void Map2World(Vector4& vWorld, const Vector4& vMap);

	void World2Map(Vector4& vMap, const Vector4& vWorld);

	uint nWidth();

	uint nDepth();

	float fPanelSize();

	int HitCheck(Vector4 pPos, Vector4 pVec, float fRange);

	cTaskGimmickBase* SearchGimmick(float mx, float mz);

	cTaskGimmickBase* SearchGimmickNo(int x, int z);

	cTaskGimmickBase* StepOnCnt(cTaskChar* pChar);

	cTaskGimmickBase* StepOnCntM(Vector4 mtrans);

	cTaskGimmickBase* StepOnCntW(Vector4 wtrans);

protected:
	void ReleaseTerm() override { cTaskMap::Term(); cTaskKingdom::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
