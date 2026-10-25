#pragma once

#include <vector>

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"

// Galaga Remix (g01) collision pass: kingdom::game::g01::cTaskCollisionMan (vtable 0x80439638; KingdomGlobal::GalagaSys->pTaskCollisionMan()).
// Objects register once (0x800d41c8); every frame HitCheckAll (vtable slot 13, 0x800d3b70) keeps the active (+8) and collidable (+0x14) ones,
// drops those behind the camera (kind 1, the console players, are always kept) and sorts the rest by distance; Execute (0x800d40b8) then
// calls A->HitCheck(B) for every ordered pair and on a hit A->HitCallback(B) and B->HitCallback(A).
namespace kingdom::game::g01
{
class cObject;

class cTaskCollisionMan : public kingdom::cTaskKingdom
{
public:
	cTaskCollisionMan();                       // DOL 0x801b971c
	void Regist(cObject* p);                   // DOL 0x800d41c8
	void Remove(cObject* p);
	void Stop() { m_bActive = false; }         // DOL 0x800d44b0 (the app's MISS state)
	bool Execute(cJetTask* pParam) override;   // DOL 0x800d40b8

private:
	void HitCheckAll();                        // DOL 0x800d3b70
	bool m_bActive{true};                      // +0x1b4
	std::vector<cObject*> m_Regist;            // +0x1b8
	std::vector<std::pair<cObject*, float>> m_Sorted;   // +0x1c4 (object, squared distance)
};
}
