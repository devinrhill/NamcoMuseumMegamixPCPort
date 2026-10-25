#pragma once

#include "common.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom
{

class Constants
{
public:
	static constexpr int WPAD_MAX_CONTROLLERS = 4;

	static void Kingdom_System_Init();

	static void Kingdom_System_Term();

	static cKingdomSys* Kingdom_System();

	static void Kingdom_SetJobNo(uint job, bool save = false, bool load = false);

	static bool IS_PAUSE();
};

}
