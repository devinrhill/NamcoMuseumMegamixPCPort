#include "InGamePadManager.hpp"

#include <algorithm>

#include "WiiPad.hpp"

namespace
{
bool g_disable = false;
bool g_forcedStop = false;

constexpr float AXIS_THRESHOLD = 0.5f;

// What each game pad code reads: a Wii button mask, or a stick direction.
struct Mapping
{
	unsigned mask{};
	int dir{-1};     // WiiPad::Dir of a D-pad code: the mask depends on the CURRENT orientation (Pac-N-Roll switches it), so it is resolved per query
	int stick{-1};   // 0 = nunchuk / left stick, 1 = right stick; -1 = not a stick code
	int axis{};      // 0 = x, 1 = y
	int sign{};      // +1 / -1: the direction that counts as pressed
};

Mapping g_map[PadDefine::eKEY_COUNT];
bool g_mapReady = false;

Mapping Button(unsigned mask)
{
	Mapping m;
	m.mask = mask;
	return m;
}

Mapping Dpad(WiiPad::Dir d)
{
	Mapping m;
	m.dir = (int)d;
	return m;
}

Mapping StickDir(int stick, int axis, int sign)
{
	Mapping m;
	m.stick = stick;
	m.axis = axis;
	m.sign = sign;
	return m;
}

void InitMap()
{
	if (g_mapReady) return;
	g_mapReady = true;
	using P = PadDefine;
	// "Enter" is A and "cancel" is B, like the Wii menus; the remaining face buttons land on 1 and 2.
	g_map[P::eKEY_CIRCLE] = Button(WiiPad::BTN_A);
	g_map[P::eKEY_CROSS] = Button(WiiPad::BTN_B);
	g_map[P::eKEY_SQUARE] = Button(WiiPad::BTN_1);
	g_map[P::eKEY_TRIANGLE] = Button(WiiPad::BTN_2);
	g_map[P::eKEY_START] = Button(WiiPad::BTN_MINUS);   // DOL pause: get_pad1(0)->trig & 0x500 = MINUS (game bit 0x100) | HOME (0x400), see cAppG03PacnRollGame::Execute
	g_map[P::eKEY_SELECT] = Button(WiiPad::BTN_PLUS);
	// directions follow what the player presses, so they go through the Wiimote orientation (sideways by default)
	g_map[P::eKEY_UP] = Dpad(WiiPad::DIR_UP);
	g_map[P::eKEY_DOWN] = Dpad(WiiPad::DIR_DOWN);
	g_map[P::eKEY_LEFT] = Dpad(WiiPad::DIR_LEFT);
	g_map[P::eKEY_RIGHT] = Dpad(WiiPad::DIR_RIGHT);
	g_map[P::eKEY_L1] = Button(WiiPad::BTN_Z);
	g_map[P::eKEY_R1] = Button(WiiPad::BTN_C);
	g_map[P::eKEY_LSTICK_UP] = StickDir(0, 1, +1);
	g_map[P::eKEY_LSTICK_DOWN] = StickDir(0, 1, -1);
	g_map[P::eKEY_LSTICK_LEFT] = StickDir(0, 0, -1);
	g_map[P::eKEY_LSTICK_RIGHT] = StickDir(0, 0, +1);
	g_map[P::eKEY_RSTICK_UP] = StickDir(1, 1, +1);
	g_map[P::eKEY_RSTICK_DOWN] = StickDir(1, 1, -1);
	g_map[P::eKEY_RSTICK_LEFT] = StickDir(1, 0, -1);
	g_map[P::eKEY_RSTICK_RIGHT] = StickDir(1, 0, +1);
}

float StickValue(int channel, int stick, int axis)
{
	WiiPad::Stick s = stick == 0 ? WiiPad::NunchukStick(channel) : WiiPad::ClassicStick(channel, 1);
	return axis == 0 ? s.x : s.y;
}

// A stick direction code is "down" while the stick is held past the threshold; edges are derived
// from the previous frame's value so GetKeyDown/Up work for stick codes too.
struct StickEdge
{
	bool now[WiiPad::MAX_CHANNELS][PadDefine::eKEY_COUNT]{};
	bool before[WiiPad::MAX_CHANNELS][PadDefine::eKEY_COUNT]{};
};
StickEdge g_edge;

bool StickDown(int channel, const Mapping& m)
{
	return StickValue(channel, m.stick, m.axis) * m.sign > AXIS_THRESHOLD;
}

int Chan(int padNum)
{
	return std::clamp(padNum, 0, WiiPad::MAX_CHANNELS - 1);
}
}

void InGamePadManager::Update()
{
	InitMap();
	for (int c = 0; c < WiiPad::MAX_CHANNELS; c++)
	{
		for (int key = 0; key < PadDefine::eKEY_COUNT; key++)
		{
			g_edge.before[c][key] = g_edge.now[c][key];
			g_edge.now[c][key] = g_map[key].stick >= 0 && StickDown(c, g_map[key]);
		}
	}
}

bool InGamePadManager::get_Disable()
{
	return g_disable;
}

void InGamePadManager::set_Disable(bool value)
{
	g_disable = value;
}

