#pragma once

#include "common.hpp"

struct SEGMENT3
{
	Ray This;

	Vector3 StartPoint;

	Vector3 EndPoint;

	SEGMENT3() = default;

	SEGMENT3(Vector3 start, const Vector3& end);
};
