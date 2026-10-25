// Host-independent test of WiiPad + InGamePadManager using a fake platform input layer.
#include <cassert>
#include <cstdio>
#include <fstream>
#include <set>
#include <string>
#include "WiiPad.hpp"
#include "InGamePadManager.hpp"
#include "platform/input.hpp"

static std::set<int> g_keys;
static float g_axis[8];
namespace platform
{
int KeyFromName(const char* n) { std::string s = n; if (s == "1") return 1; if (s == "2") return 2; if (s == "Space") return 3; if (s == "W") return 4;
	if (s == "J") return 5; if (s == "Enter") return 6; if (s == "H") return 7; if (s == "Up") return 8; return s.size() == 1 ? 100 + s[0] : -1; }
int PadButtonFromName(const char*) { return 50; }
int PadAxisFromName(const char* n) { return std::string(n) == "LeftX" ? 0 : std::string(n) == "LeftY" ? 1 : 2; }
int MouseButtonFromName(const char*) { return 0; }
bool IsKeyDown(int k) { return g_keys.count(k) > 0; }
bool IsPadConnected(int) { return false; }
bool IsPadButtonDown(int, int) { return false; }
float PadAxis(int, int a) { return g_axis[a]; }
bool IsMouseButtonDown(int) { return false; }
void MousePosition(float& x, float& y) { x = y = 0; }
void WindowSize(int& w, int& h) { w = h = 100; }
}

#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); return 1; } } while (0)

static void frame() { WiiPad::Update(); InGamePadManager::Update(); }

