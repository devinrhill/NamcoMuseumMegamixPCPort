#pragma once

#include "common.hpp"
#include "jet/sDatTex.hpp"

namespace jet
{

union uDatTex
{
	sDatTex tex;

	byte dmy[16];
};

}
