#pragma once

#include "common.hpp"

namespace kingdom::game::g03
{

struct sSaveDataFallArea
{
public:
	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	int vnTrans[3]{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	int vnRotate[3]{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	int vnScale[3]{};

	uint nAreaCheckKind{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
	uint _pad[6]{};
};

}
