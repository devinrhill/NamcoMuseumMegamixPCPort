#pragma once

#include <vector>

#include "cJetTask.hpp"
#include "kingdom/cKingdomShadow.hpp"
#include "wii/Anim.hpp"

// Native stand-in for the g3d scene the Wii game draws its models into (the Unity port used GameObjects + a Camera; both scrapped).
// Models submit themselves while the tasks draw (cKingdomModelBase::UpdateMatrix), the scene task draws them all in one 3D pass between the
// model tasks (priority 65536..) and the 2D layers (0x80000000..), with the camera the camera task set this frame.
//   Light sets: the game numbers them (SetLightSetIdx): 0 background, 1 items, 2 characters, 3 goal / gates (from the stage's BIN_*_SCA files).
namespace kingdom
{
class cKingdomModelBase;

class NativeScene
{
public:
	static void SetCamera(const float eye[3], const float target[3], float fovyDeg, float nearZ, float farZ);
	// nw4r Camera::SetOrtho (top, bottom, left, right, near, far) with an aim posture eye -> target (the staff roll); SetCamera goes back to perspective
	static void SetOrthoCamera(const float eye[3], const float target[3], float top, float bottom, float left, float right, float nearZ, float farZ);
	// The split screen (cTaskScnRoot +0x1b8, DOL Draw 0x8009bc4c): `layout` 0 = one camera over the whole screen, 1 / 4 = two, 2 / 5 = three,
	// 3 / 6 / 7 = four cameras; 4 puts them above each other, 7 in the four quarters (zz_8009b884: viewport of camera n). Camera 0 is the
	// SetCamera one; cameras 1..3 are set here (the ScnRoot's default camera, eye (0, 0, 100.3) aiming at the origin, fovy 30, until then).
	static void SetLayout(int layout);
	static void SetCameraN(int n, const float eye[3], const float target[3], float fovyDeg, float nearZ, float farZ);
	// The viewport (x, y, w, h; the 640 x 480 screen, y down) of camera n of the current layout; false when the layout has no such camera
	// (layout 0 has camera 0 over the whole screen). The push-back layers of the split screen use it (g02 cTaskConsPBMulti*, the mud).
	static bool GetViewport(int camera, float rect[4]);
	// Depth shadows (cTaskScnRoot +0x1f8 enable, the cKingdomShadow object at +0x1fc = the light camera 4, DOL Draw 0x8009bc4c / 0x80024ce4): while
	// enabled and a light camera is set, every frame renders the casters (models flagged IsDrawShadowDepth or ShadowOnly) depth only from the
	// light (an orthographic aim camera: eye -> target, SetOrtho top / bottom / left / right / near / far) into a 512 x 512 map, and the receiver
	// materials (ramp16_tex / depth_tex) darken what that map covers. Reset() switches it off again.
	static void SetShadow(bool enabled);
	static cKingdomShadow& Shadow();                 // the light camera's parameters (cTaskScnRoot +0x1fc)
	static bool HasCamera();
	static void GetCamera(float eye[3], float target[3], float& fovyDeg);
	static void GetClip(float& nearZ, float& farZ);            // the near / far planes of the last SetCamera
	static bool GetMatrices(float view[16], float proj[16]);   // column-major view / projection of the 3D pass
	static void SetLight(int set, const wii::LightResult& light);
	static const wii::LightResult* Light(int set);
	static void ClearLights();
	// Models register when they load (Add) and leave when destroyed (Remove); children attached with RegistExpand are drawn by their parent.
	// The DOL draws only the models a task submitted this frame (cTaskModel::Draw 0x8009a650 -> cTaskScnRoot 0x8009c3f4; the list is emptied
	// by cTaskScnRoot::Draw 0x8009bc4c), so a model whose task skips its Draw (a dead Galaga enemy) disappears. SetPerFrame(true) gives that
	// behaviour (the DOL-only hand ports: Galaga); otherwise every registered visible model is drawn, which the Unity-converted games (g03,
	// g07, the hub) rely on: their code does not submit every model every frame. Reset() goes back to registered mode.
	static void Add(cKingdomModelBase* model);
	static void Remove(cKingdomModelBase* model);
	static void Submit(cKingdomModelBase* model);    // this frame (cKingdomModelBase::UpdateMatrix)
	static void SetPerFrame(bool b);
	static void Flush();                             // 3D pass: every submitted visible model, then the list is cleared
	static void Reset();                             // camera, lights, submissions (a new stage)
};

// Draws the scene (priority 131072: after every model task, before the HUD). Create one per app that uses models.
class cTaskNativeScene : public cJetTask
{
public:
	cTaskNativeScene() { nDrawPriority(131072u); SetName("cTaskNativeScene"); }
	void Draw(int nStatus, cJetTask* pParam) override { NativeScene::Flush(); }
};
}
