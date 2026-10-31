#pragma once

#include "kingdom/cTaskPBPtkMan.hpp"

// Grobda Remix (g11) particle manager: kingdom::game::g11::cTaskGrobdaEffect (0xd50 bytes; ctor FUN_80034948, setup 0x80035b0c), the app's
// pTaskPtkMan. cKingdomPtkMan::Create(0x25 slots, 0x100 emitters, 0x800 generators, 0x4000 particles); slots by CRC (FUN_80021c58) or by name
// (FUN_80021be4). Effects are generated through FUN_80035ec0(id, pos, rot, handle) with id = the slot.
namespace kingdom::game::g11
{
class cTaskGrobdaEffect : public kingdom::cTaskPBPtkMan
{
public:
	enum ESlot
	{
		eSHIELD_L_PLAYER = 0x00,   // + player
		eSHIELD_M_PLAYER = 0x04,
		eSHIELD_S_PLAYER = 0x08,
		eSHOT_PLAYER = 0x0c,       // + player
		eSHOT_ENEMY = 0x10,        // 0x10..0x16
		eHIT = 0x17,
		eBOMB = 0x18,
		eBOMB_GRENADE = 0x19,
		eGRENADE_LOCUS = 0x1a,     // + player
		eSHIELD_ENEMY = 0x1e,
		eFRAGMENT = 0x1f,
		eSHORT = 0x20,
		eP1P = 0x21,               // + player
		eSLOT_MAX = 0x25
	};
	cTaskGrobdaEffect();            // DOL 0x80035b0c
	bool Execute(cJetTask* pParam) override;   // DOL 0x80035e4c
	// FUN_8009ff60(app, slot, pos, rot, 0, 0): a one-shot effect at a place (the DOL's effect task keeps the handle; natively a ring of handles)
	static void Spawn(uint slot, const float pos[3], const float rot[3] = nullptr);
	static const char* const SLOT_NAME[eSLOT_MAX];
};
}
