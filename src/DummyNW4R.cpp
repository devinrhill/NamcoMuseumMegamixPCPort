#include "DummyNW4R.hpp"
#include "kingdom/cKingdomModel.hpp"

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

bool DummyNW4R::IntersectionSphereAABB(const SPHERE& sphere, const AABB& aabb)
{
	return aabb.This.Intersects(sphere.Bound);
}

bool DummyNW4R::IntersectionRay3AABB(const RAY3& ray3, const AABB& aabb, float& outLength)
{
	return aabb.This.IntersectRay(ray3.This, outLength);
}

bool DummyNW4R::IntersectionSegment3Plane(const SEGMENT3& Segment3, const PLANE& plane, float& outDist, Vector3& outCross)
{
	outCross = Vector3();
	bool flag = plane.This.Raycast(Segment3.This, outDist);
	if (flag)
	{
		outCross = Segment3.This.GetPoint(outDist);
		Vector3 vector = Segment3.StartPoint - Segment3.EndPoint;
		Vector3 vector2 = Segment3.StartPoint - outCross;
		if (vector.sqrMagnitude() < vector2.sqrMagnitude())
		{
			flag = false;
			outDist = 0.0f;
			outCross = Vector3::zero;
		}
	}
	return flag;
}

bool DummyNW4R::IntersectionRay3Plane(const RAY3& ray3, const PLANE& plane, float& outDist, Vector3& outCross)
{
	outCross = Vector3();
	bool num = plane.This.Raycast(ray3.This, outDist);
	if (num)
	{
		outCross = ray3.This.GetPoint(outDist);
	}
	return num;
}

bool DummyNW4R::IntersectionLine3Plane(const LINE3& line3, const PLANE& plane, float& outDist, Vector3& outCross)
{
	outCross = Vector3();
	bool flag = plane.This.Raycast(line3.This, outDist);
	if (flag)
	{
		outCross = line3.This.GetPoint(outDist);
	}
	else
	{
		flag = plane.This.Raycast(line3.Reverse, outDist);
		if (flag)
		{
			outCross = line3.Reverse.GetPoint(outDist);
		}
	}
	return flag;
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
