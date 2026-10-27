// 1:1 port of the gotch swork helpers (nmwii2.rso). The stores keep the original order; runs of zeroed fields are written as loops over the same offsets.
#include "gotch/SWork.hpp"

namespace gotch
{
// RSO2 0x80d2362c
void swork_HotStart(SWork& sw)
{
	for (int o = 0x04; o <= 0x08; o++) sw.b(o) = 0;
	sw.set32(0x0c, 3);
	sw.set32(0x10, 3);
	sw.set32(0x18, -1);
	sw.set32(0x1c, 0);
	for (int o = 0x20; o <= 0x2d; o++) sw.b(o) = 0;
	sw.b(0x2f) = 0;
	for (int o = 0x32; o <= 0x39; o++) sw.b(o) = 0;
	sw.set32(0x3c, 0);
	sw.b(0x50) = 1;
	sw.b(0x52) = 0;
	sw.set32(0x58, 0);
	sw.set32(0x60, 0);
	for (int o = 0x6c; o <= 0x77; o++) sw.b(o) = 0;
	for (int o = 0x78; o <= 0x9c; o += 4) sw.set32(o, 0);
	sw.b(0xa3) = sw.b(0xa4);
	sw.b(0xaf) &= 0xfc;
	sw.b(0xb0) &= 0x7f;
	sw.set32(0xdc, 0);
	sw.set32(0xe0, 0);
	// four players: +0x100 / +0x114 / +0xf0 / +0x118 / +0xe4 (the original interleaves them per player)
	for (int p = 0; p < 4; p++)
	{
		if (p == 0)
		{
			sw.set32(0x100, 0);
			sw.b(0x114) = 0;
			sw.set32(0xf0, 0);
			sw.b(0x118) = 0;
			continue;
		}
		sw.set32(0xe0 + p * 4, 0);
		sw.set32(0x100 + p * 4, 0);
		sw.b(0x114 + p) = 0;
		sw.set32(0xf0 + p * 4, 0);
		sw.b(0x118 + p) = 0;
	}
	for (int o = 0x1a0; o <= 0x21c; o += 4) sw.set32(o, 0);
	for (int o = 0xc8; o <= 0xd4; o += 4) sw.set32(o, 0);
	sw.b(0xd8) = 0;
	sw.b(0xd9) = 0;
	sw.b(0x222) = 0;
	sw.b(0x223) = 0;
}

// RSO2 0x80d23840
void swork_ColdStart(SWork& sw, int gameLocale)
{
	sw.set32(0x00, 0);  // the original stores the StringNoTitle pointer here
	sw.set32(0x14, 0);
	sw.b(0x2e) = 0;
	sw.b(0x44) = 0;
	sw.set32(0x48, 0);
	sw.set32(0x4c, 0);
	sw.b(0x51) = 0;
	sw.b(0x53) = 0;
	swork_HotStart(sw);
	sw.b(0xa0) = 0xff;
	sw.b(0xa1) = 0xff;
	sw.b(0xa2) = 0xff;
	sw.b(0x54) = 0;
	sw.b(0x5c) = 0;
	sw.set32(0x64, 0);
	sw.set32(0x68, 0);
	sw.b(0x30) = 0;
	sw.b(0x31) = 0;
	sw.b(0xa5) = 0;
	sw.b(0x6c) = 1;
	sw.b(0xa4) = 0;
	sw.b(0xa7) = 1;
	sw.b(0xa8) = 1;
	sw.b(0xa9) = 1;
	sw.b(0xaa) = 1;
	sw.b(0xab) = 1;
	sw.b(0xac) = 1;
	sw.b(0xa6) = 0;
	sw.b(0xad) = 0;
	sw.b(0xae) &= 0xf3;
	sw.b(0xaf) = (std::uint8_t)(gameLocale << 4 | (sw.b(0xaf) & 0xf) | 0xc);
	for (int o = 0x120; o <= 0x21c; o += 4) sw.set32(o, 0);
	sw.b(0x220) = 0;
	sw.b(0x221) = 0;
}

// RSO2 0x80d23a24 (players 0 = 1P, 1 = 2P, other = keep; cont 0 / 1 = continue off / on, other = keep)
void swork_SetWork_GameStart(SWork& sw, int players, int cont)
{
	sw.b(0x21) = 0;
	sw.b(0x38) = 0;
	sw.b(0x2f) = 1;
	sw.b(0x20) = 0;
	if (sw.i32(0x0c) < 0x3c) sw.set32(0x0c, 0x3c);
	sw.b(0x51) = 1;
	if (players == 0 || players == 1)
	{
		sw.b(0x33) = 0;
		sw.b(0x34) = players == 0 ? 1 : 0;
		sw.b(0x35) = players == 1 ? 1 : 0;
		swork_SetWork_1P_PauseStatus(sw, 1, 10);
		swork_SetWork_2P_PauseStatus(sw, players == 1 ? 1 : 0, 10);
	}
	if (cont == 0)
	{
		sw.b(0x39) = 0;
		return;
	}
	if (cont == 1) sw.b(0x39) = 1;
}

// RSO2 0x80d23b78
void swork_SetWork_GameDemo(SWork& sw)
{
	sw.b(0x21) = 0;
	sw.b(0x38) = 0;
	sw.b(0x2f) = 0;
	sw.b(0x20) = 1;
	if (sw.i32(0x0c) < 3) sw.set32(0x0c, 3);
	sw.b(0x51) = 1;
}

// RSO2 0x80d23bb0
void swork_SetWork_1P_PauseStatus(SWork& sw, std::uint8_t v, int minTime)
{
	sw.b(0x04) = v;
	sw.b(0x06) = 0;
	sw.b(0x08) = 0;
	if (sw.i32(0x0c) < minTime) sw.set32(0x0c, minTime);
	sw.b(0x26) = 1;
}

// RSO2 0x80d23bdc
void swork_SetWork_2P_PauseStatus(SWork& sw, std::uint8_t v, int minTime)
{
	sw.b(0x05) = v;
	sw.b(0x07) = 0;
	sw.b(0x08) = 0;
	if (sw.i32(0x0c) < minTime) sw.set32(0x0c, minTime);
	sw.b(0x26) = 1;
}
}
