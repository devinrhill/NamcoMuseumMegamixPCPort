#include "kingdom/game/g02/cCamera.hpp"

#include <cmath>
#include <cstring>

#include "kingdom/NativeScene.hpp"
#include "kingdom/game/g02/cParamCamera.hpp"

namespace kingdom::game::g02
{

namespace
{
constexpr float PI = 3.14159265f;        // FLOAT_8047c02c
constexpr float PI2 = 6.28318531f;       // FLOAT_8047c030
constexpr float PI4 = 12.5663706f;       // FLOAT_8047c03c
constexpr float DEG2RAD = 0.017453292f;  // FLOAT_8047c01c
constexpr float ROTY_STEP = 0.034906585f;   // FLOAT_8047c034 (2 degrees)
// the ScnRoot camera's projection besides the fovy: cTaskScnRoot's defaults (near 0.1, far 32766 as read for Galaga's; UNVERIFIED for g02)
constexpr float SCN_NEAR = 0.1f;
constexpr float SCN_FAR = 32766.0f;
float s_fScnFovy[4] = {30.0f, 30.0f, 30.0f, 30.0f};   // cTaskScnRoot default fovy 30 (UNVERIFIED for g02) until FUN_80126930 sets it

// Mtx44 of the SDK (row-major storage, column vectors)
struct Mtx44
{
	float m[4][4];
};

// PSMTX44Identity (FUN_80298810)
Mtx44 Identity()
{
	Mtx44 r{};
	for (int i = 0; i < 4; i++) r.m[i][i] = 1.0f;
	return r;
}

// MTX44RotRad (FUN_80298d00 -> PSMTX44RotTrig): axis 'X' / 'Y'
Mtx44 RotRad(char axis, float rad)
{
	const float s = (float)std::sin((double)rad), c = (float)std::cos((double)rad);
	Mtx44 r = Identity();
	if (axis == 'X')
	{
		r.m[1][1] = c;
		r.m[1][2] = -s;
		r.m[2][1] = s;
		r.m[2][2] = c;
	}
	else   // 'Y'
	{
		r.m[0][0] = c;
		r.m[0][2] = s;
		r.m[2][0] = -s;
		r.m[2][2] = c;
	}
	return r;
}

// PSMTX44Trans (FUN_80298c80)
Mtx44 Trans(float x, float y, float z)
{
	Mtx44 r = Identity();
	r.m[0][3] = x;
	r.m[1][3] = y;
	r.m[2][3] = z;
	return r;
}

// PSMTX44Concat(a, b, ab) (FUN_802988a0): a * b
Mtx44 Concat(const Mtx44& a, const Mtx44& b)
{
	Mtx44 r{};
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			r.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j] + a.m[i][3] * b.m[3][j];
	return r;
}

// the inlined 4x4 * vec4 of the follow functions
void MulVec4(const Mtx44& a, const float v[4], float out[4])
{
	float r[4];
	for (int i = 0; i < 4; i++) r[i] = a.m[i][0] * v[0] + a.m[i][1] * v[1] + a.m[i][2] * v[2] + a.m[i][3] * v[3];
	std::memcpy(out, r, sizeof(r));
}

// PSVECAdd / PSVECSubtract: xyz only (w of the destination stays)
void VecAdd(const float a[3], const float b[3], float out[3])
{
	for (int i = 0; i < 3; i++) out[i] = a[i] + b[i];
}
void VecSub(const float a[3], const float b[3], float out[3])
{
	for (int i = 0; i < 3; i++) out[i] = a[i] - b[i];
}
}

// ---- construction --------------------------------------------------------------------------------------------------------------------

// DOL 0x80127808 (this, cTaskScnRoot, 0): vtable, Init (slot +0x08), then +0x04
cCamera::cCamera(const s32* pScnLayout)
{
	Init();
	m_pScnLayout = pScnLayout;
}

// DOL 0x8012785c: Term (slot +0x0c), delete
cCamera::~cCamera()
{
	Term();
}

