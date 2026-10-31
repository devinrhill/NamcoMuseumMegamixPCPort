#include "SEGMENT3.hpp"

SEGMENT3::SEGMENT3(Vector3 start, const Vector3& end)
{
	Vector3 normalized = (end - start).normalized();
	StartPoint = start;
	EndPoint = end;
	This = Ray(start, normalized);
}
