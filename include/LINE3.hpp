#pragma once

#include "common.hpp"

struct LINE3
{
	Ray This;

	Ray Reverse;

	LINE3() = default;

	LINE3(const Vector3& baseStart, Vector3 dir);
};
