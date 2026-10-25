#pragma once

#include "kingdom/cTaskPBPtkMan.hpp"

// Galaga Remix (g01) particle manager: kingdom::game::g01::cTaskEffect (0xd50 bytes, vtable 0x8044b100), the app's pTaskPtkMan. Setup
// 0x801c1ec0: cKingdomPtkMan::Create(0x44 slots, 100 emitters, 400 generators, 8000 particles) (0x80021b80), then slot i = the PTK file of
// CRC table 0x8037e248 (0x80021c58); natively registered by entry name (the port's RegistData by CRC is not implemented).
// Execute 0x801c1f6c = the particle calc (FUN_8008ad34) unless paused / the HOME button is held / the rate is 0: natively the cTaskPBPtkMan
// Execute (the pause and HOME checks are not ported).
namespace kingdom::game::g01
{
class cTaskEffect : public kingdom::cTaskPBPtkMan
{
public:
	bool Execute(cJetTask* pParam) override;   // DOL 0x801c1f6c: the particle calc unless Pac-Man missed (0x800afe2c)
	enum ESlot
	{
		eBOMB_SMALL01 = 0x00,   // + player (0x800e4220: size 1)
		eBOMB_MIDDLE01 = 0x04,  // + player (size 2)
		eBOMB_BIG01 = 0x08,     // size 3
		ePACMAN_HIT01 = 0x10,
		eENEMY_ATTACK_SIGN01 = 0x13,
		eM_COMET01 = 0x1c,
		eSLOT_MAX = 0x44
	};
	cTaskEffect();
	static const char* const SLOT_NAME[eSLOT_MAX];
};
}
