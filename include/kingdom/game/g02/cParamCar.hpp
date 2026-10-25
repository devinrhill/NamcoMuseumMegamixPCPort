#pragma once

#include "common.hpp"

// Rally-X Remix (g02) car parameters. In the DOL these are cJetCSV subclasses allocated by the cars' Init (the vtable slot named after the
// class, e.g. cParamCar 0x80068b14, is the deleting destructor): the values are read straight after RegistData into fixed members.
// Rows are 0-based from the first line of the CSV, column 1 = the value (cJetCSV::F32 / S32(csv, row, 1)). FLOAT_8047aa38 = 60 (seconds ->
// frames), FLOAT_8047aa24 = pi / 180, FLOAT_8047aa3c = 0.01.
namespace kingdom::game::g02
{

// kingdom::game::g02::cParamCar (CSV_CAR 0x58739259, 0x100 bytes, vtable 0x80391afc), read by cTaskUnit::Init 0x8007e358
struct cParamCar
{
	float fSpeedBase{};      // +0x20 row 5 * row 0 / 60 (SPEED_BASE * SPEED_1; 0x80087c1c re-reads it with row (level - 1))
	float fSpeedEmpty{};     // +0x24 row 6 / 60 (no fuel)
	float fSpeedBrake{};     // +0x28 row 7 / 60
	float fSpeedNitro{};     // +0x2c row 8 / 60
	float fSpeedAdd{};       // +0x30 row 9 / 60 (acceleration per frame)
	float fSpeedSub{};       // +0x34 row 10 / 60
	float fSpeedOil{};       // +0x38 row 0x12 / 60
	float fSpeedSpike{};     // +0x3c row 0x13 / 60
	float fRotAdd{};         // +0x40 row 0xb / 60 * pi / 180 (turn speed, radians per frame)
	float fNitroTime{};      // +0x44 row 0xc * 60
	float fFuelCost{};       // +0x48 row 0xd / 60 (multiplayer: rows 0x29 / 0x33 / 0x36 by the sys' pattern +0x04)
	float fFuelCostNitro{};  // +0x4c row 0xe / 60 (multi: 0x2a / 0x34 / 0x37)
	float fFuelNitroStart{}; // +0x50 row 0xf (multi: 0x2b / 0x35 / 0x38)
	s32 nSmoke{};            // +0x54 row 0x10 (smoke screens per use)
	float fSmokeTime{};      // +0x58 row 0x11 * 60
	float fSpinTimeSmoke{};  // +0x5c row 0x14 * 60
	float fSpinTimeClash{};  // +0x60 row 0x15 * 60
	float fSpinRotSpeed{};   // +0x64 row 0x1f / 60 (degrees per frame)
	float fFuelBurst{};      // +0x68 row 0x16 * 60
	float fChameleon{};      // +0x6c row 0x17 * 60
	float fMudTime0{};       // +0x70 row 0x18 * 60, at least 13 (FLOAT_8047aa40)
	float fMudTime1{};       // +0x74 row 0x19 * 60
	s32 nMud{};              // +0x78 row 0x1a
	s32 nMudBig{};           // +0x7c row 0x1b
	float fSndInterior{};    // +0x80 row 0x1c (3D sound listener)
	float fSndUnitDist{};    // +0x84 row 0x1d
	float fSndMaxVolDist{};  // +0x88 row 0x1e
	float fListenerRange{};  // +0x8c row 0x24
	float fStartWait{};      // +0x90 row 0x20 * 60
	float fBlendNormal{};    // +0x94 row 0x21 / 60
	float fBlendLeft{};      // +0x98 row 0x22 / 60
	float fBlendRight{};     // +0x9c row 0x23 / 60
	float fBlendCtrl{};      // +0xa0 row 0x32 / 60
	float fVisibleTime{};    // +0xa4 row 0x25 * 60 (blinking after a hit)
	float fSpikeY{};         // +0xa8 row 0x26
	float fSandNodePlus{};   // +0xc0 row 0x27
	float fMiiScale{};       // +0xc4 row 0x28 * 0.01
	float fBlinkStability{}; // +0xc8 row 0x2c * 60
	float fBlinkChange{};    // +0xcc row 0x2d * 60
	s32 nArmorDropFlag{};    // +0xd0 row 0x2e
	s32 nSmokeDropFlag{};    // +0xd4 row 0x31
	float fDirectionMax{};   // +0xd8 row 0x2f
	float fDirectionSpeed{}; // +0xdc row 0x30 / 60
	float fRelocation[4]{};  // +0xe0 rows 0x39..0x3c, ms -> frames (FUN_8000d7d4: 6000 * ms / 100000)

	// 0x8007e358 (the CSV part of cTaskUnit::Init). nMultiPattern = -1 for single player, else the sys' +0x04 (0x800c5990).
	bool Load(int nMultiPattern);
	// 0x80087c1c: SPEED_BASE * SPEED_<level> / 60 (enemies' speed level from the MAPOO_OBJ record +0x14)
	void SetSpeedLevel(int level);

private:
	float m_fRow[0x3d]{};    // the CSV values (natively kept for 0x80087c1c)
};

// kingdom::game::g02::cParamCarPlayer (CSV_CARPLAYER 0xf383e0d6, 0x38 bytes, vtable 0x80393b98), read by cTaskPlayer::Init 0x8007a2e8
struct cParamCarPlayer
{
	float fPadLeft{};        // +0x20 row 1 (PAD_LEFT 0.55: acc z above it = left)
	float fPadRight{};       // +0x24 row 2 (PAD_RIGHT -0.45)
	float fPadTurn{};        // +0x28 row 0 (PAD_TURN -1.5: acc y below it = U-turn)
	float fPadWait{};        // +0x2c row 3 * 60 (FLOAT_8047a9cc)
	float fPadWaitTurn{};    // +0x30 row 4 * 60

	bool Load();             // 0x8007a2e8 (CSV part)
};

// kingdom::game::g02::cParamCarEnemy (CSV_CARENEMY 0x0a541dd9, 0x58 bytes, vtable 0x80391e30; defaults 0x8006d734), read by cTaskEnemy::Init
// 0x8006d500
struct cParamCarEnemy
{
	s32 nSearchRange{};      // +0x20 row 0 (cells)
	s32 nTargetNear{};       // +0x24 row 1 (AI type 1: cells ahead of the player while the player is within range)
	s32 nTargetFar{};        // +0x28 row 2 (AI type 1: out of range)
	float fRangeInTime{};    // +0x2c row 3 * 60 (chameleon paint reaction)
	float fRandStability{};  // +0x30 row 4 * 60
	float fRandChange{};     // +0x34 row 5 * 60
	float fCamRandStability{}; // +0x38 row 6 * 60
	float fCamRandChange{};  // +0x3c row 7 * 60
	s32 nSpinScore{100};     // +0x40 row 8
	s32 nSearchChameleon{};  // +0x44 row 9
	s32 nSearchRed{};        // +0x48 row 10
	float fRedRangeInTime{}; // +0x4c row 11 * 60
	float fNitroUseTime{};   // +0x50 row 12 * 60

	bool Load();             // 0x8006d500 (CSV part)
};

}
