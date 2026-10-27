#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// The sound side of the gotch arcade framework (nmwii2.rso): the sound chips write into SOUNDSTREAM rings, PRIMARYSTREAM mixes them (with a
// 2-pole lowpass) into one stereo ring, CWiiSound copies that ring to the DSP buffer. All of it is ported 1:1; the one native piece is the
// Wii DSP itself: two looping AX voices playing the 0x2000-sample rings, emulated by AxLoop (the platform's generated audio output pulls
// the rings at 32 kHz and advances the play position that CWiiSound::CheckTimer reads).
namespace gotch
{
class SoundModule;

// SOUNDSTREAM (0x30 bytes; RSO2 0x80d373d0 CreateStream): +0x10 read index, +0x14 write index, +0x18 samples written since the last mix,
// +0x1c ring size (u16, power of two, samples x channels), +0x20 ring, +0x24 source, +0x28 name, +0x2d index, +0x2e channels
struct SoundStream
{
	std::uint32_t readIndex = 0;   // +0x10
	std::uint32_t writeIndex = 0;  // +0x14
	std::int32_t written = 0;      // +0x18
	std::uint16_t size = 0;        // +0x1c
	std::vector<std::int16_t> ring;  // +0x20
	SoundModule* source = nullptr;   // +0x24
	std::string name;                // +0x28
	std::uint8_t channels = 1;       // +0x2e

	void Put(std::int16_t s) { ring[writeIndex & (size - 1u)] = s; writeIndex++; }
};

// the sound chip base (SoundModule vtable: +0xc Update(n), +0x10 Reset)
class SoundModule
{
public:
	virtual ~SoundModule() = default;
	virtual void Update(int n) = 0;
	virtual void Reset() = 0;
};

// PRIMARYSTREAM (0x60 bytes; ctor RSO2 0x80d36d8c, CreateAudioSpec 0x80d0c860)
class PrimaryStream
{
public:
	// rate = samples per second (the machine's audio spec), size = ring samples per channel (0x2000), gains in 1/65536 (CreateAudioSpec +0xc / +0x10)
	PrimaryStream(int rate, int size, std::int32_t gainL, std::int32_t gainR);
	int Rate() const { return rate; }
	// RSO2 0x80d373d0
	SoundStream* CreateStream(const char* name, int channels, int size, SoundModule* source);
	std::uint32_t readIndex = 0;       // +0x10: CWiiSound::SoundOut reads from here
	// RSO2 0x80d3766c: every stream's source makes the samples it still owes (n - written), the streams are summed, low-passed and stored as
	// n stereo pairs in the primary ring
	void Update(int n);
	std::int16_t At(std::uint32_t i) const { return ring[i & (size - 1u)]; }
	std::uint16_t Size() const { return size; }
	bool muted = false;  // +0x58: Update writes silence

private:
	int rate;
	std::uint16_t size;                 // +0x1c (pairs x 2)
	std::vector<std::int16_t> ring;     // +0x20
	std::uint32_t writeIndex = 0;       // +0x14
	float gainL, gainR;                 // +0x30 / +0x34
	float wL1 = 0, wL2 = 0, wR1 = 0, wR2 = 0;  // +0x38 / +0x3c, +0x40 / +0x44: the filter state
	std::vector<std::unique_ptr<SoundStream>> streams;  // +0x48 list (the original links new streams at the tail)
	std::vector<std::int32_t> mixL, mixR;  // +0x50 / +0x54
};

// CWiiSound (0x4c bytes; RSO2 0x80d38090 ctor, built by CConsole's ctor 0x80d0022c as CWiiSound(0, 32000000, 60000) = 32 kHz / 60 Hz in
// thousandths): the DSP side. The rings are module statics (DAT_811dd2e0 left, DAT_811e12e0 right, 0x2000 samples each).
class CWiiSound
{
public:
	static constexpr int kRing = 0x2000;
	CWiiSound(std::uint32_t rateMilli, std::uint32_t fpsMilli);
	~CWiiSound();
	bool Init();                    // RSO2 0x80d381b4
	bool Play();                    // RSO2 0x80d38408
	bool Stop();                    // RSO2 0x80d38468
	void BufferClear();             // RSO2 0x80d384c8
	bool SoundOut(std::uint32_t n, PrimaryStream& primary);  // RSO2 0x80d38518
	std::uint32_t CheckTimer();     // RSO2 0x80d386b0
	void SetSoundOutMode(int mode); // RSO2 0x80d3876c: 1 = stereo (voice 0 left, voice 1 right), else mono (both voices 0.75 on both sides)

