#pragma once

#include "common.hpp"

namespace kingdom::game::g07
{

class cMotosGetTime
{
private:
	long m_nNowTime{};

	long m_nExecTime{};

public:
	uint GetTime();

	uint GetMilliSeconds();
};

}
