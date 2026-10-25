#pragma once

#include <cstdint>

// DSP-ADPCM (the GameCube / Wii 4-bit format used by RSTM streams and RWAV waves): 8 byte frames = 1 header byte (predictor index << 4 |
// scale exponent) + 14 nibbles. AdpcmParam layout from NintendoWare snd/global.h.
namespace snd
{
struct AdpcmParam
{
	std::int16_t coef[16]{};
	std::uint16_t gain{}, ps{};
	std::int16_t yn1{}, yn2{};
};
struct AdpcmLoopParam
{
	std::uint16_t ps{};
	std::int16_t yn1{}, yn2{};
};

// Decodes one 8 byte frame into 14 samples, updating the two history samples.
inline void DecodeDspFrame(const std::uint8_t* f, const std::int16_t coef[16], int& h1, int& h2, std::int16_t out[14])
{
	const int scale = 1 << (f[0] & 0xf), idx = (f[0] >> 4) & 7;
	const int c1 = coef[idx * 2], c2 = coef[idx * 2 + 1];
	for (int i = 0; i < 14; i++)
	{
		int v = (i & 1) ? (f[1 + i / 2] & 0xf) : (f[1 + i / 2] >> 4);
		if (v >= 8) v -= 16;
		int x = (v * scale * 2048 + 1024 + c1 * h1 + c2 * h2) >> 11;
		x = x < -32768 ? -32768 : x > 32767 ? 32767 : x;
		h2 = h1;
		h1 = x;
		out[i] = (std::int16_t)x;
	}
}
}
