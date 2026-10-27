#pragma once

#include <map>
#include <string>

#include "WiiSnd.hpp"
#include "common.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cKingdomSndActor.hpp"
#include "kingdom/cTaskModel.hpp"

namespace kingdom { class cKingdomModel; }

// Rally-X Remix (g02) cars: kingdom::game::g02::cTaskUnit (0x9c8 bytes, vtable 0x80393d50, a cTaskModel), base of cTaskPlayer (0x9f0, vtable
// 0x80393ab8), cTaskPlayerRedMode (0x9f0, 0x803939d0), cTaskEnemy (0xa30, 0x80391d20) and cTaskEnemyRedMode (0xa30, 0x80391c38).
// Ported from main.dol (no C# exists for Rally-X). The app builds the cars (player 0x80124e54 / boss stage 0x80124ac4, enemies 0x801255a0 /
// boss stage 0x801251e8): cTaskModel ctor, the cTaskUnit vtable, cTaskUnit::Init (vtable +0x3c), +0x1cc = index, the models FUN_8008120c,
// then the derived vtable and the derived Init (which does NOT chain to cTaskUnit::Init).
//
// The MAPOO grid: the car's cell is ((int)(x +- 0.5) + 4, (int)(z +- 0.5) + 8) (round half away from zero); cTaskMap::Cell = the MAP value
// (2 road, 4 stone, 5 oil, 6 / 7 mud, 8 spike), cTaskMap::Attr bit 0 = road (FUN_80079aec). Directions (+0x228 / +0x22c / +0x240): 0 = heading
// 180 deg (moves -z), 1 = 0 deg (+z), 2 = 270 deg (-x), 3 = 90 deg (+x); the move is pos += RotY(heading) * (0, 0, speed) (MTX44, column
// vectors: RotY(a) * (0, 0, 1) = (sin a, 0, cos a)). Turn kinds (+0x1ec, FUN_80082120): 1 straight, 2 left (heading +90 deg, 0x800862f0),
// 3 right (-90 deg, 0x8008678c), 4 U-turn (+180 deg, 0x80086c50). Steering state (+0x1f0): 1 straight, 2 left, 3 right, 4 back.
// Spin kinds (+0x260, FUN_80086e50, a higher kind replaces a lower one): 1 oil, 2 spike, 3 smoke, 4 car clash, 5 stone (enemy), 6 crash.
// Unit ids (vtable +0x44): player = index | 0x10, enemy = index | 0x20, base 0.
//
// Wii space (no Unity X negation). The DOL unit reaches the app through its parent (+0x14); natively the app implements cRallyUnitHost.
namespace kingdom::game::g02
{
class cCamera;
class cTaskGimmick;
class cTaskMap;
class cTaskUnit;
class cRallyModelShadow;
struct cParamCar;
struct sConsCar;

// The app side of the cars (DOL: the unit's parent cAppG02RallyXGame, reached through +0x14, and the map functions the port's cTaskMap does not
// have yet). Every default is the neutral value; the integrator overrides them in cAppG02RallyXGame (see the agent report).
class cRallyUnitHost
{
public:
	virtual ~cRallyUnitHost() = default;
	virtual bool IsUnitDead() const { return false; }      // FUN_80126928 (the app's getter the car Executes test first: always 0 in the DOL)
	virtual int nUnitGameState() const { return 0; }        // app +0x2e4 (the game flow state 0..0xc; 0x800881c4 compares it)
	virtual bool IsUnitPause() const { return false; }      // app +0x1b4 (cAppKingdom pause byte)
	virtual bool IsUnitStageEnd() const { return false; }   // app +0x284 (stage end: the fuel bonus count runs)
	virtual int nUnitStageEndId() const { return 0; }       // app +0x288 (unit id that ends it)
	// FUN_80126918: app +0x2c4 + 4 i (7 slots: players, then enemies)
	virtual cTaskUnit* pUnit(int i) const { (void)i; return nullptr; }
	virtual cTaskMap* pUnitMap() const { return nullptr; }  // app +0x298
	virtual void UnitAddScore(int add) { (void)add; }       // FUN_80126a10(app, add, 0): +0x300 += add (max 999999)
	virtual bool UnitStageEnd(bool b, int unitId) { (void)b; (void)unitId; return false; }   // FUN_80126a54 (lucky flag)
	virtual void UnitRedCrash() {}                          // FUN_80126ac8: next state 7 (clear) + BGM fade (the red car crashed)
	virtual void UnitMultiHit(int index) { (void)index; }   // FUN_801272c0 -> console FUN_800666e8 (multiplayer only)
	// map parts not in the port's cTaskMap yet (Wave 2: flags / smoke / gimmicks)
	virtual bool MapDropFlag(const float* pos, int dir) { (void)pos; (void)dir; return false; }   // FUN_80079b5c(map, 1, pos, dir)
	virtual bool MapPutSmoke(int unitId, const float* pos, int index) { (void)unitId; (void)pos; (void)index; return false; }   // FUN_80079c1c(map, id, 3, pos, idx)
	virtual bool MapSmokeFree(int unitId) { (void)unitId; return true; }   // FUN_80079fc4 -> FUN_8007c59c (no live smoke of this unit)
	virtual int MapFlagTotal() const { return 0; }          // FUN_80079ad0 (flags of the stage)
	virtual bool MapNextFlag(int index, float out[4]) const { (void)index; (void)out; return false; }   // FUN_80079fcc(map, index): the index-th flag (the red car passes its +0x2d0)
	virtual void MapContact(cTaskUnit* u, int x, int z) { (void)u; (void)x; (void)z; }   // FUN_80079b50 -> gimmick man FUN_80073a2c -> OnGimmick
	// effects of the app's g02::cTaskEffect (+0x1bc, not ported)
	virtual void FxFlagScore(int count, int special, const Vector4& pos) { (void)count; (void)special; (void)pos; }   // FUN_800881dc -> FUN_8006cfa0
	virtual void FxRedFlag(const Vector4& pos) { (void)pos; }   // FUN_8006d114
};

class cTaskUnit : public kingdom::cTaskModel
{
public:
	// the shared part of the DOL car ctors (0x80124e54 / 0x801255a0 up to the derived vtable): Init, +0x1cc = index, models FUN_8008120c
	cTaskUnit(cRallyUnitHost* pHost, int nColour, int nIndex);
	~cTaskUnit() override;

