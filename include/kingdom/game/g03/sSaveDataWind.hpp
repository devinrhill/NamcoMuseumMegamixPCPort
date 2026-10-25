#pragma once

#include "common.hpp"

namespace kingdom::game::g03
{

struct sSaveDataWind
{
public:
	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	int vnTrans[3]{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	int vnRotate[3]{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	int vnScale[3]{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	int vnRotWind[3]{};

	int nForce{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	uint _pad[3]{};
};

}
