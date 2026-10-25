#pragma once

#include "common.hpp"
#include "AABB.hpp"
#include "LINE3.hpp"
#include "PLANE.hpp"
#include "RAY3.hpp"
#include "SEGMENT3.hpp"
#include "SPHERE.hpp"

class DummyNW4R
{
public:
	static void SetMtx(GameObject* self, const Matrix4x4& matrix, bool convert = true);

	static void GetMtx(GameObject* self, Matrix4x4& outMatrix);

	static void SetScale(GameObject* self, Vector4 scale);

	static void SetScale(GameObject* self, float x, float y, float z);

	static bool IntersectionSphereAABB(const SPHERE& sphere, const AABB& aabb);

	static bool IntersectionRay3AABB(const RAY3& ray3, const AABB& aabb, float& outLength);

	static bool IntersectionSegment3Plane(const SEGMENT3& Segment3, const PLANE& plane, float& outDist, Vector3& outCross);

	static bool IntersectionRay3Plane(const RAY3& ray3, const PLANE& plane, float& outDist, Vector3& outCross);

	static bool IntersectionLine3Plane(const LINE3& line3, const PLANE& plane, float& outDist, Vector3& outCross);

	static float DistSqPoint3ToAABB(const Vector3& trans, const AABB& aabb, float& val);

	static bool Intersection(const Vector3& minA, const Vector3& maxA, const Vector3& maxB, const Bounds& boundB);
};
