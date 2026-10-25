#pragma once

#include "common.hpp"

namespace kingdom::game::g03
{

struct sSaveDataGoal
{
public:
	Vector4 trans{};

	int rotY{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	uint dummy[3]{};
};

}
