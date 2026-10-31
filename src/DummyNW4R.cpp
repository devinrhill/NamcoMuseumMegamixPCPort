#include "DummyNW4R.hpp"
#include "kingdom/cKingdomModel.hpp"

#include <cmath>
#include <utility>

// native: the GameObject is the stand-in of a kingdom::cKingdomModel (see cKingdomModel::pScnMdl). `convert` (Wii -> Unity mirror) does not apply: the
// native scene is in Wii space.
void DummyNW4R::SetMtx(GameObject* self, const Matrix4x4& matrix, bool convert)
{
	if (self != nullptr && self->native != nullptr) static_cast<kingdom::cKingdomModelBase*>(static_cast<kingdom::cKingdomModel*>(self->native))->SetNativeMtx(matrix);
}

void DummyNW4R::GetMtx(GameObject* self, Matrix4x4& outMatrix)
{
	outMatrix = Matrix4x4::identity();
	if (self != nullptr && self->native != nullptr) static_cast<kingdom::cKingdomModel*>(self->native)->GetNativeMtx(outMatrix);
}

void DummyNW4R::SetScale(GameObject* self, Vector4 scale)
{
	if (self != nullptr && self->native != nullptr) static_cast<kingdom::cKingdomModel*>(self->native)->SetNativeScale(scale.x, scale.y, scale.z);
}

void DummyNW4R::SetScale(GameObject* self, float x, float y, float z)
{
	if (self != nullptr && self->native != nullptr) static_cast<kingdom::cKingdomModel*>(self->native)->SetNativeScale(x, y, z);
}

// nw4r::math (NintendoWare math_geometry.cpp, the functions main.dol calls); the C#'s Unity Plane / Bounds stand-ins are gone
namespace
{
constexpr float kEpsilon = 0.0001f;    // detail::epsilon
constexpr float kInfinity = 1.0e8f;    // detail::infinity

float Dot(const Vector3& a, const Vector3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

// true = INTERSECTION_1 (INTERSECTION_LINE3_ON_PLANE is not 1: every caller in main.dol treats it as no hit, or cannot reach it)
bool LinePlane(const Vector3& p, const Vector3& d, const Plane& plane, float& t)
{
	const float denom = Dot(d, plane.normal);
	if (std::fabs(denom) < kEpsilon) return false;
	t = -(Dot(plane.normal, p) + plane.distance) / denom;
	return true;
}

bool RayAABBAxis(float min, float max, float o, float dir, float& tNear, float& tFar)
{
	if (std::fabs(dir) < kEpsilon) return !(o < min || o > max);
	const float invDir = 1.0f / dir;
	float t0 = (min - o) * invDir;
	float t1 = (max - o) * invDir;
	if (t0 > t1) std::swap(t0, t1);
	if (t0 > tNear) tNear = t0;
	if (t1 < tFar) tFar = t1;
	if (tNear > tFar) return false;
	return !(tFar < 0.0f);
}
}

bool DummyNW4R::IntersectionSphereAABB(const SPHERE& sphere, const AABB& aabb)
{
	const Vector3 c = sphere.Bound.center;
	const float r = sphere.Bound.size.x * 0.5f;
	const Vector3 lo = aabb.This.min(), hi = aabb.This.max();
	float distSq = 0.0f;
	if (c.x < lo.x) distSq += (c.x - lo.x) * (c.x - lo.x);
	else if (c.x > hi.x) distSq += (c.x - hi.x) * (c.x - hi.x);
	if (c.y < lo.y) distSq += (c.y - lo.y) * (c.y - lo.y);
	else if (c.y > hi.y) distSq += (c.y - hi.y) * (c.y - hi.y);
	if (c.z < lo.z) distSq += (c.z - lo.z) * (c.z - lo.z);
	else if (c.z > hi.z) distSq += (c.z - hi.z) * (c.z - hi.z);
	return distSq <= r * r;
}

bool DummyNW4R::IntersectionRay3AABB(const RAY3& ray3, const AABB& aabb, float& outLength)
{
	float tNear = -kInfinity, tFar = kInfinity;
	const Vector3 lo = aabb.This.min(), hi = aabb.This.max();
	const Vector3 o = ray3.This.origin, d = ray3.This.direction;
	if (!RayAABBAxis(lo.x, hi.x, o.x, d.x, tNear, tFar)) return false;
	if (!RayAABBAxis(lo.y, hi.y, o.y, d.y, tNear, tFar)) return false;
	if (!RayAABBAxis(lo.z, hi.z, o.z, d.z, tNear, tFar)) return false;
	outLength = tNear > 0.0f ? tNear : tFar;
	return true;
}

bool DummyNW4R::IntersectionSegment3Plane(const SEGMENT3& Segment3, const PLANE& plane, float& outDist, Vector3& outCross)
{
	outCross = Vector3();
	const Vector3 dir = Segment3.EndPoint - Segment3.StartPoint;
	const float len = std::sqrt(Dot(dir, dir));
	float t;
	if (!LinePlane(Segment3.StartPoint, Segment3.This.direction, plane.This, t)) return false;
	if (!(t >= 0.0f && t <= len)) return false;
	outDist = t / len;
	outCross = Segment3.This.GetPoint(t);
	return true;
}

bool DummyNW4R::IntersectionRay3Plane(const RAY3& ray3, const PLANE& plane, float& outDist, Vector3& outCross)
{
	outCross = Vector3();
	float t;
	if (!LinePlane(ray3.This.origin, ray3.This.direction, plane.This, t) || !(t >= 0.0f)) return false;
	outDist = t;
	outCross = ray3.This.GetPoint(t);
	return true;
}

bool DummyNW4R::IntersectionLine3Plane(const LINE3& line3, const PLANE& plane, float& outDist, Vector3& outCross)
{
	outCross = Vector3();
	float t;
	if (!LinePlane(line3.This.origin, line3.This.direction, plane.This, t)) return false;
	outDist = t;
	outCross = line3.This.GetPoint(t);
	return true;
}

float DummyNW4R::DistSqPoint3ToAABB(const Vector3& trans, const AABB& aabb, float& val)
{
	val = 0.0f;
	return aabb.This.SqrDistance(trans);
}

bool DummyNW4R::Intersection(const Vector3& minA, const Vector3& maxA, const Vector3& maxB, const Bounds& boundB)
{
	if (minA.x <= maxB.x && minA.y <= maxB.y && minA.z <= maxB.z && maxA.x >= boundB.min().x && maxA.y >= boundB.min().y)
	{
		return maxA.z >= boundB.min().z;
	}
	return false;
}
