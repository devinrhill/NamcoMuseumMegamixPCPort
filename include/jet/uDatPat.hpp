#pragma once

#include "common.hpp"
#include "jet/sDatPat.hpp"

namespace jet
{

union uDatPat
{
	sDatPat pat;

	byte dmy[16];
};

}
