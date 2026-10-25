#pragma once

#include "common.hpp"

namespace kingdom::game::g03
{

struct sSaveDataEnemy
{
public:
	Vector4 pos{};

	int type{};

	int option{};

	int ride{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1)]
	uint empty[1]{};
};

}
