#pragma once

#include "common.hpp"

namespace jet::ptk
{

struct datGen
{
	byte name[256];

	byte tex[256];

	uint gen_flag;

	uint ptc_flag;

	uint tex_flag;

	uint dmy05[1];

	float gen_max;

	float gen_max_frm;

	float gen_born;

	float gen_dead;

	int gen_style;

	uint dmy6[3];

	float gen_size_x;

	float gen_size_y;

	float gen_size_z;

	float gen_size_w;

	float gen_power;

	float gen_power_rnd;

	float gen_angle;

	float gen_angle_rnd;

	float gen_pos_x;

	float gen_pos_y;

	float gen_pos_z;

	float gen_pos_w;

	float gen_dir_y;

	float gen_dir_x;

	float gen_dir_z;

	float gen_dir_w;

	float gen_rot;

	float gen_rot_rnd;

	float gen_rot_spd;

	float gen_rot_spd_rnd;

	float gen_accele;

	uint dmyF[3];

	int ptc_scale_x_curve;

	float ptc_scale_xs;

	float ptc_scale_xs_rnd;

	float ptc_scale_xe;

	int ptc_scale_y_curve;

	float ptc_scale_ys;

	float ptc_scale_ys_rnd;

	float ptc_scale_ye;

	int ptc_rgba_r_curve;

	int ptc_rgba_g_curve;

	int ptc_rgba_b_curve;

	int ptc_rgba_a_curve;

	float ptc_rgba_r_refs;

	float ptc_rgba_g_refs;

	float ptc_rgba_b_refs;

	float ptc_rgba_a_refs;

	float ptc_rgba_r_refe;

	float ptc_rgba_g_refe;

	float ptc_rgba_b_refe;

	float ptc_rgba_a_refe;

	int ptc_alpha_test_func;

	float ptc_alpha_test_ref;

	uint dmy04[2];

	float ptc_rot1_rnd;

	float ptc_rot1_spd;

	float ptc_rot1_spd_rnd;

	float ptc_rot1;

	float ptc_rot2_rnd;

	float ptc_rot2_spd;

	float ptc_rot2_spd_rnd;

	float ptc_rot2;

	float ptc_rot3_rnd;

	float ptc_rot3_spd;

	float ptc_rot3_spd_rnd;

	float ptc_rot3;

	int ptc_weight_curve;

	float ptc_weight_s;

	float ptc_weight_s_rnd;

	float ptc_weight_e;

	float ptc_age;

	float ptc_age_rnd;

	uint dmyE[2];

	int ptc_style;

	uint dmyG[3];

	float ptc_size_x;

	float ptc_size_y;

	float ptc_size_x_rnd;

	float ptc_size_y_rnd;

	int ptc_blend_style;

	uint dmyD[3];

	float ptc_jump;

	uint dmyC[3];

	float ptc_alpha_frm;

	uint dmyB[3];

	int ptc_feel_air_curve;

	float ptc_feel_air_s;

	float ptc_feel_air_e;

	uint dmyA[1];

	float ptc_feel_rot_gen_s;

	float ptc_feel_rot_gen_rnd;

	float ptc_feel_rot_gen_e;

	int ptc_feel_rot_gen_curve;

	float tex_pat_frm;

	int tex_pat_curve;

	uint dmy8[2];

	int tex_filter;

	int tex_div_x;

	int tex_div_y;

	uint dmy7[1];

	int tex_pat_range_s;

	int tex_pat_range_e;

	uint dmyH[1];

	float ptc_scale_xe_rnd;

	float ptc_scale_ye_rnd;

	uint dmyI[2];
};

}
