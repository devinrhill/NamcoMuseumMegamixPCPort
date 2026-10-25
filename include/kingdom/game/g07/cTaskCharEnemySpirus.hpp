#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemySpirus : public cTaskCharEnemy
{
private:
	enum EMOModeEN
	{
		MO_MODE_EN_SPIRUS_NORMAL = 1,
		MO_MODE_EN_SPIRUS_EXMOVE = 2
	};

	ushort m_nExDir{};

	bool m_bHit{};

public:
	cTaskCharEnemySpirus();

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
