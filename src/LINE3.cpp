#include "LINE3.hpp"

LINE3::LINE3(const Vector3& baseStart, Vector3 dir)
{
	dir = dir.normalized();
	Vector3 origin = baseStart - dir * 0.0001f;
	This = Ray(origin, dir);
	dir = -dir;
	origin = baseStart - dir * 0.0001f;
	Reverse = Ray(origin, dir);
}
