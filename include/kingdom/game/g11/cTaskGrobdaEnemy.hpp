#pragma once

#include <string>
#include <vector>

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g11/cTaskGrobdaModel.hpp"

namespace kingdom { class cKingdomPtkHnd; }

// Grobda Remix (g11) enemies, ported 1:1 from main.dol (no C# exists for Grobda).
//   cTaskGrobdaEnemy (vtable 0x8038ad00, base cTaskGrobdaModel; ctor 0x80026e54(type), Init 0x80026fa0, Term 0x800271e8, Execute 0x800272ec, Resolve 0x8002777c, damage 0x80027488)
//     Two state words: the PHASE (+0xb6c, Execute's switch; enter flag +0xb70) and the object flag word (cGrobdaObject +0x93c = the "pose", set by the virtual hooks +0x98..+0xb4 =
//     SetFlags(0..7) and read by IsFlags(0..7) +0xb8..+0xd4). Phases: 0 wait for the stage to start (0x80027bc0) -> 2, 1 drop in from the sky (0x80027c48), 2 active, 3 destroyed (the
//     explosion), 4 wreck (debris, list 4), 5 wreck leaves, 6 the stage ended, 7 gone (flag word 7 once no shot is left, 0x80028234).
//   cTaskGrobdaEnemyTank (0x8002c704 ...; Purple 0x8002e7f0, Orange 0x8002dff4, Yellow 0x8002ec98): the walking tank. Params EnmTank*Param.csv (labels in Japanese): see sParam.
//   Types 0x2000001 (Purple tank) .. 0x2000003 (Yellow tank), 4 / 5 Frossor (green / brown), 6 Fortres (NOT PORTED: 4 / 5 / 6).
namespace kingdom::game::g11
{
class cTaskGrobdaShotEnemy;

// the eight and sixteen directions the DOL's tables hold (0x8038ab60 / 0x8038abe0; unit vectors on XZ, 0 = +z, clockwise seen from above)
namespace grobda_dir
{
const float* Dir8(int i);                     // 0x800282ec
const float* Dir16(int i);                    // 0x80028314
float AngleOf(const float* v);                // 0x80300950(x, z) * (pi / 128) = atan2(x, z)
bool IsNonZero(const float* v);               // 0x801941c4
// 0x8002842c (8 directions, 45 degrees) / 0x800286cc (16 directions, 22 degrees): the direction `from` turned by the part t / total of one step (dir 0: clockwise, 1: back)
float Turn8(int dir, int from, uint t, uint total, float out[4]);
float Turn16(int dir, int from, uint t, uint total, float out[4]);
int Nearest8(const float* v);                 // the best dot product among the first 8 (0x80028a8c)
int Nearest16(const float* v);                // 0x80028bd0
}

class cTaskGrobdaEnemy : public cTaskGrobdaModel
{
public:
	explicit cTaskGrobdaEnemy(uint type);                 // DOL 0x80026e54
	~cTaskGrobdaEnemy() override;
	bool Execute(cJetTask* pParam) override;              // DOL 0x800272ec
	void Resolve(int a, int b) override;                  // DOL 0x8002777c

	virtual void SetStart(int level, int delay, int angleDeg) = 0;   // vtable +0x90 (Tank 0x8002c830)
	virtual bool HasShots() { return false; }             // vtable +0x94 (Tank 0x8002cad0)
	void SetPhase(int p) { m_nPhase = p; m_bEnter = true; }         // DOL 0x80027b38
	void Begin() { SetPhase(1); }                          // DOL 0x80027464 (a new wave drops in)
	void Leave() { SetPhase(5); }                          // DOL 0x80027474 (the wreck of the last wave leaves)
	bool IsReady() const { return nFlags() == 4u; }        // vtable +0xc8 (0x80028df0): the wreck pose
	bool IsGone() const { return nFlags() == 7u; }         // vtable +0xd4 (0x80028d6c)
	bool IsFlags(uint n) const { return nFlags() == n; }

protected:
	// the phases (vtable +0xd8 .. +0xf4)
	virtual void Phase0();   // 0x80027bc0
	virtual void Phase1();   // 0x80027c48
	virtual void Phase2();   // 0x80027f24
	virtual void Phase3();   // 0x80027fb8
	virtual void Phase4();   // 0x800280b0
	virtual void Phase5();   // 0x800281a8
	virtual void Phase6();   // 0x800282a8
	virtual void Phase7();   // 0x80028234
	bool TakeEnter(bool clear);                           // DOL 0x80027b48
	void SetMode(int m) { m_nModePrev = m_nMode; m_nMode = m; m_bModeChanged = true; }   // DOL 0x80027b84
	bool TakeModeChanged() { bool b = m_bModeChanged; m_bModeChanged = false; return b; }   // DOL 0x80027b9c
	virtual void Damage(std::vector<const sGrobdaHit*>& hits);   // DOL 0x80027488
	void SwitchModel(int idx);                            // DOL 0x80036b68
	void SetLevel(int level, int delay) { m_nLevel = level > 2 ? 2 : level; m_nDelay = delay; }   // DOL 0x8002744c

