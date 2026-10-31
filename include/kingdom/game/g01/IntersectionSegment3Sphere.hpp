#pragma once

#include <cmath>

// nw4r::math::IntersectionSegment3Sphere (DOL 0x80302150, math_geometry.cpp): does the segment a -> b cross the sphere's SURFACE? The line
// through a along (b - a) is intersected with the sphere; a segment that lies completely inside the sphere (or misses it) does not count.
// Returns true for INTERSECTION_1 / INTERSECTION_2.
namespace kingdom::game::g01
{
inline bool IntersectionSegment3Sphere(const float a[3], const float b[3], const float c[3], float r)
{
	float dir[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
	const float len = std::sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);   // P0P1
	const float l = len;
	for (float& v : dir) v = l > 0.0f ? v / l : v;   // LINE3 normalises its direction
	const float pmc[3] = {a[0] - c[0], a[1] - c[1], a[2] - c[2]};
	const float bq = 2.0f * (dir[0] * pmc[0] + dir[1] * pmc[1] + dir[2] * pmc[2]);
	const float cq = pmc[0] * pmc[0] + pmc[1] * pmc[1] + pmc[2] * pmc[2] - r * r;
	const float discrm = bq * bq - 4.0f * cq;
	if (discrm > 0.0f)
	{
		const float sq = std::sqrt(discrm);
		const float t0 = (-bq - sq) * 0.5f, t1 = (-bq + sq) * 0.5f;
		if (t0 >= 0.0f)
			return t1 <= len || t0 <= len;
		return t1 >= 0.0f && t1 <= len;
	}
	if (discrm == 0.0f)
	{
		const float t0 = -0.5f * bq;
		return t0 >= 0.0f && t0 <= len;
	}
	return false;
}
}
