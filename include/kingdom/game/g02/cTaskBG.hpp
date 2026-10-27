#pragma once

#include <memory>
#include <vector>

#include "kingdom/cKingdomPtkHnd.hpp"

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"

// Rally-X Remix (g02) stage scenery: kingdom::game::g02::cTaskBG (0x1e4 bytes; Init FUN_8005f8e8, Term FUN_8006002c, Execute FUN_800600a4).
// The stage archive's STAGE_BG_SET (0x33e3393c) is a skeleton: the 12 models STAGE_BG_1A .. 3D (CRC table 0x8036b590) go to the position of
// its nodes "_1a" .. "_3d" (name table 0x8036b5c0), with their STAGE_BG_xx_ANIM clips (table 0x8036b5f0); then STAGE_SKY (+ STAGE_SKY_ANIM)
// at node 1 of STAGE_SKY_SET (0xa9d70415, FUN_8001de14 = node position by index); STAGE_BG_OUT_OBJ_1..8 (table 0x8036b620, clips 0x8036b660) at
// the nodes "ob1".."ob8" (0x8036b640 -> 0x8047a8a8) of STAGE_BG_OUT_OBJ_SET (0x0da2ab81); STAGE_BG_OUT (DAT_8047a8c8) at node 1 of
// STAGE_BG_OUT_SET (0xc5b4d2b2). Every model: light set 0, fog 0.
namespace kingdom::game::g02
{
class cTaskBG : public kingdom::cTaskModel
{
public:
	cTaskBG();                                 // DOL 0x80060898-region ctor + Init 0x8005f8e8
	bool Execute(cJetTask* pParam) override;   // DOL 0x800600a4
	void Term() override;                      // DOL 0x8006002c (run by cTaskKingdom::TermOnce at the job change)
	// DOL 0x8006016c (the app's stage reset 0x80125a14, single player): the butterflies, particle slot 0x28 at every child of node "ef_butterfly01"
	// of STAGE_BG_PTK_POINT; false when the stage has none
	bool ResetPtk();

private:
	std::vector<std::unique_ptr<kingdom::cKingdomPtkHnd>> m_Ptk;   // +0x1d0 (a vector of handles)

protected:
	void ReleaseTerm() override { cTaskBG::Term(); kingdom::cTaskModel::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};
}
