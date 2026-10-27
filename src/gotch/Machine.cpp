// 1:1 port of the gotch machine scheduler (nmwii2.rso). Every function cites its RSO2 address. The float steps are single precision like the
// original (fmuls / fdivs, __cvt_sll_flt = signed 64-bit -> float, __cvt_dbl_usll = float -> unsigned 64-bit).
//
// Quirks of the original, kept on purpose:
//  * Execute1V drops the fractional cycles of a CPU whenever that CPU ran: the accumulator restarts at the cycles Z80_Run left over (<< 40).
//  * CreateTimerEvent stores the node BEFORE the old tail as the new event's "previous" (nothing reads it).
#include "gotch/Machine.hpp"

namespace gotch
{
namespace
{
// RSO2 0x80dd297c: CalcNextEventTime's start value (0x7fffff << 8 in the high word); RSO2 0x80dd2978 / 0x80dd2980 = 0: the "no period" value
constexpr ETIME kNoEvent = (ETIME)((std::uint64_t)(0x7fffffu << 8) << 32);
constexpr ETIME kNoPeriod = 0;

// __cvt_dbl_usll (float -> unsigned 64-bit): negative values give 0 (UNVERIFIED for the runtime routine; the scheduler never feeds it one)
std::uint64_t ToU64(float f) { return f <= 0.0f ? 0 : (std::uint64_t)f; }
}

ETIME PeriodFromHz(float hz)
{
	// RSO2 0x80d471b4 (PACMAN's static init of 0x811fa310): (u64)(2^40 * (1.0f / ((float)(s64)(u64)(2^40 * hz) / 2^40)))
	const float perSecond = (float)(std::int64_t)ToU64(kETimeSecond * hz);
	return (ETIME)ToU64(kETimeSecond * (1.0f / (perSecond / kETimeSecond)));
}

ETIME TimeFromSeconds(float seconds) { return (ETIME)ToU64(kETimeSecond * seconds); }

// RSO2 0x80d363dc
int CreateTimerEvent(MACHINE_SPEC& m, EVENT_SPEC& ev, const char* name, int type, void (*callback)(), ETIME period, ETIME counter)
{
	EVENT_SPEC* beforeTail = nullptr;
	if (m.events == nullptr)
	{
		m.events = &ev;
	}
	else
	{
		EVENT_SPEC* cur = nullptr;
		EVENT_SPEC* nxt = m.events;
		do
		{
			beforeTail = cur;
			cur = nxt;
			nxt = cur->next;
		} while (cur->next != nullptr);
		cur->next = &ev;
	}
	ev.name = name;
	ev.type = type;
	ev.callback = callback;
	ev.period = period;
	ev.counter = counter;
	ev.next = nullptr;
	ev.prev = beforeTail;  // quirk: see the top of the file
	return 1;
}

// RSO2 0x80d0c600
void CreateMachineSpecEx(MACHINE_SPEC& m, const char* name, ETIME framePeriod, ETIME vblankTime, void (*render)(), void (*convertIO)(),
                         void (*hardwareInit)(), void (*vblankIn)(), void (*vblankOut)(), void (*exitFn)(), void (*hotReset)())
{
	m.name = name;
	m.cpus = nullptr;
	m.cpuCount = 0;
	m.ConvertIO = convertIO;
	m.Render = render;
	m.HardwareInit = hardwareInit;
	m.Exit = exitFn;
	m.HotReset = hotReset;
	m.framePeriod = framePeriod;
	m.vblankTime = vblankTime;
	// VBLANK_IN fires every frame period from 0; VBLANK_OUT (only when the game has a callback) runs the vblank time later
	CreateTimerEvent(m, m.vblankIn, "VBLANK_IN", 3, vblankIn, m.framePeriod, 0);
	if (vblankOut != nullptr) CreateTimerEvent(m, m.vblankOut, "VBLANK_OUT", 4, vblankOut, m.framePeriod, 0);
	m.vblankTime = -m.vblankTime;
	m.vblankOut.counter = m.vblankTime;  // written even without the event (puVar2[0x3a])
}

// RSO2 0x80d0c9ac
int AddCPU(MACHINE_SPEC& m, CPU_SPEC& cpu)
{
	int index = 0;
	if (m.cpus == nullptr)
	{
		m.cpus = &cpu;
	}
	else
	{
		index = 1;
		CPU_SPEC* c = m.cpus;
		while (c->next != nullptr)
		{
			index++;
			c = c->next;
		}
		c->next = &cpu;
	}
	// ETIME per cycle = 2^40 * (1 / (clock / 2^40))
	const float hz = (float)(std::int64_t)cpu.clock / kETimeSecond;
	cpu.perCycle = ToU64(kETimeSecond * (1.0f / hz));
	cpu.index = index;
	cpu.unk40 = 0;
	cpu.acc = 0;
	cpu.next = nullptr;
	int n = 0;
	for (CPU_SPEC* c = m.cpus; c != nullptr; c = c->next) n++;
	m.cpuCount = n;
	return 1;
}

// RSO2 0x80d0cce4
int ResetMachine(MACHINE_SPEC& m)
{
	for (CPU_SPEC* c = m.cpus; c != nullptr; c = c->next)
	{
		c->Init(c->env);
		c->acc = 0;
		c->unk40 = 0;
	}
	if (m.HardwareInit != nullptr) m.HardwareInit();
	// PRIMARYSTREAM__Reset (the audio stream): FUN_80d...: not ported (sound is not ported yet)
	m.vblankCount = 0;
	m.running = true;
	m.unk114 = 0;
	m.skipFrames = 0;
	return 1;
}

// RSO2 0x80d0ce20
int HotResetMachine(MACHINE_SPEC& m)
{
	const int r = ResetMachine(m);
	if (m.HotReset != nullptr) m.HotReset();
	return r;
}

// RSO2 0x80d36218
void RunEvent(EVENT_SPEC* list, ETIME dt)
{
	for (EVENT_SPEC* ev = list; ev != nullptr; ev = ev->next)
	{
		if (ev->type != 0 && ev->type != 5 && ev->period != kNoPeriod)
		{
			ev->counter += dt;
			bool stopped = false;
			do
			{
				if (ev->counter < ev->period)
				{
					stopped = true;
					break;
				}
				ev->counter -= ev->period;
				if (ev->callback != nullptr)
				{
					ev->callback();
					if (ev->type == 0)
					{
						stopped = true;
						break;
					}
				}
			} while (ev->type != 2);
			// a one-shot event switches itself off after it fired
			if (!stopped) ev->type = 0;
		}
		if (ev->type == 5) ev->type = 1;
	}
}

// RSO2 0x80d36340: the time until the earliest event fires
ETIME CalcNextEventTime(const EVENT_SPEC* list)
{
	ETIME best = kNoEvent;
	for (const EVENT_SPEC* ev = list; ev != nullptr; ev = ev->next)
	{
		if (ev->type == 0 || ev->period == kNoPeriod) continue;
		const ETIME d = ev->period - ev->counter;
		if (d < best) best = d;
	}
	return best;
}

// RSO2 0x80d367f4
int Execute1V(MACHINE_SPEC& m)
{
	m.ConvertIO();
	for (;;)
	{
		int before;
		do
		{
			before = m.vblankCount;
			const ETIME dt = CalcNextEventTime(m.events);
			CPU_SPEC* c = m.cpus;
			for (int i = 0; i < m.cpuCount; i++)
			{
				// cycles of this slice (<< 40) = 2^40 * dt / ETIME per cycle
				const float slice = (kETimeSecond * (float)dt) / (float)(std::int64_t)c->perCycle;
				c->acc = (ETIME)((std::uint64_t)c->acc + ToU64(slice));
				// whole cycles: the high word (cycles << 8, a set bit for any low-word fraction) / 256, rounded towards zero (srawi + addze)
				const std::uint32_t lo = (std::uint32_t)c->acc;
				const std::int32_t hi = (std::int32_t)((std::uint64_t)c->acc >> 32) | (lo != 0 ? 1 : 0);
				const int cycles = hi / 256;
				if (cycles > 0)
				{
					c->SetFlag(c->env, 10, (void*)(std::intptr_t)cycles);  // CPUFLAG_SETCYCLE
					c->Run(c->env);
					int left = 0;
					c->SetFlag(c->env, 11, &left);  // CPUFLAG_GETCYCLE
					c->acc = (ETIME)((std::uint64_t)((std::uint32_t)left << 8) << 32);  // quirk: the fraction is dropped
				}
				c = c->next;
			}
			RunEvent(m.events, dt);
		} while (before == m.vblankCount);
		if (m.skipFrames == 0) break;
		m.skipFrames--;
	}
	m.Render();
	return m.running ? 0 : 1;
}
}
