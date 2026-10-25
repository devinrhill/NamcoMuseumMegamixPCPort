#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskBoss02.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyBoss02 : public cTaskBoss02
{
public:
	cTaskCharEnemyBoss02();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	uint Input() override;

	bool Create(uint no) override;

	void UpdateMatrix() override;
};

}
