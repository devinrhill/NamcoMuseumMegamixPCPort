#pragma once

#include "common.hpp"

namespace jet
{

struct sDatLayP
{
	float u0;

	float v0;

	float u1;

	float v1;

	float su;

	float sv;

	float suh;

	float svh;

	float px;

	float py;

	float isx;

	float isy;

	float irot;

	float ocx;

	float ocy;

	float osx;

	float osy;

	float orot;

	byte a;

	byte b;

	byte g;

	byte r;

	int tex;

	int blend;

	int filter;

	int flag;
};

}
