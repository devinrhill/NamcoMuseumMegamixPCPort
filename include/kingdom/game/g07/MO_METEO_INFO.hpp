#pragma once

#include "common.hpp"

namespace kingdom::game::g07
{

class MO_METEO_INFO
{
public:
	int m_nLimit{};

	int m_nInterval_Min{};

	int m_nInterval_Max{};

	int m_nStep{};

	Vector4 m_vShootPos[4]{};
};

}
