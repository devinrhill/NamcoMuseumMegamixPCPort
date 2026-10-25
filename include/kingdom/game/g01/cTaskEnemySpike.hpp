#pragma once

#include "kingdom/game/g01/cTaskEnemy.hpp"
#include "kingdom/game/g01/cTaskShootMarker.hpp"

namespace wii { struct Mdl0Node; }

// Galaga Remix (g01) spike: kingdom::game::g01::cTaskEnemySpike (0x370 bytes, vtable 0x8043a8a0), placed by cTaskStage 0x800fa2fc at the
// skeleton nodes m01m_g11_%03d (stages 5..8), registered for collision. An enemy of type 13 (CHAR_ENEMY_SPIKE_MDL, kind 0xb) at its node:
// SpikeGalaga_s%x.csv row n = {action frame, collision from, collision until} (cSpikeParam 0x800e82d0). From the action frame it grows
// (CHA_04 / CLA_04) and then loops CHA_01; inside the collision window it hurts Pac-Man on contact (spheres 0.35 + 1.0, not while he
// jumps; PTK_SPIKE_HIT01) and can be shot down like any enemy; its shoot marker (0x800f84d4) shows inside the window.
namespace kingdom::game::g01
{
class cTaskEnemySpike : public cTaskEnemy
{
public:
	cTaskEnemySpike();
	// DOL 0x800e8310 (vtable +0xd4) + 0x800e8514 (+0xcc); the spike master (0x800e8e78 / 0x800e9000) uses model 14 and its own CSV
	void SetupSpike(const wii::Mdl0Node& node, int nStage, int nIndex, int nModel = 13, const char* csvFormat = "SpikeGalaga_s%x.csv");
	bool Execute(cJetTask* pParam) override;                               // DOL 0x800e8ab0
	bool HitCheck(cObject* other) override;                                // DOL 0x800e8cdc
	void HitCallback(cObject* other) override;                             // DOL 0x800e8d4c
	void Dead() override;                                                  // DOL 0x800e8e38

protected:
	void GrowStep();                                         // DOL 0x800e8934 (vtable +0x7c)
	float m_fAction{}, m_fHitFrom{}, m_fHitUntil{};          // cSpikeParam +0 / +4 / +8
	int m_nGrow{};                                           // +0x35c
	bool m_bHitOn{};                                         // +0x360
	float m_fFlash{};                                        // +0x284
	cTaskShootMarker* m_pMarker{};                           // +0x358
};
}
