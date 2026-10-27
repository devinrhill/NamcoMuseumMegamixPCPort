#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyBall : public cTaskCharEnemy
{
private:
	uint m_BallType{};

public:
	cTaskCharEnemyBall();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	void FallEffect(int nRefID) override;

	bool Create(uint no) override;

	uint Input() override;

protected:
	void ReleaseTerm() override { cTaskCharEnemyBall::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
