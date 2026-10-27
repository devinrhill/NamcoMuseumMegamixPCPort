#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyBeetle : public cTaskCharEnemy
{
public:
	cTaskCharEnemyBeetle();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;

	uint Input() override;

	bool ActionBoth() override;

	void SetMMSParam(int para1, int para2, int para3) override;

protected:
	void ReleaseTerm() override { cTaskCharEnemyBeetle::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