Vector2 InGamePadManager::GetStick(int padNum, int stickNum, bool)
{
	if (g_disable || g_forcedStop) return Vector2();
	WiiPad::Stick s = stickNum == PadDefine::scLSTICK ? WiiPad::NunchukStick(Chan(padNum)) : WiiPad::ClassicStick(Chan(padNum), 1);
	return Vector2(s.x, s.y);
}

float InGamePadManager::GetLTrigger(int padNum)
{
	return GetKey(padNum, PadDefine::eKEY_L2) ? 1.0f : 0.0f;
}

float InGamePadManager::GetRTrigger(int padNum)
{
	return GetKey(padNum, PadDefine::eKEY_R2) ? 1.0f : 0.0f;
}

// Per-code queries ----------------------------------------------------------------------------------

static bool Query(int padNum, PadDefine::EPadKeycode key, int what)
{
	if (g_disable || g_forcedStop) return false;
	InitMap();
	if (key < 0 || key >= PadDefine::eKEY_COUNT) return false;
	int c = Chan(padNum);
	const Mapping& m = g_map[key];
	if (m.stick >= 0)
	{
		// Stick codes: edges come from the per-frame snapshot taken in Update().
		bool down = g_edge.now[c][key];
		bool was = g_edge.before[c][key];
		switch (what)
		{
		case 0: return down;
		case 1: return down && !was;
		case 2: return !down && was;
		default: return down && !was;
		}
	}
	const unsigned mask = m.dir >= 0 ? WiiPad::DpadMask((WiiPad::Dir)m.dir) : m.mask;
	if (mask == 0) return false;
	switch (what)
	{
	case 0: return (WiiPad::Held(c) & mask) != 0;
	case 1: return (WiiPad::Pressed(c) & mask) != 0;
	case 2: return (WiiPad::Released(c) & mask) != 0;
	default: return (WiiPad::Repeat(c) & mask) != 0;
	}
}

bool InGamePadManager::GetKey(int padNum, PadDefine::EPadKeycode key, bool) { return Query(padNum, key, 0); }
bool InGamePadManager::GetKeyDown(int padNum, PadDefine::EPadKeycode key, bool) { return Query(padNum, key, 1); }
bool InGamePadManager::GetKeyUp(int padNum, PadDefine::EPadKeycode key, bool) { return Query(padNum, key, 2); }
bool InGamePadManager::GetKeyRepeat(int padNum, PadDefine::EPadKeycode key, bool) { return Query(padNum, key, 3); }

// EPadKeyFlag is a bit-per-code mask in the same order as EPadKeycode (CROSS = 1, CIRCLE = 2, ...).
static bool QueryFlag(int padNum, EPadKeyFlag flag, int what)
{
	unsigned bits = static_cast<unsigned>(flag);
	for (int code = 0; code < PadDefine::eKEY_COUNT && code < 32; code++)
	{
		if ((bits >> code) & 1u)
		{
			if (Query(padNum, static_cast<PadDefine::EPadKeycode>(code), what)) return true;
		}
	}
	return false;
}

bool InGamePadManager::GetKeyFlag(int padNum, EPadKeyFlag flag, bool) { return QueryFlag(padNum, flag, 0); }
bool InGamePadManager::GetKeyDownFlag(int padNum, EPadKeyFlag flag, bool) { return QueryFlag(padNum, flag, 1); }
bool InGamePadManager::GetKeyUpFlag(int padNum, EPadKeyFlag flag, bool) { return QueryFlag(padNum, flag, 2); }
bool InGamePadManager::GetKeyRepeatFlag(int padNum, EPadKeyFlag flag, bool) { return QueryFlag(padNum, flag, 3); }

bool InGamePadManager::GetEnterKeyDown(int padNum, bool) { return Query(padNum, PadDefine::scENTER_CODE, 1); }
bool InGamePadManager::GetCancelKeyDown(int padNum, bool) { return Query(padNum, PadDefine::scCANCEL_CODE, 1); }

// Pad bookkeeping -----------------------------------------------------------------------------------

bool InGamePadManager::IsConnectPad(int padNum) { return WiiPad::IsConnected(Chan(padNum)); }
bool InGamePadManager::IsConnectPad2(int padNum) { return IsConnectPad(padNum); }

PadDefine::EPadStyle InGamePadManager::GetPadStyle(int padNum)
{
	return IsConnectPad(padNum) ? PadDefine::eNUNCHUK : PadDefine::eNONE;
}

void InGamePadManager::SetForcedStop(bool isFlag) { g_forcedStop = isFlag; }
bool InGamePadManager::GetForcedStop() { return g_forcedStop; }

int InGamePadManager::GetPadNumFirst() { return 0; }
int InGamePadManager::GetPadNumSecond() { return 1; }
int InGamePadManager::GetPadNum(int playerIdx) { return Chan(playerIdx); }
int InGamePadManager::GetPad1No() { return 0; }
int InGamePadManager::GetPad2No() { return 1; }
int InGamePadManager::GetPad3No() { return 2; }
int InGamePadManager::GetPad4No() { return 3; }
int InGamePadManager::GetCurrentPadNo() { return 0; }

void InGamePadManager::SetKeycodeMapping(PadDefine::EPadKeycode key, unsigned wiiButtonMask)
{
	InitMap();
	if (key >= 0 && key < PadDefine::eKEY_COUNT) g_map[key] = Button(wiiButtonMask);
}
