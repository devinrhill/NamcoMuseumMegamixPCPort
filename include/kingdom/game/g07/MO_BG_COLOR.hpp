#pragma once

#include "common.hpp"

namespace kingdom::game::g07
{

struct MO_BG_COLOR
{
public:
	// [FieldOffset(0)]
	byte aCol_0{};

	// [FieldOffset(1)]
	byte aCol_1{};

	// [FieldOffset(2)]
	byte aCol_2{};

	// [FieldOffset(3)]
	byte aCol_3{};

	// [FieldOffset(0)]
	uint nRgba{};
};

}
