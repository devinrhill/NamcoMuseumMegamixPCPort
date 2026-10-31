#pragma once
// The DOL's libc rand() / srand() (MSL, DOL 0x80211048: next = next * 0x41c64e6d + 0x3039; return (next >> 16) & 0x7fff).
// One global state like the original: all game code and the particles (PtkSim) draw from it.
#include <cstdint>

namespace kingdom
{
inline std::uint32_t& MslRandState()
{
	static std::uint32_t next = 1u;
	return next;
}

// DOL 0x80211048 (rand)
inline int MslRand()
{
	std::uint32_t& next = MslRandState();
	next = next * 0x41c64e6du + 0x3039u;
	return (int)((next >> 16) & 0x7fffu);
}

// min + rand() % (max - min): the DOL's idiom for a ranged integer (DOL 0x801ea290; Unity's Random.Range(min, max) in the converted C#)
inline int MslRandRange(int min, int max)
{
	return min + MslRand() % (max - min);
}

// srand
inline void MslSrand(std::uint32_t seed)
{
	MslRandState() = seed;
}

// The value cAppTitle::Init seeds with: the DOL's low word of OSGetTime(), or the --seed N override (scripted runs repeat).
inline bool& RandSeedForced()
{
	static bool forced = false;
	return forced;
}
inline std::uint32_t& RandSeedValue()
{
	static std::uint32_t value = 1u;
	return value;
}
inline std::uint32_t RandSeedOverride(std::uint64_t osTime)
{
	return RandSeedForced() ? RandSeedValue() : (std::uint32_t)osTime;
}
}
