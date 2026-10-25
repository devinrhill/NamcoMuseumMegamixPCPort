#pragma once

#include "common.hpp"

class CommonProc
{
public:
	static bool BitCheck(byte* p, uint Num);

	static void BitOn(byte* p, uint Num);

	static void BitOff(byte* p, uint Num);
};
