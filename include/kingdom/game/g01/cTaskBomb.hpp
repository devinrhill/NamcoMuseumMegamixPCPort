#pragma once

#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g01/cObject.hpp"

// Galaga Remix (g01) the bomb of a bomb enemy: kingdom::game::g01::cTaskBomb (0x2a0 bytes, vtable 0x80438a20; ctor 0x800c9674). When a player
// kills an enemy of type 5 or 8 (cTaskEnemy::HitCallback 0x800e27b4) a free bomb of the enemy manager's pool (+0x1e4, tWorkObject<cTaskBomb>,
// 5 per stage: table 0x80373138 column 6) starts there (0x800c9a78): effect 0x2a (one-time, dependent, clipped) and a sphere of kind 0xc that
// keeps its offset to the camera aim point and grows to BombParam.csv AREA in INTERVAL frames; after ENDURANCE frames it is gone. Every enemy
// (kind 0xb) whose box the sphere touches is hit as if by the bomb's player (its HitCallback asks the bomb for its owner, 0x800c9d54), so the
// bomb's kills chain into further bombs.
namespace kingdom::game::g01
{
class cTaskConsolePlayer;

class cTaskBomb : public cObject, public kingdom::cTaskKingdom
{
public:
	cTaskBomb();                                    // DOL 0x800c9674
	void Start(cTaskConsolePlayer* pOwner, const float vOffset[3]);   // DOL 0x800c9a78
	bool Execute(cJetTask* pParam) override;        // DOL 0x800c9bc0
	bool HitCheck(cObject* other) override;         // DOL 0x800c9cf4 (kind 0xb: sphere against the box)
	cTaskConsolePlayer* pOwner() const { return m_pOwner; }   // DOL 0x800c9d54 (+0x210; task vtable +0x54)
	bool IsFree() const { return !m_b08; }          // the pool's test (+0x1c8 = cObject +0x08)

private:
	void Follow();                                  // the shared part of 0x800c9a78 / 0x800c9bc0: position, effect, sphere, radius step
	struct sParam { float fArea{}, fInterval{1.0f}, fEndurance{}; };   // cBombParam (0x10 bytes, vtable 0x80438aac)
	sParam m_Param;                                 // +0x214 (a pointer in the DOL; BombParam.csv AREA / INTERVAL / ENDURANCE)
	cTaskConsolePlayer* m_pOwner{};                 // +0x210
	float m_vOffset[3]{};                           // +0x220 position - camera aim point
	float m_fRadius{};                              // +0x230
	float m_fGrow{};                                // +0x234 frames grown
	float m_fAge{};                                 // +0x238
	kingdom::cKingdomPtkHnd m_Hnd;                  // +0x240 (translate +0x250)
	float m_vCenter[3]{};                           // +0x290 the sphere (centre = the effect's translate, radius +0x29c)
	float m_fSphereR{};                             // +0x29c
};
}
