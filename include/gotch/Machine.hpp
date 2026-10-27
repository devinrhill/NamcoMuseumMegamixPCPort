#pragma once

#include <cstdint>

// The machine scheduler of the gotch arcade framework (nmwii2.rso): CPUs and timer events advanced in ETIME, one Execute1V = one video
// frame. A 1:1 port of CreateMachineSpecEx / AddCPU / ResetMachine / Execute1V / RunEvent / CalcNextEventTime / CreateTimerEvent; the services
// InitMachine creates (CDraw, CDataStorage, CGAMEIO, CConsole) are owned by each game here (see src/gotch/PacMan.cpp).
namespace gotch
{
// 64-bit signed time, 2^40 units per second (the float constant 1099511627776.0 at RSO2 0x80dc8608 / 0x80dc82f8 / 0x80d93480)
using ETIME = std::int64_t;
constexpr float kETimeSecond = 1099511627776.0f;

// EVENT_SPEC (CreateTimerEvent's layout: +0 name, +4 type, +8 callback, +0xc next, +0x10 previous, +0x18 period, +0x20 counter)
struct EVENT_SPEC
{
	const char* name = nullptr;
	int type = 0;  // 0 = off, 2 = one-shot (off after it fired), 5 = armed next RunEvent (becomes 1); VBLANK_IN = 3, VBLANK_OUT = 4
	void (*callback)() = nullptr;
	EVENT_SPEC* next = nullptr;
	EVENT_SPEC* prev = nullptr;
	ETIME period = 0;
	ETIME counter = 0;
};

// CPU_SPEC: +4 interface, +8 Init, +0xc Reset, +0x10 Run, +0x14 SetFlag, +0x18 name, +0x20 clock (ETIME per second, i.e. Hz << 40),
// +0x28 env, +0x30 ETIME per cycle, +0x38 cycle accumulator (cycles << 40), +0x40, +0x44 next
struct CPU_SPEC
{
	int index = 0;
	int (*Init)(void* env) = nullptr;
	int (*Reset)(void* env) = nullptr;
	int (*Run)(void* env) = nullptr;
	int (*SetFlag)(void* env, int flag, void* value) = nullptr;
	const char* name = nullptr;
	std::uint64_t clock = 0;
	void* env = nullptr;
	std::uint64_t perCycle = 0;
	ETIME acc = 0;
	int unk40 = 0;
	CPU_SPEC* next = nullptr;
};

// MACHINE_SPEC, the fields the scheduler uses (offsets of the original in the comments)
struct MACHINE_SPEC
{
	const char* name = nullptr;     // +0x00
	ETIME framePeriod = 0;          // +0x10 (screen spec): one video frame
	ETIME vblankTime = 0;           // +0x18 (screen spec): VBLANK_IN -> VBLANK_OUT; negated by CreateMachineSpecEx (UNVERIFIED: whether anything else reads it)
	CPU_SPEC* cpus = nullptr;       // +0x98
	EVENT_SPEC* events = nullptr;   // +0x9c
	EVENT_SPEC vblankIn;            // +0xa0
	EVENT_SPEC vblankOut;           // +0xc8
	void (*HardwareInit)() = nullptr;  // +0xf0
	void (*Exit)() = nullptr;          // +0xf4
	void (*HotReset)() = nullptr;      // +0xf8
	void (*Render)() = nullptr;        // +0xfc
	void (*ConvertIO)() = nullptr;     // +0x100
	int vblankCount = 0;            // +0x104: VBlankOut counts it; Execute1V runs until it changes
	int skipFrames = 0;             // +0x108: extra frames Execute1V runs before it renders
	int cpuCount = 0;               // +0x10c
	bool running = false;           // +0x110
	int unk114 = 0;                 // +0x114
};

// RSO2 0x80d0c600 (the arguments the port needs: screen timing from CreateScreenSpec, the machine callbacks)
void CreateMachineSpecEx(MACHINE_SPEC& m, const char* name, ETIME framePeriod, ETIME vblankTime, void (*render)(), void (*convertIO)(),
                         void (*hardwareInit)(), void (*vblankIn)(), void (*vblankOut)(), void (*exitFn)(), void (*hotReset)());
int AddCPU(MACHINE_SPEC& m, CPU_SPEC& cpu);   // RSO2 0x80d0c9ac
int ResetMachine(MACHINE_SPEC& m);            // RSO2 0x80d0cce4 (InitMachine 0x80d0caac ends with it)
int HotResetMachine(MACHINE_SPEC& m);         // RSO2 0x80d0ce20: ResetMachine, then the game's hot reset (+0xf8)
// RSO2 0x80d363dc
int CreateTimerEvent(MACHINE_SPEC& m, EVENT_SPEC& ev, const char* name, int type, void (*callback)(), ETIME period, ETIME counter);
void RunEvent(EVENT_SPEC* list, ETIME dt);        // RSO2 0x80d36218
ETIME CalcNextEventTime(const EVENT_SPEC* list);  // RSO2 0x80d36340
// RSO2 0x80d367f4: one video frame (ConvertIO, CPUs + events up to the next VBLANK_OUT, Render); returns !running like the original
int Execute1V(MACHINE_SPEC& m);

// the float math of the module's static initialisers (e.g. PACMAN's at RSO2 0x80d471b4): ETIME of 1 / hz seconds, of `seconds`, and a CPU clock
ETIME PeriodFromHz(float hz);
ETIME TimeFromSeconds(float seconds);
}
