#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharGolvis.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskCharGolvis03 : public cTaskCharGolvis
{
protected:
	static constexpr float GOLVIS_X_MIN = -4.0f;

	static constexpr float GOLVIS_X_MAX = 4.0f;

	static constexpr float GOLVIS_Z_MIN = -21.0f;

	static constexpr float GOLVIS_Z_MAX = -14.0f;

	static const Constants::LAND_ATTACK_INFO* s_aKind00[5];

	static const Constants::LAND_ATTACK_INFO* s_aKind01[5];

	static const Constants::LAND_ATTACK_INFO* s_aKind02[5];

	static const Constants::LAND_ATTACK_INFO* s_aKind03[5];

	static const Constants::LAND_ATTACK_INFO* s_aKind04[6];

	static const Constants::LAND_ATTACK_INFO* s_aKind05[6];

	static const Constants::LAND_ATTACK_INFO* s_aKind06[6];

	static const Constants::LAND_ATTACK_INFO* const* s_aaAttackTBL[7];

	static const Constants::gmSGolvisAttack* s_AttackTBL[2];

	Constants::gmEMonsterStateGolvisLand m_iState{};

	Constants::gmEMonsterPlayMotionGolvisLand m_iPlayMotion{};

	static constexpr float MOVE_SPEED_DELTA = 0.0009f;

	static uint uiPlayMotionTable[5];

public:
	cTaskCharGolvis03(cJetTask* pParent);

private:
	void Init();

public:
	void Term() override;

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
	void ReleaseTerm() override { cTaskCharGolvis03::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