	// vtable 0x80393d50
	void InitUnit();                            // 0x8007e358 (+0x3c; the param is cParamCar::Load)
	void Term() override;                       // 0x8007f638 (+0x40)
	bool Execute(cJetTask* pParam) override;    // 0x8007f714 (+0x18; called by the derived Executes)
	// DOL 0x800662c0 (+0x44)
	virtual u32 nUnitId() const { return 0; }
	virtual void Reset();                       // 0x8007eb80 (+0x48)
	// DOL 0x80085f4c (+0x4c)
	virtual void Decide() {}
	// DOL 0x8006f2f8 (+0x50; a requested turn was possible)
	virtual void OnTurn(int nKind) { (void)nKind; }
	virtual float FuelCost() const;             // 0x80087c9c (+0x54)
	// DOL 0x8007003c (+0x58; touched by a car of the other side)
	virtual void OnCrash(cTaskUnit* pOther) { (void)pOther; }
	// DOL 0x8008211c (+0x5c)
	virtual void OnStone() {}
	// DOL 0x800862e4 (+0x60)
	virtual void OnSmoke(cTaskGimmick* g) { (void)g; }
	// DOL 0x800862e8 (+0x64)
	virtual void OnOil(cTaskGimmick* g) { (void)g; }
	// DOL 0x800862ec (+0x68)
	virtual void OnSpike(cTaskGimmick* g) { (void)g; }
	// DOL 0x80070038 (+0x6c)
	virtual void OnMud(cTaskGimmick* g) { (void)g; }
	// DOL 0x80070034 (+0x70)
	virtual void OnItemFuel(cTaskGimmick* g) { (void)g; }
	// DOL 0x80070030 (+0x74)
	virtual void OnItemPaint(cTaskGimmick* g) { (void)g; }
	// DOL 0x8007002c (+0x78)
	virtual void OnItemSmoke(cTaskGimmick* g) { (void)g; }
	// DOL 0x80070028 (+0x7c)
	virtual void OnItemArmor(cTaskGimmick* g) { (void)g; }
	// DOL 0x80070024 (+0x80)
	virtual void OnFlagSpecial(cTaskGimmick* g) { (void)g; }
	// DOL 0x80070020 (+0x84)
	virtual void OnFlagLucky(cTaskGimmick* g) { (void)g; }
	// DOL 0x8007001c (+0x88)
	virtual void OnFlag(cTaskGimmick* g) { (void)g; }
	// DOL 0x8006f2f4 (+0x8c)
	virtual void OnCellChange(const int nNew[2], const int nOld[2]) { (void)nNew; (void)nOld; }
	// DOL 0x8006f2f0 (+0x90)
	virtual void OnCellCentre() {}
	// DOL 0x8007b7e0 (+0x94)
	virtual float FuelUse() const { return m_fFuelUse; }
	virtual float NitroFuel() const;            // 0x80087998 (+0x98)
	// DOL 0x80088684 (+0x9c; players: the sys per-player view / control word)
	virtual int nView() const { return 2; }

