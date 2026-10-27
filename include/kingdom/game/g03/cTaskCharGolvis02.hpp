#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharGolvis.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cGolvisAIArea.hpp"

namespace kingdom::game::g03
{

class cTaskCharGolvis02 : public cTaskCharGolvis
{
protected:
	enum gmEMonsterStateGolvisArea
	{
		STATE_GOLVISAREA_RANDOM = 0,
		STATE_GOLVISAREA_PURSUIT = 1,
		STATE_GOLVISAREA_ESCAPE = 2,
		STATE_GOLVISAREA_JUMP = 3,
		STATE_GOLVISAREA_MAX = 4
	};

	static constexpr float MOVE_SPEED_DELTA = 0.01f;

	static constexpr int FINDPATH_FAILEDTIME_MAX = 300;

	static const Constants::gmSGolvisAttack* s_AttackTBL[4];

	cTaskCharGolvis02::gmEMonsterStateGolvisArea m_iState{};

	cGolvisAIArea* m_pAI{};

	int m_iCount_Area[3]{};

public:
	cTaskCharGolvis02(cJetTask* pParent);

private:
	void Init();

public:
	void Term() override;

protected:
	bool IsNearAttack();

	bool IsForceAttack() override;

	bool ChangeToAttack(int iAttackKind) override;

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void Setting(bool bRelocate) override;

	void InitStateFirst() override;

	void InitStateStart() override;

protected:
	void MonsterFrameMove_PreProcess() override;

	void MonsterFrameMove_Free() override;

	void MonsterFrameMove_Escape() override;

public:
	Constants::gmSGolvisAttack* GetAttackKind(uint nEatNum, uint nIjikeLevel, int nNum) override;

protected:
	void ReleaseTerm() override { cTaskCharGolvis02::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
