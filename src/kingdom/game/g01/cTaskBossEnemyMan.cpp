#include "kingdom/game/g01/cTaskBossEnemyMan.hpp"

#include <cstdio>
#include <cstdlib>

#include "KingdomGlobal.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskBoss.hpp"
#include "kingdom/game/g01/cTaskBossEnemy.hpp"
#include "kingdom/game/g01/cTaskBossMissile.hpp"
#include "kingdom/game/g01/cTaskCollisionMan.hpp"

namespace kingdom::game::g01
{

namespace
{
constexpr u32 BOSS_MISSILES = 10;   // DAT_80373148 + stage 10 * 0x24
constexpr u32 SP_MISSILES = 10;     // DAT_8037314c + stage 10 * 0x24
constexpr u32 BOSS_ENEMIES = 50;    // the boss enemy pool (tWorkObject<cTaskBossEnemy>)
}

cTaskBossEnemyMan::cTaskBossEnemyMan()
{
	SetName("g01::cTaskBossEnemyMan");
}

// DOL 0x800e7368 (+ the Bullet_s%x.csv parameters the missiles read, DAT_80479054: loaded by the stage in the DOL, 0x800efef4)
void cTaskBossEnemyMan::Init(int nStage, cTaskScene* pScene, cTaskPacman* pPacman)
{
	cTaskEnemyMan::Init(nStage, pScene, pPacman);
	cTaskBossMissile::s_Param.Load(nStage);
	cTaskCollisionMan* col = KingdomGlobal::GalagaSys->pTaskCollisionMan();
	for (int k = 0; k < 2; k++)
	{
		const u32 n = k == 0 ? BOSS_MISSILES : SP_MISSILES;
		for (u32 i = 0; i < n; i++)
		{
			cTaskBossMissile* m = new cTaskBossMissile(k == 1);
			m_BossMissile[k].push_back(m);
			ConnectChild(m);
			if (col != nullptr) col->Regist(m);
		}
	}
	for (u32 i = 0; i < BOSS_ENEMIES; i++)
	{
		cTaskBossEnemy* e = new cTaskBossEnemy();
		m_BossEnemy.push_back(e);
		ConnectChild(e);
		if (col != nullptr) col->Regist(e);   // UNVERIFIED where the DOL registers them (as the stage enemy pool)
	}
}

// DOL 0x800e7ca8: a free pool entry (vtable +0x10), set up (0x800e2e2c) and started (+0xbc = 0x800e2da8); none free: nothing
void cTaskBossEnemyMan::SpawnBossEnemy(const float vLocal[3], int nType, int nOrbit, cTaskBoss* pBoss)
{
	for (cTaskBossEnemy* e : m_BossEnemy)
	{
		if (!e->IsFree()) continue;
		e->Setup(vLocal, nType, nOrbit, pBoss);
		e->Start();
		if (std::getenv("MEGAMIX_G01DBG")) std::fprintf(stderr, "g01 boss enemy spawn type %d orbit %d local (%.1f %.1f %.1f)\n", nType, nOrbit, vLocal[0], vLocal[1], vLocal[2]);
		return;
	}
	if (std::getenv("MEGAMIX_G01DBG")) std::fprintf(stderr, "g01 boss enemy pool full\n");
}

cTaskBossMissile* cTaskBossEnemyMan::GetBossMissile(bool bSp)
{
	for (cTaskBossMissile* m : m_BossMissile[bSp ? 1 : 0])
		if (m->IsFree()) return m;
	return nullptr;
}

// devtools: --g01-autoaim (not 1:1): the flying boss missiles; the boss itself once it struggles (its lasers then hurt it)
void cTaskBossEnemyMan::MoreTargets(void (*fn)(const cObject*, void*), void* ctx) const
{
	for (int k = 0; k < 2; k++)
		for (const cTaskBossMissile* m : m_BossMissile[k])
			if (m->nState() == 1) fn(m, ctx);
	for (const cTaskBossEnemy* e : m_BossEnemy)
		if (!e->IsFree() && e->bCollide()) fn(e, ctx);
	if (cTaskBoss::s_pInstance != nullptr && cTaskBoss::s_pInstance->nState() == 4) fn(cTaskBoss::s_pInstance, ctx);
}

}
