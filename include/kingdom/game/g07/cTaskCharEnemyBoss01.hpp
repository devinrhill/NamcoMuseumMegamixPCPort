#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskBoss01.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyBoss01 : public cTaskBoss01
{
public:
	cTaskCharEnemyBoss01();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	uint Input() override;

	bool Create(uint no) override;

	void UpdateMatrix() override;

protected:
	void ReleaseTerm() override { cTaskCharEnemyBoss01::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
