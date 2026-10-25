#include "gfx/ModelDraw.hpp"
#include "kingdom/NativeScene.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "gfx/Gfx3D.hpp"
#include "gfx/PtkSim.hpp"
#include "kingdom/cKingdomModelBase.hpp"

namespace kingdom
{
namespace
{
float g_eye[3] = {0, 0, 5}, g_target[3] = {0, 0, 0}, g_fovy = 45.0f, g_near = 0.2f, g_far = 4000.0f;
bool g_hasCamera = false;
bool g_ortho = false;
float g_orthoBox[4] = {};   // top, bottom, left, right
wii::LightResult g_light[8];
bool g_hasLight[8] = {};
std::vector<cKingdomModelBase*> g_list;    // registered models
std::vector<cKingdomModelBase*> g_frame;   // submitted this frame
bool g_perFrame = false;

float Dot(const float a[3], const float b[3]) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
}

void NativeScene::SetCamera(const float eye[3], const float target[3], float fovyDeg, float nearZ, float farZ)
{
	for (int i = 0; i < 3; i++) { g_eye[i] = eye[i]; g_target[i] = target[i]; }
	g_fovy = fovyDeg; g_near = nearZ; g_far = farZ;
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
bool NativeScene::HasCamera() { return g_hasCamera; }
void NativeScene::GetCamera(float eye[3], float target[3], float& fovyDeg)
{
	for (int i = 0; i < 3; i++) { eye[i] = g_eye[i]; target[i] = g_target[i]; }
	fovyDeg = g_fovy;
}
void NativeScene::SetLight(int set, const wii::LightResult& l) { if (set >= 0 && set < 8) { g_light[set] = l; g_hasLight[set] = true; } }
const wii::LightResult* NativeScene::Light(int set) { return (set >= 0 && set < 8 && g_hasLight[set]) ? &g_light[set] : nullptr; }
void NativeScene::ClearLights() { for (bool& b : g_hasLight) b = false; }
void NativeScene::Add(cKingdomModelBase* m) { if (std::find(g_list.begin(), g_list.end(), m) == g_list.end()) g_list.push_back(m); }
void NativeScene::Remove(cKingdomModelBase* m)
{
	g_list.erase(std::remove(g_list.begin(), g_list.end(), m), g_list.end());
	g_frame.erase(std::remove(g_frame.begin(), g_frame.end(), m), g_frame.end());
}
void NativeScene::Submit(cKingdomModelBase* m) { if (std::find(g_frame.begin(), g_frame.end(), m) == g_frame.end()) g_frame.push_back(m); }
void NativeScene::SetPerFrame(bool b) { g_perFrame = b; }
// a new stage / retry: models of the previous run (their tasks were released without Term) must not be drawn again (that was z-fighting on retry)
void NativeScene::Reset() { g_hasCamera = false; ClearLights(); g_list.clear(); g_frame.clear(); g_perFrame = false; }

// the camera matrices of the 3D pass (column-major, OpenGL clip space); false when the camera is degenerate
bool NativeScene::GetMatrices(float view[16], float proj[16])
{
	float f[3] = {g_target[0] - g_eye[0], g_target[1] - g_eye[1], g_target[2] - g_eye[2]};
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
	const float v[16] = {s[0], u[0], -f[0], 0, s[1], u[1], -f[1], 0, s[2], u[2], -f[2], 0, -Dot(s, g_eye), -Dot(u, g_eye), Dot(f, g_eye), 1};
	const float t = 1.0f / std::tan(0.5f * g_fovy * 3.14159265f / 180.0f), zn = g_near, zf = g_far, asp = 4.0f / 3.0f;
	const float p[16] = {t / asp, 0, 0, 0, 0, t, 0, 0, 0, 0, (zf + zn) / (zn - zf), -1, 0, 0, 2 * zf * zn / (zn - zf), 0};
	for (int i = 0; i < 16; i++) { view[i] = v[i]; proj[i] = p[i]; }
	if (g_ortho)
	{
		const float tp = g_orthoBox[0], bt = g_orthoBox[1], lf = g_orthoBox[2], rt = g_orthoBox[3];
		const float o[16] = {2 / (rt - lf), 0, 0, 0, 0, 2 / (tp - bt), 0, 0, 0, 0, -2 / (zf - zn), 0, -(rt + lf) / (rt - lf), -(tp + bt) / (tp - bt), -(zf + zn) / (zf - zn), 1};
		for (int i = 0; i < 16; i++) proj[i] = o[i];
	}
	return true;
}

void NativeScene::Flush()
{
	static const bool dbg = std::getenv("MEGAMIX_SCENE_DEBUG") != nullptr;
	if (dbg) std::fprintf(stderr, "scene: %zu models, camera %d eye (%.1f %.1f %.1f) target (%.1f %.1f %.1f) fovy %.1f near %.2f far %.1f\n", g_list.size(), (int)g_hasCamera, g_eye[0], g_eye[1], g_eye[2], g_target[0], g_target[1], g_target[2], g_fovy, g_near, g_far);
	// per-frame mode: only this frame's submissions, emptied after the pass (cTaskScnRoot::Draw 0x8009bc4c)
	struct ClearFrame { ~ClearFrame() { g_frame.clear(); } } clearFrame;
	const std::vector<cKingdomModelBase*>& list = g_perFrame ? g_frame : g_list;
	if (list.empty()) return;
	float view[16], proj[16];
	if (!GetMatrices(view, proj)) return;
	gfx::Begin3D(view, proj, true);
	static int frames = 0;
	if (dbg && ++frames == 100) for (cKingdomModelBase* m : list) m->DebugPrint();
	if (dbg && frames == 100 && g_perFrame)   // per-frame mode: registered visible models no task submitted this frame
		for (cKingdomModelBase* m : g_list)
			if (m->IsVisible() && std::find(g_frame.begin(), g_frame.end(), m) == g_frame.end()) { std::fprintf(stderr, "not submitted:"); m->DebugPrint(); }
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
		for (cKingdomModelBase* m : pass == 2 ? xlu : list) if (m->IsVisible() && !m->IsShadowOnly()) m->DrawNative(g_eye);
	}
	gfx::ModelDraw::SetPass(0);
	gfx::DrawParticles();
	gfx::End3D();
}
}
