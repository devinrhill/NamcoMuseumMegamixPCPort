#pragma once

#include "common.hpp"

namespace jet::ptk
{

struct datScn
{
	byte name[256];

	byte path_relative[256];

	uint flag;

	uint dmy00[3];

	int scn_max;

	int emi_max;

	int gen_max;

	uint dmy01[1];

	int path_type;

	uint dmy02[3];

	int bg_tex_filter;

	uint dmy03[3];

	byte bg_tex_name[256];

	float bg_r;

	float bg_g;

	float bg_b;

	float bg_a;

	float gakaku;

	uint dmy04[3];

	Vector4 ui_si_pos;

	Vector4 ui_si_itr;

	Vector4 ui_si_rot;

	float loop_frame;

	float draw_scale;

	uint dmy05[2];

	float fps_draw;

	uint dmy08[3];

	float air_power;

	uint dmy06[3];

	float air_dir_x;

	float air_dir_y;

	uint dmy07[2];

	Vector4 lit_pos;

	Vector4 lit_itr;

	float lit_r;

	float lit_g;

	float lit_b;

	float lit_a;
};

}
