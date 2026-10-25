#pragma once

#include "common.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cKingdomAnitaVector.hpp"

namespace kingdom::boot
{

// Job 132 (0x08400000): the startup screens (strap warning, logo, nunchuk notice, rights). Ported from main.dol:
//   RTTI 0x80477218, vtable 0x80394af8, ctor inlined in the job init 0x8009f7a0, setup FUN_8009e024 (0x8009e024),
//   Execute 0x8009e2cc (0x12f8 bytes), Draw 0x8009f5c4, dtor 0x8009f698. Not in the Unity port.
//
// Execute is a 20 state machine (jump table 0x80394aa8) over seven Anita layers at +0x280 loaded from boot.lzs:
//   layers 0,1 ANITA_STRAP   layers 2,3 ANITA_LOGO   layer 4 ANITA_NUNCHUK   layers 5,6 ANITA_RIGHT
// The screens advance when their animation ends, or (the timed ones) on any button but B after 1 s / automatically
// after a per-screen timeout; holding B pauses the timer. The last state switches to the title job.
class cAppBoot : public cAppKingdom
{
public:
	cAppBoot();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	void Init(); // DOL 0x8009e024

	cKingdomAnitaVector m_Anita; // +0x280
};

}
