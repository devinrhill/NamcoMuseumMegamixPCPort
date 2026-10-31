#pragma once
// Thin platform API. Game code includes this; only platform_raylib.cpp includes raylib.h
// (raylib's Vector3/Vector4/Camera/Matrix collide with the game's own types in common.hpp).

namespace platform
{
bool Init(int width, int height, const char* title);
// Edge smoothing (4x MSAA on the window); on by default. Call before Init (--no-smooth, MEGAMIX_NOSMOOTH=1 / MEGAMIX_NOFXAA=1).
void SetSmoothing(bool on);
bool IsFullscreen();
void ToggleWindowFullscreen(); // borderless fullscreen on / off (F11, Alt+Enter, the dev window's UI tab)
void SetFullscreen(bool on); // before Init: start in borderless fullscreen (--fullscreen); F11 / Alt+Enter toggle it later
void Shutdown();
bool ShouldClose();
// the game asks to end (job 147, OSReturnToMenu): ShouldClose() is true from the next check on
void RequestClose();
void BeginFrame();
void SetClearColor(unsigned char r, unsigned char g, unsigned char b); // the frame clear colour (DOL 0x80138e2c), kept until changed
void EndFrame();
void DrawText(const char* text, int x, int y, int size);
void DrawFps(int x, int y);
// True on the frame the key (an ASCII letter, case-insensitive) went down.
bool KeyPressed(char key);
}

namespace platform
{
// Mouse state for tools such as the model viewer
void MouseDelta(float& dx, float& dy);
float MouseWheel();
bool MouseDown(int button); // 0 left, 1 right, 2 middle
bool KeyDown(char key);
int WindowWidth();
int WindowHeight();
}
