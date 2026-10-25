#pragma once

#include "common.hpp"

namespace kingdom::game::g03
{

struct sSaveDataGate
{
public:
	uint cookie{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	uint empty[3]{};

	Vector4 trans{};

	int rotY{};

	uint kind{};

	uint no{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1)]
	uint dummy[1]{};
};

}
