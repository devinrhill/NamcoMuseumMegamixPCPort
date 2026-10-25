#pragma once

#include "kingdom/game/g11/cTaskGrobdaEnemy.hpp"

// Grobda Remix (g11) the Fortres (type 0x20000006, 4 x 4, radius 2; Init 0x80028fa0, Execute 0x8002936c, AI 0x80029798, ring 0x80029f64, Resolve 0x8002a570) and the Frossors
// (Green 0x20000004 / Brown 0x20000005; Init 0x8002a708 + 0x8002c28c / 0x8002be0c, AI 0x8002acc8, danger 0x8002b578, shoot 0x8002b824). Params: EnmFortresParam.csv (rows 0 speed,
// 1 shot speed, 2 wait ms, 3 move ms, 4 min move ms, 5 ring interval, 6 aimed interval, 7 HP, 8 score, 9 combo bonus) and EnmFrossor{Green,Brown}Param.csv (rows 0 speed, 1 speed
// over wrecks, 2 shot speed, 3 shot count, 4 move ms, 5 min move ms, 6 turn ms, 7 wall wait ms, 8 shot interval, 9 interval when seen, 10 dodge speed, 11 dodge ms, 12 min dodge ms,
// 13 HP, 14 score, 15 combo bonus; three level columns).
namespace kingdom::game::g11
{
class cTaskGrobdaShotEnemy;

class cTaskGrobdaEnemyFortres : public cTaskGrobdaEnemy
{
public:
	cTaskGrobdaEnemyFortres();                            // DOL 0x80028ec4 + Init
	void SetStart(int level, int delay, int angleDeg) override;   // DOL 0x8002947c
	bool HasShots() override;                             // DOL 0x80029700
	bool Execute(cJetTask* pParam) override;              // DOL 0x8002936c
	void Resolve(int a, int b) override;                  // DOL 0x8002a570
protected:
	void Phase2() override;   // 0x8002a1e8
	void Phase3() override;   // 0x8002a260
	void Phase4() override;   // 0x8002a3f4
	void Phase5() override;   // 0x8002a478
private:
	void Anim(int mode);      // DOL 0x80029d38: 0 WAIT 1 RUN 2 LEVITATION 3 LAND (chr), 4 HIT 5 HIT_DAMAGE (clr), 6 DIE
	void Ai();                // DOL 0x80029798
	void FireRing();          // DOL 0x80029f64
	struct sPar { float speed{}, shotSpeed{}; uint wait{}, move{}, moveMin{}, ring{}, aimed{}; } m_Par[3];   // +0xb94 .. (0x1c each)
	float m_vDir[4]{};        // +0xbf0
	float m_vOffset[4]{};     // +0xc10 the cannon node
	int m_nAnim{-1};          // +0xb90
	int m_nBurst{0};          // +0xc00
	cTaskGrobdaShotEnemy* m_pRing[16]{};    // +0xc20 (type 0x2000004)
	cTaskGrobdaShotEnemy* m_pAimed[8]{};    // +0xc60 (type 0x2000007)
	std::string m_sAnim[2];
};

class cTaskGrobdaEnemyFrossor : public cTaskGrobdaEnemy
{
public:
	cTaskGrobdaEnemyFrossor(uint type, bool brown);       // DOL 0x8002bd30 / 0x8002c1b0 + Init
	void SetStart(int level, int delay, int angleDeg) override;   // DOL 0x8002a840
	bool HasShots() override;                             // DOL 0x8002ab08
	bool Execute(cJetTask* pParam) override { return cTaskGrobdaEnemy::Execute(pParam); }   // DOL 0x8002c128 / 0x8002c5a8
protected:
	void Phase2() override;   // 0x8002b9a4
	void Phase3() override;   // 0x8002ba1c
	void Phase4() override;   // 0x8002bb8c
	void Phase5() override;   // 0x8002bc10
private:
	void Anim(int mode);      // DOL 0x8002ab68: 0 WAIT 1 RUN 2 SHOT 3 DIE
	void Ai();                // DOL 0x8002acc8
	bool Danger();            // DOL 0x8002b578: a player's shot line passes close: picks the dodge direction
	void Shoot();             // DOL 0x8002b824
	struct sPar { float speed{}, speedDebris{}, shotSpeed{}; uint shotCount{}, move{}, moveMin{}, turn{}, wallWait{}, interval{}, intervalSeen{}; float dodgeSpeed{}; uint dodge{}, dodgeMin{}; } m_Par[3];   // 0x34 each
	float m_vDir[4]{};        // +0xc40 the facing (16 directions)
	float m_vMoveDir[4]{};    // +0xc50 the walking direction
	int m_nAnim{-1};          // +0xc2c
	int m_nTurnSteps{0};      // +0xc64
	int m_nDirIdx{0};         // +0xc68 current 16-direction index of the facing
	int m_nDirTarget{0};      // +0xc6c
	int m_nTurnDir{0};        // +0xc70
	int m_nDodgeIdx{0};       // +0xc60
	cTaskGrobdaShotEnemy* m_pShot[8]{};     // +0xc74
	std::string m_sAnim;
};
}