	int m_nPhase{0};          // +0xb6c
	bool m_bEnter{true};      // +0xb70
	int m_nMode{0};           // +0xb74 (the action)
	int m_nModePrev{0};       // +0xb78
	bool m_bModeChanged{};    // +0xb7c
	bool m_bTouchMover{};     // +0xb7d: a mover / wall touched it this frame (kinds 1, 2)
	bool m_bTouchPickup{};    // +0xb7e: it stands on a wreck (kind 4)
	int m_nLevel{0};          // +0xb80
	int m_nDelay{0};          // +0xb84 (ms before the drop)
	kingdom::cKingdomPtkHnd* m_pSpark{};      // +0xb88 (slot 0x20)
	kingdom::cKingdomPtkHnd* m_pFragment{};   // +0xb8c (slot 0x1f)
	kingdom::cKingdomCSV m_Csv;               // +0xb4c
	int& Hp() { return m_n940; }              // +0x940
	int& BaseScore() { return m_n944; }       // +0x944
	int& ComboStep() { return m_n948; }       // +0x948

protected:
	void ReleaseTerm() override;   // DOL 0x800271e8 (Term: the spark / fragment handles)
};

// one level of EnmTank*Param.csv: rows 0..10 of a column
struct sTankParam
{
	float speed{}, speedDebris{}, shotSpeed{};   // rows 0..2
	uint shotCount{}, moveTime{}, moveMin{}, turnTime{}, wallWait{}, shotInterval{}, shotIntervalSeen{}, turretTime{};   // rows 3..10 (ms)
};

class cTaskGrobdaEnemyTank : public cTaskGrobdaEnemy
{
public:
	cTaskGrobdaEnemyTank(uint type, int model);           // DOL 0x80026e54 + the kind's Init
	void SetStart(int level, int delay, int angleDeg) override;   // DOL 0x8002c830
	bool HasShots() override;                             // DOL 0x8002cad0
	bool Execute(cJetTask* pParam) override;              // DOL 0x8002c7d0 (-> Enemy::Execute)
	void Resolve(int a, int b) override;                  // Orange / Yellow 0x8002e4c4 / 0x8002f168 (the shield flash)

protected:
	void Phase1() override;   // 0x8002da88
	void Phase2() override;   // 0x8002dbd8 (Orange / Yellow 0x8002e5d4 / 0x8002f278)
	void Phase3() override;   // 0x8002dc50 (Orange / Yellow 0x8002e68c / 0x8002f330)
	void Phase4() override;   // 0x8002dd74
	void Phase5() override;   // 0x8002ddf8

private:
	void Anim(int mode);            // DOL 0x8002cb30
	void Ai();                      // DOL 0x8002cdc0
	void Cannon();                  // DOL 0x8002d548
	void Shoot();                   // DOL 0x8002d830
	void CreateShots(uint type);    // DOL 0x8002ca34
	void LoadParams(const char* csv);
	void SetRotY(float a);          // the body's yaw (vtable +0x2c, element 1)

	sTankParam m_Par[3];            // +0xb90
	float m_vDir[4]{};              // +0xc20 the body's direction
	int m_nDir{0};                  // +0xc30
	int m_nDirTarget{0};            // +0xc34
	int m_nTurn{2};                 // +0xc38 (0 clockwise, 1 counter clockwise, 2 none)
	cTaskGrobdaShotEnemy* m_pShot[8]{};   // +0xc3c
	int m_nAnim{-1};                // +0xc5c
	bool m_bTurret{};               // +0xc60: Orange / Yellow aim the cannon on their own
	int m_nTurretState{0};          // +0xc64
	float m_vTurret[4]{};           // +0xc70
	int m_nTurretDir{0};            // +0xc80
	int m_nTurretTarget{0};         // +0xc84
	int m_nTurretTurn{0};           // +0xc88
	kingdom::cKingdomPtkHnd* m_pShield{};   // +0xc8c (Orange / Yellow, slot 0x1e)
	std::string m_sAnim[2][2];      // the registered clips (blend 0 / 1, chr / other)
	uint m_nKindType{};

protected:
	void ReleaseTerm() override;   // Orange / Yellow Term 0x8002e404 / 0x8002f0a8 (the shield handle)
};
}
