#pragma once

#include <cstdint>

// The pad side of the gotch frontend (nmwii2.rso): CWiiInput reads the remotes (KPAD) into one CONTROLLER_VALUE per channel, CGAMEIO
// translates them with the module's table (the remote is held SIDEWAYS: the table turns the D-pad), rotates / 4-way converts them, makes the
// edges and runs the pause / exit / reset checks the games call. A 1:1 port; the translate, D-pad rotate and auto-repeat tables are module
// data, read from the player's nmwii2.rso at run time. The KPAD bits come from WiiPad (its raw KPAD layout, already turned for the sideways
// hold the module expects).
//
// A CONTROLLER_VALUE bit set is 64 bits: bit n < 32 is the low word (+4 of the pair), bit n >= 32 the high word (+0), as the module's
// __shl2i(0, 1, n) builds them. Raw layout (CWiiInput::GetPadData): low 0x1 up, 0x2 right, 0x4 down, 0x8 left (the remote's cross as KPAD
// reports it), 0x10.. the classic controller's cross; high 0x1 A, 0x2 B, 0x4 C, 0x400 1, 0x800 2, 0x1000 HOME, 0x4000 MINUS, 0x10000 PLUS,
// 0x20000 Z (the Nunchuk stick and the pointer are the analog words +0x5c..).
namespace gotch
{
class RsoModule;
struct SWork;

// CConvert8To4Way::sData (0x10 bytes): the 4-way history of one nibble
struct Convert8To4WayData
{
	int size = 4;             // +0x0
	int count = 0;            // +0x4
	std::uint8_t hist[8]{};   // +0x8 (1 = vertical, 2 = horizontal)
};

// CONTROLLER_VALUE (0xd0 bytes; ctor RSO2 0x80d50e3c)
struct ControllerValue
{
	int type = 0;                     // +0x00: 8 = remote, 9 = remote + Nunchuk, 10 = classic controller, 0 = none
	std::uint64_t raw = 0;            // +0x08
	std::uint64_t value = 0;          // +0x10: translated
	std::uint64_t old = ~0ull;        // +0x18
	std::uint64_t way4 = 0;           // +0x20: value with every direction nibble turned 4-way
	std::uint64_t oldWay4 = ~0ull;    // +0x28
	std::uint64_t edge = 0;           // +0x30
	std::uint64_t repeat = 0;         // +0x38: edge + auto-repeat
	std::uint64_t edgeWay4 = 0;       // +0x40
	std::int32_t unk48 = 0;           // +0x48
	std::uint64_t invalid = 0;        // +0x50: bits ignored until released (SetPadInvalidUntilReleaseEdgeDetect)
	std::uint32_t table = 0;          // +0x58: the translate table (module address)
	std::uint16_t analog[0x20]{};     // +0x5c: 0x8000 = centre
	std::uint8_t repeatCount[0x40]{}; // +0xbc
	std::uint8_t holdCount[0x40]{};   // +0xc0
	std::uint8_t speed[0x40]{};       // +0xc4
	int repeatTable = 0;              // +0xc8
	Convert8To4WayData conv[8];       // +0xcc

	void Init();                                // RSO2 0x80d5144c
	void Translate(const RsoModule& rso);       // RSO2 0x80d51a8c
	void MakeEdge(const RsoModule& rso, bool clear);   // RSO2 0x80d514e4
	void DPadRotate(const RsoModule& rso, int rot);    // RSO2 0x80d51914 (on `value`)
};

// CWiiInput (RSO2 0x80d07bec): the per-channel KPAD reads; only what CGAMEIO uses is kept (the pointer / DPD cursor of the menus is not)
class CWiiInput
{
public:
	static constexpr int kPads = 4;   // +0x3c4c
	// RSO2 0x80d07fe0: false when no channel was read (never natively: WiiPad always has channel 0)
	bool GetPadData(ControllerValue* const* pads);

private:
	int lastType[kPads]{};            // +0x3c64
	int blank[kPads]{};               // +0x3c68: frames of no input after the device changed
};

// CGAMEIO (0x40 bytes; ctor RSO2 0x80d51c50, built by InitMachine)
class CGameIO
{
public:
	static constexpr int kPads = CWiiInput::kPads;
	// the module image the tables are read from, the machine's stop flag (+0x110 / +0x114) and the HotReset the checks call
	CGameIO(const RsoModule& rso, bool* machineRunning, int* machineResult, void (*hotReset)());

	void SetPadData();                              // RSO2 0x80d50ae0
	void MakePadEG();                               // RSO2 0x80d52898
	bool GetPadData(ControllerValue* const** out) const;   // RSO2 0x80d52874
	bool DigitalPadDirectionState(int pad) const;   // RSO2 0x80d52598
	void InitPadData();                             // RSO2 0x80d523b4
	void SetComboButton(int which, std::uint32_t hi, std::uint32_t lo);   // RSO2 0x80d52b14
	void SetTranslateTable(std::uint32_t table);    // RSO2 0x80d52dd0
	void SetDefaultTranslateTable();                // RSO2 0x80d50a04
	void SetPadInvalidUntilReleaseEdgeDetect(std::uint32_t lo, std::uint32_t hi);   // RSO2 0x80d52478

	// the checks take the game's swork
	bool PauseCheck(SWork& sw, bool* primaryMuted);                     // RSO2 0x80d52bb4
	void DebugModeCheck(SWork& sw);                                     // RSO2 0x80d529b4
	void ROMRAMSkipCheck(SWork& sw, bool enabled, std::uint8_t* done);  // RSO2 0x80d52d34
	void SoftResetCheck(SWork& sw);                                     // RSO2 0x80d529c0
	bool ExitCheck(SWork& sw);                                          // RSO2 0x80d52a58
	void ExitRequest(SWork& sw);                                        // RSO2 0x80d529a0
	void ExitGo();                                                             // RSO2 0x80d52af8
	void GameReset(SWork& sw);                                          // RSO2 0x80d52988
	void ResetGo();                                                            // RSO2 0x80d52998
	bool ExitCheckDebug() const;                                               // RSO2 0x80d52b54

	ControllerValue& Pad(int n) { return pads[n]; }
	int PadCount() const { return count; }   // +0x34
	int invalidFrames = 0;   // +0x20: MakePadEG clears the edges while it counts down; GetPadData fails while it is set

private:
	void PadRotate();                       // RSO2 0x80d524ac
	void Convert4WayFrom8Way();             // RSO2 0x80d525d8
	void InvalidUntilReleaseEdgeDetect();   // RSO2 0x80d5252c

	const RsoModule& rso;
	bool* running;
	int* result;
	void (*hotReset)();
	ControllerValue pads[kPads];            // +0x04 (array of pointers in the original)
	ControllerValue* padPtrs[kPads];
	std::uint64_t combo[3]{};               // +0x08 / +0x10 / +0x18 (high word first): 0 debug exit, 1 pause, 2 ROM / RAM test skip
	int rotate = 0;                         // +0x30
	int count = kPads;                      // +0x34
	CWiiInput input;                        // the console's CWiiInput (CConsole +0xbc)
};

// CConsole::GetForceCloseThis (the reset / power buttons): never set natively
inline int Console_GetForceCloseThis() { return 0; }
}
