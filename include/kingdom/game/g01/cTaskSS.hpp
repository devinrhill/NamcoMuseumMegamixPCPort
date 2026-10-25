#pragma once

#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g01/cObject.hpp"

// Galaga Remix (g01) shooting stars: kingdom::game::g01::cTaskSS (0x340 bytes, vtable 0x8043bbc0), made by the stage setup for stages 1..6
// (0x800fa80c -> ctor 0x800f8f74). The locator skeleton m01e_53_star01.brres; every 40..100 frames (rand) the particle slot 0x20 starts at a
// random node (index 2.., its translate and rotate, scale 10), replacing the previous one.
namespace kingdom::game::g01
{
class cTaskSS : public cObject, public kingdom::cTaskModel
{
public:
	cTaskSS();                                 // DOL 0x800f8f74 (+ vtable +0xa8 = 0x800f9450: state 1)
	bool Execute(cJetTask* pParam) override;   // DOL 0x800f9184 (active, not paused, state 1: +0xa0)

private:
	void Step();                               // DOL 0x800f91e8 (vtable +0xa0)
	static float NextWait();                   // (rand % 60000 + 40000) / 1000 (FLOAT_8047b710)
	int m_nNodes{};                            // +0x2e8 the skeleton's node count
	float m_fWait{};                           // +0x2ec frames to the next star
	kingdom::cKingdomPtkHnd m_Hnd;             // +0x2f0
};
}
