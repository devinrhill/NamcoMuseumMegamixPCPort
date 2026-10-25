#include "PLANE.hpp"

PLANE::PLANE(const Vector3& inNormal, const float& distance)
{
	This = Plane(inNormal, distance);
}
