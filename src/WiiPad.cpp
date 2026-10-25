#include "WiiPad.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>

#include "platform/input.hpp"

namespace WiiPad
{
namespace
{
constexpr float STICK_DEADZONE = 0.2f;
constexpr int REPEAT_DELAY = 20; // frames held before auto-repeat starts
constexpr int REPEAT_RATE = 5;   // frames between repeats afterwards

struct Source
{
	enum Kind { KEY, PAD_BUTTON, PAD_AXIS, MOUSE } kind{KEY};
	int code{};
	int sign{1};     // axes: +1 or -1 direction counted as "pressed"
	int pad{-1};     // gamepad index, -1 = the channel's own gamepad
	std::string text; // as written in the config, for WriteConfig
};

struct ButtonName { const char* name; Button bit; };
const ButtonName kButtons[] = {
	{"Left", BTN_LEFT}, {"Right", BTN_RIGHT}, {"Down", BTN_DOWN}, {"Up", BTN_UP}, {"Plus", BTN_PLUS},
	{"2", BTN_2}, {"1", BTN_1}, {"B", BTN_B}, {"A", BTN_A}, {"Minus", BTN_MINUS},
	{"Z", BTN_Z}, {"C", BTN_C}, {"Home", BTN_HOME}, {"Shake", BTN_SHAKE},
};
const char* kStickNames[STICK_DIR_COUNT] = {"StickUp", "StickDown", "StickLeft", "StickRight"};

std::map<std::uint32_t, std::vector<Source>> g_buttonSources;
std::vector<Source> g_stickSources[STICK_DIR_COUNT];
std::vector<Source> g_pointerSources[2]; // PointerX / PointerY analog axes (gamepad right stick)

struct Channel
{
	bool connected{};
	std::uint32_t held{}, prev{}, pressed{}, released{}, repeat{};
	int heldFrames[16]{};
	Stick nunchuk{};
	Pointer pointer{};
};
Channel g_ch[MAX_CHANNELS];
std::uint32_t g_inject[4] = {};
bool g_injectPtr = false;
float g_injectPtrX = 0, g_injectPtrY = 0; // game screen pixels, top-left origin
bool g_sideways = true;
bool g_uprightOverride = false;
bool EffectiveSideways() { return g_sideways && !g_uprightOverride; }

// physical d-pad directions (as bound) -> the bits the game reads
std::uint32_t RotateDpad(std::uint32_t held)
{
	if (!EffectiveSideways()) return held;
	std::uint32_t out = held & ~(std::uint32_t)(BTN_LEFT | BTN_RIGHT | BTN_UP | BTN_DOWN);
	if (held & BTN_LEFT) out |= BTN_UP;
	if (held & BTN_RIGHT) out |= BTN_DOWN;
	if (held & BTN_UP) out |= BTN_RIGHT;
	if (held & BTN_DOWN) out |= BTN_LEFT;
	return out;
}

std::string Trim(const std::string& s)
{
	size_t a = s.find_first_not_of(" \t\r\n"), b = s.find_last_not_of(" \t\r\n");
	return a == std::string::npos ? std::string() : s.substr(a, b - a + 1);
}

bool Parse(const std::string& textIn, Source& out)
{
	std::string text = Trim(textIn);
	size_t colon = text.find(':');
	if (colon == std::string::npos) return false;
	std::string kind = text.substr(0, colon), name = text.substr(colon + 1);
	out = Source{};
	out.text = text;
	if (kind == "Key") { out.kind = Source::KEY; out.code = platform::KeyFromName(name.c_str()); }
	else if (kind == "Mouse") { out.kind = Source::MOUSE; out.code = platform::MouseButtonFromName(name.c_str()); }
	else if (kind.rfind("Pad", 0) == 0 || kind.rfind("Axis", 0) == 0)
	{
		bool axis = kind.rfind("Axis", 0) == 0;
		std::string idx = kind.substr(axis ? 4 : 3);
		if (!idx.empty()) out.pad = std::atoi(idx.c_str());
		if (axis)
		{
			out.kind = Source::PAD_AXIS;
			if (!name.empty() && (name.back() == '+' || name.back() == '-'))
			{
				out.sign = name.back() == '+' ? 1 : -1;
				name.pop_back();
			}
			out.code = platform::PadAxisFromName(name.c_str());
		}
		else
		{
			out.kind = Source::PAD_BUTTON;
			out.code = platform::PadButtonFromName(name.c_str());
		}
	}
	else return false;
	return out.code >= 0;
}

bool SourceDown(const Source& s, int channel)
{
	int pad = s.pad >= 0 ? s.pad : channel;
	switch (s.kind)
	{
	case Source::KEY: return channel == 0 && platform::IsKeyDown(s.code); // the keyboard drives channel 0 only
	case Source::MOUSE: return channel == 0 && platform::IsMouseButtonDown(s.code);
	case Source::PAD_BUTTON: return platform::IsPadButtonDown(pad, s.code);
	case Source::PAD_AXIS: return platform::PadAxis(pad, s.code) * s.sign > 0.5f;
	}
	return false;
}

float SourceAnalog(const Source& s, int channel)
{
	int pad = s.pad >= 0 ? s.pad : channel;
	if (s.kind == Source::PAD_AXIS) { float v = platform::PadAxis(pad, s.code) * s.sign; return v > STICK_DEADZONE ? v : 0.0f; }
	return SourceDown(s, channel) ? 1.0f : 0.0f;
}

bool AnyDown(const std::vector<Source>& v, int channel)
{
	for (const Source& s : v) if (SourceDown(s, channel)) return true;
	return false;
}

float MaxAnalog(const std::vector<Source>& v, int channel)
{
	float m = 0.0f;
	for (const Source& s : v) m = std::max(m, SourceAnalog(s, channel));
	return m;
}

void Add(std::uint32_t bit, const char* src)
{
	Source s;
	if (Parse(src, s)) g_buttonSources[bit].push_back(s);
	else std::fprintf(stderr, "WiiPad: bad default source '%s'\n", src);
}

void AddStick(int dir, const char* src)
{
	Source s;
	if (Parse(src, s)) g_stickSources[dir].push_back(s);
	else std::fprintf(stderr, "WiiPad: bad default source '%s'\n", src);
}
}

void ResetToDefaults()
{
	g_sideways = true;
	g_buttonSources.clear();
	for (auto& v : g_stickSources) v.clear();
	for (auto& v : g_pointerSources) v.clear();

	// Wiimote. "1" and "2" sit on the keyboard's 1 and 2 keys.
	Add(BTN_1, "Key:1");       Add(BTN_1, "Pad:West");
	Add(BTN_2, "Key:2");       Add(BTN_2, "Pad:North");
	Add(BTN_A, "Key:J");       Add(BTN_A, "Key:Enter");   Add(BTN_A, "Mouse:Left");  Add(BTN_A, "Pad:South");
	Add(BTN_B, "Key:K");       Add(BTN_B, "Mouse:Right"); Add(BTN_B, "Pad:East");
	Add(BTN_PLUS, "Key:Equal");  Add(BTN_PLUS, "Pad:Start");
	Add(BTN_MINUS, "Key:Minus"); Add(BTN_MINUS, "Pad:Back");
	Add(BTN_HOME, "Key:H");      Add(BTN_HOME, "Pad:Guide");
	Add(BTN_UP, "Key:Up");       Add(BTN_UP, "Pad:DUp");
	Add(BTN_DOWN, "Key:Down");   Add(BTN_DOWN, "Pad:DDown");
	Add(BTN_LEFT, "Key:Left");   Add(BTN_LEFT, "Pad:DLeft");
	Add(BTN_RIGHT, "Key:Right"); Add(BTN_RIGHT, "Pad:DRight");
	// Nunchuk
	Add(BTN_Z, "Key:LShift");    Add(BTN_Z, "Pad:L1");
	Add(BTN_C, "Key:LCtrl");     Add(BTN_C, "Pad:R1");
	Add(BTN_SHAKE, "Key:Space"); Add(BTN_SHAKE, "Pad:R2");   // swing the remote (Pac-N-Roll dash / hang escape)
	AddStick(STICK_UP, "Key:W");    AddStick(STICK_UP, "Axis:LeftY-");
	AddStick(STICK_DOWN, "Key:S");  AddStick(STICK_DOWN, "Axis:LeftY+");
	AddStick(STICK_LEFT, "Key:A");  AddStick(STICK_LEFT, "Axis:LeftX-");
	AddStick(STICK_RIGHT, "Key:D"); AddStick(STICK_RIGHT, "Axis:LeftX+");
}

bool Sideways() { return EffectiveSideways(); }
void SetUprightOverride(bool on) { g_uprightOverride = on; }
void SetSideways(bool sideways) { g_sideways = sideways; }

std::uint32_t DpadMask(Dir d)
{
	switch (d)
	{
	case DIR_LEFT: return EffectiveSideways() ? BTN_UP : BTN_LEFT;
	case DIR_RIGHT: return EffectiveSideways() ? BTN_DOWN : BTN_RIGHT;
	case DIR_UP: return EffectiveSideways() ? BTN_RIGHT : BTN_UP;
	default: return EffectiveSideways() ? BTN_LEFT : BTN_DOWN;
	}
}

void InjectPointer(float gameX, float gameY) { g_injectPtr = true; g_injectPtrX = gameX; g_injectPtrY = gameY; }
void ClearInjectedPointer() { g_injectPtr = false; }

bool g_injectStick = false;
float g_injectStickX = 0, g_injectStickY = 0;
void InjectStick(float x, float y) { g_injectStick = true; g_injectStickX = x; g_injectStickY = y; }

void InjectHeld(std::uint32_t mask, int channel)
{
	if (channel >= 0 && channel < 4) g_inject[channel] |= mask;
}

bool SetBinding(const std::string& input, const std::vector<std::string>& sources)
{
	std::vector<Source> parsed;
	for (const std::string& t : sources)
	{
		Source s;
		if (!Parse(t, s)) { std::fprintf(stderr, "WiiPad: unknown source '%s' for %s\n", t.c_str(), input.c_str()); return false; }
		parsed.push_back(s);
	}
	for (const ButtonName& b : kButtons)
	{
		if (input == b.name) { g_buttonSources[b.bit] = parsed; return true; }
	}
	for (int d = 0; d < STICK_DIR_COUNT; d++)
	{
		if (input == kStickNames[d]) { g_stickSources[d] = parsed; return true; }
	}
	std::fprintf(stderr, "WiiPad: unknown input '%s'\n", input.c_str());
	return false;
}

bool WriteConfig(const std::string& path)
{
	std::ofstream f(path);
	if (!f) return false;
	f << "# Namco Museum Megamix (native) - controller bindings\n"
	     "#\n"
	     "# <input> = <source>, <source>, ...      (any listed source triggers the input)\n"
	     "#\n"
	     "# Inputs : Left Right Up Down Plus Minus Home 1 2 A B Z C   (Wiimote / Nunchuk)\n"
	     "#          StickUp StickDown StickLeft StickRight           (Nunchuk stick)\n"
	     "# Sources: Key:<name>        keyboard  (channel 1 only)  e.g. Key:1  Key:Space  Key:LShift  Key:F5\n"
	     "#          Pad:<button>      gamepad of the same channel: South East West North L1 R1 L2 R2\n"
	     "#                            Back Start Guide L3 R3 DUp DDown DLeft DRight\n"
	     "#          Pad2:<button>     a specific gamepad (index from 0); likewise Axis2:...\n"
	     "#          Axis:<axis>+/-    analog axis past half travel: LeftX LeftY RightX RightY LTrigger RTrigger\n"
	     "#          Mouse:<button>    Left Right Middle (channel 1 only)\n"
	     "#\n"
	     "# Orientation = Sideways | Upright\n"
	     "#   The game is played with the Wiimote held sideways. Sideways (default): the Left/Right/Up/Down bindings below mean the direction you\n"
	     "#   press, and are rotated into the D-pad bits the game expects. Upright: passed through as written.\n"
	     "# Delete this file to get the defaults back.\n\n";
	auto join = [](const std::vector<Source>& v) {
		std::string s;
		for (size_t i = 0; i < v.size(); i++) s += (i ? ", " : "") + v[i].text;
		return s;
	};
	f << "Orientation = " << (g_sideways ? "Sideways" : "Upright") << "\n\n";
	for (const ButtonName& b : kButtons) f << b.name << " = " << join(g_buttonSources[b.bit]) << "\n";
	f << "\n";
	for (int d = 0; d < STICK_DIR_COUNT; d++) f << kStickNames[d] << " = " << join(g_stickSources[d]) << "\n";
	return true;
}

bool Init(const std::string& path)
{
	ResetToDefaults();
	std::ifstream f(path);
	if (!f)
	{
		WriteConfig(path);
		return true;
	}
	bool ok = true;
	std::string line;
	int lineNo = 0;
	while (std::getline(f, line))
	{
		lineNo++;
		size_t hash = line.find('#');
		if (hash != std::string::npos) line.erase(hash);
		line = Trim(line);
		if (line.empty()) continue;
		size_t eq = line.find('=');
		if (eq == std::string::npos)
		{
			std::fprintf(stderr, "%s:%d: expected 'input = source, ...'\n", path.c_str(), lineNo);
			ok = false;
			continue;
		}
		std::string input = Trim(line.substr(0, eq));
		if (input == "Orientation")
		{
			std::string v = Trim(line.substr(eq + 1));
			if (v == "Sideways" || v == "Upright") g_sideways = (v == "Sideways");
			else { std::fprintf(stderr, "%s:%d: Orientation must be Sideways or Upright\n", path.c_str(), lineNo); ok = false; }
			continue;
		}
		std::vector<std::string> sources;
		std::stringstream ss(line.substr(eq + 1));
		std::string tok;
		while (std::getline(ss, tok, ','))
		{
			tok = Trim(tok);
			if (!tok.empty()) sources.push_back(tok);
		}
		if (!SetBinding(input, sources))
		{
			std::fprintf(stderr, "%s:%d: ignored\n", path.c_str(), lineNo);
			ok = false;
		}
	}
	return ok;
}

void Update()
{
	for (int c = 0; c < MAX_CHANNELS; c++)
	{
		Channel& ch = g_ch[c];
		ch.prev = ch.held;
		ch.held = 0;
		for (auto& [bit, sources] : g_buttonSources)
		{
			if (AnyDown(sources, c)) ch.held |= bit;
		}
		if (c < 4 && g_inject[c])
		{
			ch.held |= g_inject[c];
			g_inject[c] = 0;
		}
		ch.held = RotateDpad(ch.held);
		ch.connected = (c == 0) || platform::IsPadConnected(c);
		ch.pressed = ch.held & ~ch.prev;
		ch.released = ch.prev & ~ch.held;
		ch.repeat = ch.pressed;
		for (int i = 0; i < 16; i++)
		{
			std::uint32_t bit = 1u << i;
			if (!(ch.held & bit)) { ch.heldFrames[i] = 0; continue; }
			ch.heldFrames[i]++;
			if (ch.heldFrames[i] > REPEAT_DELAY && (ch.heldFrames[i] - REPEAT_DELAY) % REPEAT_RATE == 0) ch.repeat |= bit;
		}
		float up = MaxAnalog(g_stickSources[STICK_UP], c), down = MaxAnalog(g_stickSources[STICK_DOWN], c);
		float left = MaxAnalog(g_stickSources[STICK_LEFT], c), right = MaxAnalog(g_stickSources[STICK_RIGHT], c);
		ch.nunchuk.x = right - left;
		ch.nunchuk.y = up - down;
		if (c == 0 && g_injectStick) { ch.nunchuk.x = g_injectStickX; ch.nunchuk.y = g_injectStickY; g_injectStick = false; }
		float len = std::sqrt(ch.nunchuk.x * ch.nunchuk.x + ch.nunchuk.y * ch.nunchuk.y);
		if (len > 1.0f) { ch.nunchuk.x /= len; ch.nunchuk.y /= len; }
	}
	// IR pointer: the mouse, for channel 0. The game screen is the 4:3 rectangle centred in the window (the 2D layers are letterboxed the same way).
	int w = 0, h = 0;
	platform::WindowSize(w, h);
	Pointer& p = g_ch[0].pointer;
	if (g_injectPtr)
	{
		p.x = g_injectPtrX / 320.0f - 1.0f;
		p.y = 1.0f - g_injectPtrY / 240.0f;
		p.valid = true;
		return;
	}
	float mx = 0, my = 0;
	platform::MousePosition(mx, my);
	p.valid = w > 0 && h > 0 && mx >= 0 && my >= 0 && mx < w && my < h;
	if (p.valid)
	{
		const float vw = std::min((float)w, h * 4.0f / 3.0f), vh = vw * 3.0f / 4.0f;
		const float vx = (w - vw) / 2, vy = (h - vh) / 2;
		p.x = (mx - vx) / vw * 2.0f - 1.0f;
		p.y = 1.0f - (my - vy) / vh * 2.0f;
	}
}

static const Channel& Ch(int c) { return g_ch[std::clamp(c, 0, MAX_CHANNELS - 1)]; }
bool IsConnected(int c) { return c >= 0 && c < MAX_CHANNELS && g_ch[c].connected; }
std::uint32_t Held(int c) { return Ch(c).held; }
std::uint32_t Pressed(int c) { return Ch(c).pressed; }
std::uint32_t Released(int c) { return Ch(c).released; }
std::uint32_t Repeat(int c) { return Ch(c).repeat; }
Stick NunchukStick(int c) { return Ch(c).nunchuk; }
static bool g_nunchukUnplugged[4] = {};
bool NunchukConnected(int c) { return c < 0 || c >= 4 || !g_nunchukUnplugged[c]; }
void SetNunchukConnected(int c, bool connected) { if (c >= 0 && c < 4) g_nunchukUnplugged[c] = !connected; }
Stick ClassicStick(int c, int which)
{
	Stick s{};
	if (platform::IsPadConnected(c))
	{
		s.x = platform::PadAxis(c, platform::PadAxisFromName(which ? "RightX" : "LeftX"));
		s.y = -platform::PadAxis(c, platform::PadAxisFromName(which ? "RightY" : "LeftY"));
		if (std::fabs(s.x) < STICK_DEADZONE) s.x = 0;
		if (std::fabs(s.y) < STICK_DEADZONE) s.y = 0;
	}
	return s;
}
Pointer GetPointer(int c) { return Ch(c).pointer; }
}
