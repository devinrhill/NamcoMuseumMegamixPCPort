#include "snd/Mixer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "WiiSystem.hpp"
#include "audio/PcmOut.hpp"
#include "snd/Sound.hpp"
#include "wii/Archive.hpp"

namespace snd
{
namespace
{
constexpr int kRate = 48000;

struct Voice
{
	int id{};
	std::unique_ptr<Source> src;
	float volume{1};
	float gain[2]{1, 1};        // 3D volume x pan ratio per output channel (SetSpatial)
	float fade{0};              // volume change per output frame while fading out
	bool fading{false};
	float pauseVol{1}, pauseStep{0};   // nw4r BasicSound::Pause: fades to 0, then the voice stands still; pauseStep < 0 = pausing
	float ext{1}, extTarget{1}, extStep{0};   // nw4r BasicSound::SetVolume: the external volume, moving to its target (MoveValue)
	double step{1};             // source frames per output frame
	double frac{0};
	std::int16_t a[2]{}, b[2]{};   // the two source frames being interpolated
	bool primed{false};
	bool ended{false};
	std::vector<std::int16_t> chunk;
	int chunkPos{0}, chunkLen{0};
};

std::mutex g_mutex;
std::vector<std::unique_ptr<Voice>> g_voices;
int g_nextId = 1;
bool g_tried = false, g_ready = false;
Archive g_archive;
std::map<int, std::shared_ptr<Wave>> g_waveCache;
std::string g_soundDir;
std::string g_dumpPath;
std::vector<std::int16_t> g_dump;

bool PullFrame(Voice& v, std::int16_t out[2])
{
	const int ch = v.src->channels();
	if (v.chunkPos >= v.chunkLen)
	{
		v.chunk.resize((std::size_t)256 * ch);
		v.chunkLen = v.src->Read(v.chunk.data(), 256);
		v.chunkPos = 0;
		if (v.chunkLen <= 0) return false;
	}
	const std::int16_t* f = &v.chunk[(std::size_t)v.chunkPos * ch];
	out[0] = f[0];
	out[1] = ch > 1 ? f[1] : f[0];
	v.chunkPos++;
	return true;
}

void Generate(std::int16_t* out, unsigned frames)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	const float master = WiiSystem::Volume();
	for (unsigned i = 0; i < frames; i++)
	{
		float l = 0, r = 0;
		for (auto& vp : g_voices)
		{
			Voice& v = *vp;
			if (v.ended) continue;
			if (v.pauseStep < 0.0f && v.pauseVol <= 0.0f) continue;
			if (!v.primed)
			{
				if (!PullFrame(v, v.a) || !PullFrame(v, v.b)) { v.ended = true; continue; }
				v.primed = true;
			}
			const float t = (float)v.frac;
			l += (v.a[0] + (v.b[0] - v.a[0]) * t) * v.volume * v.ext * v.pauseVol * v.gain[0];
			r += (v.a[1] + (v.b[1] - v.a[1]) * t) * v.volume * v.ext * v.pauseVol * v.gain[1];
			if (v.pauseStep != 0.0f)
			{
				v.pauseVol = std::max(0.0f, std::min(1.0f, v.pauseVol + v.pauseStep));
				if (v.pauseStep > 0.0f && v.pauseVol >= 1.0f) v.pauseStep = 0.0f;
			}
			if (v.ext != v.extTarget)
			{
				v.ext += v.extStep;
				if ((v.extStep > 0) == (v.ext > v.extTarget)) v.ext = v.extTarget;
			}
			v.frac += v.step;
			while (v.frac >= 1.0)
			{
				v.frac -= 1.0;
				v.a[0] = v.b[0]; v.a[1] = v.b[1];
				if (!PullFrame(v, v.b)) { v.ended = true; break; }
			}
			if (v.fading)
			{
				v.volume -= v.fade;
				if (v.volume <= 0) v.ended = true;
			}
		}
		l *= master; r *= master;
		out[2 * i] = (std::int16_t)std::max(-32768.0f, std::min(32767.0f, l));
		out[2 * i + 1] = (std::int16_t)std::max(-32768.0f, std::min(32767.0f, r));
	}
	g_voices.erase(std::remove_if(g_voices.begin(), g_voices.end(), [](const std::unique_ptr<Voice>& v) { return v->ended; }), g_voices.end());
}
}

bool Init()
{
	if (g_tried) return g_ready;
	g_tried = true;
	if (!WiiSystem::AudioEnabled()) return false;
	const std::string path = wii::ArchiveStore::ResolvePath("sound/Wii_Wander.brsar");
	if (path.empty() || !g_archive.Load(path))
	{
		std::fprintf(stderr, "sound: Wii_Wander.brsar not found under the data root, running silent\n");
		return false;
	}
	g_soundDir = path.substr(0, path.find_last_of('/') + 1);
	if (!g_dumpPath.empty()) { g_ready = true; return true; }
	if (!audio::OpenGenerated(kRate, 2, Generate))
	{
		std::fprintf(stderr, "sound: no audio device, running silent\n");
		return false;
	}
	g_ready = true;
	return true;
}

void SetDumpFile(const std::string& path) { g_dumpPath = path; }

void PumpDump()
{
	if (g_dumpPath.empty() || !Init()) return;
	std::int16_t buf[2 * (kRate / 60)];
	Generate(buf, kRate / 60);
	g_dump.insert(g_dump.end(), buf, buf + 2 * (kRate / 60));
}

