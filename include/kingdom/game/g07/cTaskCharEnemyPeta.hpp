#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyPeta : public cTaskCharEnemy
{
private:
	Matrix4x4 m_mRoll{};

	ushort m_nRotDir{};

	bool m_bStop{};

public:
	cTaskCharEnemyPeta();

	static cTaskCharEnemy* Construct();

	bool& bStop();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;

	uint Input() override;

	bool FallMove() override;

	bool ActionBoth() override;

	void SetMMSParam(int para1, int para2, int para3) override;
};

}
