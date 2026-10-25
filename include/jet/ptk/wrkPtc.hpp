#pragma once

#include "common.hpp"

namespace jet::ptk
{

class wrkPtc
{
public:
	Vector4 p{};

	Vector4 pn{};

	Vector4 v{};

	Color color{};

	uint flag{};

	float sx{};

	float sy{};

	float age{};

	float age_max{};

	float weight{};

	uint tex_pat{};

	float tex_frm{};

	float alpha_frm{};

	float weight_s{};

	float weight_e{};

	float scale_x_s{};

	float scale_y_s{};

	float scale_x_e{};

	float scale_y_e{};

	float rot1{};

	float rot1_spd{};

	float rot2{};

	float rot2_spd{};

	float rot3{};

	float rot3_spd{};

	float size_x{};

	float size_y{};

	float feel_rot_gen_s{};

	float feel_rot_gen_e{};

	uint tex_pat_min{};

	uint tex_pat_max{};

	float distance{};

	void Reset();
};

}
