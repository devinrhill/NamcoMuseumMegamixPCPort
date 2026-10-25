#pragma once

#include "common.hpp"

namespace kingdom::game::g07
{

class NET_RF_PACKET
{
public:
	byte m_nRefID{};

	byte m_nAxis_Status{};

	byte m_nParts{};

	byte m_nCounter{};

	byte m_nChar[2]{};
};

}
