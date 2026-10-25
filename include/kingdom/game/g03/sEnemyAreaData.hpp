#pragma once

#include "common.hpp"
#include "kingdom/game/g03/sSaveDataArea.hpp"
#include "kingdom/game/g03/sSaveDataEnemy.hpp"
#include "kingdom/game/g03/sSaveDataMovePoint.hpp"

namespace kingdom::game::g03
{

struct sEnemyAreaData
{
public:
	int nAreaNum{};

	int nPointNum{};

	int nEnmeyNum{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1)]
	uint empty[1]{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 48)]
	sSaveDataArea Area[48]{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
	sSaveDataMovePoint Point[64]{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
	sSaveDataEnemy Enemy[8]{};
};

}
