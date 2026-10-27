#pragma once

// Mouse / view math shared by the dev window's stage editors (devtools_g03edit.cpp, devtools_g07edit.cpp): the 3D pass' matrices,
// window pixels <-> world. Matrices are column-major float[16] like NativeScene::GetMatrices. NOT part of the 1:1 game.

namespace devtools
{
bool ViewProj(float m[16]);                                      // projection * view of the last 3D pass; false before the first
bool Invert(const float m[16], float out[16]);
void Transform(const float m[16], const float p[4], float out[4]);
bool Project(const float vp[16], const float p[3], float& sx, float& sy);              // world -> window pixels (devui::GameView); false behind the camera
bool MouseRay(const float vp[16], float mx, float my, float o[3], float d[3]);          // window pixels -> world ray (d normalised)
}
