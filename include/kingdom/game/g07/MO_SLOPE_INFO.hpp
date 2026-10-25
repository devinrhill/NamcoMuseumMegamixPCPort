#pragma once

#include "common.hpp"
#include "kingdom/game/g07/MO_SLOPE_PAT.hpp"

namespace kingdom::game::g07
{

class MO_SLOPE_INFO
{
public:
	MO_SLOPE_PAT* m_Pat[5]{};

	int m_nPatNum{};

	int m_nTotalTime{};
};

}
