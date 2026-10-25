#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemyPupaRed.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyPupaBlue : public cTaskCharEnemyPupaRed
{
public:
	cTaskCharEnemyPupaBlue();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;
};

}
