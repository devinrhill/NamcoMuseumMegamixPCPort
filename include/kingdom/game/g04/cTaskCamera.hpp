#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"

// Gator Panic Remix (g04) the camera task: kingdom::game::g04::cTaskCamera (0x1c4 bytes, vtable 0x804496d0; ctor 0x8019d314, app +0x2a4).
// It owns a kingdom::game::g04::cCamera (+0x1b4, vtable 0x80394400), CSV_CAMERA (cParamCamera +0x1b8: shake ROW / COL radius, SUB per frame,
// SIN step in degrees) and CSV_CAMERA_PARAM (cParamCameraSetting +0x1bc: zoom near / far / aspect / angle, game near / far / aspect / angle).
//   SetMode 0x8019d724: 0 the game camera (BG_SCA camera 0 at frame 0 with the game projection), 1 a shake (cCamera mode 3), 2 the zoom of the
//   angry event (BG_ZOOM_SCA with the zoom projection).
//   Execute 0x8019d630: while +0x1c0 == 1 (UNVERIFIED: set by the ctor) the shake's end (both radii 0) puts the game camera back; the cCamera
//   update 0x80094e2c applies the shake: eye.y += ROW * sin(t), target.x += COL * sin(t) (t += SIN, wraps at 360), radii -= SUB.
// The aspect of the projection (1.242) is not applied natively (the scene uses the 4:3 viewport).
namespace kingdom::game::g04
{
class cTaskCamera : public kingdom::cTaskKingdom
{
public:
	cTaskCamera();                              // DOL 0x8019d314
	bool Execute(cJetTask* pParam) override;    // DOL 0x8019d630
	void SetMode(int mode);                     // DOL 0x8019d724

private:
	bool TakeSceneCamera(const char* sca, float fovy, float nearZ, float farZ);   // DOL 0x80094ba8(frame 0, cam, sca, 0, 0)
	void Apply();                               // DOL 0x80094e2c

	int m_nCamMode{1};                          // cCamera +0x54: 1 posture, 3 shake
	float m_fEye[3]{}, m_fTarget[3]{};          // cCamera +0x10 / +0x20
	float m_fFovy{33.3f}, m_fNear{1.0f}, m_fFar{10000.0f};
	float m_fT{};                               // +0x64
	float m_fCol{}, m_fSub{}, m_fRow{}, m_fStep{};   // +0x68 / +0x6c / +0x70 / +0x74
	float m_fShakeRow{0.2f}, m_fShakeCol{0.1f}, m_fShakeSub{0.02f}, m_fShakeSin{30.0f};   // cParamCamera +0x20..+0x2c
	float m_fZoom[4]{1.0f, 10000.0f, 1.242f, 11.4949f};   // cParamCameraSetting +0x20.. (near, far, aspect, angle)
	float m_fGame[4]{1.0f, 10000.0f, 1.242f, 33.3f};      // +0x30..
	int m_nTaskMode{1};                         // +0x1c0
};
}
