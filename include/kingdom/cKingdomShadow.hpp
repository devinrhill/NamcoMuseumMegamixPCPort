#pragma once

// The light camera of the depth shadows: DOL kingdom::cKingdomShadow (vtable 0x8038aa10 region, ctor 0x80023e8c, 0xf0 bytes), the object at
// cTaskScnRoot +0x1fc. It drives the scene root's g3d camera 4 (the "light camera"): an orthographic aim camera. The game code only sets the
// parameters below (the DOL calls are named per setter); NativeScene renders the casters with the resulting camera every frame while the shadow
// is enabled (cTaskScnRoot +0x1f8, NativeScene::SetShadow) and the receivers (materials with ramp16_tex / depth_tex) compare against that map.
// NOT PORTED: mode 1 (UpdateNativeCamera 0x800244dc: position / target fit to a size, no game of this port selects it), the ramp matrix
// (+0x20..) the receivers' TexMtxEffect used (replaced by the shader's own projection) and the 8 bit depth format (+4 = 0: Z8).
namespace kingdom
{
class cKingdomShadow
{
public:
	void Reset();                                                   // DOL 0x800241e0 (the ctor's defaults)
	void SetMode(int mode) { m_nMode = mode; }                      // DOL 0x80091fc0 (+0x80; 0 = the target + angles + radius camera below)
	void SetTarget(float x, float y, float z) { m_vTarget[0] = x; m_vTarget[1] = y; m_vTarget[2] = z; }   // DOL 0x800242f0 (+0xa0)
	void SetAngles(float a, float b, float c) { m_fAngle[0] = a; m_fAngle[1] = b; m_fAngle[2] = c; }       // DOL 0x800244c4 (+0xc0)
	void SetRadius(float r) { m_fRadius = r; }                      // DOL 0x80091fd0 (+0xdc)
	// DOL 0x80024308: SetOrtho(top = w / 2, bottom = -w / 2, left = -h / 2, right = h / 2, near, far) on the camera and the ramp matrix
	void SetOrtho(float w, float h, float zNear, float zFar) { m_fWidth = w; m_fHeight = h; m_fNear = zNear; m_fFar = zFar; m_bOrtho = true; }
	const float* vTarget() const { return m_vTarget; }
	float fRadius() const { return m_fRadius; }
	bool HasOrtho() const { return m_bOrtho; }
	// DOL 0x800244dc mode 0: eye = target + radius * (sin ry cos rx, sin rx, cos ry cos rx) with rx = angle 0, ry = angle 1; aim posture, twist angle 2.
	// Returns false when no projection was set yet.
	bool GetCamera(float eye[3], float target[3], float box[4], float& zNear, float& zFar) const;

private:
	int m_nMode{1};                    // +0x80
	float m_vTarget[3]{};              // +0xa0
	float m_fAngle[3]{};               // +0xc0
	float m_fRadius{};                 // +0xdc
	float m_fWidth{}, m_fHeight{}, m_fNear{}, m_fFar{};
	bool m_bOrtho{};
};
}
