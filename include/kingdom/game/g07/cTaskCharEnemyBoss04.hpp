#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskBoss04.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyBoss04 : public cTaskBoss04
{
private:
	bool m_bBoss04FirstInit{};

public:
	cTaskCharEnemyBoss04();

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
