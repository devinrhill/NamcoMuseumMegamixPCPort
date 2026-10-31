#include "RAY3.hpp"

RAY3::RAY3(Vector3 start, Vector3 dir)
{
	dir = dir.normalized();
	This = Ray(start, dir);
}
