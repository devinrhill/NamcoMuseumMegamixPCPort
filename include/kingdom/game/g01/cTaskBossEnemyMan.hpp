#pragma once

#include <vector>

#include "kingdom/game/g01/cTaskEnemy.hpp"

// Galaga Remix (g01) stage 10 enemy manager: kingdom::game::g01::cTaskBossEnemyMan (0x210 bytes, vtable 0x8043a4d8; Init 0x800e7368 = the
// cTaskEnemyMan Init + three pools sized by the stage row of table 0x80373138: boss missiles +0x1fc (cBossMissileWork, 10), SP missiles +0x200
// (cBossSpMissileWork, 10), boss enemies +0x1f8 (tWorkObject<cTaskBossEnemy>, 50).
namespace kingdom::game::g01
{
class cTaskBoss;
class cTaskBossEnemy;
class cTaskBossMissile;

class cTaskBossEnemyMan : public cTaskEnemyMan
{
public:
	cTaskBossEnemyMan();                                                    // DOL 0x801b9404
	void Init(int nStage, cTaskScene* pScene, cTaskPacman* pPacman) override;   // DOL 0x800e7368
	cTaskBossMissile* GetBossMissile(bool bSp);                             // GetFreeWork 0x800cb218 / 0x800cb1b8
	void SpawnBossEnemy(const float vLocal[3], int nType, int nOrbit, cTaskBoss* pBoss);   // DOL 0x800e7ca8
	void MoreTargets(void (*fn)(const cObject*, void*), void* ctx) const override;   // devtools: flying boss missiles, the struggling boss

private:
	std::vector<cTaskBossMissile*> m_BossMissile[2];                        // +0x1fc normal, +0x200 SP
	std::vector<cTaskBossEnemy*> m_BossEnemy;                               // +0x1f8
};
}
