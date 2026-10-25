#include "AABB.hpp"

Vector3 AABB::m_tmpPos[8] = {};

AABB::AABB(const Bounds& bounds)
{
	This = bounds;
	IsSet = true;
}

AABB::AABB(const Vector3& min, const Vector3& max, bool val)
{
	Bounds bounds = Bounds();
	bounds.SetMinMax(min, max);
	This = bounds;
	IsSet = true;
}

void AABB::Set(const AABB& target, const Matrix4x4& matrix)
{
	m_tmpPos[0] = target.This.min();
	m_tmpPos[1] = target.This.max();
	m_tmpPos[2].x = m_tmpPos[1].x;
	m_tmpPos[2].y = m_tmpPos[0].y;
	m_tmpPos[2].z = m_tmpPos[0].z;
	m_tmpPos[3].x = m_tmpPos[1].x;
	m_tmpPos[3].y = m_tmpPos[1].y;
	m_tmpPos[3].z = m_tmpPos[0].z;
	m_tmpPos[4].x = m_tmpPos[0].x;
	m_tmpPos[4].y = m_tmpPos[1].y;
	m_tmpPos[4].z = m_tmpPos[0].z;
	m_tmpPos[5].x = m_tmpPos[0].x;
	m_tmpPos[5].y = m_tmpPos[1].y;
	m_tmpPos[5].z = m_tmpPos[1].z;
	m_tmpPos[6].x = m_tmpPos[0].x;
	m_tmpPos[6].y = m_tmpPos[0].y;
	m_tmpPos[6].z = m_tmpPos[1].z;
	m_tmpPos[7].x = m_tmpPos[1].x;
	m_tmpPos[7].y = m_tmpPos[0].y;
	m_tmpPos[7].z = m_tmpPos[1].z;
	Bounds bounds = GeometryUtility::CalculateBounds(m_tmpPos, matrix);
	This = bounds;
}
