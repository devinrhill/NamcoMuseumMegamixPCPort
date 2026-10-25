#pragma once
// Host input API. Game code never sees raylib: keys, gamepad buttons and axes are plain ints that the
// platform layer translates. Names (for input.cfg) are resolved here, so the config can say "Key:1",
// "Pad:South" or "Axis:LeftX-" without the game headers knowing about the backend.

namespace platform
{
// name -> host code, or -1 when the name is unknown
int KeyFromName(const char* name);
int PadButtonFromName(const char* name);
int PadAxisFromName(const char* name);
int MouseButtonFromName(const char* name);

bool IsKeyDown(int key);
bool IsPadConnected(int pad);
bool IsPadButtonDown(int pad, int button);
float PadAxis(int pad, int axis); // -1..1
void PadRumble(int pad, float seconds); // the gamepad's motors at full strength for that long; 0 stops them (no-op without a gamepad)
bool IsMouseButtonDown(int button);
void MousePosition(float& x, float& y); // window pixels
void WindowSize(int& w, int& h);
}
