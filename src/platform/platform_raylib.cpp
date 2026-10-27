#include "platform/platform.hpp"
#include "gfx/Gfx.hpp"
#include "platform/devui.hpp"
#include <raylib.h>
#include <rlgl.h>

#include <cstdio>
#include <cstdlib>

namespace platform
{
namespace
{
// Edge smoothing = 4x MSAA on the window's framebuffer (GLFW samples hint, set before the window exists). It only smooths
// polygon edges: textures keep their own (linear) filtering and text / 2D quads on pixel boundaries are left untouched.
// It replaced an FXAA post pass, which blurred along any luma edge it found and broke up thin text (the overlay, small fonts).
bool g_smooth = true;
}

void SetSmoothing(bool on) { g_smooth = on && std::getenv("MEGAMIX_NOFXAA") == nullptr && std::getenv("MEGAMIX_NOSMOOTH") == nullptr; }

bool Init(int width, int height, const char* title)
{
	SetTraceLogLevel(LOG_ERROR);   // before InitWindow: most of raylib's INFO lines come from inside it
	if (g_smooth) SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(width, height, title);
	SetTargetFPS(std::getenv("MEGAMIX_UNCAPPED") ? 0 : 60);   // MEGAMIX_UNCAPPED=1: no frame cap (dev measurement, with MEGAMIX_DRAWSTATS)
	InitAudioDevice(); // may fail on machines without a sound card: audio::Open then returns false and the game runs silent
	return IsWindowReady();
}

void Shutdown()
{
	CloseAudioDevice();
	CloseWindow();
}
bool g_closeRequested = false;
bool ShouldClose() { return g_closeRequested || WindowShouldClose(); }
void RequestClose() { g_closeRequested = true; }

void BeginFrame()
{
	BeginDrawing();
	static const char* bg = std::getenv("MEGAMIX_BG");   // debug: clear colour "r,g,b" (0..255), e.g. to tell black geometry from nothing
	int r = 0, g = 0, b = 0;
	if (bg != nullptr && std::sscanf(bg, "%d,%d,%d", &r, &g, &b) == 3) ClearBackground(Color{(unsigned char)r, (unsigned char)g, (unsigned char)b, 255});
	else ClearBackground(BLACK);
}

void EndFrame()
{
	gfx::EndFrameFlush();
	EndDrawing();
}
void DrawText(const char* text, int x, int y, int size) { ::DrawText(text, x, y, size, RAYWHITE); }
void DrawFps(int x, int y) { DrawFPS(x, y); }

bool KeyPressed(char key)
{
	if (key >= 'a' && key <= 'z') key -= 'a' - 'A';
	return !devui::Typing() && IsKeyPressed(key);   // devtools: not while the dev window has the keyboard
}
}

namespace platform
{
void MouseDelta(float& dx, float& dy)
{
	Vector2 d = GetMouseDelta();
	dx = d.x;
	dy = d.y;
}
float MouseWheel() { return GetMouseWheelMove(); }
bool MouseDown(int button) { return IsMouseButtonDown(button); }
bool KeyDown(char key)
{
	if (key >= 'a' && key <= 'z') key -= 'a' - 'A';
	return IsKeyDown(key);
}
int WindowWidth() { return GetScreenWidth(); }
int WindowHeight() { return GetScreenHeight(); }
}
