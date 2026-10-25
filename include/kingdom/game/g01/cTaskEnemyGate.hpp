#pragma once

#include <vector>

#include "kingdom/game/g01/cTaskEnemy.hpp"
#include "kingdom/game/g01/cTaskShootMarker.hpp"

namespace wii { struct Mdl0Node; }

// Galaga Remix (g01) enemy gate: kingdom::game::g01::cTaskEnemyGate (0x3d0 bytes, vtable 0x8043a2a0), placed by cTaskStage 0x800fa0a4 at the
// skeleton nodes m01m_g10_%03d (n = 1, 2 ...), registered for collision. An enemy of type 12 (CHAR_ENEMY_GATE_MDL, kind 0xb) that stays at its
// node: GateGalaga_s%xp%d.csv row n - 1 = {orbit, action start, formation group, hittable from} (cGateParam 0x800e5008). It waits for the
// course time "action start", runs its orbit only as a timetable (fire patterns), then plays CHAR_ENEMY_GATE_ANM_CHA_03 and dies at its end;
// shot down before that, it dies at once. Either way it leaves and clears its formation group (stage 1: group 1, which the course's last
// gate waits for).
namespace kingdom::game::g01
{
class cTaskEnemyGate : public cTaskEnemy
{
public:
	cTaskEnemyGate();
	~cTaskEnemyGate() override;
	static std::vector<cTaskEnemyGate*> s_Live;              // devtools: --g01-autoaim also aims at gates (not 1:1)
	void SetupGate(const wii::Mdl0Node& node, int nStage, int nIndex);   // DOL 0x800e4814
	bool Execute(cJetTask* pParam) override;                              // DOL 0x800e5778
	void Dead() override;                                                 // DOL 0x800e5048

private:
	bool UpdateTimetable(float fStep);                       // DOL 0x800e5234 (vtable +0x7c)
	bool DieStep();                                          // DOL 0x800e53b8 (vtable +0xa8): true when gone
	void UpdateGateModel();                                  // DOL 0x800e54f8 (vtable +0x90)
	int m_nOrbitNo{};                                        // cGateParam +0x00 (CSV column 1)
	float m_fStart{};                                        // +0x08 column 2: the course frame that starts it
	float m_fHit{};                                          // +0x0c column 4: the course frame from which it can be shot
	bool m_bArrived{};                                       // +0x374
	bool m_bHittable{};                                      // +0x3c4
	float m_vPlace[4]{0, 0, 0, 1};                           // +0x360 the node position (minus the model's root offset)
	float m_mBase[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};   // +0x380 node "base" of the clip once arrived
	float m_fFlash{};                                        // +0x284
	cTaskShootMarker* m_pMarker{};                           // +0x358
	int m_nDbg{-1};
};
}