void Shutdown()
{
	if (!g_dumpPath.empty() && !g_dump.empty())
	{
		if (FILE* f = std::fopen(g_dumpPath.c_str(), "wb"))
		{
			const std::uint32_t bytes = (std::uint32_t)(g_dump.size() * 2), rate = kRate, br = kRate * 4, riff = 36 + bytes, fmt = 16;
			const std::uint16_t pcm = 1, ch = 2, ba = 4, bits = 16;
			std::fwrite("RIFF", 1, 4, f); std::fwrite(&riff, 4, 1, f); std::fwrite("WAVEfmt ", 1, 8, f); std::fwrite(&fmt, 4, 1, f);
			std::fwrite(&pcm, 2, 1, f); std::fwrite(&ch, 2, 1, f); std::fwrite(&rate, 4, 1, f); std::fwrite(&br, 4, 1, f);
			std::fwrite(&ba, 2, 1, f); std::fwrite(&bits, 2, 1, f); std::fwrite("data", 1, 4, f); std::fwrite(&bytes, 4, 1, f);
			std::fwrite(g_dump.data(), 2, g_dump.size(), f);
			std::fclose(f);
		}
		g_dump.clear();
	}
	audio::CloseGenerated();
	std::lock_guard<std::mutex> lock(g_mutex);
	g_voices.clear();
	g_ready = false;
}

namespace
{
void PanGains(float volume, float pan, float out[2])
{
	// nw4r Util::CalcPanRatio with the SQRT curve, centre scaled to 1 (the mixer plays unpanned sounds at 1); UNVERIFIED: the sound's own
	// pan curve (SoundCommonInfo panCurve) is not read
	pan = std::max(-1.0f, std::min(1.0f, pan));
	out[0] = volume * std::sqrt(1.0f - pan);
	out[1] = volume * std::sqrt(1.0f + pan);
}
}

int Play(const std::string& cue) { return Play(cue, 1.0f, 0.0f); }

int Play(const std::string& cue, float volume3d, float pan3d)
{
	if (!Init()) return -1;
	const SoundDef* def = g_archive.Find(cue);
	if (def == nullptr) { std::fprintf(stderr, "sound: unknown cue '%s'\n", cue.c_str()); return -1; }
	auto v = std::make_unique<Voice>();
	v->volume = def->volume;
	if (def->type == 2)
	{
		auto s = std::make_unique<StrmSource>();
		if (!s->Open(g_soundDir + def->extPath, def->startPosition)) return -1;
		v->src = std::move(s);
	}
	else if (def->type == 3)
	{
		std::shared_ptr<Wave> w;
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			auto it = g_waveCache.find(def->id);
			if (it != g_waveCache.end()) w = it->second;
		}
		if (!w)
		{
			w = g_archive.LoadWave(*def);
			if (!w) { std::fprintf(stderr, "sound: cannot decode '%s'\n", cue.c_str()); return -1; }
			std::lock_guard<std::mutex> lock(g_mutex);
			g_waveCache[def->id] = w;
		}
		v->volume *= w->volume;
		v->src = std::make_unique<WaveSource>(w);
	}
	else { std::fprintf(stderr, "sound: '%s' is a sequence sound (not supported)\n", cue.c_str()); return -1; }
	v->step = (double)v->src->sampleRate() / kRate;
	PanGains(volume3d, pan3d, v->gain);
	std::lock_guard<std::mutex> lock(g_mutex);
	v->id = g_nextId++;
	const int id = v->id;
	g_voices.push_back(std::move(v));
	if (std::getenv("MEGAMIX_SND_LOG")) std::fprintf(stderr, "sound: play '%s' (voice %d, %s) at frame %d\n", cue.c_str(), id, def->type == 2 ? "stream" : "wave", (int)(g_dump.size() / 2 / (kRate / 60)));
	return id;
}

void Stop(int voice)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	for (auto& v : g_voices) if (v->id == voice) v->ended = true;
}

void FadeOut(int voice, int ms)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	for (auto& v : g_voices)
		if (v->id == voice)
		{
			v->fading = true;
			v->fade = v->volume / std::max(1.0f, ms * kRate / 1000.0f);
		}
}

void Pause(int voice, bool pause, int ms)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	const float step = 1.0f / std::max(1.0f, ms * kRate / 1000.0f);
	for (auto& v : g_voices)
		if (v->id == voice) v->pauseStep = pause ? -step : step;
}

void SetVolume(int voice, float volume, int ms)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	for (auto& v : g_voices)
		if (v->id == voice)
		{
			v->extTarget = volume;
			if (ms <= 0) { v->ext = volume; v->extStep = 0; }
			else v->extStep = (volume - v->ext) / std::max(1.0f, ms * kRate / 1000.0f);
		}
}

bool IsPlaying(int voice)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	for (auto& v : g_voices) if (v->id == voice && !v->ended) return true;
	return false;
}

void SetSpatial(int voice, float volume, float pan)
{
	float g[2];
	PanGains(volume, pan, g);
	std::lock_guard<std::mutex> lock(g_mutex);
	for (auto& v : g_voices) if (v->id == voice) { v->gain[0] = g[0]; v->gain[1] = g[1]; }
}

bool Sound3DParamOf(const std::string& cue, unsigned& flags, int& decayCurve, float& decayRatio)
{
	if (!Init()) return false;
	const SoundDef* def = g_archive.Find(cue);
	if (def == nullptr) return false;
	flags = def->flags3d;
	decayCurve = def->decayCurve3d;
	decayRatio = def->decayRatio3d;
	return true;
}

void StopAll()
{
	std::lock_guard<std::mutex> lock(g_mutex);
	for (auto& v : g_voices) v->ended = true;
}

void FadeOutAll(int ms)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	for (auto& v : g_voices)
	{
		v->fading = true;
		v->fade = v->volume / std::max(1.0f, ms * kRate / 1000.0f);
	}
}
}