	// gimmick contact (0x80085f50, called by the gimmick man FUN_80073a2c): dispatches by the gimmick type (cTaskMap.hpp table);
	// nSmokeOwner = the smoke gimmick's owner id (FUN_800b1d00, type 0 only)
	bool OnGimmick(cTaskGimmick* g, int nType, int nSmokeOwner);

	// the app / map / console interface
	void CreateCamera(const s32* pScnLayout);   // 0x80081cec (app ctor 0x801236bc; only when +0x1cc < 4)
	void CameraStart(int nWhich, int bFlag);    // 0x80081dd0 (0 game view, 1 the opening turn, 2 to game)
	void SetStart(int nCellX, int nCellZ, int nDir);   // the unit part of the map's placement FUN_80078260 (world = cell - (4, 8))
	void SetDir(int nDir);                      // 0x80087c0c
	void SetSpeedLevel(int nLevel);             // 0x80087c1c (enemies: MAPOO_OBJ +0x14)
	void SetAiType(int n) { m_nAiType = n; }    // +0x1e0 (enemies: MAPOO_OBJ +0x0c)
	void StartEngine();                         // 0x8008868c (engine loop by the axis of the direction)
	void Clear();                               // 0x80088a70 (stage clear: camera orbit, smile / win clip)
	// DOL 0x80088ba0
	void ResetNitro() { m_fNitro = 0.0f; }
	// DOL 0x800884c8 (+0x2c8 == 1)
	int nArmorLeft() const { return m_nArmor == 1 ? 1 : 0; }
	// DOL 0x80088664 (+0x2c4 > 0)
	bool IsMud() const { return m_fMud > 0.0f; }
	// DOL 0x80088498 (+0x2c0 > 0)
	bool IsChameleon() const { return m_fChameleon > 0.0f; }
	// DOL 0x80088458 (+0x2bc > 0)
	bool IsFuelBurst() const { return m_fFuelBurst > 0.0f; }
	// DOL 0x80088438 (+0x1d8: the crash; the app's "miss" test)
	bool IsStop() const { return m_bStop; }
	// DOL 0x80088140
	bool IsSpeedZero() const { return m_fSpeed == 0.0f; }
	// DOL 0x80087950
	bool HasFuel() const { return m_fFuel > 0.0f || m_fFuelBurst > 0.0f; }
	int nNitroState() const;                    // 0x80088a40 (0 nitro, 1 normal, 2 brake)
	static int Rock(const cTaskUnit* a, const cTaskUnit* b);   // 0x80088934 (multiplayer clash winner: 0 a, 1 both, 2 b)
	float fRelocation(int i) const;             // 0x80088bac / 0x80088bb8 / 0x80088bc4 / 0x80088bd0 (cParamCar +0xe0..)
	// DOL 0x80088138 (+0x2dc)
	cCamera* pCamera() const { return m_pCamera; }
	int nIndex() const { return m_nIndex; }     // +0x1cc
	int nColour() const { return m_nColour; }   // +0x1d0
	int nFlags() const { return m_nFlags; }     // +0x2d0
	void nFlags(int n) { m_nFlags = n < 0 ? 0 : n; }
	float fFuel() const { return m_fFuel; }     // +0x2ac (0..100)
	float& fFuelRef() { return m_fFuel; }       // +0x2ac (the app's stage-clear bonus counts it down, 0x80126b18)
	float fStartWait() const { return m_fStartWait; }   // +0x1d4
	const float* vPos() const { return m_vPos; }        // +0x200
	const float* vDir() const { return m_vDir; }        // +0x230 ([1] = heading, radians)
	int nDir() const { return m_nDir; }         // +0x22c
	int nCellX() const { return CellX(); }      // the MAPOO cell of +0x200 (the enemies read the player's)
	int nCellZ() const { return CellZ(); }
	int nSmokeStock() const { return m_nSmokeStock; }   // +0x2cc
	const cParamCar* pParam() const { return m_pParam; }   // +0x2d4
	cRallyUnitHost* pHost() const { return m_pHost; }

protected:
	// the unit step and its parts
	void Step();                                // 0x80084d6c
	void StepMove();                            // 0x80084d1c (the movement block of the step, the boss stage's state 2)
	void Fuel();                                // 0x800837b4
	void Speed();                               // 0x800838bc
	void Move();                                // 0x80082680
	void CellCentre();                          // 0x80081f20
	int Bump(int nKind, int bStone);            // 0x80082120 (try the turn kind; returns the kind done)
	void SpinStep();                            // 0x800835a0
	void Heading();                             // 0x8008241c
	void Collision();                           // 0x80083a14
	bool CanLeft(int bStone) const;             // 0x80086f58
	bool CanRight(int bStone) const;            // 0x800870dc
	bool CanBack(int bStone) const;             // 0x80087260
	void TurnLeft();                            // 0x800862f0
	void TurnRight();                           // 0x8008678c
	void TurnBack();                            // 0x80086c50
	bool TurnBackHalf();                        // 0x800873e4 (U-turn, cell progress mirrored)
	bool Spin(int nKind);                       // 0x80086e50
	bool SpinStart(int nKind);                  // 0x80088004
	bool Nitro();                               // 0x80087424
	bool NitroFree();                           // 0x80087744 (the enemies' nitro: no fuel cost)
	bool BrakeOn();                             // 0x800879a4
	bool BrakeOff();                            // 0x80087a3c
	bool Smoke();                               // 0x80087bd4
	bool IsSmokeFree();                         // 0x80088158
	bool DropFlag();                            // 0x800881a0
	void ReqLeft();                             // 0x80087ca8
	void ReqRight();                            // 0x80087cc8
	void ReqBack();                             // 0x80087ce8
	bool Want(int nDir);                        // 0x80087d04 / 0x80087dc4 / 0x80087e84 / 0x80087f44 (absolute direction 0 / 1 / 2 / 3)
	bool Crash();                               // 0x80088264
	void Bounce(cTaskUnit* pOther);             // 0x80084984 (car clash: both turn back and spin)
	void SmokeHit();                            // 0x80084c58
	bool ArmorHit();                            // 0x800884dc (true = the hit counts; armor absorbs it)
	void AddScore(int n);                       // 0x800881b8
	bool IsState(int n) const;                  // 0x800881c4
	void FlagFx(int nCount, int nSpecial, const Vector4& pos);   // 0x800881dc (single player only)
	void FuelBurstOn();                         // 0x80088440
	void FuelBurstStep();                       // 0x80088450
	void ChameleonOn();                         // 0x8008847c
	void ChameleonStep();                       // 0x8008848c
	// DOL 0x800884bc
	void ArmorOn() { m_nArmor = 1; }
	void MudOn();                               // 0x800885bc
	void MudStep();                             // 0x80088658

