#pragma once

#include "kingdom/cTaskKingdom.hpp"

// Gator Panic Remix (g04) kingdom::game::g04::cTaskCollision (0x1c0 bytes, vtable 0x80449730; builder 0x80093920 at app +0x28c): a vector of
// cHitCallback<cTaskPacmanBase, cTaskEnemy> (0x18 bytes, vtable 0x803940f8: {enemy, PTMF 0x801b48d0, pacman}) for every Pac-Man x gator pair
// (4 x 4, Pac-Man outer); Execute 0x8019d828 calls each (0x8009460c -> cTaskPacmanBase::HitEnemy).
namespace kingdom::game::g04
{
class cTaskEnemyManager;
class cTaskPacmanManager;

class cTaskCollision : public kingdom::cTaskKingdom
{
public:
	cTaskCollision(cTaskEnemyManager* pEnemy, cTaskPacmanManager* pPacman);   // the builder 0x80093920 (+ 0x8019d8ac per pair)
	bool Execute(cJetTask* pParam) override;    // DOL 0x8019d828

private:
	cTaskEnemyManager* m_pEnemy{};
	cTaskPacmanManager* m_pPacman{};
};
}
