#include "kingdom/game/g01/cTaskCamera.hpp"

#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include <cmath>

#include "kingdom/NativeScene.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "SoundManager.hpp"

namespace kingdom::game::g01
{

namespace
{
void Normalize(float v[3])
{
	const float l = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (l > 0.0f)
		for (int i = 0; i < 3; i++) v[i] /= l;
}
void Cross(const float a[3], const float b[3], float out[3])
{
	const float x = a[1] * b[2] - a[2] * b[1], y = a[2] * b[0] - a[0] * b[2], z = a[0] * b[1] - a[1] * b[0];
	out[0] = x; out[1] = y; out[2] = z;
}
}

// DOL 0x800d3194 [audited]
// The 3D sound listener (+0x26c, a 0x70-byte nw4r Sound3DListener made by 0x800d2ee8, added to the sound manager by 0x80137768):
// vtable +0x50 = 0x800d3064 reads PlayerParam.csv column 1 rows INTERIOR_SIZE (+0x260), DISTANCE (+0x264), DISTANCE_MAX (+0x268), then
// SetInteriorSize (0x801378d0), unit distance (0x801378d4 -> listener +0x50), max volume distance (0x801378d8 -> +0x4c)
cTaskCamera::cTaskCamera()
{
	SetName("g01::cTaskCamera");
	kingdom::cKingdomCSV csv;
	csv.RegistData("PlayerParam.csv");
	float interior = 0.0f, unit = 0.0f, maxVolume = 0.0f;
	csv.F32(csv.SearchRow(0, "INTERIOR_SIZE", 0, 0), 1, interior);
	csv.F32(csv.SearchRow(0, "DISTANCE", 0, 0), 1, unit);
	csv.F32(csv.SearchRow(0, "DISTANCE_MAX", 0, 0), 1, maxVolume);
	SoundManager::get_SE()->SetListenerParam(interior, maxVolume, unit);
}

// DOL 0x800d3868 [audited] (the listener leaves the sound manager)
cTaskCamera::~cTaskCamera()
{
	SoundManager::get_SE()->ClearListener();
}

// DOL 0x800d322c [audited: a camera twist is not available natively (up = the roll-free up, equal to the DOL's Euler up for twist 0); non-aim cameras are updated too] (nothing while paused, DAT_80479118). Only aim cameras (posture type 2) update the frame. The DOL builds "up" by turning
// (0, 1, 0) (DAT_80371e18) with the twist about the view direction and the Euler angles of that direction (0x801c1ccc); natively the camera
// twist is not available from kingdom::NativeScene, so up = world up made orthogonal to the view (UNVERIFIED for twisted cameras).
void cTaskCamera::Update()
{
	if (!kingdom::NativeScene::HasCamera()) return;
	float eye[3], target[3], fovy;
	kingdom::NativeScene::GetCamera(eye, target, fovy);
	for (int i = 0; i < 3; i++)
	{
		m_vPos[i] = eye[i];
		m_vAim[i] = target[i];
		m_vForward[i] = target[i] - eye[i];
	}
	Normalize(m_vForward);
	float right[3];
	const float worldUp[3] = {0.0f, 1.0f, 0.0f};
	Cross(m_vForward, worldUp, right);
	Normalize(right);
	Cross(right, m_vForward, m_vUp);
	Normalize(m_vUp);
	Cross(m_vForward, m_vUp, m_vRight);   // PSVECCrossProduct(+0x250, +0x240, +0x230)
	// vtable +0x40 = 0x800d304c: the listener's look-at (0x80137824: eye +0x210, target +0x220, up +0x240)
	SoundManager::get_SE()->SetListener(Vector4(m_vPos[0], m_vPos[1], m_vPos[2], 1.0f), Vector4(m_vAim[0], m_vAim[1], m_vAim[2], 1.0f),
	                                    Vector4(m_vUp[0], m_vUp[1], m_vUp[2], 0.0f));
}

bool cTaskCamera::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	(void)pParam;
	Update();
	return true;
}

// DOL 0x800d3520 [audited]
void cTaskCamera::LocalToWorld(float out[3], const float local[3]) const
{
	LocalDirToWorld(out, local);
	for (int i = 0; i < 3; i++) out[i] += m_vAim[i];
}

// DOL 0x800d3628 [audited]
void cTaskCamera::LocalDirToWorld(float out[3], const float local[3]) const
{
	const float x = -local[0], y = local[1], z = local[2];
	for (int i = 0; i < 3; i++) out[i] = m_vRight[i] * x + m_vUp[i] * y + m_vForward[i] * z;
}

// DOL 0x800d3714 [audited]
void cTaskCamera::WorldToLocal(float out[3], const float world[3]) const
{
	float d[3];
	for (int i = 0; i < 3; i++) d[i] = world[i] - m_vAim[i];
	out[0] = -(d[0] * m_vRight[0] + d[1] * m_vRight[1] + d[2] * m_vRight[2]);
	out[1] = d[0] * m_vUp[0] + d[1] * m_vUp[1] + d[2] * m_vUp[2];
	out[2] = d[0] * m_vForward[0] + d[1] * m_vForward[1] + d[2] * m_vForward[2];
}

// DOL 0x800d37c8 [audited]
void cTaskCamera::WorldDirToLocal(float out[3], const float world[3]) const
{
	const float x = -(world[0] * m_vRight[0] + world[1] * m_vRight[1] + world[2] * m_vRight[2]);
	const float y = world[0] * m_vUp[0] + world[1] * m_vUp[1] + world[2] * m_vUp[2];
	const float z = world[0] * m_vForward[0] + world[1] * m_vForward[1] + world[2] * m_vForward[2];
	out[0] = x;
	out[1] = y;
	out[2] = z;
}

}
