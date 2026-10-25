#pragma once

#include "common.hpp"

struct AABB
{
private:
	static Vector3 m_tmpPos[8];

public:
	Bounds This;

	bool IsSet;

	AABB() = default;

	AABB(const Bounds& bounds);

	AABB(const Vector3& min, const Vector3& max, bool val);

	void Set(const AABB& target, const Matrix4x4& matrix);
};
