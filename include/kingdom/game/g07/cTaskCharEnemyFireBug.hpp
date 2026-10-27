#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyFireBug : public cTaskCharEnemy
{
private:
	enum EMOModeEN
	{
		MO_MODE_EN_FIREBUG_NORMAL = 1,
		MO_MODE_EN_FIREBUG_EXMOVE = 2
	};

public:
	cTaskCharEnemyFireBug();

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
	void ReleaseTerm() override { cTaskCharEnemyFireBug::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
