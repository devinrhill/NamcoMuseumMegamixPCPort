#include "SEGMENT3.hpp"

SEGMENT3::SEGMENT3(Vector3 start, const Vector3& end)
{
	Vector3 normalized = (end - start).normalized();
	start -= normalized * 0.0001f;
	StartPoint = start;
	EndPoint = end;
	This = Ray(start, normalized);
}
