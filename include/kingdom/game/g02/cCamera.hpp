#pragma once

#include "common.hpp"

// Rally-X Remix (g02) car camera: kingdom::game::g02::cCamera (0xb0 bytes, vtable 0x80441308 at +0xa8: slot +0x08 Init 0x801278c4, +0x0c Term
// 0x80128148, +0x10 dtor 0x8012785c). NOT a task: every human car (cTaskUnit +0x2dc, made by FUN_80081cec when the unit's +0x1cc index < 4)
// owns one, built with the app's cTaskScnRoot (app +0x1b8: +0x1b4 the nw4r ScnRoot, +0x1b8 the screen layout 0 / 4 / 7 = 1 / 2 / 3-4
// players, table 0x80378a58 by the sys player count +0x0c, set by the app's FUN_80126930).
//   Modes (+0x00): 0 nothing, 1 FIRST (the opening turn around the car), 2 to-game (eases eye / target onto the follow position),
//   3 QUARTER (the 1P "quarter" follow view), 4 TOP (the eye keeps its offset to the target and follows the car), 5 CLEAR (orbits the car).
//   The car starts them through FUN_80081dd0 (which 1 -> StartFirst; 2 -> StartTop / StartQuarter(.., 1) then StartToGame; 0 ->
//   StartTop / StartQuarter(.., flag) by the car's view (vtable +0x9c: 2 / 3 top, 1 quarter)), StartClear from FUN_80088a70, and calls Update
//   every frame from cTaskPlayer::Execute FUN_8007a558 (pos = unit +0x200, dir = unit +0x230 (dir[1] = the heading, radians), speed mode 0 normal /
//   1 nitro / 2 brake). SetSpeedCurve comes from the unit step FUN_80084d6c.
//   The app copies the camera into ScnRoot camera i every frame (Apply, from Execute FUN_80125b1c) and sets the four ScnRoot fovys once from car
//   0's camera (GetFovy by the sys per-player view +0x14 + i * 0x14, FUN_80126930).
// Wii space (no Unity X negation). Matrices follow the SDK's MTX44 (column vectors, v' = M v; RotY(a) * (0, 0, 1) = (sin a, 0, cos a)).
// Native: one scene camera (NativeScene) = ScnRoot camera 0; cameras 1..3 (split screen) keep their data but are NOT drawn (NOT PORTED).
namespace kingdom::game::g02
{
class cParamCamera;

class cCamera
{
public:
	enum MODE
	{
		eMODE_NONE = 0,
		eMODE_FIRST = 1,
		eMODE_TO_GAME = 2,
		eMODE_QUARTER = 3,
		eMODE_TOP = 4,
		eMODE_CLEAR = 5
	};
	enum SPEED { eSPEED_NORMAL = 0, eSPEED_NITRO = 1, eSPEED_BRAKE = 2 };

	// pScnLayout: the app's mirror of cTaskScnRoot +0x1b8 (read live, as the DOL does; nullptr = 0, one screen)
	explicit cCamera(const s32* pScnLayout);   // DOL 0x80127808
	~cCamera();                                 // DOL 0x8012785c
	void Init();                                // DOL 0x801278c4
	void Term();                                // DOL 0x80128148

	void StartFirst(int nCam, const float* pos, const float* dir);                // DOL 0x80128238 (mode 1)
	void StartToGame(int nCam, const float* pos, const float* dir);               // DOL 0x80128504 (mode 2)
	void StartTop(int nCam, const float* pos, const float* dir, int bReset);      // DOL 0x801285c0 (mode 4)
	void StartQuarter(int nCam, const float* pos, const float* dir, int bReset);  // DOL 0x80128890 (mode 3)
	void StartClear(int nCam);                                                    // DOL 0x80128b60 (mode 5)
	void ModeChange(int nMode);                 // DOL 0x80128c54 (0 = re-apply the current mode's parameters)
	float GetFovy(int nView) const;             // DOL 0x80128e4c (nView = the sys per-player view 1 quarter, 2 / 3 top)
	void SetSpeedCurve(int bStraight);          // DOL 0x80128ed8
	void Update(const float* pos, const float* dir, int nSpeed);   // DOL 0x80128f24
	void Apply(int nCam) const;                 // DOL 0x80129c94: eye / target / up -> ScnRoot camera nCam
	void GetCameraMtx(float mtx[3][4], int nCam) const;   // DOL 0x80129d30 (natively from this camera's own eye / target / up)

	static float Approach(float target, float cur, float speed, float between);   // DOL 0x80127734

	// native stand-in for the ScnRoot cameras' fovy (nw4r Camera +0xac; FUN_80126930 writes it): Apply draws with it
	static void SetScnCameraFovy(int nCam, float fFovy);
	static float ScnCameraFovy(int nCam);

	void SetRotYGoal(float f) { m_fRotYGoal = f; }   // DOL 0x80128f1c (the player's D-pad look)
	float fRotYNow() const { return m_fRotYNow; }    // +0x70 (the app's start test reads it through the car: 1 = the opening turn is over)
	int nMode() const { return m_nMode; }
	const float* vEye() const { return m_vEye; }
	const float* vTarget() const { return m_vTarget; }
	cParamCamera* pParam() const { return m_pParam; }

private:
	int nScnLayout() const { return m_pScnLayout != nullptr ? *m_pScnLayout : 0; }
	void UpdateFirst(const float* pos, const float* dir);                 // DOL 0x80129050
	void UpdateToGame(const float* pos, const float* dir);                // DOL 0x80129388
	void UpdateQuarter(const float* pos, const float* dir, int nSpeed);   // DOL 0x80129714
	void UpdateClear(const float* pos);                                   // DOL 0x80129ad8
	void ResetFollow(const float* dir);         // the live part of the starters' tail (see cCamera.cpp)

	int m_nMode{};                              // +0x00
	const s32* m_pScnLayout{};                  // +0x04 (DOL: the cTaskScnRoot*)
	cParamCamera* m_pParam{};                   // +0x08
	float m_vEye[4]{0.0f, 0.0f, 50.0f, 1.0f};   // +0x10
	float m_vTarget[4]{0.0f, 0.0f, 0.0f, 1.0f}; // +0x20
	float m_vUp[4]{0.0f, 1.0f, 0.0f, 1.0f};     // +0x30
	float m_fSpd{};                             // +0x40 SPD_MAX / SPD_CURVE_MAX (no reader found in the camera)
	float m_fSpdBetween{};                      // +0x44 SPD_BITWEEN / SPD_CURVE_BITWEEN (no reader found in the camera)
	float m_vOffset[4]{0.0f, 0.0f, 0.0f, 1.0f}; // +0x50 target offset (entry +0x00)
	float m_fRotX{};                            // +0x60 entry +0x10
	float m_fRange{};                           // +0x64 entry +0x14
	float m_fRangeNow{};                        // +0x68
	float m_fRotYGoal{};                        // +0x6c
	float m_fRotYNow{};                         // +0x70 extra turn (the opening's ROTY_FIRST -> 0)
	float m_fRotY{};                            // +0x74 smoothed pi + heading
	float m_vSaveEye[4]{0.0f, 0.0f, 0.0f, 1.0f};    // +0x80 (StartClear)
	float m_vSaveTarget[4]{0.0f, 0.0f, 0.0f, 1.0f}; // +0x90
	float m_fOrbit{};                           // +0xa0 CLEAR orbit angle
	bool m_bPosYFix{true};                      // +0xa4 mode 2 pins the eye height to POSY_LIMIT (StartQuarter 1, StartTop 0)
};
}
