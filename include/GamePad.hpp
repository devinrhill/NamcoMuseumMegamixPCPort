#pragma once

#include "common.hpp"

// The game's own pad words (DOL: get_pad1(n) 0x800219bc = block 0x8047eb00 + n * 0x200, filled by the per-frame wrapper 0x80021514 from the
// source block 0x80489210 + n * 0x2a0, which FUN_8014099c remaps through the 32 entry table at 0x80444ce0).
// The words are NOT KPAD masks: game bit n is set when the Wii button(s) of table entry n are down. The retail table:
//   0x1 UP(KPAD 0x8)  0x2 DOWN(0x4)  0x4 LEFT(0x1)  0x8 RIGHT(0x2)  0x10 ONE  0x20 A  0x40 B  0x80 TWO  0x100 MINUS  0x200 PLUS  0x400 HOME  0x800 Z  0x1000 C
// The wrapper clears bit 0x400 in all four words; the Home Button Menu sets it while it is open, which is why almost every task's Execute starts with
// `if ((get_pad1(0)->hold & 0x400) == 0)`. This port has no HBM: HOME held stands in for "HBM open" (freezes the game logic), see Update().
// Word order: hold, trig, repeat (the dpad bits auto-repeat; bit 0..3 only), release.
namespace GamePad
{
struct State
{
	uint hold{};      // +0x0
	uint trig{};      // +0x4
	uint repeat{};    // +0x8  (the dpad bits pressed this frame, then every 5 frames after 30 held: 0x80021720)
	uint release{};   // +0xc  (held last frame and not now: 0x8002170c..0x8002171c)
	// IR pointer (DOL: floats at pad+0x48 / +0x4c, int pointer count at +0xa8, read by kingdom::cTaskCursor::Execute 0x80095be4 and
	// every IsDpdHit): x -1 (left) .. 1 (right), y -1 (TOP) .. 1 (bottom) over the 640 x 480 screen (cursor = x * 320, y * 240, then y negated for
	// the y-up layouts). UNVERIFIED: the y direction is inferred from that negation, the exact KPAD range of y (+-0.75?) is not.
	// KPAD acc_speed (the game reads it at pad + 0x44): how fast the remote's acceleration changes, i.e. a swing. Pac-N-Roll dashes when it exceeds
	// PAD_SWING_SPEED_DASH (0.75) and escapes a hang above PAD_SWING_SPEED_ESCAPE_HANG (0.55). No accelerometer here: the Shake input (Space) gives a fixed 2.0 for 3 frames.
	float accSpeed{};
	float pointerX{};
	float pointerY{};
	int pointerCount{}; // > 0 while the remote points at the screen
	// KPAD acc (pad + 0x30, x y z in g) and the Nunchuk stick (pad + 0x80 / + 0x84, -1..1, y up): Gator Panic reads them directly (0x801b5258
	// swing detection, 0x801b50e0 hole choice). No accelerometer here: at rest acc = (0, 0, -1); the Shake input gives (0, 2.5, -1) for its 3
	// frames, which the SDK swing filter the game uses reads as a strong vertical swing with a falling pitch. UNVERIFIED: KPAD's real rest
	// vector / axes (the emulation only has to satisfy the game's test).
	float acc[3]{0.0f, 0.0f, -1.0f};
	float stick[2]{};
};

constexpr uint PAD_UP = 0x1, PAD_DOWN = 0x2, PAD_LEFT = 0x4, PAD_RIGHT = 0x8, PAD_1 = 0x10, PAD_A = 0x20, PAD_B = 0x40, PAD_2 = 0x80,
               PAD_MINUS = 0x100, PAD_PLUS = 0x200, PAD_HOME = 0x400, PAD_Z = 0x800, PAD_C = 0x1000;
constexpr uint BTN_B = PAD_HOME;   // legacy name used by the boot / title / attract ports: bit 0x400 = the HBM-open "freeze" bit (HOME here), not the B button
constexpr uint TRIG_DECIDE = PAD_A; // cAppTitle / the debug menus: get_pad1(0)->trig & 0x20 = A (the old synthesised "A / Plus / 1 / 2" was wrong)

// The game bit (PAD_UP / DOWN / LEFT / RIGHT) a physical d-pad direction produces under the current Wiimote orientation (WiiPad::Sideways()).
// Debug menus use it so that the Up arrow always moves up, whatever orientation the running game needs.
uint PhysicalBit(int wiiPadDir);   // a WiiPad::Dir

void Update();             // call once per frame after WiiPad::Update()
// DOL 0x80021a14 (Kingdom rumble): the remote's motor for nMSec ms (cKingdomSys::nMSecToFrame -> frames, 0x801411f4 starts the motor,
// cSysTaskPadSync::PreExecute 0x80140ad4 counts the frames down and stops it); 0 stops it at once (0x80141280). Refused (false) while the
// remote has lain still for 600 frames (pad +0x1e0, counted by 0x80021514). Natively the motor is the channel's gamepad (platform::PadRumble).
bool Rumble(int n, int nMSec);
int IdleFrames(int n);     // pad +0x1e0
// DOL 0x800219d4: the Nunchuk stick doubles as the dpad (DAT_8047f300; 0x80021514 turns a tilt past 0.5 into UP / DOWN / LEFT / RIGHT while no
// dpad bit is down). The game menus turn it on while they run (cAppGameBaseMenu 0x801434f8 / 0x801436ec, the matching 0x801cf828 / 0x801d0864,
// Gator Panic's menu mode).
void SetStickDpad(bool on);
int Count();               // number of pads (FUN_801411ec)
const State& Get(int n);   // get_pad1(n)
}
