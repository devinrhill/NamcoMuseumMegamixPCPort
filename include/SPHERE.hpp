#pragma once

#include "common.hpp"

struct SPHERE
{
	Bounds Bound;

	SPHERE() = default;

	SPHERE(const Vector3& center, const float& radius);
};
