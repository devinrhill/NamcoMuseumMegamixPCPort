#pragma once

#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g02/cTaskUnit.hpp"

// Rally-X Remix (g02) computer cars: kingdom::game::g02::cTaskEnemy (0xa30 bytes, vtable 0x80391d20; the red cars chasing the player) and the
// boss ("RED") stage variant kingdom::game::g02::cTaskEnemyRedMode (0xa30, vtable 0x80391c38; the car that collects the flags against the
// player). Ported from main.dol (no C# exists for Rally-X).
// The app ctor 0x801236bc news 0xa30 bytes per enemy (count = the map's enemy starts) and runs 0x801255a0 / 0x801251e8 (boss stage) with
// (colour, index): colour 5 (red), 6 (Pac-Man) on the boss stage. Both are the cTaskUnit part (Init, +0x1cc, the models FUN_8008120c), then the
// cTaskEnemy vtable, the +0x9d0 particle handle and cTaskEnemy::Init 0x8006d500; the boss ctor then switches to the cTaskEnemyRedMode vtable
// and clears +0xa24 / +0xa28. The map placement (FUN_800780e4) sets the start cell / direction, the speed level (0x80087c1c) and the AI type
// (+0x1e0) from the MAPOO_OBJ record.
//
// The chase (0x8006dac0): the target is the player's cell (car 0 of the app, FUN_80126918(app, 0)), for AI type 1 moved ahead of him along his
// direction by CSV_CARENEMY TARGET_NEAR / TARGET_FAR cells (within / out of SEARCH_RANGE); the car asks for the left / right turn that reduces
// the larger of the two axis distances. Out of range while the player stands still it wanders (a left turn every RAND_STABILITY +
// rand() % RAND_CHANGE frames). Near a painted (chameleon) player it brakes once it has been near for RANGE_IN_TIME frames.
// Wii space (no Unity X negation).
namespace kingdom::game::g02
{
struct cParamCarEnemy;

class cTaskEnemy : public cTaskUnit
{
public:
	// 0x801255a0 (the normal-stage enemy; the app news it with 0xa30 bytes)
	cTaskEnemy(cRallyUnitHost* pHost, int nColour, int nIndex);
	~cTaskEnemy() override;

	// vtable 0x80391d20
	void InitEnemy();                           // (+0x3c; see the .cpp)
	void Term() override;                       // (+0x40)
	bool Execute(cJetTask* pParam) override;    // (+0x18)
	u32 nUnitId() const override;               // (+0x44: index | 0x20)
	void Reset() override;                      // (+0x48)
	void Decide() override;                     // (+0x4c: the chase)
	void OnStone() override;                    // (+0x5c)
	void OnSmoke(cTaskGimmick* g) override;     // (+0x60)
	void OnOil(cTaskGimmick* g) override;       // (+0x64)
	void OnSpike(cTaskGimmick* g) override;     // (+0x68)
	float FuelUse() const override;             // (+0x94: 0, enemies never run dry)
	float NitroFuel() const override;           // (+0x98: 0)

protected:
	cTaskUnit* pPlayer() const;                 // FUN_80126918(app, 0): the app's car 0
	// the decision tail shared by both modes: turn toward a target that lies (dx, dz) cells from the car (dx = own - target, |dz| - |dx|
	// picks the axis)
	void ChaseTurn(int nAbsDiff, int dx, int dz);

	float m_fNearTime{};                        // +0x9c8 (frames near the painted player, up to RANGE_IN_TIME)
	float m_fWander{};                          // +0x9cc (frames to the next wander turn)
	kingdom::cKingdomPtkHnd m_FxConfused;       // +0x9d0 (slot 0x29: shown while it brakes at the painted player)
	cParamCarEnemy* m_pEnemyParam{};            // +0xa20 (CSV_CARENEMY)
};

class cTaskEnemyRedMode : public cTaskEnemy
{
public:
	// 0x801251e8 (the boss-stage enemy: the cTaskEnemy ctor, then this vtable, +0xa24 = +0xa28 = 0)
	cTaskEnemyRedMode(cRallyUnitHost* pHost, int nColour, int nIndex);

	// vtable 0x80391c38
	void Decide() override;                     // (+0x4c: toward the next flag)
	void OnCrash(cTaskUnit* pOther) override;   // (+0x58: the player caught it)
	// DOL 0x8006ed80 (+0x5c: nothing)
	void OnStone() override {}
	void OnSmoke(cTaskGimmick* g) override;     // (+0x60)
	void OnOil(cTaskGimmick* g) override;       // (+0x64)
	// DOL 0x8006ed90 (+0x68: nothing)
	void OnSpike(cTaskGimmick* g) override { (void)g; }
	void OnMud(cTaskGimmick* g) override;       // (+0x6c)
	void OnItemFuel(cTaskGimmick* g) override;  // (+0x70)
	void OnItemPaint(cTaskGimmick* g) override; // (+0x74)
	void OnItemSmoke(cTaskGimmick* g) override; // (+0x78)
	void OnItemArmor(cTaskGimmick* g) override; // (+0x7c)
	// DOL 0x8006efb8 (+0x80: nothing)
	void OnFlagSpecial(cTaskGimmick* g) override { (void)g; }
	// DOL 0x8006efbc (+0x84: nothing)
	void OnFlagLucky(cTaskGimmick* g) override { (void)g; }
	void OnFlag(cTaskGimmick* g) override;      // (+0x88)

protected:
	void FlagTarget(int out[2], int nAhead) const;   // (the next flag's cell difference)

	float m_fNearTime2{};                       // +0xa24 (frames near the player, up to RED_RANGE_IN_TIME)
	float m_fNitroTime{};                       // +0xa28 (frames of free nitro left)
};

// The app's car creation (FUN_801236bc: operator new(0xa30) + the ctor). bBossStage = sys 0x800c5ba8.
cTaskEnemy* CreateEnemy(cRallyUnitHost* pHost, int nColour, int nIndex, bool bBossStage);
}
