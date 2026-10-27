#include "platform/input.hpp"
#include "platform/devui.hpp"
#include <raylib.h>
#include <cctype>
#include <cstring>
#include <string>

namespace
{
struct Named { const char* name; int code; };

const Named kKeys[] = {
	{"0", KEY_ZERO}, {"1", KEY_ONE}, {"2", KEY_TWO}, {"3", KEY_THREE}, {"4", KEY_FOUR}, {"5", KEY_FIVE},
	{"6", KEY_SIX}, {"7", KEY_SEVEN}, {"8", KEY_EIGHT}, {"9", KEY_NINE},
	{"A", KEY_A}, {"B", KEY_B}, {"C", KEY_C}, {"D", KEY_D}, {"E", KEY_E}, {"F", KEY_F}, {"G", KEY_G}, {"H", KEY_H},
	{"I", KEY_I}, {"J", KEY_J}, {"K", KEY_K}, {"L", KEY_L}, {"M", KEY_M}, {"N", KEY_N}, {"O", KEY_O}, {"P", KEY_P},
	{"Q", KEY_Q}, {"R", KEY_R}, {"S", KEY_S}, {"T", KEY_T}, {"U", KEY_U}, {"V", KEY_V}, {"W", KEY_W}, {"X", KEY_X},
	{"Y", KEY_Y}, {"Z", KEY_Z},
	{"Up", KEY_UP}, {"Down", KEY_DOWN}, {"Left", KEY_LEFT}, {"Right", KEY_RIGHT},
	{"Enter", KEY_ENTER}, {"Space", KEY_SPACE}, {"Escape", KEY_ESCAPE}, {"Tab", KEY_TAB}, {"Backspace", KEY_BACKSPACE},
	{"LShift", KEY_LEFT_SHIFT}, {"RShift", KEY_RIGHT_SHIFT}, {"LCtrl", KEY_LEFT_CONTROL}, {"RCtrl", KEY_RIGHT_CONTROL},
	{"LAlt", KEY_LEFT_ALT}, {"RAlt", KEY_RIGHT_ALT},
	{"Minus", KEY_MINUS}, {"Equal", KEY_EQUAL}, {"Comma", KEY_COMMA}, {"Period", KEY_PERIOD}, {"Slash", KEY_SLASH},
	{"Semicolon", KEY_SEMICOLON}, {"Apostrophe", KEY_APOSTROPHE}, {"LBracket", KEY_LEFT_BRACKET}, {"RBracket", KEY_RIGHT_BRACKET},
	{"F1", KEY_F1}, {"F2", KEY_F2}, {"F3", KEY_F3}, {"F4", KEY_F4}, {"F5", KEY_F5}, {"F6", KEY_F6},
	{"F7", KEY_F7}, {"F8", KEY_F8}, {"F9", KEY_F9}, {"F10", KEY_F10}, {"F11", KEY_F11}, {"F12", KEY_F12},
	{"Num0", KEY_KP_0}, {"Num1", KEY_KP_1}, {"Num2", KEY_KP_2}, {"Num3", KEY_KP_3}, {"Num4", KEY_KP_4},
	{"Num5", KEY_KP_5}, {"Num6", KEY_KP_6}, {"Num7", KEY_KP_7}, {"Num8", KEY_KP_8}, {"Num9", KEY_KP_9},
};

// Gamepad buttons use positional (Xbox-style) names so they mean the same on every controller.
const Named kPadButtons[] = {
	{"South", GAMEPAD_BUTTON_RIGHT_FACE_DOWN}, {"East", GAMEPAD_BUTTON_RIGHT_FACE_RIGHT},
	{"West", GAMEPAD_BUTTON_RIGHT_FACE_LEFT}, {"North", GAMEPAD_BUTTON_RIGHT_FACE_UP},
	{"L1", GAMEPAD_BUTTON_LEFT_TRIGGER_1}, {"R1", GAMEPAD_BUTTON_RIGHT_TRIGGER_1},
	{"L2", GAMEPAD_BUTTON_LEFT_TRIGGER_2}, {"R2", GAMEPAD_BUTTON_RIGHT_TRIGGER_2},
	{"Back", GAMEPAD_BUTTON_MIDDLE_LEFT}, {"Guide", GAMEPAD_BUTTON_MIDDLE}, {"Start", GAMEPAD_BUTTON_MIDDLE_RIGHT},
	{"L3", GAMEPAD_BUTTON_LEFT_THUMB}, {"R3", GAMEPAD_BUTTON_RIGHT_THUMB},
	{"DUp", GAMEPAD_BUTTON_LEFT_FACE_UP}, {"DDown", GAMEPAD_BUTTON_LEFT_FACE_DOWN},
	{"DLeft", GAMEPAD_BUTTON_LEFT_FACE_LEFT}, {"DRight", GAMEPAD_BUTTON_LEFT_FACE_RIGHT},
};

const Named kPadAxes[] = {
	{"LeftX", GAMEPAD_AXIS_LEFT_X}, {"LeftY", GAMEPAD_AXIS_LEFT_Y},
	{"RightX", GAMEPAD_AXIS_RIGHT_X}, {"RightY", GAMEPAD_AXIS_RIGHT_Y},
	{"LTrigger", GAMEPAD_AXIS_LEFT_TRIGGER}, {"RTrigger", GAMEPAD_AXIS_RIGHT_TRIGGER},
};

const Named kMouse[] = {{"Left", MOUSE_BUTTON_LEFT}, {"Right", MOUSE_BUTTON_RIGHT}, {"Middle", MOUSE_BUTTON_MIDDLE}};

template<size_t N>
int Find(const Named (&table)[N], const char* name)
{
	for (const Named& n : table)
	{
		if (strcasecmp(n.name, name) == 0) return n.code;
	}
	return -1;
}
}

namespace platform
{
int KeyFromName(const char* name) { return Find(kKeys, name); }
int PadButtonFromName(const char* name) { return Find(kPadButtons, name); }
int PadAxisFromName(const char* name) { return Find(kPadAxes, name); }
int MouseButtonFromName(const char* name) { return Find(kMouse, name); }

bool IsKeyDown(int key) { return key >= 0 && !devui::Typing() && ::IsKeyDown(key); }   // devtools: not while the dev window has the keyboard
bool IsPadConnected(int pad) { return ::IsGamepadAvailable(pad); }
bool IsPadButtonDown(int pad, int button) { return ::IsGamepadAvailable(pad) && ::IsGamepadButtonDown(pad, button); }
float PadAxis(int pad, int axis) { return ::IsGamepadAvailable(pad) ? ::GetGamepadAxisMovement(pad, axis) : 0.0f; }
void PadRumble(int pad, float seconds)
{
	if (!::IsGamepadAvailable(pad)) return;
	if (seconds > 0.0f) ::SetGamepadVibration(pad, 1.0f, 1.0f, seconds);
	else ::SetGamepadVibration(pad, 0.0f, 0.0f, 0.0f);
}
bool IsMouseButtonDown(int button) { return !devui::WantsMouse() && ::IsMouseButtonDown(button); }   // devtools: not over the dev window
void MousePosition(float& x, float& y) { Vector2 p = ::GetMousePosition(); x = p.x; y = p.y; }
void WindowSize(int& w, int& h) { w = ::GetScreenWidth() - devui::ReservedWidth(); h = ::GetScreenHeight(); }   // devtools: the game area (left of the dev window)
}
