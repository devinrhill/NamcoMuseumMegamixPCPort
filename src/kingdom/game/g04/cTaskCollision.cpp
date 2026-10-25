#include "kingdom/game/g04/cTaskCollision.hpp"

#include "kingdom/game/g04/cTaskEnemyManager.hpp"
#include "kingdom/game/g04/cTaskPacmanBase.hpp"

namespace kingdom::game::g04
{

cTaskCollision::cTaskCollision(cTaskEnemyManager* pEnemy, cTaskPacmanManager* pPacman) : m_pEnemy(pEnemy), m_pPacman(pPacman)
{
	SetName("g04::cTaskCollision");
}

// DOL 0x8019d828 (the callbacks run even while paused: the Pac-Man only tests while lunging, and a paused Pac-Man does not move)
bool cTaskCollision::Execute(cJetTask* pParam)
{
	for (uint p = 0; p < 4; p++)
		for (uint e = 0; e < 4; e++) m_pPacman->pPacman(p)->HitEnemy(m_pEnemy->pEnemy(e));
	return cTaskKingdom::Execute(pParam);
}

}
