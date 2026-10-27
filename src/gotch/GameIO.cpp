#include "gotch/GameIO.hpp"

#include <cstring>

#include "WiiPad.hpp"
#include "gotch/RsoModule.hpp"
#include "gotch/SWork.hpp"

namespace gotch
{
namespace
{
// module data (nmwii2.rso): the default translate table (count, entries), the D-pad rotate table and the auto-repeat thresholds
constexpr std::uint32_t kDefaultTranslateTable = 0x80e02780;   // DAT_80e02780
constexpr std::uint32_t kRotTable = 0x80e02808;                // @LOCAL@DPadRotate__16CONTROLLER_VALUE...@rottable
constexpr std::uint32_t kRepeatTable = 0x80e027f8;             // MakeEdge: [speed + table * 5]

// the translate table's entries / pair lists are pointer words: they hold their targets only once relocated (0 in the file)
std::uint32_t R32(const RsoModule& rso, std::uint32_t a) { return rso.Word(a); }
int S8(const RsoModule& rso, std::uint32_t a)
{
	const std::uint8_t* p = rso.At(a, 1);
	return p == nullptr ? 0 : (std::int8_t)p[0];
}
std::uint8_t U8(const RsoModule& rso, std::uint32_t a)
{
	const std::uint8_t* p = rso.At(a, 1);
	return p == nullptr ? 0xff : p[0];
}
std::uint64_t Bit(int n) { return (std::uint64_t)1 << (n & 63); }   // __shl2i(0, 1, n)

// CWiiInput::GetPadData: -1..1 -> 0..0xffff (DAT_80d93400 * (DAT_80d93408 + v) + DAT_80d933f8 = 32768 * (1 + v) + 0.5, clamped)
std::uint16_t Analog(float v)
{
	const double d = 32768.0 * (1.0 + (double)v) + 0.5;
	if (d <= 0.0) return 0;
	return d > 65535.0 ? 0xffff : (std::uint16_t)(std::uint32_t)d;
}
}

// RSO2 0x80d5144c
void ControllerValue::Init()
{
	old = ~0ull;
	oldWay4 = ~0ull;
	edge = 0;
	edgeWay4 = 0;
	unk48 = 0;
	repeatTable = 0;
	std::memset(analog, 0, sizeof(analog));
	std::memset(repeatCount, 0, sizeof(repeatCount));
	std::memset(holdCount, 0, sizeof(holdCount));
	std::memset(speed, 0, sizeof(speed));
}

// RSO2 0x80d51a8c: the entry of the table for this device type rebuilds `value` from the analog words (beyond half way = a bit) and the
// raw bits (pairs: raw bit -> value bit); no entry = the raw bits as they are
void ControllerValue::Translate(const RsoModule& rso)
{
	value = raw;
	if (table == 0 || type == 0) return;
	const std::uint32_t n = R32(rso, table), entries = R32(rso, table + 4);
	for (std::uint32_t i = 0; i < n; i++)
	{
		const std::uint32_t e = entries + i * 0x14;
		if ((std::uint32_t)type != R32(rso, e)) continue;
		value = 0;
		const std::uint32_t na = R32(rso, e + 4), pa = R32(rso, e + 8);
		for (std::uint32_t j = 0; j < na; j++)
		{
			const int v = (int)analog[S8(rso, pa + j * 3) & 0x1f] - 0x8000;
			if (v > 0x4000) value |= Bit(S8(rso, pa + j * 3 + 2));
			if (v < -0x4000) value |= Bit(S8(rso, pa + j * 3 + 1));
		}
		const std::uint32_t nb = R32(rso, e + 0xc), pb = R32(rso, e + 0x10);
		for (std::uint32_t j = 0; j < nb; j++)
			if ((raw & Bit(S8(rso, pb + j * 2))) != 0) value |= Bit(S8(rso, pb + j * 2 + 1));
		return;
	}
}

// RSO2 0x80d514e4: edges, and the auto-repeat (a held bit repeats after [speed] frames; every 30 frames held the speed steps up, to 4)
void ControllerValue::MakeEdge(const RsoModule& rso, bool clear)
{
	const std::uint64_t prev = old, prev4 = oldWay4;
	old = value;
	edge = (prev ^ value) & value;
	oldWay4 = way4;
	edgeWay4 = (prev4 ^ way4) & way4;
	std::uint64_t rep = 0;
	for (int i = 0; i < 0x40; i++)
	{
		const std::uint64_t bit = Bit(i);
		if ((value & bit) == 0) continue;
		if ((edge & bit) != 0)
		{
			speed[i] = 0;
			holdCount[i] = 0;
			repeatCount[i] = 0;
		}
		repeatCount[i]++;
		if (U8(rso, kRepeatTable + (std::uint32_t)((std::int8_t)speed[i] + repeatTable * 5)) <= repeatCount[i])
		{
			rep |= bit;
			repeatCount[i] = 0;
		}
		if (++holdCount[i] == 0x1e)
		{
			holdCount[i] = 0;
			if ((std::int8_t)++speed[i] > 4) speed[i] = 4;
		}
	}
	repeat = edge | rep;
	if (clear)
	{
		edge = 0;
		repeat = 0;
	}
}

// RSO2 0x80d51914: the direction bits of every byte of the low word move by the table (rot = 0 keeps them); the high word stays
void ControllerValue::DPadRotate(const RsoModule& rso, int rot)
{
	const int t0 = (int)R32(rso, kRotTable + rot * 4), t1 = (int)R32(rso, kRotTable + (rot + 1) * 4);
	const int t2 = (int)R32(rso, kRotTable + (rot + 2) * 4), t3 = (int)R32(rso, kRotTable + (rot + 3) * 4);
	const std::uint32_t lo = (std::uint32_t)value;
	std::uint32_t out = 0;
	for (int k = 0; k < 32; k += 8)
	{
		if (lo & (1u << (t0 + k))) out |= 1u << k;
		if (lo & (1u << (t1 + k))) out |= 1u << (k + 1);
		if (lo & (1u << (t2 + k))) out |= 1u << (k + 2);
		if (lo & (1u << (t3 + k))) out |= 1u << (k + 3);
		if (lo & (1u << (t0 + k + 4))) out |= 1u << (k + 4);
		if (lo & (1u << (t1 + k + 4))) out |= 1u << (k + 5);
		if (lo & (1u << (t2 + k + 4))) out |= 1u << (k + 6);
		if (lo & (1u << (t3 + k + 4))) out |= 1u << (k + 7);
	}
	value = (value & 0xffffffff00000000ull) | out;
}

// RSO2 0x80d07fe0 (the button part; the pointer drift / DPD cursor of the menus (+0x3c08..+0x3c10, +0x3c50) is not kept). KPAD's bits come
// from WiiPad; a Nunchuk makes the device type 1 (the classic controller, type 2, has no native counterpart yet)
bool CWiiInput::GetPadData(ControllerValue* const* pads)
{
	for (int i = 0; i < kPads; i++)
	{
		ControllerValue& cv = *pads[i];
		cv.type = 0;
		cv.raw = 0;
		if (!WiiPad::IsConnected(i)) continue;   // WPADProbe != 0
		const std::uint32_t hold = WiiPad::Held(i);
		std::uint32_t lo = 0, hi = 0;
		auto remote = [&]() {
			if (hold & WiiPad::BTN_UP) lo |= 1;
			if (hold & WiiPad::BTN_DOWN) lo |= 4;
			if (hold & WiiPad::BTN_LEFT) lo |= 8;
			if (hold & WiiPad::BTN_RIGHT) lo |= 2;
			if (hold & WiiPad::BTN_2) hi |= 0x800;
			if (hold & WiiPad::BTN_1) hi |= 0x400;
			if (hold & WiiPad::BTN_A) hi |= 1;
			if (hold & WiiPad::BTN_B) hi |= 2;
			if (hold & WiiPad::BTN_PLUS) hi |= 0x10000;
			if (hold & WiiPad::BTN_MINUS) hi |= 0x4000;
			if (hold & WiiPad::BTN_HOME) hi |= 0x1000;
		};
		const WiiPad::Pointer ptr = WiiPad::GetPointer(i);
		if (!WiiPad::NunchukConnected(i))
		{
			cv.type = 8;   // dev type 0 (or 0xfb / 0xfc / 0xff)
			remote();
			if (ptr.valid)
			{
				cv.analog[0] = Analog(ptr.x);
				cv.analog[1] = Analog(-ptr.y);   // KPAD pos.y grows downwards
			}
		}
		else
		{
			cv.type = 9;   // dev type 1
			remote();
			if (hold & WiiPad::BTN_Z) hi |= 0x20000;
			if (hold & WiiPad::BTN_C) hi |= 4;
			if (ptr.valid)
			{
				cv.analog[2] = Analog(ptr.x);
				cv.analog[3] = Analog(-ptr.y);
			}
			const WiiPad::Stick st = WiiPad::NunchukStick(i);
			cv.analog[0] = Analog(st.x);
			cv.analog[1] = Analog(st.y);
		}
		cv.raw = (std::uint64_t)hi << 32 | lo;
		// a device change blanks the channel for 0x10 frames
		if (lastType[i] != cv.type)
		{
			blank[i] = 0x10;
			lastType[i] = cv.type;
		}
		if (blank[i] != 0)
		{
			blank[i]--;
			cv.raw = 0;
		}
	}
	return true;
}

// RSO2 0x80d51c50 (+ PlatformDependInit 0x80d50a3c: the default table, combos 0 = 0xf000 (debug exit), 1 = 8 (pause), 2 = 0xf008 (ROM / RAM test
// skip), all in the high word)
CGameIO::CGameIO(const RsoModule& rso_, bool* machineRunning, int* machineResult, void (*hotReset_)())
	: rso(rso_), running(machineRunning), result(machineResult), hotReset(hotReset_)
{
	for (int i = 0; i < kPads; i++)
	{
		padPtrs[i] = &pads[i];
		pads[i].Init();
	}
	count = kPads;
	rotate = 0;
	SetDefaultTranslateTable();
	SetComboButton(0, 0xf000, 0);
	SetComboButton(1, 8, 0);
	SetComboButton(2, 0xf008, 0);
	invalidFrames = 0;
	for (int i = 0; i < count; i++) pads[i].Init();
}

// RSO2 0x80d50ae0
void CGameIO::SetPadData()
{
	if (!input.GetPadData(padPtrs))
		for (int i = 0; i < count; i++)
		{
			pads[i].raw = 0;
			std::memset(pads[i].analog, 0, sizeof(pads[i].analog));
		}
	for (int i = 0; i < count; i++) pads[i].Translate(rso);
	PadRotate();
	Convert4WayFrom8Way();
	InvalidUntilReleaseEdgeDetect();
}

// RSO2 0x80d524ac: machine +0x70 (the screen spec's +0x60, 0 from CreateScreenSpec) + the pad rotation
void CGameIO::PadRotate()
{
	for (int i = 0; i < count; i++) pads[i].DPadRotate(rso, (0 + rotate) & 3);
}

// RSO2 0x80d525d8: every nibble of the low word as a 4-way stick: opposite directions cancel, a diagonal keeps the axis of the last single
// direction (the history of CConvert8To4Way::sData)
void CGameIO::Convert4WayFrom8Way()
{
	for (int p = 0; p < count; p++)
	{
		ControllerValue& cv = pads[p];
		const std::uint32_t lo = (std::uint32_t)cv.value;
		std::uint32_t out = 0, sign = 0;
		for (int k = 0; k < 8; k++)
		{
			Convert8To4WayData& d = cv.conv[k];
			const std::uint32_t nib = (lo >> (k * 4)) & 0xf;
			std::uint32_t up = nib & 1, right = (nib >> 1) & 1, down = (nib >> 2) & 1, left = (nib >> 3) & 1;
			int n = (int)(up + right + down + left);
			if ((left && right) || (up && down))
			{
				up = right = down = left = 0;
				n = 0;
			}
			if (n == 0)
			{
				std::memset(d.hist, 0, sizeof(d.hist));
				d.count = 0;
			}
			else
			{
				const std::uint8_t cls = (std::uint8_t)((up || down ? 1 : 0) | (left || right ? 2 : 0));
				if (n == 1)
				{
					std::memset(d.hist, 0, sizeof(d.hist));
					d.hist[0] = cls;
					d.count = 1;
				}
				else
				{
					int vert = 0, horiz = 0;
					for (int j = 0; j < d.size; j++)
					{
						if (d.hist[j] & 1) vert++;
						if (d.hist[j] & 2) horiz++;
					}
					if (vert < horiz) up = down = 0;
					if (vert >= horiz) right = left = 0;
				}
			}
			const std::uint32_t bits = (up | right << 1 | down << 2 | left << 3) << (k * 4);
			out |= bits;
			sign |= (std::uint32_t)((std::int32_t)bits >> 31);   // the 64-bit OR of a sign-extended word (quirk kept)
		}
		cv.way4 = ((cv.value >> 32 | sign) << 32) | out;
	}
}

// RSO2 0x80d5252c
void CGameIO::InvalidUntilReleaseEdgeDetect()
{
	for (int i = 0; i < count; i++)
	{
		pads[i].invalid &= pads[i].value;
		pads[i].value &= ~pads[i].invalid;
	}
}

// RSO2 0x80d52898 (the CDraw +0x38 == 0 && +0x53 != 0 case that clears the edges is the screen adjust's: never natively, UNVERIFIED)
void CGameIO::MakePadEG()
{
	for (int i = 0; i < count; i++) pads[i].MakeEdge(rso, false);
	if (invalidFrames != 0)
	{
		invalidFrames--;
		for (int i = 0; i < count; i++)
		{
			pads[i].edge = 0;
			pads[i].repeat = 0;
		}
	}
}

// RSO2 0x80d52874
bool CGameIO::GetPadData(ControllerValue* const** out) const
{
	if (invalidFrames != 0) return false;
	*out = padPtrs;
	return true;
}

// RSO2 0x80d52598 (the bound test is `pad <= count` in the original; the port also keeps the index inside the array)
bool CGameIO::DigitalPadDirectionState(int pad) const
{
	if (pad >= 0 && pad <= count && pad < kPads) return (std::uint32_t)pads[pad].value != 0;
	return false;
}

// RSO2 0x80d523b4
void CGameIO::InitPadData()
{
	invalidFrames = 0;
	for (int i = 0; i < count; i++) pads[i].Init();
}

// RSO2 0x80d52b14
void CGameIO::SetComboButton(int which, std::uint32_t hi, std::uint32_t lo)
{
	if (which >= 0 && which < 3) combo[which] = (std::uint64_t)hi << 32 | lo;
}

// RSO2 0x80d52dd0
void CGameIO::SetTranslateTable(std::uint32_t table)
{
	for (int i = 0; i < count; i++) pads[i].table = table;
}

// RSO2 0x80d50a04
void CGameIO::SetDefaultTranslateTable() { SetTranslateTable(kDefaultTranslateTable); }

// RSO2 0x80d52478
void CGameIO::SetPadInvalidUntilReleaseEdgeDetect(std::uint32_t lo, std::uint32_t hi)
{
	for (int i = 0; i < count; i++) pads[i].invalid = (std::uint64_t)hi << 32 | lo;
}

// RSO2 0x80d52bb4: a player who is in the game (sw+4 / sw+5) pauses with the pause combo's edge (EtcPauseCheck 0x80d50bcc = 0); sw+0xc
// counts frames during which pausing is not allowed; the PRIMARYSTREAM is muted while a player is paused (ClrPowerStateResumeComp: no native)
bool CGameIO::PauseCheck(SWork& sw, bool* primaryMuted)
{
	bool paused = false;
	if (sw.i32(0xc) == 0)
	{
		if (sw.b(0x26) != 0)
		{
			sw.b(8) = 0;
			if (sw.b(4) != 0 && sw.b(6) == 0 && (pads[0].edge & combo[1]) != 0)
			{
				sw.b(6) = 1;
				sw.b(8) = 1;
				sw.set32(0x18, 1);
				paused = true;
			}
			if (sw.b(5) != 0 && sw.b(7) == 0 && (pads[1].edge & combo[1]) != 0)
			{
				sw.b(7) = 1;
				sw.b(8) = 1;
				sw.set32(0x18, 1);
				paused = true;
			}
		}
	}
	else
		sw.set32(0xc, sw.i32(0xc) - 1);
	if (primaryMuted != nullptr) *primaryMuted = (sw.b(6) | sw.b(7)) != 0;
	return paused;
}

// RSO2 0x80d529b4
void CGameIO::DebugModeCheck(SWork& sw) { sw.b(0x30) = 0; }

// RSO2 0x80d52d34
void CGameIO::ROMRAMSkipCheck(SWork&, bool enabled, std::uint8_t* done)
{
	for (int i = 0; i < count; i++)
		if ((combo[2] & pads[i].edge) != 0 && enabled && *done == 0)
		{
			hotReset();
			*done = 1;
		}
}

// RSO2 0x80d529c0: 1 + 2 + the classic controller's ZL / ZR held and the pause combo's 4-way edge: the exit request outside a game, a hot
// reset during one
void CGameIO::SoftResetCheck(SWork& sw)
{
	std::uint32_t word = 0, edge4 = 0;
	if (invalidFrames == 0)
	{
		word = (std::uint32_t)(pads[0].value >> 32) | (std::uint32_t)pads[0].value << 4;
		edge4 = (std::uint32_t)(pads[0].edgeWay4 >> 32);
	}
	if ((word & 0xf000) != 0xf000 || (edge4 & 8) == 0) return;
	if (sw.b(0x2f) == 0)
	{
		sw.b(0x6f) = 1;
		sw.b(0x6e) = 1;
		sw.b(0x70) = 1;
		return;
	}
	sw.b(0x51) = 1;
	hotReset();
}

// RSO2 0x80d52a58: sw+0x70 = an exit was asked for; it stops the machine once sw+0x6f (the frontend's exit fade, cleared by VBlankIn) is 0
bool CGameIO::ExitCheck(SWork& sw)
{
	const int fc = Console_GetForceCloseThis();
	if (fc != 0)
	{
		*running = false;
		*result = fc;
		return true;
	}
	if (sw.b(0x70) == 0) return false;
	if (sw.b(0x6f) == 0)
	{
		*running = false;
		*result = -1;
	}
	return true;
}

// RSO2 0x80d529a0
void CGameIO::ExitRequest(SWork& sw)
{
	sw.b(0x6f) = 1;
	sw.b(0x6e) = 1;
	sw.b(0x70) = 1;
}

// RSO2 0x80d52af8
void CGameIO::ExitGo()
{
	*running = false;
	*result = -1;
}

// RSO2 0x80d52988
void CGameIO::GameReset(SWork& sw)
{
	sw.b(0x51) = 1;
	hotReset();
}

// RSO2 0x80d52998
void CGameIO::ResetGo() { hotReset(); }

// RSO2 0x80d52b54
bool CGameIO::ExitCheckDebug() const
{
	const ControllerValue& p = pads[0];
	return (p.edge & combo[0]) != 0 && combo[0] == (combo[0] & p.value);
}
}
