#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"

// Rally-X Remix (g02) car camera parameters: kingdom::game::g02::cParamCamera (0x150 bytes, a cJetCSV with its vtable at +0x1c: 0x8044131c;
// slot +0x08 Clear 0x801280b0, +0x0c an empty Term 0x80128234, +0x10 dtor 0x801281a0). Made and filled by cCamera::Init, which reads CSV_CAMERA
// (0xe12a4259, g02game.lzs) column 1 by ROW INDEX (the DOL's cJetCSV call labelled "S32" at 0x8013969c returns a float: it is F32; default 0).
// Every line of the file is a row (the ",," separators included):
//   rows  0.. 12 eCSV_CAM_QUARTER_*  (TARGET_X/Y/Z, ROTX, ROTY_SPEED, ROTY_BITWEEN, RANGE_BRAKE / NORMAL / NITRO, RANGE_SPEED, RANGE_BITWEEN,
//                                     POSY_LIMIT, FOVY)
//   rows 14.. 26 eCSV_CAM_QUARTER_2P_* (same order; RANGE_LIMIT in place of RANGE_BITWEEN)
//   rows 28.. 33 eCSV_CAM_TOP_*        (TARGET_X/Y/Z, ROTX, RANGE, FOVY)       rows 35..40 eCSV_CAM_TOP_2P_*
//   rows 42.. 53 eCSV_CAM_FIRST_*      (TARGET_X/Y/Z, ROTX, ROTY_FIRST, ROTY_SPEED, ROTY_BITWEEN, RANGE_START, RANGE_END, RANGE_SPEED,
//                                     RANGE_BITWEEN, POSY_LIMIT)
//   rows 55.. 58 eCSV_CAM_FIRST_TO_GAME_* (POSITION_SPD, POSITION_BITWEEN, TARGET_SPD, TARGET_BITWEEN)
//   rows 65.. 68 eCSV_CAM_SPD_*        (MAX, CURVE_MAX, BITWEEN, CURVE_BITWEEN)  row 70 eCSV_CAM_CLEAR_ROTSPEED_Y
// Rows the DOL never reads: QUARTER_2P ROTY_SPEED / ROTY_BITWEEN / POSY_LIMIT (18, 19, 25), FIRST RANGE_SPEED / RANGE_BITWEEN / POSY_LIMIT
// (51..53), the CLEAR_NOTUSE rows.
namespace kingdom::game::g02
{
class cParamCamera : public kingdom::cKingdomCSV
{
public:
	// one view setting (0x20 bytes at +0x20 + i * 0x20); the camera's mode switch copies one of them (cCamera::ModeChange)
	struct Entry
	{
		float vTarget[4];   // +0x00 target offset from the car (w = 0: cleared, never read from the CSV)
		float fRotX;        // +0x10 radians (CSV degrees * FLOAT_8047c01c)
		float fRange;       // +0x14 eye distance
		float fFovy;        // +0x18 degrees
		float f1c;          // +0x1c (never written)
	};
	enum
	{
		eENTRY_QUARTER = 0,      // 1P quarter view (camera mode 3)
		eENTRY_QUARTER_2P = 1,   //   ... when the ScnRoot layout is the two-player split (4)
		eENTRY_TOP = 2,          // top view (camera mode 4)
		eENTRY_TOP_2P = 3,
		eENTRY_FIRST = 4,        // the opening turn (camera mode 1); range = RANGE_START, fovy = QUARTER_FOVY
		eENTRY_FIRST_2P = 5,     //   ... the same rows again
		eENTRY_MAX = 6
	};

	cParamCamera();                         // the construction inlined in cCamera::Init (cJetCSV ctor FUN_801393c0, then Clear)
	~cParamCamera() override;               // DOL 0x801281a0
	void Load();                            // the CSV_CAMERA reads inlined in cCamera::Init

protected:
	void Init() override;                   // DOL 0x801280b0 (vtable +0x08: clear the block)
	void Term() override;                   // DOL 0x80128234 (vtable +0x0c: empty)

public:
	Entry m_Entry[eENTRY_MAX]{};            // +0x20..+0xdf
	float m_fPosYLimit{};                   // +0xe0  QUARTER_POSY_LIMIT (row 11): the quarter eye height
	float m_fRotYSpeed{};                   // +0xe4  QUARTER_ROTY_SPEED (row 4, not converted to radians)
	float m_fRotYBetween{};                 // +0xe8  QUARTER_ROTY_BITWEEN (row 5)
	float m_fRangeBrake{};                  // +0xec  QUARTER_RANGE_BRAKE (row 6)
	float m_fRangeNitro{};                  // +0xf0  QUARTER_RANGE_NITRO (row 8)
	float m_fRangeSpeed{};                  // +0xf4  QUARTER_RANGE_SPEED (row 9; not cleared by Init)
	float m_fRangeBetween{};                // +0xf8  QUARTER_RANGE_BITWEEN (row 10; not cleared by Init)
	float m_f2PRangeBrake{};                // +0xfc  QUARTER_2P_RANGE_BRAKE (row 20; no reader found)
	float m_f2PRangeNitro{};                // +0x100 QUARTER_2P_RANGE_NITRO (row 22; no reader found)
	float m_f2PRangeSpeed{};                // +0x104 QUARTER_2P_RANGE_SPEED (row 23; no reader found; not cleared by Init)
	float m_f2PRangeLimit{};                // +0x108 QUARTER_2P_RANGE_LIMIT (row 24; no reader found; not cleared by Init)
	float m_fToGamePosSpd{};                // +0x10c FIRST_TO_GAME_POSITION_SPD (row 55): mode 2 moves the TARGET with it
	float m_fToGamePosBetween{};            // +0x110 FIRST_TO_GAME_POSITION_BITWEEN (row 56)
	float m_fToGameTgtSpd{};                // +0x114 FIRST_TO_GAME_TARGET_SPD (row 57): mode 2 moves the EYE with it (names swapped in the DOL)
	float m_fToGameTgtBetween{};            // +0x118 FIRST_TO_GAME_TARGET_BITWEEN (row 58)
	float m_fFirstRotY{};                   // +0x11c FIRST_ROTY_FIRST (row 46) radians
	float m_fFirstRotYSpeed{};              // +0x120 FIRST_ROTY_SPEED (row 47) radians
	float m_fFirstRotYBetween{};            // +0x124 FIRST_ROTY_BITWEEN (row 48) radians
	float m_fFirstRangeStart{};             // +0x128 FIRST_RANGE_START (row 49)
	float m_fFirstRangeEnd{};               // +0x12c FIRST_RANGE_END (row 50)
	float m_fSpdMax{};                      // +0x130 SPD_MAX (row 65)
	float m_fSpdCurveMax{};                 // +0x134 SPD_CURVE_MAX (row 66)
	float m_fSpdBetween{};                  // +0x138 SPD_BITWEEN (row 67)
	float m_fSpdCurveBetween{};             // +0x13c SPD_CURVE_BITWEEN (row 68)
	float m_fClearRotSpeedY{};              // +0x140 CLEAR_ROTSPEED_Y (row 70) / 60 (FLOAT_8047c020)
	bool m_bLoaded{};                       // +0x144
};
}
