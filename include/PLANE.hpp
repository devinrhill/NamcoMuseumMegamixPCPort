#pragma once

#include "common.hpp"

struct PLANE
{
	Plane This;

	PLANE() = default;

	PLANE(const Vector3& inNormal, const float& distance);
};
