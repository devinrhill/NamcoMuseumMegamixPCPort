#pragma once

#include "common.hpp"

namespace jet::ptk
{

class wrkGen
{
public:
	float ptc_alive{};

	float ptc_born{};

	Vector4 axis_vec{};

	float axis_rot{};

	float axis_rot_spd{};

	Matrix4x4 mtx_trn{};

	Matrix4x4 mtx_rot{};

	Matrix4x4 mtx_all{};

	void Reset();
};

}
