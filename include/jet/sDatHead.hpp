#pragma once

#include "common.hpp"

namespace jet
{

struct sDatHead
{
	byte ver[8];

	int tex_max;

	int pat_max;

	int lay_max;

	int anm_max;

	int seq_max;

	int tex_offset;

	int pat_offset;

	int lay_offset;

	int anm_offset;

	int seq_offset;
};

}
