#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyBeecon : public cTaskCharEnemy
{
public:
	cTaskCharEnemyBeecon();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	void FallEffect(int nRefID) override;

	bool Create(uint no) override;
};

}
