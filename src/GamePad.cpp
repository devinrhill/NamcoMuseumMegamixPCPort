#include "GamePad.hpp"

#include "WiiPad.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "platform/input.hpp"

namespace GamePad
{
namespace
{
State g_pad[WiiPad::MAX_CHANNELS];
int g_shakeFrames[WiiPad::MAX_CHANNELS]{};
int g_idle[WiiPad::MAX_CHANNELS]{};       // pad +0x1e0
int g_motor[WiiPad::MAX_CHANNELS]{};      // frames left (0x804893a0 + n * 0x2a0)
float g_lastPtr[WiiPad::MAX_CHANNELS][2]{};
bool g_stickDpad = false;                 // DAT_8047f300 (0x800219d4)
int g_repeat[WiiPad::MAX_CHANNELS][4]{};  // pad +0x1ec.. (frames each dpad bit has been held)
constexpr uint kRepeatDelay = 0x1e, kRepeatRate = 5;   // pad +0x1e4 / +0x1e8 (0x800214a8)
constexpr float kSimulatedSwing = 2.0f;   // well above both thresholds of PAC_COMMON (0.75 dash, 0.55 hang escape)
constexpr int kShakeLength = 3;

// DOL table 0x80444ce0: the KPAD buttons behind game bit 0..12 (FUN_8014099c ORs the bit in when any of them is down)
constexpr uint kTable[13] = {WiiPad::BTN_UP, WiiPad::BTN_DOWN, WiiPad::BTN_LEFT, WiiPad::BTN_RIGHT, WiiPad::BTN_1, WiiPad::BTN_A, WiiPad::BTN_B,
                             WiiPad::BTN_2, WiiPad::BTN_MINUS, WiiPad::BTN_PLUS, WiiPad::BTN_HOME, WiiPad::BTN_Z, WiiPad::BTN_C};

uint Remap(uint kpad)
{
	uint out = 0;
	for (int i = 0; i < 13; i++)
		if (kpad & kTable[i]) out |= 1u << i;
	return out;
}
}

uint PhysicalBit(int dir) { return Remap(WiiPad::DpadMask((WiiPad::Dir)dir)); }

void Update()
{
	for (int c = 0; c < WiiPad::MAX_CHANNELS; c++)
	{
		const uint prev = g_pad[c].hold;   // pad +0x1b0: last frame's hold word (with the stick bits)
		g_pad[c].hold = Remap(WiiPad::Held(c));
		g_pad[c].trig = Remap(WiiPad::Pressed(c));
		g_pad[c].release = Remap(WiiPad::Released(c));
		g_pad[c].repeat = g_pad[c].trig & 0xfu;
		const WiiPad::Stick st = WiiPad::NunchukStick(c);
		// 0x80021660: with the switch on and no dpad bit down, the Nunchuk stick past +-0.5 is the dpad (y up = UP). The DOL ORs every bit held
		// last frame and not in the stick bits into release (held buttons included), kept as is.
		if (g_stickDpad && (g_pad[c].hold & 0xfu) == 0)
		{
			uint dir = 0;
			if (0.5f < st.y) dir |= PAD_UP;
			if (st.y < -0.5f) dir |= PAD_DOWN;
			if (st.x < -0.5f) dir |= PAD_LEFT;
			if (0.5f < st.x) dir |= PAD_RIGHT;
			g_pad[c].trig |= dir & (dir ^ prev);
			g_pad[c].repeat |= dir & (dir ^ prev);
			g_pad[c].hold |= dir;
			g_pad[c].release |= prev & ~dir;
		}
		// 0x80021720: a dpad bit held for 30 frames repeats, then every 5
		for (int b = 0; b < 4; b++)
		{
			if ((g_pad[c].hold & (1u << b)) == 0)
			{
				g_repeat[c][b] = 0;
				continue;
			}
			const uint n = (uint)++g_repeat[c][b];
			if (kRepeatDelay <= n && (n - kRepeatDelay) % kRepeatRate == 0) g_pad[c].repeat |= 1u << b;
		}
		if (WiiPad::Pressed(c) & WiiPad::BTN_SHAKE) g_shakeFrames[c] = kShakeLength;
		g_pad[c].accSpeed = g_shakeFrames[c] > 0 ? kSimulatedSwing : 0.0f;
		g_pad[c].acc[0] = 0.0f;
		g_pad[c].acc[1] = g_shakeFrames[c] > 0 ? 2.5f : 0.0f;
		g_pad[c].acc[2] = -1.0f;
		g_pad[c].stick[0] = st.x;
		g_pad[c].stick[1] = st.y;
		if (g_shakeFrames[c] > 0) g_shakeFrames[c]--;
		const WiiPad::Pointer ptr = WiiPad::GetPointer(c);
		g_pad[c].pointerCount = ptr.valid ? 1 : 0;
		g_pad[c].pointerX = ptr.valid ? ptr.x : 0.0f;
		g_pad[c].pointerY = ptr.valid ? -ptr.y : 0.0f;
		// 0x80021514 tail: the idle counter restarts on any held button, a swing, a moving pointer, a tilted stick or a changed
		// acceleration (thresholds r2 constants), else counts up to 600. UNVERIFIED: the native inputs only approximate the KPAD tests.
		const bool moved = g_pad[c].pointerCount > 0 &&
		                   (std::fabs(g_pad[c].pointerX - g_lastPtr[c][0]) > 0.001f || std::fabs(g_pad[c].pointerY - g_lastPtr[c][1]) > 0.001f);
		g_lastPtr[c][0] = g_pad[c].pointerX;
		g_lastPtr[c][1] = g_pad[c].pointerY;
		if (g_pad[c].hold != 0 || g_pad[c].accSpeed > 0.0f || moved || std::fabs(st.x) > 0.1f || std::fabs(st.y) > 0.1f) g_idle[c] = 0;
		else if (g_idle[c] < 600) g_idle[c]++;
		// 0x80140ad4: the motor's frame count runs down, then the motor stops
		if (g_motor[c] > 0 && --g_motor[c] < 1)
		{
			g_motor[c] = 0;
			platform::PadRumble(c, 0.0f);
		}
	}
}

// DOL 0x80021a14 [audited]
bool Rumble(int n, int nMSec)
{
	if (n < 0 || n >= WiiPad::MAX_CHANNELS) return false;
	if (g_idle[n] >= 600)
	{
		if (std::getenv("MEGAMIX_RUMBLE_LOG")) std::fprintf(stderr, "rumble: pad %d %d ms refused (the remote lies still)\n", n, nMSec);
		return false;
	}
	if (nMSec == 0)
	{
		g_motor[n] = 0;   // 0x80141280: WPADControlMotor(n, 0)
		platform::PadRumble(n, 0.0f);
		return true;
	}
	const int frames = nMSec * 6000 / 100000;   // cKingdomSys::nMSecToFrame (0x8000d7b4)
	g_motor[n] = frames;                        // 0x801411f4: WPADControlMotor(n, 1)
	platform::PadRumble(n, (float)frames / 60.0f + 0.1f);   // raylib needs a duration: the countdown above stops it on time
	if (std::getenv("MEGAMIX_RUMBLE_LOG")) std::fprintf(stderr, "rumble: pad %d %d ms (%d frames)\n", n, nMSec, frames);
	return true;
}

// DOL 0x800219d4
void SetStickDpad(bool on) { g_stickDpad = on; }

int IdleFrames(int n) { return g_idle[n < 0 ? 0 : (n >= WiiPad::MAX_CHANNELS ? WiiPad::MAX_CHANNELS - 1 : n)]; }

int Count()
{
	return WiiPad::MAX_CHANNELS;
}

const State& Get(int n)
{
	return g_pad[n < 0 ? 0 : (n >= WiiPad::MAX_CHANNELS ? WiiPad::MAX_CHANNELS - 1 : n)];
}
}
