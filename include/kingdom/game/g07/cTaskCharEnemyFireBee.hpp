#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyFireBee : public cTaskCharEnemy
{
private:
	enum EMOModeEN
	{
		MO_MODE_EN_FIREBEE_NORMAL = 1,
		MO_MODE_EN_FIREBEE_STOP = 2,
		MO_MODE_EN_FIREBEE_SETMOVE = 3,
		MO_MODE_EN_FIREBEE_FALLPASS = 4,
		MO_MODE_EN_FIREBEE_HERETIC = 5
	};

public:
	cTaskCharEnemyFireBee();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;

	uint Input() override;

	bool ActionBoth() override;

	void SetMMSParam(int para1, int para2, int para3) override;
};

}
