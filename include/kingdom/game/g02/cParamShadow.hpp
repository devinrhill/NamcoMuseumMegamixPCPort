#pragma once

#include "common.hpp"

// Rally-X Remix (g02) car shadow parameters: kingdom::game::g02::cParamShadow (CSV_SHADOW 0xaf402e71, 0x40 bytes, vtable 0x80393efc; the
// slot named after the class, 0x8007c774, is the deleting destructor). Read inline by the car models FUN_8008120c into fixed members, then
// +0x20..+0x2c is copied to the shadow (+0x2a0) as the light vector.
namespace kingdom::game::g02
{
struct cParamShadow
{
	float vLight[4]{0.0f, 0.0f, 0.0f, 1.0f};   // +0x20 rows 0 / 1 / 2 * pi / 180 (FLOAT_8047aa24), w 1 (FLOAT_8047aa2c)
	float fPosY{};                              // +0x30 row 3 (added to the car's height)
	float fScaleY{};                            // +0x34 row 4 (the shadow matrix's [1][1])

	bool Load();                                // 0x8008120c (the CSV_SHADOW part)
};
}
