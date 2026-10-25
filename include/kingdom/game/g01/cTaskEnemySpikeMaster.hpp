#pragma once

#include "kingdom/game/g01/cTaskEnemySpike.hpp"

// Galaga Remix (g01) spike master (stage 8): kingdom::game::g01::cTaskEnemySpikeMaster (0x370 bytes, vtable 0x8043a768, a cTaskEnemySpike), placed
// by cTaskStage 0x800fa578 at the skeleton nodes m01m_g12_%03d (n = 1, 2 ...) and registered for collision. Model type 14
// (CHAR_ENEMY_SPIKEMASTER), SpikeMasterGalaga_s%x.csv row n - 1 = {action frame, collision from, -}. It grows at the action frame
// (CHA_04 / CLA_04 -> CHA_01 + TSA_01) and is collidable from "collision from" on for good; shots only spark (PTK_M_COMET01, m01se_03hit1):
// it cannot be destroyed. Touching it hurts Pac-Man (PTK_SPIKE_HIT01; the damage is Pac-Man's own callback).
namespace kingdom::game::g01
{
class cTaskEnemySpikeMaster : public cTaskEnemySpike
{
public:
	cTaskEnemySpikeMaster();
	void Setup(const wii::Mdl0Node& node, int nStage, int nIndex);   // DOL 0x800e8e78 (vtable +0xd0) + 0x800e9000 (+0xcc)
	bool Execute(cJetTask* pParam) override;    // DOL 0x800e4704 -> 0x800e9574 (vtable +0xa0)
	void HitCallback(cObject* other) override;  // DOL 0x800e9710

private:
	void MasterGrowStep();                      // DOL 0x800e9420 (vtable +0x7c)
};
}
