#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskBoss03.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyBoss03 : public cTaskBoss03
{
public:
	cTaskCharEnemyBoss03();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	uint Input() override;

	bool Create(uint no) override;

	void UpdateMatrix() override;

protected:
	void ReleaseTerm() override { cTaskCharEnemyBoss03::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