	// clips / faces
	bool IsSpinClip(int nPhase);                // 0x8007d0cc (the L / R spin clip of the phase 0 start, 1 loop, 2 end is attached)
	bool Face(int nFace);                       // 0x8007d328 (0 normal, 1 blink, 2 spin, 3 smile)
	void SpinClip(int nSide, int nPhase);       // 0x8007d74c
	bool WinClip();                             // 0x8007dd14
	bool LoseClip();                            // 0x8007de2c
	void BlendNormal(float rate);               // 0x8007ded0 (colours 1..5)
	void BlendNormalPac(float rate);            // 0x8007e0d0 (colour 6)
	void Blend(float rate) { if (m_nColour == 6) BlendNormalPac(rate); else BlendNormal(rate); }
	void ClipsNormal();                         // the NORMAL / LEFT / RIGHT clips in slots 0..2 (inlined everywhere in the DOL)
	void BlendTurn(float rate, bool bLeft, bool bHeadByColour);   // the slot 1 / 2 weight steps of the tilt and turn functions
	bool IsAttached(kingdom::cKingdomModel* m, const char* name, uint slot) const;   // 0x8007ca24 (natively remembered per kind)
	void Attach(kingdom::cKingdomModel* m, const char* name, uint slot);   // "if (!0x8007ca24) 0x8001a544(m, slot, crc, 0, 1)"
	void Release(kingdom::cKingdomModel* m, uint slot, int nKind);         // the ReleaseAnm* calls (keeps the bookkeeping)
	void Models(int nColour);                   // 0x8008120c
	kingdom::cKingdomModel* pCar() { return pModel(0); }
	kingdom::cKingdomModel* pBody();            // car +0x60 (expand 0)
	kingdom::cKingdomModel* pHead();            // body +0x60 (expand 0)
	kingdom::cKingdomModel* pHeadSpin();        // body +0x6c (expand 1, the red heads' spin model)
	bool HasHead() const { return m_nColour == 5 || m_nColour == 6; }   // (colour < 7 && 4 < colour)
	int CellX() const;                          // (int)(x +- 0.5) + 4
	int CellZ() const;                          // (int)(z +- 0.5) + 8
	int MapCell(int x, int z) const;            // cTaskMap::Cell (MAPOO_MAP value; 0 without a map)
	bool MapRoad(int x, int z) const;           // FUN_80079aec (cTaskMap::Attr bit 0)
	void SeRemote(const char* cue);             // a player's unit (id & 0x10): FUN_80025b6c(handles, sys 0x800c5b98(index), cue, 0)
	void Se(const char* cue);                   // FUN_80129d78(handle, cue, 0, actor +0x378) with a temporary handle (3D actor NOT PORTED)
	void Effects();                             // the particle part of Execute 0x8007f714
	Vector4 NodePos(const char* node, const Vector4& add);   // model pos + RotY(rot.y) * (node translation + add) (FUN_8001dd54 + MTX44)
	static void Fx(kingdom::cKingdomPtkHnd& h, uint nSlot, bool bOnetime, bool bDepend, bool bClip, const Vector4& pos);   // FUN_80021a9c
	static void FxOff(kingdom::cKingdomPtkHnd& h, bool b = false);   // FUN_8008a508(h, b)
	bool IsFrozen() const;                      // app +0x1b4 || app +0x284 || get_pad1(0) & 0x400 (HOME)

public:
	// the shared counters (FUN_800c53c0 .. FUN_800c54ac; true when the target is reached, the value is clamped to it)
	static bool Up(float target, float step, float& v);     // 0x800c53c0
	static bool Down(float target, float step, float& v);   // 0x800c5450 (step < 0)
	static bool Inc1(float target, float& v);               // 0x800c53ec (+1)
	static bool Dec1(float target, float& v);               // 0x800c547c (-1)
	static bool Dec1Floor(float& v);                        // 0x800c54ac (-1 down to -99999)
protected:
	static int DirDeg(int nDir);                            // the direction switch inlined everywhere: 0 -> 180, 1 -> 0, 2 -> 270, 3 -> 90
	static int CellOf(float v) { return (int)(v + (v >= 0.0f ? 0.5f : -0.5f)); }   // FLOAT_8047aa48 / 8047aa4c
	// sys (r13 - 0x5700) parts the port's cG02RallyXSys has no getter for yet: the DOL defaults of its ctor 0x800c54e0 (NOT PORTED getters)
	static bool IsMultiStage();                 // 0x800c5ca0 (world 0xb)
	static bool IsBossStageNow();               // 0x800c5ba8
	static int SysView(int nIndex);             // 0x800c5b88: sys +0x14 + 0x14 i (default 2)
	static int SysPad(int nIndex);              // 0x800c5b98: sys +0x1c + 0x14 i (default = the index)
	static int SysPattern();                    // 0x800c5990: sys +0x04 (default 1)
	static bool SysWinner(int nIndex);          // 0x800c58b0: sys +0x10 bit i (default 0)

