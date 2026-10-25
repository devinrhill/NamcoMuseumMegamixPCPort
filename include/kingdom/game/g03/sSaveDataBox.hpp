#pragma once

#include "common.hpp"

namespace kingdom::game::g03
{

struct sSaveDataBox
{
public:
	uint kind{};

	uint treasure{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	uint empty[2]{};

	Vector4 trans{};

	int rotY{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	uint dummy[3]{};
};

}
