#pragma once

#include "common.hpp"
#include "kingdom/game/g03/sSaveDataEnemy.hpp"
#include "kingdom/game/g03/sSaveDataEnemyPoint.hpp"

namespace kingdom::game::g03
{

struct sEnemyRouteData
{
public:
	int nRouteNum{};

	int nBombStatus{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	uint empty[2]{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
	sSaveDataEnemyPoint sEnemyRoute[32]{};

	sSaveDataEnemy sEnemy{};
};

}