int main()
{
	std::remove("/tmp/wiipad_test.cfg");
	CHECK(WiiPad::Init("/tmp/wiipad_test.cfg"));           // writes defaults
	std::ifstream f("/tmp/wiipad_test.cfg"); CHECK(f.good());

	// keyboard 1 / 2 -> Wiimote 1 / 2, with edges
	frame();
	CHECK(WiiPad::Held(0) == 0);
	g_keys = {1}; frame();
	CHECK(WiiPad::Pressed(0) == WiiPad::BTN_1 && WiiPad::Held(0) == WiiPad::BTN_1);
	frame();
	CHECK(WiiPad::Pressed(0) == 0 && WiiPad::Held(0) == WiiPad::BTN_1);
	g_keys = {1, 2}; frame();
	CHECK(WiiPad::Pressed(0) == WiiPad::BTN_2);
	g_keys = {2}; frame();
	CHECK(WiiPad::Released(0) == WiiPad::BTN_1);
	g_keys = {}; frame();

	// keyboard drives channel 0 only
	g_keys = {1}; frame();
	CHECK(WiiPad::Held(1) == 0);
	g_keys = {};

	// auto-repeat: first on press, then after the delay, then every few frames
	g_keys = {1}; int repeats = 0;
	for (int i = 0; i < 60; i++) { frame(); if (WiiPad::Repeat(0) & WiiPad::BTN_1) repeats++; }
	CHECK(repeats > 3 && repeats < 15);
	g_keys = {}; frame();

	// nunchuk stick from keys and from an analog axis
	g_keys = {4}; frame();
	CHECK(WiiPad::NunchukStick(0).y == 1.0f && WiiPad::NunchukStick(0).x == 0.0f);
	g_keys = {}; g_axis[0] = 0.6f; frame();
	CHECK(WiiPad::NunchukStick(0).x > 0.5f);
	g_axis[0] = 0.1f; frame();                                // inside the deadzone
	CHECK(WiiPad::NunchukStick(0).x == 0.0f);
	g_axis[0] = 0;

	// user remap: put Wiimote A on Space only
	CHECK(WiiPad::SetBinding("A", {"Key:Space"}));
	g_keys = {5}; frame(); CHECK(!(WiiPad::Held(0) & WiiPad::BTN_A));   // J no longer A
	g_keys = {3}; frame(); CHECK(WiiPad::Held(0) & WiiPad::BTN_A);
	CHECK(!WiiPad::SetBinding("A", {"Bogus:Thing"}));                   // rejected, binding kept
	CHECK(!WiiPad::SetBinding("NotAnInput", {"Key:1"}));
	g_keys = {3}; frame(); CHECK(WiiPad::Held(0) & WiiPad::BTN_A);

	// config file round trip
	{ std::ofstream o("/tmp/wiipad_test.cfg"); o << "# comment\n1 = Key:Space\nA = Key:J, Key:Enter\n"; }
	CHECK(WiiPad::Init("/tmp/wiipad_test.cfg"));
	g_keys = {3}; frame(); CHECK(WiiPad::Held(0) & WiiPad::BTN_1);
	g_keys = {5}; frame(); CHECK(WiiPad::Held(0) & WiiPad::BTN_A);
	{ std::ofstream o("/tmp/wiipad_test.cfg"); o << "A = Key:Nope\nthis is not valid\n"; }
	CHECK(!WiiPad::Init("/tmp/wiipad_test.cfg"));                       // reports errors, keeps defaults for the rest

	// InGamePadManager: game pad codes -> Wii buttons
	WiiPad::Init("/tmp/wiipad_test_defaults.cfg");
	g_keys = {}; frame();
	g_keys = {5}; frame();                                              // J = Wiimote A = "enter"
	CHECK(InGamePadManager::GetEnterKeyDown(0));
	CHECK(InGamePadManager::GetKey(0, PadDefine::eKEY_CIRCLE));
	frame();
	CHECK(!InGamePadManager::GetEnterKeyDown(0) && InGamePadManager::GetKey(0, PadDefine::eKEY_CIRCLE));
	g_keys = {}; frame();
	g_keys = {4}; frame();                                              // W = nunchuk stick up
	CHECK(InGamePadManager::GetKeyDown(0, PadDefine::eKEY_LSTICK_UP));
	frame();
	CHECK(!InGamePadManager::GetKeyDown(0, PadDefine::eKEY_LSTICK_UP) && InGamePadManager::GetKey(0, PadDefine::eKEY_LSTICK_UP));
	CHECK(InGamePadManager::GetStick(0, PadDefine::scLSTICK).y == 1.0f);
	InGamePadManager::set_Disable(true);
	CHECK(!InGamePadManager::GetKey(0, PadDefine::eKEY_LSTICK_UP));
	InGamePadManager::set_Disable(false);
	InGamePadManager::SetKeycodeMapping(PadDefine::eKEY_START, WiiPad::BTN_HOME);   // remap a game code
	g_keys = {7}; frame(); CHECK(InGamePadManager::GetKeyDown(0, PadDefine::eKEY_START));

	// Wiimote orientation: sideways by default (the game reads the upright bits), bindings name the physical direction
	{
		std::ofstream o("/tmp/wiipad_test.cfg"); o << "Left = Key:Space\nRight = Key:Z\nUp = Key:Q\nDown = Key:E\nShake = Key:X\n";
	}
	CHECK(WiiPad::Init("/tmp/wiipad_test.cfg"));
	CHECK(WiiPad::Sideways());
	g_keys = {3}; frame(); CHECK(WiiPad::Held(0) == WiiPad::BTN_UP && WiiPad::Pressed(0) == WiiPad::BTN_UP);   // physical Left -> KPAD up
	g_keys = {100 + 'Z'}; frame(); CHECK(WiiPad::Held(0) == WiiPad::BTN_DOWN);                                 // Right -> down
	g_keys = {100 + 'Q'}; frame(); CHECK(WiiPad::Held(0) == WiiPad::BTN_RIGHT);                                // Up -> right
	g_keys = {100 + 'E'}; frame(); CHECK(WiiPad::Held(0) == WiiPad::BTN_LEFT);                                 // Down -> left
	CHECK(WiiPad::DpadMask(WiiPad::DIR_LEFT) == WiiPad::BTN_UP && WiiPad::DpadMask(WiiPad::DIR_RIGHT) == WiiPad::BTN_DOWN);
	g_keys = {}; frame();
	// game pad codes follow the physical direction: eKEY_LEFT is what the player pressed on the left
	g_keys = {3}; frame(); CHECK(InGamePadManager::GetKeyDown(0, PadDefine::eKEY_LEFT) && !InGamePadManager::GetKeyDown(0, PadDefine::eKEY_RIGHT));
	g_keys = {}; frame();
	{
		std::ofstream o("/tmp/wiipad_test.cfg"); o << "Orientation = Upright\nLeft = Key:Space\nShake = Key:X\n";
	}
	CHECK(WiiPad::Init("/tmp/wiipad_test.cfg"));
	CHECK(!WiiPad::Sideways());
	g_keys = {3}; frame(); CHECK(WiiPad::Held(0) == WiiPad::BTN_LEFT);                                         // passed through
	CHECK(WiiPad::DpadMask(WiiPad::DIR_LEFT) == WiiPad::BTN_LEFT);
	g_keys = {}; frame();
	{
		std::ofstream o("/tmp/wiipad_test.cfg"); o << "Orientation = Diagonal\n";
	}
	CHECK(!WiiPad::Init("/tmp/wiipad_test.cfg"));                                                              // bad value reported
	WiiPad::SetSideways(true);

	std::printf("wiipad tests passed\n");
	return 0;
}
