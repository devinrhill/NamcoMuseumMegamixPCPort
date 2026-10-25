#include "CommonProc.hpp"

bool CommonProc::BitCheck(byte* p, uint Num)
{
	return 0 < (p[Num / 8] & (byte)(1 << (int)Num % 8));
}

void CommonProc::BitOn(byte* p, uint Num)
{
	p[Num / 8] |= (byte)(1 << (int)Num % 8);
}

void CommonProc::BitOff(byte* p, uint Num)
{
	p[Num / 8] &= (byte)(~(1 << (int)(Num % 8)));
}
