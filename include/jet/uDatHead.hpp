#pragma once

#include "common.hpp"
#include "jet/sDatHead.hpp"

namespace jet
{

union uDatHead
{
	sDatHead head;

	byte dmy[48];
};

}
