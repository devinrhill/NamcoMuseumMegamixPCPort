#pragma once

#include <cstdint>
#include <cstring>

// The gotch "swork" block every arcade game keeps (PACMAN::THISGAME_sw, 0x228 bytes at RSO2 0x811fa330): flags shared by the emulated game,
// the Wii menu and the frontend. The port keeps the original byte offsets (the games' code addresses it as sw + offset); 32-bit fields are
// stored in host order (nothing reads them as bytes).
namespace gotch
{
struct SWork
{
	std::uint8_t m[0x228] = {};

	std::uint8_t& b(int off) { return m[off]; }
	std::int32_t i32(int off) const
	{
		std::int32_t v;
		std::memcpy(&v, &m[off], 4);
		return v;
	}
	void set32(int off, std::int32_t v) { std::memcpy(&m[off], &v, 4); }
};

void swork_HotStart(SWork& sw);                           // RSO2 0x80d2362c
void swork_ColdStart(SWork& sw, int gameLocale);          // RSO2 0x80d23840 (calls GetGameLocale; the port passes it)
void swork_SetWork_GameStart(SWork& sw, int players, int cont);   // RSO2 0x80d23a24
void swork_SetWork_GameDemo(SWork& sw);                   // RSO2 0x80d23b78
void swork_SetWork_1P_PauseStatus(SWork& sw, std::uint8_t v, int minTime);  // RSO2 0x80d23bb0
void swork_SetWork_2P_PauseStatus(SWork& sw, std::uint8_t v, int minTime);  // RSO2 0x80d23bdc
}
