#pragma once

#include "common.hpp"
#include "kingdom/game/g07/MO_SLOPE_PAT.hpp"

namespace kingdom::game::g07
{

class MO_SLOPE_INFO
{
public:
	// DOL: five inline 0x28-byte records at cTaskPanelMan +0x710 (zeroed by the ctor 0x801fdcd4); a stage that names an unknown pattern keeps a zero record
	MO_SLOPE_PAT* m_Pat[5]{new MO_SLOPE_PAT(), new MO_SLOPE_PAT(), new MO_SLOPE_PAT(), new MO_SLOPE_PAT(), new MO_SLOPE_PAT()};

	int m_nPatNum{};

	int m_nTotalTime{};
};

}
