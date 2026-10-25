#pragma once

#include "common.hpp"
#include "jet/sDatLayP.hpp"

namespace jet
{

union uDatLayP
{
	sDatLayP lay;

	byte dmy[96];
};

}
