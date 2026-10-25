#pragma once

#include "common.hpp"

namespace kingdom::game::g03
{

struct sSaveDataEnemyPoint
{
public:
	int speed{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	uint empty[3]{};

	Vector4 pos{};
};

}
