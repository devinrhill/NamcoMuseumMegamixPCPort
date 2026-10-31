#include "gfx/ModelDraw.hpp"
#include "kingdom/NativeScene.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "gfx/Gfx3D.hpp"
#include "gfx/PtkSim.hpp"
#include "kingdom/cKingdomModelBase.hpp"
#include "platform/devtools.hpp"
#include "platform/devtools_g07edit.hpp"

namespace kingdom
{
namespace
{
float g_eye[3] = {0, 0, 5}, g_target[3] = {0, 0, 0}, g_fovy = 45.0f, g_near = 0.2f, g_far = 4000.0f;
bool g_hasCamera = false;
bool g_ortho = false;
float g_fixedAspect = 0.0f;   // > 0: camera 0 keeps this aspect (scene root flag +0x1d8 off)
float g_orthoBox[4] = {};   // top, bottom, left, right
bool g_shadowOn = false;
cKingdomShadow g_shadow;
float g_shEye[3], g_shTarget[3], g_shBox[4], g_shNear = 0.0f, g_shFar = 1.0f;
wii::LightResult g_light[8];
bool g_hasLight[8] = {};
NativeScene::FogSlot g_fogSlot[8];
bool g_hasFogSlot[8] = {};
std::vector<cKingdomModelBase*> g_list;    // registered models
std::vector<cKingdomModelBase*> g_frame;   // submitted this frame
bool g_perFrame = false;
int g_layout = 0;
struct Cam
{
	float eye[3], target[3], fovy, nearZ, farZ;
};
Cam g_cam[4] = {{{0, 0, 5}, {0, 0, 0}, 45.0f, 0.2f, 4000.0f},
                {{0, 0, 100.3f}, {0, 0, 0}, 30.0f, 0.1f, 32766.0f},
                {{0, 0, 100.3f}, {0, 0, 0}, 30.0f, 0.1f, 32766.0f},
                {{0, 0, 100.3f}, {0, 0, 0}, 30.0f, 0.1f, 32766.0f}};   // cameras 1..3 (0 = g_eye / g_target / ...)

int CameraCount(int layout)   // DOL 0x8009bc4c [audited: camera counts per layout and per-frame list compared; the GX scene setup is the native pass] (the switch at its start)
{
	switch (layout)
	{
	case 1: case 4: return 2;
	case 2: case 5: return 3;
	case 3: case 6: case 7: return 4;
	default: return 1;
	}
}
// DOL 0x8009b884 [audited]: the viewport (x, y, w, h) of camera n of a layout on the 640 x 480 screen
void ViewportOf(int layout, int n, float r[4])
{
	const unsigned w = 640, h = 480;
	switch (layout)
	{
	case 1: r[0] = (float)((w / 2) * n); r[1] = 0; r[2] = (float)(w / 2 - 1); r[3] = (float)h; break;
	case 2: r[0] = (float)((w / 3) * n); r[1] = 0; r[2] = (float)(w / 3 - 1); r[3] = (float)h; break;
	case 3: r[0] = (float)((w / 4) * n); r[1] = 0; r[2] = (float)(w / 4 - 1); r[3] = (float)h; break;
	case 4: r[0] = 0; r[1] = (float)((h / 2) * n); r[2] = (float)w; r[3] = (float)(h / 2 - 1); break;
	case 5: r[0] = 0; r[1] = (float)((h / 3) * n); r[2] = (float)w; r[3] = (float)(h / 3 - 1); break;
	case 6: r[0] = 0; r[1] = (float)((h / 4) * n); r[2] = (float)w; r[3] = (float)(h / 4 - 1); break;
	case 7: r[0] = (n & 1) ? (float)(w / 2) : 0.0f; r[1] = (n & 2) ? (float)(h / 2) : 0.0f; r[2] = (float)(w / 2 - 1); r[3] = (float)(h / 2 - 1); break;
	default: r[0] = 0; r[1] = 0; r[2] = (float)w; r[3] = (float)h; break;
	}
}

float Dot(const float a[3], const float b[3]) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
}

void NativeScene::SetCamera(const float eye[3], const float target[3], float fovyDeg, float nearZ, float farZ)
{
	for (int i = 0; i < 3; i++) { g_eye[i] = eye[i]; g_target[i] = target[i]; }
	g_fovy = fovyDeg; g_near = nearZ; g_far = farZ;
	devtools::G03EditCamera(g_eye, g_target, g_far); // devtools: the Pac-N-Roll editor's orbit camera (not 1:1)
	devtools::G07EditCamera(g_eye, g_target, g_far); // devtools: the Pac-Motos editor's orbit camera (not 1:1)
	g_hasCamera = true;
	g_ortho = false;
}
void NativeScene::SetOrthoCamera(const float eye[3], const float target[3], float top, float bottom, float left, float right, float nearZ, float farZ)
{
	for (int i = 0; i < 3; i++) { g_eye[i] = eye[i]; g_target[i] = target[i]; }
	g_orthoBox[0] = top; g_orthoBox[1] = bottom; g_orthoBox[2] = left; g_orthoBox[3] = right;
	g_near = nearZ; g_far = farZ;
	g_hasCamera = true;
	g_ortho = true;
}
void NativeScene::SetShadow(bool enabled) { g_shadowOn = enabled; }
cKingdomShadow& NativeScene::Shadow() { return g_shadow; }
void NativeScene::SetLayout(int layout) { g_layout = layout; }
void NativeScene::SetCameraN(int n, const float eye[3], const float target[3], float fovyDeg, float nearZ, float farZ)
{
	if (n == 0)
	{
		SetCamera(eye, target, fovyDeg, nearZ, farZ);
		return;
	}
	if (n < 0 || n > 3) return;
	for (int i = 0; i < 3; i++) { g_cam[n].eye[i] = eye[i]; g_cam[n].target[i] = target[i]; }
	g_cam[n].fovy = fovyDeg; g_cam[n].nearZ = nearZ; g_cam[n].farZ = farZ;
}
bool NativeScene::GetViewport(int camera, float rect[4])
{
	if (camera < 0 || camera >= CameraCount(g_layout)) return false;
	ViewportOf(g_layout, camera, rect);
	return true;
}
bool NativeScene::HasCamera() { return g_hasCamera; }
void NativeScene::GetCamera(float eye[3], float target[3], float& fovyDeg)
{
	for (int i = 0; i < 3; i++) { eye[i] = g_eye[i]; target[i] = g_target[i]; }
	fovyDeg = g_fovy;
}
void NativeScene::GetClip(float& nearZ, float& farZ) { nearZ = g_near; farZ = g_far; }
void NativeScene::SetLight(int set, const wii::LightResult& l) { if (set >= 0 && set < 8) { g_light[set] = l; g_hasLight[set] = true; } }
const wii::LightResult* NativeScene::Light(int set) { return (set >= 0 && set < 8 && g_hasLight[set]) ? &g_light[set] : nullptr; }
void NativeScene::ClearLights() { for (bool& b : g_hasLight) b = false; for (bool& b : g_hasFogSlot) b = false; }
void NativeScene::SetFogSlot(int idx, const FogSlot& f) { if (idx >= 0 && idx < 8) { g_fogSlot[idx] = f; g_hasFogSlot[idx] = true; } }
NativeScene::FogSlot* NativeScene::GetFogSlot(int idx) { return (idx >= 0 && idx < 8 && g_hasFogSlot[idx]) ? &g_fogSlot[idx] : nullptr; }
void NativeScene::Add(cKingdomModelBase* m) { if (std::find(g_list.begin(), g_list.end(), m) == g_list.end()) g_list.push_back(m); }
void NativeScene::Remove(cKingdomModelBase* m)
{
	g_list.erase(std::remove(g_list.begin(), g_list.end(), m), g_list.end());
	g_frame.erase(std::remove(g_frame.begin(), g_frame.end(), m), g_frame.end());
}
void NativeScene::Submit(cKingdomModelBase* m) { if (std::find(g_frame.begin(), g_frame.end(), m) == g_frame.end()) g_frame.push_back(m); }
// MEGAMIX_SCENE_REGISTERED=1 (debug): every loaded visible model is drawn, as before the per-frame audit (for comparison shots)
void NativeScene::SetPerFrame(bool b) { static const bool registered = std::getenv("MEGAMIX_SCENE_REGISTERED") != nullptr; g_perFrame = b && !registered; }
// a new stage / retry: models of the previous run (their tasks were released without Term) must not be drawn again (that was z-fighting on retry)
void NativeScene::Reset() { g_fixedAspect = 0.0f; g_layout = 0; g_shadowOn = false; g_shadow.Reset(); for (int i = 1; i < 4; i++) g_cam[i] = Cam{{0, 0, 100.3f}, {0, 0, 0}, 30.0f, 0.1f, 32766.0f}; g_hasCamera = false; ClearLights(); g_list.clear(); g_frame.clear(); g_perFrame = false; }

// the camera matrices of the 3D pass (column-major, OpenGL clip space); false when the camera is degenerate
namespace
{
// the matrices of a camera (eye -> target, fovy / near / far, the aspect of its viewport; the ortho box only for camera 0 when set)
bool BuildMatrices(const float eye[3], const float target[3], float fovy, float zn, float zf, float asp, bool ortho, float view[16], float proj[16])
{
	float f[3] = {target[0] - eye[0], target[1] - eye[1], target[2] - eye[2]};
	float l = std::sqrt(Dot(f, f));
	if (l < 1e-6f) return false;
	for (float& c : f) c /= l;
	// nw4r g3d_camera.cpp (aim posture): straight down the screen's up is -z, straight up +z (the twist is undefined there)
	const bool vertical = std::fabs(f[0]) < 1e-6f && std::fabs(f[2]) < 1e-6f;
	const float up[3] = {0, vertical ? 0.0f : 1.0f, vertical ? (f[1] < 0 ? -1.0f : 1.0f) : 0.0f};
	float s[3] = {f[1] * up[2] - f[2] * up[1], f[2] * up[0] - f[0] * up[2], f[0] * up[1] - f[1] * up[0]};
	l = std::sqrt(Dot(s, s));
	if (l < 1e-6f) return false;
	for (float& c : s) c /= l;
	const float u[3] = {s[1] * f[2] - s[2] * f[1], s[2] * f[0] - s[0] * f[2], s[0] * f[1] - s[1] * f[0]};
	const float v[16] = {s[0], u[0], -f[0], 0, s[1], u[1], -f[1], 0, s[2], u[2], -f[2], 0, -Dot(s, eye), -Dot(u, eye), Dot(f, eye), 1};
	const float t = 1.0f / std::tan(0.5f * fovy * 3.14159265f / 180.0f);
	const float p[16] = {t / asp, 0, 0, 0, 0, t, 0, 0, 0, 0, (zf + zn) / (zn - zf), -1, 0, 0, 2 * zf * zn / (zn - zf), 0};
	for (int i = 0; i < 16; i++) { view[i] = v[i]; proj[i] = p[i]; }
	if (ortho)
	{
		const float tp = g_orthoBox[0], bt = g_orthoBox[1], lf = g_orthoBox[2], rt = g_orthoBox[3];
		const float o[16] = {2 / (rt - lf), 0, 0, 0, 0, 2 / (tp - bt), 0, 0, 0, 0, -2 / (zf - zn), 0, -(rt + lf) / (rt - lf), -(tp + bt) / (tp - bt), -(zf + zn) / (zf - zn), 1};
		for (int i = 0; i < 16; i++) proj[i] = o[i];
	}
	return true;
}
}

bool NativeScene::GetMatrices(float view[16], float proj[16])
{
	return BuildMatrices(g_eye, g_target, g_fovy, g_near, g_far, g_fixedAspect > 0.0f ? g_fixedAspect : 4.0f / 3.0f, g_ortho, view, proj);
}

void NativeScene::SetFixedAspect(float aspect) { g_fixedAspect = aspect; }

namespace
{
// one 3D pass of the scene with the given camera: translucent models are sorted by this camera's view z (rect = the split screen's viewport, or null)
void DrawPass(const std::vector<cKingdomModelBase*>& list, const float view[16], const float proj[16], const float eye[3], const float rect[4])
{
	if (rect != nullptr)
		gfx::Begin3DRect(view, proj, rect[0], rect[1], rect[2], rect[3]);
	else
		gfx::Begin3D(view, proj, true);
	// g3d (ScnRoot DrawOpa / DrawXlu): every opaque stream first, then every translucent one, so a translucent surface without depth
	// writes (the ringed planet's ring) is not painted over by an opaque model registered after it (the stage sky)
	// The translucent pass runs back to front: the engine calls ScnRoot::ZSort (0x802f80d0, from 0x8009b6e4) before DrawOpa / DrawXlu, and
	// LessZSortXlu orders by the view-space z of each model's origin (MTX_VIEW._23), smallest (farthest) first; equal z keeps the list order.
	// Without it a translucent model that writes depth with alpha 0 (a hanging boss missile's wings, CLA_01) hid the waterfall behind it.
	// (The ScnObj draw priorities are not modelled: every model has the default.)
	static const bool noZSort = std::getenv("MEGAMIX_NO_ZSORT") != nullptr;   // debug: translucent models in list order
	std::vector<cKingdomModelBase*> xlu(list.begin(), list.end());
	if (!noZSort)
	{
		std::vector<std::pair<float, cKingdomModelBase*>> z;
		z.reserve(xlu.size());
		for (cKingdomModelBase* m : xlu)
		{
			float w[16];
			m->CalcWorld(w);
			z.push_back({view[2] * w[12] + view[6] * w[13] + view[10] * w[14] + view[14], m});
		}
		std::stable_sort(z.begin(), z.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
		for (size_t i = 0; i < z.size(); i++) xlu[i] = z[i].second;
	}
	static const bool onePass = std::getenv("MEGAMIX_ONE_PASS") != nullptr;   // debug: the old per-model order
	for (int pass = onePass ? 0 : 1; pass <= (onePass ? 0 : 2); pass++)
	{
		gfx::ModelDraw::SetPass(pass);
		for (cKingdomModelBase* m : pass == 2 ? xlu : list) if (m->IsVisible() && !m->IsShadowOnly()) m->DrawNative(eye);
	}
	gfx::ModelDraw::SetPass(0);
	gfx::DrawParticles();
	devtools::DrawOverlay3D(); // devtools: --g03-show-hit / --g03-show-gimmick-hit (not 1:1)
	gfx::End3D();
}
}

namespace
{
// the light camera's matrices (an aim camera like the main one, orthographic box g_shBox)
bool BuildLight(float view[16], float proj[16])
{
	if (!g_shadow.GetCamera(g_shEye, g_shTarget, g_shBox, g_shNear, g_shFar)) return false;
	float dummy[16];
	if (!BuildMatrices(g_shEye, g_shTarget, 45.0f, g_shNear, g_shFar, 1.0f, false, view, dummy)) return false;
	const float tp = g_shBox[0], bt = g_shBox[1], lf = g_shBox[2], rt = g_shBox[3], zn = g_shNear, zf = g_shFar;
	if (tp == bt || lf == rt || zn == zf) return false;
	const float o[16] = {2 / (rt - lf), 0, 0, 0, 0, 2 / (tp - bt), 0, 0, 0, 0, -2 / (zf - zn), 0, -(rt + lf) / (rt - lf), -(tp + bt) / (tp - bt), -(zf + zn) / (zf - zn), 1};
	for (int i = 0; i < 16; i++) proj[i] = o[i];
	return true;
}

// DOL 0x80024ce4: the casters from the light camera into the depth map; the receivers of this frame's passes use it
void ShadowPass(const std::vector<cKingdomModelBase*>& list)
{
	gfx::SetShadowReceiver(nullptr, nullptr);
	if (!g_shadowOn) return;
	float view[16], proj[16];
	if (!BuildLight(view, proj)) return;
	if (!gfx::ShadowBeginPass(view, proj, 512, 512)) return;
	gfx::ModelDraw::SetPass(0);
	for (cKingdomModelBase* m : list)
		if (m->IsVisible() && (m->IsDrawShadowDepth() || m->IsShadowOnly())) m->DrawNative(g_shEye);
	gfx::ShadowEndPass();
	gfx::SetShadowReceiver(view, proj);
}
}

bool NativeScene::ShadowMatrices(const cKingdomShadow& sh, float view[16], float proj[16])
{
	const cKingdomShadow saved = g_shadow;
	g_shadow = sh;
	const bool ok = BuildLight(view, proj);
	g_shadow = saved;
	return ok;
}

void NativeScene::Flush()
{
	static const bool dbg = std::getenv("MEGAMIX_SCENE_DEBUG") != nullptr;
	if (dbg) std::fprintf(stderr, "scene: %zu models, camera %d eye (%.1f %.1f %.1f) target (%.1f %.1f %.1f) fovy %.1f near %.2f far %.1f\n", g_list.size(), (int)g_hasCamera, g_eye[0], g_eye[1], g_eye[2], g_target[0], g_target[1], g_target[2], g_fovy, g_near, g_far);
	// per-frame mode: only this frame's submissions, emptied after the pass (cTaskScnRoot::Draw 0x8009bc4c)
	struct ClearFrame { ~ClearFrame() { g_frame.clear(); } } clearFrame;
	if (g_perFrame) devtools::G07EditPreviewFrame(g_frame); // devtools: the Motos editor's model preview (not 1:1)
	const std::vector<cKingdomModelBase*>& list = g_perFrame ? g_frame : g_list;
	if (list.empty()) return;
	static int frames = 0;
	static const int dbgFrame = dbg && std::atoi(std::getenv("MEGAMIX_SCENE_DEBUG")) > 1 ? std::atoi(std::getenv("MEGAMIX_SCENE_DEBUG")) : 100;   // the 3D pass to list
	if (dbg && ++frames == dbgFrame) for (cKingdomModelBase* m : list) m->DebugPrint();
	if (dbg && frames == dbgFrame)   // registered visible models no task submitted this frame (in registered mode: what per-frame mode would drop)
		for (cKingdomModelBase* m : g_list)
			if (m->IsVisible() && std::find(g_frame.begin(), g_frame.end(), m) == g_frame.end()) { std::fprintf(stderr, "not submitted:"); m->DebugPrint(); }
	ShadowPass(list);
	struct ShadowOff { ~ShadowOff() { gfx::SetShadowReceiver(nullptr, nullptr); } } shadowOff;
	const int nCam = CameraCount(g_layout);
	if (nCam <= 1)
	{
		float view[16], proj[16];
		if (!GetMatrices(view, proj)) return;
		DrawPass(list, view, proj, g_eye, nullptr);
		return;
	}
	// the split screen: every camera of the layout draws the scene into its viewport (DOL 0x8009bc4c: SetViewport / SetScissor per camera, the
	// aspect of each camera = its viewport's, flags +0x1d8..+0x1db)
	for (int c = 0; c < nCam; c++)
	{
		float r[4], view[16], proj[16];
		ViewportOf(g_layout, c, r);
		const float asp = r[2] / r[3];
		bool ok;
		if (c == 0)
			ok = BuildMatrices(g_eye, g_target, g_fovy, g_near, g_far, g_fixedAspect > 0.0f ? g_fixedAspect : asp, false, view, proj);
		else
			ok = BuildMatrices(g_cam[c].eye, g_cam[c].target, g_cam[c].fovy, g_cam[c].nearZ, g_cam[c].farZ, asp, false, view, proj);
		if (!ok) continue;
		DrawPass(list, view, proj, c == 0 ? g_eye : g_cam[c].eye, r);
	}
}
}
