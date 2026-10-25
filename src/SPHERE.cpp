#include "SPHERE.hpp"


SPHERE::SPHERE(const Vector3& center, const float& radius)
{
	// Unity: the shared SphereCollider's bounds = the axis aligned box around the sphere
	Bound = Bounds(center, Vector3(radius * 2.0f, radius * 2.0f, radius * 2.0f));
}
