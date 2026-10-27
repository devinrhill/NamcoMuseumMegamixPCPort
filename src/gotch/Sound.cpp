// The gotch sound chips + stream mixer (see include/gotch/Sound.hpp); 1:1 with nmwii2.rso, addresses cited as RSO2.
#include "gotch/Sound.hpp"

#include "audio/PcmOut.hpp"

#include <algorithm>
#include <atomic>
#include <cstring>

namespace gotch
{
namespace
{
// the mixer's lowpass (RSO2 0x80dc830c..0x80dc831c): w = a * x - (b1 * w1 + b2 * w2); y = c * w2 + c * w + d * w1
constexpr float kFilterA = 0.08975669741630554f;
constexpr float kFilterB1 = -0.24572893977165222f;
constexpr float kFilterB2 = 0.2754729390144348f;
constexpr float kFilterC = 2.707711696624756f;
constexpr float kFilterD = 5.415423393249512f;
// the WSG's oversampling taps (SND_NAMCO_WSG::_LOCAL_Update, RSO2 0x80dcc9c0)
constexpr int kTaps[4] = {13107, 19660, 19660, 13107};

std::int16_t Clamp16(int v) { return (std::int16_t)std::clamp(v, -0x8000, 0x7fff); }
// a signed division by 2^bits that rounds toward zero (the PowerPC srawi + addze pair)
int DivPow2(int v, int bits) { return (v >> bits) + ((v < 0 && (v & ((1 << bits) - 1)) != 0) ? 1 : 0); }
}

// RSO2 0x80d36d8c (PRIMARYSTREAM ctor) + CreateAudioSpec 0x80d0c860 (+0x30 / +0x34 = gain / 65536.0f)
PrimaryStream::PrimaryStream(int rate_, int size_, std::int32_t gL, std::int32_t gR)
    : rate(rate_), size((std::uint16_t)(size_ * 2)), ring((std::size_t)size_ * 2, 0), gainL((float)gL / 65536.0f), gainR((float)gR / 65536.0f)
{
}

// RSO2 0x80d373d0
SoundStream* PrimaryStream::CreateStream(const char* name, int channels, int size_, SoundModule* source)
{
	auto s = std::make_unique<SoundStream>();
	s->size = (std::uint16_t)((size_ & 0xffff) * channels);
	s->ring.assign(s->size, 0);
	s->source = source;
	s->name = name;
	s->channels = (std::uint8_t)channels;
	streams.push_back(std::move(s));
	return streams.back().get();
}

// RSO2 0x80d3766c
void PrimaryStream::Update(int n)
{
	const float g = (gainL + gainR) * 0.5f;  // RSO2 0x80dc8308
	if (!muted)
	{
		mixL.assign((std::size_t)n, 0);
		mixR.assign((std::size_t)n, 0);
		for (auto& sp : streams)
		{
			SoundStream& s = *sp;
			if (!s.source) continue;
			if (n - s.written > 0) s.source->Update(n - s.written);
			s.written = 0;
			const std::uint32_t mask = s.size - 1u;
			for (int i = 0; i < n; i++)
			{
				if (s.channels == 2)
				{
					mixL[i] += s.ring[s.readIndex++ & mask];
					mixR[i] += s.ring[s.readIndex++ & mask];
				}
				else
				{
					mixL[i] += s.ring[s.readIndex & mask];  // mono: the same sample on both sides
					mixR[i] += s.ring[s.readIndex++ & mask];
				}
			}
		}
		const std::uint32_t mask = size - 1u;
		for (int i = 0; i < n; i++)
		{
			float w = kFilterA * (float)mixL[i] - (kFilterB1 * wL1 + kFilterB2 * wL2);
			int y = (int)((double)(kFilterC * wL2 + kFilterC * w + kFilterD * wL1) * (double)g);
			wL2 = wL1;
			wL1 = w;
			ring[(writeIndex + i * 2) & mask] = Clamp16(y);
			w = kFilterA * (float)mixR[i] - (kFilterB1 * wR1 + kFilterB2 * wR2);
			y = (int)((double)(kFilterC * wR2 + kFilterC * w + kFilterD * wR1) * (double)g);
			wR2 = wR1;
			wR1 = w;
			ring[(writeIndex + i * 2 + 1) & mask] = Clamp16(y);
		}
	}
	else
	{
		const std::uint32_t mask = size - 1u;
		for (int i = 0; i < n * 2; i++) ring[(writeIndex + i) & mask] = 0;
	}
	writeIndex += (std::uint32_t)n * 2;
}


// ---- the DSP: AX voices looping over the CWiiSound rings (native) ----
namespace
{
std::int16_t g_dspL[CWiiSound::kRing];  // DAT_811dd2e0
std::int16_t g_dspR[CWiiSound::kRing];  // DAT_811e12e0
// the voices' current address minus the ring start (AX voice +0xa2 - 0x8ee970, read by CheckTimer): advanced by the audio thread
std::atomic<std::uint32_t> g_axPos{0};
std::atomic<bool> g_axPlaying{false};
std::atomic<int> g_axMode{1};
bool g_axOpen = false;

// the voices' mixes (RSO2 0x80dd29a8 / 0x80dd29d8: stereo, voice 0 left / voice 1 right at 0x8000; 0x80dd2a08: mono, 0x6000 on both sides)
void AxGenerate(std::int16_t* out, unsigned frames)
{
	if (!g_axPlaying.load())
	{
		std::memset(out, 0, (std::size_t)frames * 2 * sizeof(std::int16_t));
		return;
	}
	std::uint32_t pos = g_axPos.load();
	const bool stereo = g_axMode.load() == 1;
	for (unsigned i = 0; i < frames; i++)
	{
		const int l = g_dspL[pos], r = g_dspR[pos];
		if (stereo)
		{
			out[i * 2] = (std::int16_t)l;
			out[i * 2 + 1] = (std::int16_t)r;
		}
		else
		{
			const std::int16_t m = Clamp16(((l + r) * 0x6000) >> 15);
			out[i * 2] = m;
			out[i * 2 + 1] = m;
		}
		pos = (pos + 1) & (CWiiSound::kRing - 1);
	}
	g_axPos.store(pos);
}
}

// RSO2 0x80d38090
CWiiSound::CWiiSound(std::uint32_t rateMilli, std::uint32_t fpsMilli)
{
	rate = rateMilli;
	perFrame = rateMilli / fpsMilli + 1;
	fps = fpsMilli;
	block = rateMilli / fpsMilli + 0x41;
	delta = 0;
	writePos = block;
	mode = 1;
	Init();
}

CWiiSound::~CWiiSound()
{
	if (g_axOpen) audio::CloseGenerated();
	g_axOpen = false;
}

// RSO2 0x80d381b4: clears the rings, AIInit / AXInit / master volume 0x8000, acquires the two voices (PCM16, looping over the rings:
// addresses 0x8ee970..0x8f096f and 0x8f0970..0x8f296f), stopped; the mix by mode. Natively: opens the platform output at the DSP rate.
bool CWiiSound::Init()
{
	std::memset(g_dspL, 0, sizeof(g_dspL));
	std::memset(g_dspR, 0, sizeof(g_dspR));
	g_axPos.store(0);
	g_axPlaying.store(false);
	if (!g_axOpen) g_axOpen = audio::OpenGenerated((int)(rate / 1000), 2, AxGenerate);
	g_axMode.store(mode);
	playing = false;
	return true;
}

// RSO2 0x80d38408
bool CWiiSound::Play()
{
	if (!playing)
	{
		g_axPlaying.store(true);
		playing = true;
	}
	return true;
}

// RSO2 0x80d38468
bool CWiiSound::Stop()
{
	if (playing)
	{
		g_axPlaying.store(false);
		playing = false;
	}
	return true;
}

// RSO2 0x80d384c8
void CWiiSound::BufferClear()
{
	std::memset(g_dspL, 0, sizeof(g_dspL));
	std::memset(g_dspR, 0, sizeof(g_dspR));
}

// RSO2 0x80d38518: n pairs from the primary ring (its +0x10 read index) into the DSP rings at the write position
bool CWiiSound::SoundOut(std::uint32_t n, PrimaryStream& primary)
{
	if (n == 0) return true;
	for (std::uint32_t i = 0; i < n; i++)
	{
		const std::uint32_t at = (writePos + i) & (kRing - 1);
		g_dspL[at] = primary.At(primary.readIndex);
		g_dspR[at] = primary.At(primary.readIndex + 1);
		primary.readIndex += 2;
	}
	writePos = (writePos + n) & (kRing - 1);
	return true;
}

// RSO2 0x80d386b0: how far the DSP played since the last call; keeps the write position 3..6 blocks ahead of the play position
std::uint32_t CWiiSound::CheckTimer()
{
	const std::uint32_t pos = g_axPos.load();
	std::int32_t d = (std::int32_t)(pos - lastPos);
	if (d < 0) d += kRing;
	lastPos = pos;
	delta = (std::uint32_t)d & (kRing - 1);
	played = delta;
	std::int32_t ahead = (std::int32_t)(writePos - pos);
	if (ahead < 0)
	{
		if (-ahead > 0x1000) ahead += kRing;
	}
	else if (ahead > 0x1000)
		ahead -= kRing;
	if (ahead < (std::int32_t)(block * 3) || (std::int32_t)(block * 6) < ahead) writePos = (pos + block * 3) & (kRing - 1);
	return delta;
}

// RSO2 0x80d3876c
void CWiiSound::SetSoundOutMode(int mode_)
{
	mode = mode_;
	g_axMode.store(mode_);
}

// RSO2 0x80d0066c
void Console_OnWaitVSync(CWiiSound& ws) { ws.played = ws.CheckTimer(); }

// RSO2 0x80d006a4: the mid-frame mix the SoundInt of GALAGA / MOTOS / DIGDUG2 / BOSCONIAN / GROBDA asks for (only with the console's +0x33 flag).
// time = the machine time since the frame began, in 2^40 units per second (the scheduler's unit). The samples it stands for, time x rate (milli-Hz)
// x 2^40 (float 0x80d93180) / (1000 << 40) (0x80dc9e10), in floats as the module computes them, then >> 40 rounded up; at most the samples the
// DSP played and nobody has mixed yet (played - carry); the mixed count goes to carry, so OnRender only mixes the rest.
void Console_UpdateStream(CWiiSound& ws, PrimaryStream* primary, std::int64_t time, bool enabled)
{
	if (!enabled) return;
	const std::int64_t prod = time * (std::int64_t)(std::int32_t)ws.rate;
	const float f1 = (float)(1099511627776.0 * (double)(float)prod);
	const float f2 = (float)((double)f1 / (double)(float)(1000.0 * 1099511627776.0));
	const std::uint64_t u = f2 <= 0.0f ? 0u : (std::uint64_t)f2;
	const std::int32_t hi = (std::int32_t)(std::uint32_t)(u >> 32) | ((std::uint32_t)u != 0 ? 1 : 0);
	std::int32_t n = hi >> 8;
	const std::int32_t owed = (std::int32_t)ws.played - ws.carry;
	if (owed > 0)
	{
		if (owed < n) n = owed;
		if (primary) primary->Update(n);
		ws.carry += n;
	}
}

// RSO2 0x80d007c0 (the SoundOut call needs a running machine, CConsole +0x34: the caller passes primary only then)
void Console_OnRender(CWiiSound& ws, PrimaryStream* primary)
{
	const std::int32_t n = (std::int32_t)ws.played - ws.carry;
	if (n > 0 && primary) primary->Update(n);
	ws.carry = n < 0 ? -n : 0;
	if (primary) ws.SoundOut(ws.played, *primary);
}

// RSO2 0x80d21370
SndNamcoWsg::SndNamcoWsg(PrimaryStream& primary, const char* name, int voices_, bool stereo_, float clock)
{
	voices = voices_;
	stereo = stereo_;
	phase.assign(voices, 0);
	step.assign(voices, 0);
	wave.assign(voices, 0);
	volL.assign(voices, 0);
	volR.assign(voices, 0);
	noiseLfsr.assign(voices, 0);
	noiseCnt.assign(voices, 0);
	noise.assign(voices, 0);
	gain = 0x10000;
	// 65536 * clock / (1000 * rate): the rate as the machine's audio spec keeps it (ETIME per second / 2^40)
	factor = (std::uint64_t)(std::uint32_t)((65536.0f * clock) / (1000.0f * (float)primary.Rate()));
	stream = primary.CreateStream(name, stereo ? 2 : 1, 0x2000, this);
}

// RSO2 0x80d21c18
void SndNamcoWsg::Update(int n)
{
	for (int k = 0; k < n; k++)
	{
		std::int16_t l = 0, r = 0;
		for (int v = 0; v < voices; v++)
		{
			if (!noise[v])
			{
				if (step[v] == 0) continue;
				const std::int8_t* w = waves + wave[v];
				phase[v] += step[v];
				const int s1 = w[(phase[v] >> 18) & 0x1f];
				phase[v] += step[v];
				const int s2 = w[(phase[v] >> 18) & 0x1f];
				phase[v] += step[v];
				const int s3 = w[(phase[v] >> 18) & 0x1f];
				phase[v] += step[v];
				const int s4 = w[(phase[v] >> 18) & 0x1f];
				const int sum = s1 * kTaps[0] + s2 * kTaps[1] + s3 * kTaps[2] + s4 * kTaps[3];
				l = (std::int16_t)(l + DivPow2(sum * volL[v], 11));
				r = (std::int16_t)(r + DivPow2(sum * volR[v], 11));
			}
			else
			{
				const std::uint32_t old = noiseCnt[v];
				noiseCnt[v] += step[v] & 0xff;
				if ((old ^ noiseCnt[v]) & 0x100)
				{
					const std::uint32_t x = noiseLfsr[v];
					noiseLfsr[v] = (x | ((x ^ (x >> 3)) & 1) << 17) >> 1;
				}
				const int sgn = ((noiseLfsr[v] + 1) & 2) ? -0x100 : 0x100;
				l = (std::int16_t)(l + (std::int16_t)volL[v] * sgn);
				r = (std::int16_t)(r + (std::int16_t)volR[v] * sgn);
			}
		}
		stream->Put((std::int16_t)DivPow2((int)l * gain, 16));
		if (stereo) stream->Put((std::int16_t)DivPow2((int)r * gain, 16));
	}
	stream->written += n;
}

// RSO2 0x80d21f00
void SndNamcoWsg::Reset()
{
	for (int v = 0; v < voices; v++)
	{
		phase[v] = 0;
		volL[v] = 0;
		volR[v] = 0;
		wave[v] = 0;
		noiseCnt[v] = 0;
		noiseLfsr[v] = 0xffff;
		noise[v] = 0;
	}
	stream->writeIndex = 0;
	stream->readIndex = 0;
}

// RSO2 0x80d3b8b8: a mono stream "PCM SOUND" of 0x2000 samples
SndPcm::SndPcm(PrimaryStream& primary, int voices) : v((std::size_t)voices), rate(primary.Rate())
{
	stream = primary.CreateStream("PCM SOUND", 1, 0x2000, this);
	Reset();
}

// RSO2 0x80d3bc04
void SndPcm::Reset()
{
	stream->writeIndex = 0;
	stream->readIndex = 0;
	for (Voice& x : v) x.flags = 0;
}

// RSO2 0x80d3bc44: step = (rate << 16) / audio rate, in bytes (2 per sample)
void SndPcm::SetPCM(int voice, int srate, std::int32_t volume, const std::uint8_t* data, std::uint32_t length, std::uint32_t loopStart)
{
	Voice& x = v[(std::size_t)voice];
	x.flags = 0;
	x.data = data;
	x.length = length;
	x.loop = loopStart;
	x.volume = volume;
	const float q = (float)(double)((std::uint32_t)srate << 16) / (float)rate;
	x.step = (std::uint64_t)(double)q << 1;
}

// RSO2 0x80d3bd40
void SndPcm::Play(int voice)
{
	Voice& x = v[(std::size_t)voice];
	x.pos = 0;
	x.flags |= 1;
}

// RSO2 0x80d3bd6c
void SndPcm::Stop(int voice) { v[(std::size_t)voice].flags &= ~1u; }

// RSO2 0x80d3bac8
void SndPcm::Update(int n)
{
	for (int k = 0; k < n; k++)
	{
		int sum = 0;
		for (Voice& x : v)
		{
			if ((x.flags & 1) == 0) continue;
			std::uint32_t off = (std::uint32_t)((x.pos >> 32) << 16) | (std::uint32_t)(((x.pos & 0xffffffffu) >> 16) & 0xfffe);
			if (x.length <= off)
			{
				if (x.loop == 0)
				{
					x.flags &= ~1u;
					continue;
				}
				x.pos = x.loop;   // +0x1c = loop (stored unshifted, the high word is cleared; UNVERIFIED: unused by RALLYX, loop = 0)
				off = (std::uint32_t)(((x.pos & 0xffffffffu) >> 16) & 0xfffe);
			}
			const std::int16_t s = (std::int16_t)((std::uint16_t)x.data[off] << 8 | x.data[off + 1]);
			x.pos += x.step;
			sum += (std::int16_t)((std::int32_t)(x.volume * (std::int32_t)s) >> 16);
		}
		stream->Put(Clamp16(sum));
	}
	stream->written += n;
}

// RSO2 0x80d21f84: 3 voices, mono, 192000000.0f (RSO2 0x80d934ac), stream "NAMCO WSG3" (0x80d9351c); the waves are the PROM's low nibbles - 8
SndNamcoWsg3::SndNamcoWsg3(PrimaryStream& primary, const std::uint8_t* rom) : SndNamcoWsg(primary, "NAMCO WSG3", 3, false, 192000000.0f)
{
	for (int i = 0; i < 0x100; i++) waves[i] = (std::int8_t)((rom[i] & 0xf) - 8);
	Reset();
}

// RSO2 0x80d22120
void SndNamcoWsg3::SetRegister(const std::uint8_t* r)
{
	step[0] = StepOf((r[0x10] & 0xfu) | (r[0x11] & 0xfu) << 4 | (r[0x12] & 0xfu) << 8 | (r[0x13] & 0xfu) << 12 | (r[0x14] & 0xfu) << 16);
	step[1] = StepOf((r[0x16] & 0xfu) << 4 | (r[0x17] & 0xfu) << 8 | (r[0x18] & 0xfu) << 12 | (r[0x19] & 0xfu) << 16);
	step[2] = StepOf((r[0x1b] & 0xfu) << 4 | (r[0x1c] & 0xfu) << 8 | (r[0x1d] & 0xfu) << 12 | (r[0x1e] & 0xfu) << 16);
	volL[0] = r[0x15] & 0xf;
	wave[0] = (std::uint16_t)((r[0x05] & 7) << 5);
	volL[1] = r[0x1a] & 0xf;
	wave[1] = (std::uint16_t)((r[0x0a] & 7) << 5);
	volL[2] = r[0x1f] & 0xf;
	wave[2] = (std::uint16_t)((r[0x0f] & 7) << 5);
}
}
