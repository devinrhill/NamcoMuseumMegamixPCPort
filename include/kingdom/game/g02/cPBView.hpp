#pragma once

#include "gfx/Gfx.hpp"
#include "kingdom/NativeScene.hpp"

// Rally-X Remix (g02): the push-back layers of the split screen. The DOL's cTaskPBAnita objects (cTaskConsMud, cTaskConsPBMultiNO / BG)
// carry a camera number (+0x1b8 of the task, the player) and are drawn by cTaskScnRoot::Draw 0x8009bc4c inside that camera's pass: viewport
// and scissor of the camera's rectangle, an orthographic projection of the whole 640 x 480 screen, so the layer appears scaled into the
// viewport. Natively the layer is drawn after the scene with a 2D transform into the camera's viewport (no scissor: the layers stay inside).
namespace kingdom::game::g02
{
class ScopedPBView
{
public:
	// Sets the transform of camera `nCamera`; ok() is false when the current layout has no such camera (nothing may be drawn)
	explicit ScopedPBView(int nCamera)
	{
		float r[4];
		m_ok = kingdom::NativeScene::GetViewport(nCamera, r);
		if (!m_ok) return;
		const float w = (float)gfx::SCREEN_W, h = (float)gfx::SCREEN_H;
		if (r[2] >= w && r[3] >= h) return;   // the whole screen
		gfx::Set2DTransform(r[2] / w, r[3] / h, r[0] + 0.5f * r[2] - 0.5f * w, 0.5f * h - (r[1] + 0.5f * r[3]));
		m_set = true;
	}
	~ScopedPBView()
	{
		if (m_set) gfx::Reset2DTransform();
	}
	bool ok() const { return m_ok; }

private:
	bool m_ok{false};
	bool m_set{false};
};
}
