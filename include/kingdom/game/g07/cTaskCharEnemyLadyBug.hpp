#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyLadyBug : public cTaskCharEnemy
{
private:
	enum EMOModeEN
	{
		MO_MODE_EN_LADYBUG_NORMAL = 1,
		MO_MODE_EN_LADYBUG_HERETIC = 2,
		MO_MODE_EN_LADYBUG_EXMOVE = 3
	};

	bool m_bHit{};

public:
	cTaskCharEnemyLadyBug();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;

	uint Input() override;

	void SetMMSParam(int para1, int para2, int para3) override;
};

}
