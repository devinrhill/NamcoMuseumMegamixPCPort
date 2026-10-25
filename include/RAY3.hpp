#pragma once

#include "common.hpp"

struct RAY3
{
	Ray This;

	RAY3() = default;

	RAY3(Vector3 start, Vector3 dir);
};