	cRallyUnitHost* m_pHost{};                  // the DOL's parent (+0x14)
	int m_nIndex{};                             // +0x1cc
	int m_nColour{};                            // +0x1d0 (1 blue, 2 green, 3 yellow, 4 white, 5 red, 6 Pac-Man)
	float m_fStartWait{};                       // +0x1d4 (frames before the car may move)
	bool m_bStop{};                             // +0x1d8 (crashed: the speed goal stays 0)
	float m_fBlink{};                           // +0x1dc (invincible blink after the armor took a hit)
	int m_nAiType{};                            // +0x1e0 (enemies: 0 chase the player, 1 aim ahead of him)
	float m_fCellPos{1.0f};                     // +0x1e4 (progress to the next cell centre, 0..1)
	float m_fHitClear{};                        // +0x1e8 (frames since the last car hit; 180 clears the hit list)
	int m_nReqTurn{1};                          // +0x1ec (turn kind wanted at the next cell centre)
	int m_nSteer{1};                            // +0x1f0
	float m_vPos[4]{0.0f, 0.0f, 0.0f, 1.0f};    // +0x200
	float m_vOffset[4]{0.0f, 0.0f, 0.0f, 1.0f}; // +0x210 (x the tilt's lateral shift, y the spike bump)
	float m_fTurnRest{};                        // +0x220 (angle of the turn still to do)
	float m_fTurnTotal{};                       // +0x224
	int m_nDirOld{};                            // +0x228
	int m_nDir{};                               // +0x22c
	float m_vDir[4]{};                          // +0x230 (0, heading, 0, 0)
	int m_nStartDir{};                          // +0x240 (the stick's reference direction)
	float m_vTilt[4]{0.0f, 0.0f, 0.0f, 1.0f};   // +0x250 (y: the steering roll, -572.96 .. 572.96; read by nothing found)
	int m_nSpin{};                              // +0x260
	float m_fSpinTime{};                        // +0x264
	float m_fSpinRot{};                         // +0x268
	int m_nSpinCell[2]{};                       // +0x270
	int m_nHit[7]{};                            // +0x278 (unit ids touched since the last clear)
	bool m_bCrashEnd{};                         // +0x294 (hit a stone during the clear run)
	float m_fSpeedGoal{};                       // +0x298
	float m_fSpeed{};                           // +0x29c
	int m_nBrake{};                             // +0x2a0
	float m_fNitro{};                           // +0x2a4 (nitro frames left)
	int m_nFace{-1};                            // +0x2a8
	float m_fFuel{100.0f};                      // +0x2ac
	float m_fFuelUse{};                         // +0x2b0
	float m_fSmokeTime{};                       // +0x2b4
	int m_nSmokeLeft{};                         // +0x2b8 (smoke screens of the current use still to drop)
	float m_fFuelBurst{};                       // +0x2bc
	float m_fChameleon{};                       // +0x2c0
	float m_fMud{};                             // +0x2c4
	int m_nArmor{};                             // +0x2c8
	int m_nSmokeStock{5};                       // +0x2cc
	int m_nFlags{};                             // +0x2d0
	cParamCar* m_pParam{};                      // +0x2d4
	bool m_bSpike{};                            // +0x2d8
	cCamera* m_pCamera{};                       // +0x2dc
	cRallyModelShadow* m_pShadow{};             // +0x2e0
	// +0x2e4 the Mii face (FUN_8001508c): NOT PORTED
	int m_bSpecial{};                           // +0x2e8 (special flag taken: flags score double)
	int m_nFlagChain{};                         // +0x2ec (flags in a row, max 20)
	WiiSnd::Handle m_SndEngine;                 // +0x360
	WiiSnd::Handle m_SndFuel;                   // +0x36c (the low fuel alarm)
	kingdom::cKingdomSndActor m_SndActor;       // +0x378 (the car's 3D sound actor, at the car model)
	// +0x2f0 the 3D sound listener, +0x378 the sound actor: NOT PORTED
	kingdom::cKingdomPtkHnd m_FxNitroR;         // +0x380 (slot 2)
	kingdom::cKingdomPtkHnd m_FxNitroL;         // +0x3d0 (slot 2)
	kingdom::cKingdomPtkHnd m_FxNitro;          // +0x420 (slot 0xb, single player)
	kingdom::cKingdomPtkHnd m_FxCrash;          // +0x470 (slot 3)
	kingdom::cKingdomPtkHnd m_FxTurn;           // +0x4c0 (slot 4)
	kingdom::cKingdomPtkHnd m_FxCrash2;         // +0x510 (slot 0x11)
	kingdom::cKingdomPtkHnd m_FxOilL;           // +0x560 (slot 5)
	kingdom::cKingdomPtkHnd m_FxOilR;           // +0x5b0
	kingdom::cKingdomPtkHnd m_FxMudL;           // +0x600 (slot 6)
	kingdom::cKingdomPtkHnd m_FxMudR;           // +0x650
	kingdom::cKingdomPtkHnd m_FxSparkL;         // +0x6a0 (slot 7)
	kingdom::cKingdomPtkHnd m_FxSparkR;         // +0x6f0
	kingdom::cKingdomPtkHnd m_FxArmor;          // +0x740 (slot 8)
	kingdom::cKingdomPtkHnd m_FxSmokeL;         // +0x790 (slot 10)
	kingdom::cKingdomPtkHnd m_FxSmokeR;         // +0x7e0
	kingdom::cKingdomPtkHnd m_FxPaint;          // +0x830 (slot 0xf)
	kingdom::cKingdomPtkHnd m_FxSandL;          // +0x880 (slot 0x10)
	kingdom::cKingdomPtkHnd m_FxSandR;          // +0x8d0
	kingdom::cKingdomPtkHnd m_FxEmptyL;         // +0x920 (slot 0x2b)
	kingdom::cKingdomPtkHnd m_FxEmptyR;         // +0x970
	float m_fFaceTimer{};                       // +0x9c0 (blink timer)
	float m_fFaceWait{};                        // +0x9c4

private:
	friend bool FillConsCar(kingdom::cTaskModel* pCar, sConsCar& out);
	std::map<std::string, std::string> m_Clip;  // native bookkeeping of IsAttached: "model|slot|kind" -> entry

protected:
	void ReleaseTerm() override { cTaskUnit::Term(); kingdom::cTaskModel::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

// cTaskConsole::Hooks.CarState for a cTaskUnit (the unit members the console parts read, sConsCar in cTaskConsole.hpp); false (all zero)
// when pCar is not a cTaskUnit. The integrator assigns it: cTaskConsole::Hooks.CarState = &FillConsCar;
bool FillConsCar(kingdom::cTaskModel* pCar, sConsCar& out);
}
