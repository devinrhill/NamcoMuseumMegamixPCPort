#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Wii-style controller state (WPAD/KPAD channels 0-3), fed by keyboard / gamepad / mouse through bindings
// the user can change in input.cfg. Button values are the real WPAD_BUTTON_* masks from the Revolution SDK
// (RVL_SDK/include/revolution/wpad.h), so game code written against KPAD bits works unchanged.
namespace WiiPad
{
constexpr int MAX_CHANNELS = 4;

enum Button : std::uint32_t
{
	BTN_LEFT = 0x0001,
	BTN_RIGHT = 0x0002,
	BTN_DOWN = 0x0004,
	BTN_UP = 0x0008,
	BTN_PLUS = 0x0010,
	BTN_2 = 0x0100,
	BTN_1 = 0x0200,
	BTN_B = 0x0400,
	BTN_A = 0x0800,
	BTN_MINUS = 0x1000,
	BTN_Z = 0x2000,   // Nunchuk
	BTN_C = 0x4000,   // Nunchuk
	BTN_HOME = 0x8000,
	BTN_SHAKE = 0x10000,   // virtual: not a KPAD button; a keyboard stand-in for swinging the remote (see GamePad::State::accSpeed)
};

enum StickDir { STICK_UP, STICK_DOWN, STICK_LEFT, STICK_RIGHT, STICK_DIR_COUNT };

struct Stick
{
	float x{};
	float y{}; // +y is up
};

struct Pointer
{
	float x{}; // -1 (left) .. 1 (right) across the 4:3 game screen (letterboxed in the window), like the sensor bar projection
	float y{}; // -1 (bottom) .. 1 (top)
	bool valid{};
};

// Wiimote orientation. The game (and the original Namco Museum) is played with the Wiimote held SIDEWAYS: the D-pad bits the game
// reads are the upright ones, so physical Left arrives as BTN_UP, Right as BTN_DOWN, Up as BTN_RIGHT and Down as BTN_LEFT
// (the dialogs move their cursor with 0x8 / 0x4 = "up" / "down" = left / right on a sideways remote). Bindings in input.cfg name the
// direction the player presses; with `Orientation = Sideways` (the default) they are rotated into the bits the game expects.
// `Orientation = Upright` passes them through unchanged.
enum Dir { DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN };
bool Sideways();
void SetSideways(bool sideways);
// A game may need the remote held a certain way regardless of input.cfg: Pac-N-Roll is a Nunchuk game (remote upright), so its jobs force Upright;
// every other job lifts the override and the configured orientation applies again.
void SetUprightOverride(bool on);
std::uint32_t DpadMask(Dir physicalDirection); // the BTN_* bit the game sees for a physical direction under the current orientation

// Loads input.cfg from `path`; if it does not exist, writes the defaults there first.
// Returns false (and keeps the defaults) when the file has errors; messages go to stderr.
bool Init(const std::string& path = "input.cfg");
void ResetToDefaults();
bool WriteConfig(const std::string& path);

// Call once per frame, before game logic runs.
void Update();

bool IsConnected(int channel);
std::uint32_t Held(int channel);
std::uint32_t Pressed(int channel);  // went down this frame
std::uint32_t Released(int channel); // went up this frame
std::uint32_t Repeat(int channel);   // pressed, or held long enough to auto-repeat
Stick NunchukStick(int channel);
// Whether the channel counts as having a Nunchuk plugged in (games skip their "connect the Nunchuk" card). The PC always has a stick, so true
// unless --no-nunchuk (channel 0) unplugs it.
bool NunchukConnected(int channel);
void SetNunchukConnected(int channel, bool connected);
Stick ClassicStick(int channel, int which); // 0 = left, 1 = right
Pointer GetPointer(int channel);

// Test hook: the given buttons count as held on channel 0 for the next Update() only (an injected tap).
void InjectHeld(std::uint32_t mask, int channel = 0);
// Test hook (--stick FRAME X Y): the Nunchuk stick of channel 0 for the next Update (-1..1, y up); cleared after it is read.
void InjectStick(float x, float y);
// Test hook: from now on channel 0's pointer is at (x, y) in the 640 x 480 game screen (origin top-left, y down) instead of the mouse. Overrides the mouse until cleared.
void InjectPointer(float gameX, float gameY);
void ClearInjectedPointer();

// Programmatic remapping (the same thing input.cfg does): replaces the sources of one logical input.
// A source is "Key:1", "Pad:South", "Axis:LeftX-", "Mouse:Left" (see input.cfg for the full list).
bool SetBinding(const std::string& input, const std::vector<std::string>& sources);
}
