#pragma once

#include "common.hpp"

namespace kingdom::game::g03
{

struct sSaveDataCameraArea
{
public:
	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	int vnTrans[3]{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	int vnRotate[3]{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	int vnScale[3]{};

	int nArea{};

	// [MarshalAs(UnmanagedType.ByValArray, SizeConst = 11)]
	int dummy[11]{};
};

}
