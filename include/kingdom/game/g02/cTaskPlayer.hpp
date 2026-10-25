#pragma once

#include "WiiSnd.hpp"
#include "kingdom/game/g02/cTaskUnit.hpp"

// Rally-X Remix (g02) human cars: kingdom::game::g02::cTaskPlayer (0x9f0 bytes, vtable 0x80393ab8) and the boss ("RED") stage variant
// kingdom::game::g02::cTaskPlayerRedMode (0x9f0, vtable 0x803939d0). Ported from main.dol (no C# exists for Rally-X).
// The app ctor 0x801236bc news 0x9f0 bytes per player and runs 0x80124e54 (normal stages) / 0x80124ac4 (boss stage) with (colour, index):
// colour 6 (Pac-Man; multiplayer: table 0x80378a48 per player), 5 (red) on the boss stage. Both ctors are the cTaskUnit part (Init, +0x1cc,
// the models FUN_8008120c), then the cTaskPlayer vtable and cTaskPlayer::Init 0x8007a2e8 (no chain to cTaskUnit::Init); the boss ctor then
// switches to the cTaskPlayerRedMode vtable (no Init of its own).
//
// Input (0x8007a8e0, the decision, vtable +0x4c): the pad is get_pad1(sys 0x800c5b98(index)), game bits (include/GamePad.hpp).
//   Nunchuk view (sys view word 2, the ctor default): nitro = hold Z (0x800), brake = C (0x1000), smoke = trigger A (0x20); steering = the
//   Nunchuk stick (pad +0x80 / +0x84, dead zone 0.1) toward an absolute grid direction, relative to the car's start direction (+0x240).
//   Remote views (1 / 3): nitro = hold 1 (0x10), brake = 2 (0x80), smoke = A; steering = the remote's tilt (acc z above PAD_LEFT = left, below
//   PAD_RIGHT = right; acc y below PAD_TURN = U-turn; CSV_CARPLAYER); the D-pad turns the camera (cCamera +0x6c: UP pi/2, LEFT pi, DOWN -pi/2).
// Wii space (no Unity X negation).
namespace kingdom::game::g02
{
class cCamera;
struct cParamCarPlayer;

class cTaskPlayer : public cTaskUnit
{
public:
	// 0x80124e54 (the normal-stage player; the app news it with 0x9f0 bytes)
	cTaskPlayer(cRallyUnitHost* pHost, int nColour, int nIndex);
	~cTaskPlayer() override;

	// vtable 0x80393ab8
	void InitPlayer();                          // (+0x3c; see the .cpp)
	void Term() override;                       // (+0x40)
	bool Execute(cJetTask* pParam) override;    // (+0x18)
	u32 nUnitId() const override;               // (+0x44: index | 0x10)
	void Decide() override;                     // (+0x4c: the steering decision)
	void OnTurn(int nKind) override;            // (+0x50: the pad wait after a turn)
	void OnCrash(cTaskUnit* pOther) override;   // (+0x58)
	void OnStone() override;                    // (+0x5c)
	void OnSmoke(cTaskGimmick* g) override;     // (+0x60)
	void OnOil(cTaskGimmick* g) override;       // (+0x64)
	void OnSpike(cTaskGimmick* g) override;     // (+0x68)
	void OnMud(cTaskGimmick* g) override;       // (+0x6c)
	void OnItemFuel(cTaskGimmick* g) override;  // (+0x70)
	void OnItemPaint(cTaskGimmick* g) override; // (+0x74)
	void OnItemSmoke(cTaskGimmick* g) override; // (+0x78)
	void OnItemArmor(cTaskGimmick* g) override; // (+0x7c)
	void OnFlagSpecial(cTaskGimmick* g) override;   // (+0x80)
	void OnFlagLucky(cTaskGimmick* g) override;     // (+0x84)
	void OnFlag(cTaskGimmick* g) override;          // (+0x88)
	int nView() const override;                 // (+0x9c: sys 0x800c5b88(index))
	virtual bool IsSmokeReq();                  // (+0xa0, new in cTaskPlayer: trigger A)

	// The camera's D-pad look angle (cCamera +0x6c, DOL setter 0x80128f1c). cCamera keeps it private: the integrator assigns this (e.g. after
	// adding `void SetRotYGoal(float f) { m_fRotYGoal = f; }` to cCamera). nullptr = the look is dropped.
	static void (*CameraRotYGoal)(cCamera* pCam, float fRad);

protected:
	void SteerRemote();                         // (the remote's tilt + the D-pad look; views 1 / 3)
	void SteerStick();                          // (the Nunchuk stick part of the decision; view 2)
	void ScoreFlag(cTaskGimmick* g, const char* cue);   // the shared part of the three flag hooks

	cParamCarPlayer* m_pPlayerParam{};          // +0x9c8 (CSV_CARPLAYER)
	float m_fPadWait{};                         // +0x9cc (frames before the same tilt turn may be requested again)
	float m_fPadKind{1.0f};                     // +0x9d0 (the last turn kind done, as a float: 2 left, 3 right, 4 back)
	float m_fFuelSave{};                        // +0x9d4 (the fuel at the stage end, before the bonus count)
	int m_nCount{};                             // +0x9d8 (the stage end fuel count step)
	WiiSnd::Handle m_SndCount;                  // +0x9dc (m02se_21Count)
};

// kingdom::game::g02::cTaskPlayerRedMode (the boss stage's player): no items / flags / stones / spikes / smoke screens
class cTaskPlayerRedMode : public cTaskPlayer
{
public:
	// 0x80124ac4 (the boss-stage player: the cTaskPlayer ctor, then this vtable)
	cTaskPlayerRedMode(cRallyUnitHost* pHost, int nColour, int nIndex);

	// vtable 0x803939d0
	// DOL 0x8007b794 (+0x58: nothing)
	void OnCrash(cTaskUnit* pOther) override { (void)pOther; }
	// DOL 0x8007b798 (+0x5c: nothing)
	void OnStone() override {}
	void OnSmoke(cTaskGimmick* g) override;                      // (+0x60)
	void OnOil(cTaskGimmick* g) override;                        // (+0x64)
	// DOL 0x8007b7a8 (+0x68: nothing)
	void OnSpike(cTaskGimmick* g) override { (void)g; }
	void OnMud(cTaskGimmick* g) override;                        // (+0x6c)
	// DOL 0x8007b7c4 (+0x70)
	void OnItemFuel(cTaskGimmick* g) override { (void)g; }
	// DOL 0x8007b7c8 (+0x74)
	void OnItemPaint(cTaskGimmick* g) override { (void)g; }
	// DOL 0x8007b7cc (+0x78)
	void OnItemSmoke(cTaskGimmick* g) override { (void)g; }
	// DOL 0x8007b7d0 (+0x7c)
	void OnItemArmor(cTaskGimmick* g) override { (void)g; }
	// DOL 0x8007b7d4 (+0x80)
	void OnFlagSpecial(cTaskGimmick* g) override { (void)g; }
	// DOL 0x8007b7d8 (+0x84)
	void OnFlagLucky(cTaskGimmick* g) override { (void)g; }
	// DOL 0x8007b7dc (+0x88)
	void OnFlag(cTaskGimmick* g) override { (void)g; }
	// DOL 0x8007b7e8 (+0xa0)
	bool IsSmokeReq() override { return false; }
};

// The app's car creation (FUN_801236bc: operator new(0x9f0) + the ctor). bBossStage = sys 0x800c5ba8.
cTaskPlayer* CreatePlayer(cRallyUnitHost* pHost, int nColour, int nIndex, bool bBossStage);
}
