#include <algorithm>
#include <cmath>
#include "kingdom/game/g01/cObject.hpp"

namespace kingdom::game::g01
{
cObjectBase::~cObjectBase() {}

// DOL 0x801c1714 [audited]: +0x10 / +0x14 / +0x18 = 0, +0x20 = (0, 0, 0, 1), +0x30 = 0, +0x40 = ONE (member initialisers)
cObject::cObject() {}

cObject::~cObject() {}

cObjectAABB::~cObjectAABB() {}

void cObjectAABB::CalcAABB(void* pModel)
{
	(void)pModel;   // UNVERIFIED / NOT PORTED: see the header (natively SetLocalAABB with the model's vertex bounds)
}

void cObjectAABB::SetLocalAABB(const float mn[3], const float mx[3])
{
	for (int i = 0; i < 3; i++)
	{
		m_vMin[i] = m_vMin0[i] = mn[i];
		m_vMax[i] = m_vMax0[i] = mx[i];
	}
	m_bAABB = true;   // +0x80
}

// DOL 0x801c1950 [audited]: the 8 corners of the local box through M = T * Rz * Ry * Rx * S; the world box encloses them (nw4r::math::AABB::Set)
void cObjectAABB::UpdateWorldAABB()
{
	const float rx = m_v30.x, ry = m_v30.y, rz = m_v30.z;
	const float cx = std::cos(rx), sx = std::sin(rx), cy = std::cos(ry), sy = std::sin(ry), cz = std::cos(rz), sz = std::sin(rz);
	// R = Rz * Ry * Rx (column vectors)
	const float R[3][3] = {{cz * cy, cz * sy * sx - sz * cx, cz * sy * cx + sz * sx},
	                       {sz * cy, sz * sy * sx + cz * cx, sz * sy * cx - cz * sx},
	                       {-sy, cy * sx, cy * cx}};
	const float s[3] = {m_v40.x, m_v40.y, m_v40.z};
	const float t[3] = {m_v20.x, m_v20.y, m_v20.z};
	for (int i = 0; i < 3; i++)
	{
		m_vMin0[i] = 1e30f;
		m_vMax0[i] = -1e30f;
	}
	for (int c = 0; c < 8; c++)
	{
		const float p[3] = {(c & 1 ? m_vMax : m_vMin)[0] * s[0], (c & 2 ? m_vMax : m_vMin)[1] * s[1], (c & 4 ? m_vMax : m_vMin)[2] * s[2]};
		for (int i = 0; i < 3; i++)
		{
			const float w = R[i][0] * p[0] + R[i][1] * p[1] + R[i][2] * p[2] + t[i];
			m_vMin0[i] = std::min(m_vMin0[i], w);
			m_vMax0[i] = std::max(m_vMax0[i], w);
		}
	}
}

void cObjectAABB::UpdateWorldAABB(const float m[16])
{
	for (int i = 0; i < 3; i++)
	{
		m_vMin0[i] = 1e30f;
		m_vMax0[i] = -1e30f;
	}
	for (int c = 0; c < 8; c++)
	{
		const float p[3] = {(c & 1 ? m_vMax : m_vMin)[0], (c & 2 ? m_vMax : m_vMin)[1], (c & 4 ? m_vMax : m_vMin)[2]};
		for (int i = 0; i < 3; i++)
		{
			const float w = m[i] * p[0] + m[4 + i] * p[1] + m[8 + i] * p[2] + m[12 + i];
			m_vMin0[i] = std::min(m_vMin0[i], w);
			m_vMax0[i] = std::max(m_vMax0[i], w);
		}
	}
}
}
