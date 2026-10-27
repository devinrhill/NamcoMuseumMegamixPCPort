#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemyPupaRed.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyPupaBlack : public cTaskCharEnemyPupaRed
{
public:
	cTaskCharEnemyPupaBlack();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;

protected:
	void ReleaseTerm() override { cTaskCharEnemyPupaBlack::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
