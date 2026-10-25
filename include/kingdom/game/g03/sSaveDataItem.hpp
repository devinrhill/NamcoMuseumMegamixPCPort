#pragma once

#include "common.hpp"

namespace kingdom::game::g03
{

struct sSaveDataItem
{
public:
	uint kind{};

	uint flag{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	uint empty[2]{};

	Vector4 trans{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	uint dummy[4]{};
};

}