// DOL 0x801278c4: new cParamCamera (0x150) + CSV_CAMERA, then the camera defaults
void cCamera::Init()
{
	cParamCamera* p = new cParamCamera();
	p->Load();
	m_pScnLayout = nullptr;   // +0x04 = 0
	m_pParam = p;
	const float eye[4] = {0.0f, 0.0f, 50.0f, 1.0f};   // FLOAT_8047c024, w = DAT_80378b04
	const float target[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	std::memcpy(m_vEye, eye, sizeof(eye));
	std::memcpy(m_vTarget, target, sizeof(target));
	m_vUp[0] = 0.0f;
	m_vUp[1] = 1.0f;
	m_vUp[2] = 0.0f;
	m_vUp[3] = 1.0f;
	m_fSpd = m_fSpdBetween = 0.0f;
	m_vOffset[0] = m_vOffset[1] = m_vOffset[2] = 0.0f;
	m_vOffset[3] = 1.0f;
	m_fRotX = m_fRange = m_fRangeNow = m_fRotYGoal = m_fRotYNow = m_fRotY = 0.0f;
	m_bPosYFix = true;
	m_vSaveEye[0] = m_vSaveEye[1] = m_vSaveEye[2] = 0.0f;
	m_vSaveEye[3] = 1.0f;
	m_vSaveTarget[0] = m_vSaveTarget[1] = m_vSaveTarget[2] = 0.0f;
	m_vSaveTarget[3] = 1.0f;
	m_fOrbit = 0.0f;
	m_nMode = eMODE_NONE;
}

// DOL 0x80128148: delete the parameters (their vtable +0x10)
void cCamera::Term()
{
	if (m_pParam != nullptr)
	{
		delete m_pParam;
		m_pParam = nullptr;
	}
}

// ---- the ScnRoot camera ----------------------------------------------------------------------------------------------------------------

// DOL 0x80129c94: ScnRoot::GetCamera(nCam) -> SetPosition(eye), SetPosture(LOOKAT, up +0x30, target +0x20). The starters inline the same block.
// Native: camera 0 is the scene camera; 1..3 (split screen) are NOT PORTED.
void cCamera::Apply(int nCam) const
{
	if (nCam != 0) return;
	kingdom::NativeScene::SetCamera(m_vEye, m_vTarget, ScnCameraFovy(nCam), SCN_NEAR, SCN_FAR);
}

// DOL 0x80129d30: ScnRoot::GetCamera(nCam) -> Camera::GetCameraMtx. Native: C_MTXLookAt(eye, up, target) of this camera (the ScnRoot camera
// holds exactly this after Apply / a start; UNVERIFIED equivalence if another task moved ScnRoot camera nCam in between)
void cCamera::GetCameraMtx(float mtx[3][4], int nCam) const
{
	(void)nCam;
	float look[3] = {m_vEye[0] - m_vTarget[0], m_vEye[1] - m_vTarget[1], m_vEye[2] - m_vTarget[2]};
	float l = std::sqrt(look[0] * look[0] + look[1] * look[1] + look[2] * look[2]);
	if (l > 0.0f)
		for (float& c : look) c /= l;
	float right[3] = {m_vUp[1] * look[2] - m_vUp[2] * look[1], m_vUp[2] * look[0] - m_vUp[0] * look[2], m_vUp[0] * look[1] - m_vUp[1] * look[0]};
	l = std::sqrt(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
	if (l > 0.0f)
		for (float& c : right) c /= l;
	const float up[3] = {look[1] * right[2] - look[2] * right[1], look[2] * right[0] - look[0] * right[2], look[0] * right[1] - look[1] * right[0]};
	const float* rows[3] = {right, up, look};
	for (int i = 0; i < 3; i++)
	{
		mtx[i][0] = rows[i][0];
		mtx[i][1] = rows[i][1];
		mtx[i][2] = rows[i][2];
		mtx[i][3] = -(m_vEye[0] * rows[i][0] + m_vEye[1] * rows[i][1] + m_vEye[2] * rows[i][2]);
	}
}

void cCamera::SetScnCameraFovy(int nCam, float fFovy)
{
	if (nCam >= 0 && nCam < 4) s_fScnFovy[nCam] = fFovy;
}

float cCamera::ScnCameraFovy(int nCam)
{
	return (nCam >= 0 && nCam < 4) ? s_fScnFovy[nCam] : s_fScnFovy[0];
}

// ---- mode starters -----------------------------------------------------------------------------------------------------------------------

// The live part of the starters' tails (StartFirst always, StartTop / StartQuarter when their flag is set): +0x74 = pi + heading, +0x68 = +0x64.
// The rest of the tail is dead in the DOL: an identity Mtx44 times the target offset plus pos, a RotX(-rotX) concat and that matrix times
// (0, 0, range, 1) plus the first sum are computed into stack locals and never stored (inlined follow math whose results were dropped).
void cCamera::ResetFollow(const float* dir)
{
	m_fRotY = PI + dir[1];
	m_fRangeNow = m_fRange;
}

// DOL 0x80128238
void cCamera::StartFirst(int nCam, const float* pos, const float* dir)
{
	(void)pos;   // only the dead tail reads it
	m_nMode = eMODE_FIRST;
	ModeChange(eMODE_FIRST);
	Apply(nCam);
	ResetFollow(dir);
}

// DOL 0x80128504 (ModeChange(2) changes nothing: the parameters of the StartTop / StartQuarter before it stay)
void cCamera::StartToGame(int nCam, const float* pos, const float* dir)
{
	(void)pos;
	(void)dir;
	m_nMode = eMODE_TO_GAME;
	ModeChange(eMODE_TO_GAME);
	Apply(nCam);
}

// DOL 0x801285c0
void cCamera::StartTop(int nCam, const float* pos, const float* dir, int bReset)
{
	(void)pos;
	m_nMode = eMODE_TOP;
	ModeChange(eMODE_TOP);
	Apply(nCam);
	if (bReset != 0)
	{
		ResetFollow(dir);
		m_bPosYFix = false;
	}
}

// DOL 0x80128890
void cCamera::StartQuarter(int nCam, const float* pos, const float* dir, int bReset)
{
	(void)pos;
	m_nMode = eMODE_QUARTER;
	ModeChange(eMODE_QUARTER);
	Apply(nCam);
	if (bReset != 0)
	{
		ResetFollow(dir);
		m_bPosYFix = true;
	}
}

// DOL 0x80128b60: remember eye / target for the orbit
void cCamera::StartClear(int nCam)
{
	m_nMode = eMODE_CLEAR;
	ModeChange(eMODE_CLEAR);
	std::memcpy(m_vSaveEye, m_vEye, sizeof(m_vEye));
	std::memcpy(m_vSaveTarget, m_vTarget, sizeof(m_vTarget));
	m_fOrbit = 0.0f;
	Apply(nCam);
}

// DOL 0x80128c54: copy a parameter entry (the *_2P one when the ScnRoot layout is 4 = two screens). Modes 2 and 5 change nothing.
void cCamera::ModeChange(int nMode)
{
	if (nMode == 0) nMode = m_nMode;
	const bool b2P = nScnLayout() == 4;
	const cParamCamera& p = *m_pParam;
	auto copy = [&](int i) {
		const cParamCamera::Entry& e = p.m_Entry[i];
		std::memcpy(m_vOffset, e.vTarget, sizeof(m_vOffset));
		m_fRotX = e.fRotX;
		m_fRange = e.fRange;
	};
	if (nMode == eMODE_QUARTER)
	{
		copy(b2P ? cParamCamera::eENTRY_QUARTER_2P : cParamCamera::eENTRY_QUARTER);
		m_fSpd = p.m_fSpdMax;
		m_fSpdBetween = p.m_fSpdBetween;
		m_fRotYNow = 0.0f;
		m_fRotYGoal = 0.0f;
		m_fRangeNow = m_fRange;
	}
	else if (nMode == eMODE_FIRST)
	{
		copy(b2P ? cParamCamera::eENTRY_FIRST_2P : cParamCamera::eENTRY_FIRST);
		m_fRotYNow = p.m_fFirstRotY;
		m_fRotYGoal = 0.0f;
	}
	else if (nMode == eMODE_TOP)
	{
		copy(b2P ? cParamCamera::eENTRY_TOP_2P : cParamCamera::eENTRY_TOP);
		m_fSpd = p.m_fSpdMax;
		m_fSpdBetween = p.m_fSpdBetween;
	}
}

// DOL 0x80128e4c: the fovy of a view (the sys per-player view): 1 quarter, 2 / 3 top (the *_2P entry on the two-screen layout); else entry -1
// (the DOL then reads param +0x18, a word inside the cJetCSV base: garbage as a fovy; natively 0; UNVERIFIED that no caller passes another view)
float cCamera::GetFovy(int nView) const
{
	const bool b2P = nScnLayout() == 4;
	int i = -1;
	if (nView == 1) i = b2P ? cParamCamera::eENTRY_QUARTER_2P : cParamCamera::eENTRY_QUARTER;
	else if (nView == 2 || nView == 3) i = b2P ? cParamCamera::eENTRY_TOP_2P : cParamCamera::eENTRY_TOP;
	if (i < 0) return 0.0f;
	return m_pParam->m_Entry[i].fFovy;
}

// DOL 0x80128ed8: called by the unit step every frame (1 = the car's +0x220 is FLOAT_8047aa28, UNVERIFIED meaning: straight). Nothing in the
// camera reads +0x40 / +0x44 (searched every camera function, Approach .. GetCameraMtx).
void cCamera::SetSpeedCurve(int bStraight)
{
	if (bStraight == 1)
	{
		m_fSpd = m_pParam->m_fSpdMax;
		m_fSpdBetween = m_pParam->m_fSpdBetween;
	}
	else if (bStraight == 0)
	{
		m_fSpd = m_pParam->m_fSpdCurveMax;
		m_fSpdBetween = m_pParam->m_fSpdCurveBetween;
	}
}

// DOL 0x80127734: move cur toward target by speed, slowed by sin(distance / between) inside "between", never past the target
float cCamera::Approach(float target, float cur, float speed, float between)
{
	if (target < cur)
	{
		const float d = cur - target;
		if (d <= between)
		{
			cur = cur - speed * (float)std::sin((double)(d / between));
			if (cur < target) cur = target;
		}
		else
			cur = cur - speed;
	}
	else if (cur < target)
	{
		const float d = target - cur;
		if (d <= between)
		{
			cur = speed * (float)std::sin((double)(d / between)) + cur;
			if (target < cur) cur = target;
		}
		else
			cur = cur + speed;
	}
	return cur;
}

// ---- follow --------------------------------------------------------------------------------------------------------------------------------

// DOL 0x80128f24 (cam, unit +0x200 pos, unit +0x230 dir, speed mode). Mode 0 does nothing.
void cCamera::Update(const float* pos, const float* dir, int nSpeed)
{
	switch (m_nMode)
	{
	case eMODE_FIRST: UpdateFirst(pos, dir); break;
	case eMODE_TO_GAME: UpdateToGame(pos, dir); break;
	case eMODE_QUARTER: UpdateQuarter(pos, dir, nSpeed); break;
	case eMODE_CLEAR: UpdateClear(pos); break;
	case eMODE_TOP:
	{
		// inline: keep the eye's offset to the target, the target = pos + offset (xyz; w stay)
		float eye[4], target[4];
		std::memcpy(eye, m_vEye, sizeof(eye));
		std::memcpy(target, m_vTarget, sizeof(target));
		VecSub(eye, target, eye);
		VecAdd(pos, m_vOffset, target);
		VecAdd(eye, target, eye);
		std::memcpy(m_vEye, eye, sizeof(eye));
		std::memcpy(m_vTarget, target, sizeof(target));
		break;
	}
	default: break;
	}
}

// the heading wrap shared by modes 1 and 3: keep +0x74 within pi of t = pi + heading
static void WrapRotY(float& rotY, float t)
{
	if (PI + rotY < t) rotY = rotY + PI2;
	if (t < rotY - PI) rotY = rotY - PI2;
}

// DOL 0x80129050: the opening: the heading snaps, the extra turn (+0x70) eases from ROTY_FIRST to 0, the range follows it START -> END
void cCamera::UpdateFirst(const float* pos, const float* dir)
{
	const cParamCamera& p = *m_pParam;
	Mtx44 m = Identity();
	const float t = PI + dir[1];
	WrapRotY(m_fRotY, t);
	m_fRotY = t;
	if (t != 0.0f) m = Concat(m, RotRad('Y', t));
	m_fRotYNow = Approach(m_fRotYGoal, m_fRotYNow, p.m_fFirstRotYSpeed, p.m_fFirstRotYBetween);
	if (m_fRotYNow != 0.0f) m = Concat(m, RotRad('Y', m_fRotYNow));
	m_fRangeNow = (p.m_fFirstRangeStart - p.m_fFirstRangeEnd) * (m_fRotYNow / p.m_fFirstRotY) + p.m_fFirstRangeEnd;
	float target[4];
	MulVec4(m, m_vOffset, target);
	VecAdd(pos, target, target);
	std::memcpy(m_vTarget, target, sizeof(target));
	if (-m_fRotX != 0.0f) m = Concat(m, RotRad('X', -m_fRotX));
	const float local[4] = {0.0f, 0.0f, m_fRangeNow, 1.0f};
	float eye[4];
	MulVec4(m, local, eye);
	VecAdd(target, eye, eye);
	std::memcpy(m_vEye, eye, sizeof(eye));
}

// DOL 0x80129388: eases target and eye (each of x, y, z, w) to the follow position of the current entry; the heading is not smoothed here.
// FIRST_TO_GAME_POSITION_* move the target and *_TARGET_* the eye (as the DOL).
void cCamera::UpdateToGame(const float* pos, const float* dir)
{
	const cParamCamera& p = *m_pParam;
	Mtx44 m = Identity();
	const float a = PI + dir[1];
	if (a != 0.0f) m = Concat(m, RotRad('Y', a));
	float want[4];
	MulVec4(m, m_vOffset, want);
	VecAdd(pos, want, want);
	for (int i = 0; i < 4; i++) m_vTarget[i] = Approach(want[i], m_vTarget[i], p.m_fToGamePosSpd, p.m_fToGamePosBetween);
	const float local[4] = {0.0f, 0.0f, m_fRange, 1.0f};
	if (-m_fRotX != 0.0f) m = Concat(m, RotRad('X', -m_fRotX));
	float eye[4];
	MulVec4(m, local, eye);
	VecAdd(want, eye, eye);
	if (m_bPosYFix) eye[1] = p.m_fPosYLimit;
	float out[4];
	for (int i = 0; i < 4; i++) out[i] = Approach(eye[i], m_vEye[i], p.m_fToGameTgtSpd, p.m_fToGameTgtBetween);
	std::memcpy(m_vEye, out, sizeof(out));
}

// DOL 0x80129714: the quarter view. The heading (pi + dir[1]) is followed through Approach(ROTY_SPEED, ROTY_BITWEEN) (raw CSV values against
// radians: about 2.3 % of the gap a frame), the extra turn +0x70 steps 2 degrees a frame to +0x6c (both 0 in this mode), the distance eases to
// RANGE_NORMAL / NITRO / BRAKE (the 1P values also on the two-screen layout), and the eye height is ALWAYS POSY_LIMIT (not a clamp).
void cCamera::UpdateQuarter(const float* pos, const float* dir, int nSpeed)
{
	const cParamCamera& p = *m_pParam;
	Mtx44 m = Identity();
	const float t = PI + dir[1];
	WrapRotY(m_fRotY, t);
	m_fRotY = Approach(t, m_fRotY, p.m_fRotYSpeed, p.m_fRotYBetween);
	if (m_fRotY != 0.0f) m = Concat(m, RotRad('Y', m_fRotY));
	if (m_fRotYNow < m_fRotYGoal)
	{
		m_fRotYNow = m_fRotYNow + ROTY_STEP;
		if (m_fRotYGoal <= m_fRotYNow) m_fRotYNow = m_fRotYGoal;
	}
	else if (m_fRotYGoal < m_fRotYNow)
	{
		m_fRotYNow = m_fRotYNow - ROTY_STEP;
		if (m_fRotYNow <= m_fRotYGoal) m_fRotYNow = m_fRotYGoal;
	}
	if (-m_fRotYNow != 0.0f) m = Concat(m, RotRad('Y', -m_fRotYNow));
	float target[4];
	MulVec4(m, m_vOffset, target);
	VecAdd(pos, target, target);
	std::memcpy(m_vTarget, target, sizeof(target));
	if (-m_fRotX != 0.0f) m = Concat(m, RotRad('X', -m_fRotX));
	float range = m_fRange;
	if (nSpeed == eSPEED_BRAKE) range = p.m_fRangeBrake;
	else if (nSpeed == eSPEED_NITRO) range = p.m_fRangeNitro;
	m_fRangeNow = Approach(range, m_fRangeNow, p.m_fRangeSpeed, p.m_fRangeBetween);
	const float local[4] = {0.0f, 0.0f, m_fRangeNow, 1.0f};
	float eye[4];
	MulVec4(m, local, eye);
	VecAdd(target, eye, eye);
	eye[1] = p.m_fPosYLimit;
	std::memcpy(m_vEye, eye, sizeof(eye));
}

// DOL 0x80129ad8: the clear orbit around the car: the saved eye - target offset turned by +0xa0 about Y. +0xa0 grows TWICE a frame by
// CLEAR_ROTSPEED_Y / 60 degrees (once directly, once through FUN_800c53c0 which clamps it at 4 pi), then wraps once past 2 pi.
// Dead in the DOL: 0.5 * offset (PSVECScale) and the lengths of both offsets (sqrt of dot products) are computed and dropped.
void cCamera::UpdateClear(const float* pos)
{
	const cParamCamera& p = *m_pParam;
	Mtx44 m = Identity();
	float d0[3];
	VecSub(m_vSaveEye, m_vSaveTarget, d0);
	const float step = DEG2RAD * p.m_fClearRotSpeedY;
	m_fOrbit = DEG2RAD * p.m_fClearRotSpeedY + m_fOrbit;
	m_fOrbit = m_fOrbit + step;   // FUN_800c53c0(&+0xa0, 4 pi, step)
	if (PI4 <= m_fOrbit) m_fOrbit = PI4;
	if (m_fOrbit > PI2) m_fOrbit = m_fOrbit - PI2;
	if (m_fOrbit != 0.0f) m = Concat(m, RotRad('Y', m_fOrbit));
	m = Concat(m, Trans(d0[0], d0[1], d0[2]));
	const float v[4] = {m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]};   // FUN_8013ecf8: the translation column
	float target[4];
	std::memcpy(target, m_vTarget, sizeof(target));
	VecAdd(pos, m_vOffset, target);
	float eye[4];
	std::memcpy(eye, m_vEye, sizeof(eye));
	VecAdd(target, v, eye);
	std::memcpy(m_vEye, eye, sizeof(eye));
	std::memcpy(m_vTarget, target, sizeof(target));
}

}
