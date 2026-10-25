#pragma once

#include "common.hpp"

namespace kingdom::game::g03
{

class AREA_POINT
{
public:
	Vector4 m_vPos{};

	float m_fRangeSQ{};

	sbyte m_nNeighbor[4]{};
};

}
