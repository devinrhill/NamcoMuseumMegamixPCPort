#pragma once

#include "kingdom/game/g01/cAppG01GalagaGame.hpp"

// Galaga Remix (g01) stage 10, the last boss: kingdom::game::g01::cAppG01GalagaBoss (DOL vtable 0x8044a890, 0x2d8 bytes, derived from
// cAppG01GalagaGame; Init 0x801b88cc, Execute 0x801ba59c, SaveRecord 0x801ba4cc). Job 179's init builds it for stage 10 (0x800b0ed4). The scene
// is cTaskBossScene (two course segments that loop), Pac-Man cTaskBossPacman, the enemy manager cTaskBossEnemyMan (the boss missiles), the
// boss cTaskBoss (+0x2d0) and its life gauge cTaskBossLife (+0x2d4); the boss demos run on the owners (NOTES "Galaga stage 10 boss demos").
// Vtable +0x44 = 0x801ba440 (the game app has none there; no caller found): alone it memcpys the stage work over a stack temporary that holds
// pStageRecord(9) (no lasting effect); with several players it copies the multi work to multi record 9.
namespace kingdom::game::g01
{
class cTaskBoss;
class cTaskBossLife;

class cAppG01GalagaBoss : public cAppG01GalagaGame
{
public:
	cAppG01GalagaBoss() = default;               // DOL 0x801b88b8
	bool Execute(cJetTask* pParam) override;     // DOL 0x801ba59c
	void SaveRecord() override;                  // DOL 0x801ba4cc (vtable slot 13)

protected:
	cTaskScene* CreateScene() override;          // 0x801b9960 + 0x800d1638 (two segments)
	cTaskPacman* CreatePacman() override;        // 0x801b9ef4 + 0x800d0cec
	cTaskEnemyMan* CreateEnemyMan() override;    // 0x801b9404 (cTaskBossEnemyMan)
	void CreateBoss() override;                  // 0x801ba078 + 0x800cebbc (cTaskBoss +0x2d0), 0x801ba1f4 (cTaskBossLife +0x2d4)
	bool IsBossDefeated() const;                 // DOL 0x800cfb54 on the boss (+0x2d0)

private:
	bool m_bBgm{};
	cTaskBoss* m_pBoss{};                        // +0x2d0
	cTaskBossLife* m_pBossLife{};                // +0x2d4
};
}
