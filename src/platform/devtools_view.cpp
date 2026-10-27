// See include/platform/devtools_view.hpp. Moved out of devtools_g03edit.cpp (41st session) for the Pac-Motos editor. NOT part of the 1:1 game.
#include "platform/devtools_view.hpp"
#include "platform/devui.hpp"

#include <cmath>

#include "kingdom/NativeScene.hpp"

namespace devtools
{
bool ViewProj(float m[16])
{
	float v[16], p[16];
	if (!kingdom::NativeScene::GetMatrices(v, p)) return false;
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
		{
			float s = 0;
			for (int k = 0; k < 4; k++) s += p[k * 4 + r] * v[c * 4 + k];
			m[c * 4 + r] = s;
		}
	return true;
}

bool Invert(const float m[16], float out[16])
{
	float inv[16];
	inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
	inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
	inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
	inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
	inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
	inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
	inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
	inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
	inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
	inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
	inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
	inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
	inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
	inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
	inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
	inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];
	const float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	if (std::fabs(det) < 1e-12f) return false;
	for (int i = 0; i < 16; i++) out[i] = inv[i] / det;
	return true;
}

void Transform(const float m[16], const float p[4], float out[4])
{
	for (int r = 0; r < 4; r++) out[r] = m[r] * p[0] + m[4 + r] * p[1] + m[8 + r] * p[2] + m[12 + r] * p[3];
}

// world -> window pixels; false behind the camera
bool Project(const float vp[16], const float p[3], float& sx, float& sy)
{
	const float in[4] = {p[0], p[1], p[2], 1.0f};
	float c[4];
	Transform(vp, in, c);
	if (c[3] <= 1e-5f) return false;
	const devui::Rect gv = devui::GameView();
	sx = gv.x + (c[0] / c[3] * 0.5f + 0.5f) * gv.w;
	sy = gv.y + (0.5f - c[1] / c[3] * 0.5f) * gv.h;
	return true;
}

bool MouseRay(const float vp[16], float mx, float my, float o[3], float d[3])
{
	float inv[16];
	if (!Invert(vp, inv)) return false;
	const devui::Rect gv = devui::GameView();
	const float x = (mx - gv.x) / gv.w * 2.0f - 1.0f, y = 1.0f - (my - gv.y) / gv.h * 2.0f;
	const float n[4] = {x, y, -1.0f, 1.0f}, f[4] = {x, y, 1.0f, 1.0f};
	float a[4], b[4];
	Transform(inv, n, a);
	Transform(inv, f, b);
	if (std::fabs(a[3]) < 1e-12f || std::fabs(b[3]) < 1e-12f) return false;
	float len = 0;
	for (int i = 0; i < 3; i++)
	{
		o[i] = a[i] / a[3];
		d[i] = b[i] / b[3] - o[i];
		len += d[i] * d[i];
	}
	len = std::sqrt(len);
	if (len < 1e-9f) return false;
	for (int i = 0; i < 3; i++) d[i] /= len;
	return true;
}

}
