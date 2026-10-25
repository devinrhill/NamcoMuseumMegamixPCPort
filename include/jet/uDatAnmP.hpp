#pragma once

#include "common.hpp"
#include "jet/sDatAnmP.hpp"

namespace jet
{

union uDatAnmP
{
	sDatAnmP anm;

	byte dmy[32];
};

}
