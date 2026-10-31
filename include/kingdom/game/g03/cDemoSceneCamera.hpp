#pragma once

#include <string>

namespace wii
{
class Brres;
class ScnAnim;
}

namespace kingdom::game::g03
{

// Native stand-in for kingdom::game::cTaskSceneCamera as the Golvis04 death demo uses it (DOL 0x8011c62c):
//   ctor(ScnRoot of the game's scene, app +0x1b8 +0x1b4), FUN_80149f74(cam, 0, crc) = bind the SCN0 file, FUN_8014a388(cam, 0) = set it as
//   the ScnRoot's AnmScn, FUN_8014a43c(cam) = the AnmScn's UpdateFrame (vtable +0x2c). The task is stored in the app's camera slot (+0x2b0)
//   and the ScnRoot applies the AnmScn camera when the scene is calculated, so it wins over the game camera's ScnRoot camera.
// Natively: g03 cTaskCamera::Update hands the 3D pass this camera instead of its own while one is Active() (not a task: the DOL task's own
// Execute / Draw are not ported, only the three calls above).
class cDemoSceneCamera
{
public:
	cDemoSceneCamera();
	~cDemoSceneCamera();

	bool Bind(const std::string& entry);   // FUN_80149f74
	void SetAnm();                         // FUN_8014a388 (AnmScn index 0: frame 0)
	void UpdateFrame();                    // FUN_8014a43c (one frame; one-time animations stop at the end, looping ones wrap)
	bool Apply() const;                    // native: NativeScene::SetCamera from the current frame (false = nothing bound)

	// the camera that replaced the game camera (nullptr = none)
	static cDemoSceneCamera*& Active();

private:
	wii::Brres* m_pBrres{};
	wii::ScnAnim* m_pAnm{};
	bool m_bSet{};
	float m_fFrame{};
};

}
