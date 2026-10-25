#pragma once

#include "common.hpp"
#include "jet/sDatSeqP.hpp"

namespace jet
{

union uDatSeqP
{
	sDatSeqP seq;

	byte dmy[16];
};

}
