#include "LINE3.hpp"

LINE3::LINE3(const Vector3& baseStart, Vector3 dir)
{
	dir = dir.normalized();
	This = Ray(baseStart, dir);
	Reverse = Ray(baseStart, -dir);
}
