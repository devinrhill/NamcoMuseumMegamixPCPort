#pragma once

#include "common.hpp"

namespace kingdom::game::g03
{

struct sSaveDataStart
{
public:
	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	uint empty[3]{};

	bool isDrop{};

	Vector4 trans{};

	int rotY{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	uint dummy[2]{};
};

}
