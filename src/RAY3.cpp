#include "RAY3.hpp"

RAY3::RAY3(Vector3 start, Vector3 dir)
{
	dir = dir.normalized();
	start -= dir * 0.0001f;
	This = Ray(start, dir);
}