	std::uint32_t played = 0;   // +0x00: samples the DSP played since the last CheckTimer (CConsole::OnWaitVSync stores it)
	std::int32_t carry = 0;     // +0x04: samples already mixed ahead (CConsole::UpdateStream / OnRender)
	std::uint32_t rate = 0;     // +0x14 (thousandths of Hz)

private:
	bool playing = false;       // +0x0c
	int mode = 1;               // +0x10
	std::uint32_t fps = 0;      // +0x18
	std::uint32_t perFrame = 0; // +0x1c: rate / fps + 1
	std::uint32_t block = 0;    // +0x20: rate / fps + 0x41
	std::uint32_t lastPos = 0;  // +0x28
	std::uint32_t delta = 0;    // +0x2c
	std::uint32_t writePos = 0; // +0x34
};

// the part of CConsole (the gotch frontend object, RSO2 0x80d0022c) that drives the sound: +0x38 the bound PRIMARYSTREAM (BindPrimaryStream
// 0x80d00654 from InitMachine), +0x34 the running machine, +0xc0 the CWiiSound
void Console_OnWaitVSync(CWiiSound& ws);                           // RSO2 0x80d0066c
void Console_OnRender(CWiiSound& ws, PrimaryStream* primary);      // RSO2 0x80d007c0 (primary = the bound stream; null = none)
// RSO2 0x80d006a4: mix the samples up to `time` (2^40 units per second since the frame began) now; enabled = the console's +0x33 flag.
// No ported game calls it yet (PACMAN's SoundInt does not).
void Console_UpdateStream(CWiiSound& ws, PrimaryStream* primary, std::int64_t time, bool enabled);

// SND_NAMCO_WSG (RSO2 0x80d21370 ctor, 0x80d21c18 Update, 0x80d21f00 Reset): Namco's wave sound generator, `voices` 4-bit wavetable voices
// (32 samples each), 4x oversampled per output sample
class SndNamcoWsg : public SoundModule
{
public:
	// clock in Hz as the float the subclass passes (192000000.0f for WSG3), stereo = the second volume set is mixed into a right channel
	SndNamcoWsg(PrimaryStream& primary, const char* name, int voices, bool stereo, float clock);
	void Update(int n) override;
	void Reset() override;

protected:
	std::vector<std::uint32_t> phase;      // +0x08
	std::vector<std::uint32_t> step;       // +0x0c
	std::vector<std::uint16_t> wave;       // +0x10 (offset into waves)
	std::vector<std::uint16_t> volL;       // +0x14
	std::vector<std::uint16_t> volR;       // +0x18
	std::vector<std::uint32_t> noiseLfsr;  // +0x1c
	std::vector<std::uint32_t> noiseCnt;   // +0x20
	std::vector<std::uint8_t> noise;       // +0x24
	std::int32_t gain = 0x10000;           // +0x28
	std::int8_t waves[0x200] = {};         // +0x2c: the subclass fills it from the board's wave PROM
	SoundStream* stream = nullptr;         // +0x230
	std::uint64_t factor = 0;              // +0x238 / +0x23c: 65536 * clock / (1000 * rate)
	int voices = 0;                        // +0x240
	bool stereo = false;                   // +0x244

	std::uint32_t StepOf(std::uint32_t freq) const { return (std::uint32_t)((factor * freq) >> 16); }
};

// SND_PCM (0x14 bytes; ctor RSO2 0x80d3b8b8, Update 0x80d3bac8, Reset 0x80d3bc04, SetPCM 0x80d3bc44, Play 0x80d3bd40, Stop 0x80d3bd6c): `voices`
// one-shot / looping 16-bit big-endian sample players over module data, mixed into one mono stream "PCM SOUND" (RALLYX's explosion)
class SndPcm : public SoundModule
{
public:
	SndPcm(PrimaryStream& primary, int voices);
	void Update(int n) override;
	void Reset() override;
	// rate = the sample's Hz, volume = 1.0 at 0x10000, data / length in bytes, loopStart = byte offset to loop back to (0 = play once)
	void SetPCM(int voice, int rate, std::int32_t volume, const std::uint8_t* data, std::uint32_t length, std::uint32_t loopStart);
	void Play(int voice);   // restarts from the beginning
	void Stop(int voice);

private:
	struct Voice
	{
		std::uint32_t flags = 0;      // +0x00: bit 0 = playing
		std::uint32_t length = 0;     // +0x08
		std::uint32_t loop = 0;       // +0x0c
		const std::uint8_t* data = nullptr;   // +0x10
		std::int32_t volume = 0;      // +0x14
		std::uint64_t pos = 0;        // +0x18 (hi) / +0x1c (lo): byte offset in 16.16
		std::uint64_t step = 0;       // +0x20 / +0x24
	};
	std::vector<Voice> v;             // +0x04
	SoundStream* stream = nullptr;    // +0x10
	int rate;                         // the machine's audio rate (+0x0c -> +0x80)
};

// SND_NAMCO_WSG3 (RSO2 0x80d21f84 ctor, 0x80d22120 SetRegister): the 3-voice WSG of PAC-MAN class boards (registers 0x5040..0x505f)
class SndNamcoWsg3 : public SndNamcoWsg
{
public:
	// rom = the game's WSGROM (0x100 bytes: 8 waves x 32 nibbles)
	SndNamcoWsg3(PrimaryStream& primary, const std::uint8_t* rom);
	// regs = the 0x20 sound registers (low nibbles)
	void SetRegister(const std::uint8_t* regs);
};
}
