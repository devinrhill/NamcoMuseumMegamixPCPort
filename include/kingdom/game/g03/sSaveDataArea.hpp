#pragma once

#include "common.hpp"

namespace kingdom::game::g03
{

struct sSaveDataArea
{
public:
	uint kind{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	uint empty[3]{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	Vector4 pos[4]{};
};

}
