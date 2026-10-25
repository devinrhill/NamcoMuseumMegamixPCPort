#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyTaitorian : public cTaskCharEnemy
{
public:
	cTaskCharEnemyTaitorian();

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
