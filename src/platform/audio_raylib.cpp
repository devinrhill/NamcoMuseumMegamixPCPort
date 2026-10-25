// raylib backend of audio/PcmOut.hpp: one AudioStream fed from a ring buffer by raylib's audio thread
#include "audio/PcmOut.hpp"

#include <raylib.h>

#include <atomic>
#include <cstring>
#include <deque>
#include <mutex>
#include <vector>

namespace audio
{
namespace
{
AudioStream g_stream{};
bool g_open = false;
int g_rate = 0, g_channels = 0;
std::mutex g_mutex;
std::deque<std::int16_t> g_queue;
std::atomic<unsigned long long> g_consumed{0};   // frames handed to the device

void Callback(void* buffer, unsigned frames)
{
	std::int16_t* out = static_cast<std::int16_t*>(buffer);
	std::lock_guard<std::mutex> lock(g_mutex);
	const std::size_t want = (std::size_t)frames * g_channels;
	const std::size_t have = std::min(want, g_queue.size());
	for (std::size_t i = 0; i < have; i++) { out[i] = g_queue.front(); g_queue.pop_front(); }
	if (have < want) std::memset(out + have, 0, (want - have) * sizeof(std::int16_t));
	g_consumed += have / g_channels; // only real samples advance the clock, so an underrun stalls it instead of running ahead
}
}

bool Open(int sampleRate, int channels)
{
	if (g_open) Close();
	if (!IsAudioDeviceReady()) return false;
	SetAudioStreamBufferSizeDefault(1024);
	g_stream = LoadAudioStream((unsigned)sampleRate, 16, (unsigned)channels);
	if (!IsAudioStreamValid(g_stream)) return false;
	g_rate = sampleRate;
	g_channels = channels;
	g_consumed = 0;
	{
		std::lock_guard<std::mutex> lock(g_mutex);
		g_queue.clear();
	}
	SetAudioStreamCallback(g_stream, Callback);
	PlayAudioStream(g_stream);
	g_open = true;
	return true;
}

void Close()
{
	if (!g_open) return;
	StopAudioStream(g_stream);
	UnloadAudioStream(g_stream);
	g_open = false;
}

bool IsOpen() { return g_open; }

void Push(const std::int16_t* interleaved, std::size_t frames)
{
	if (!g_open) return;
	std::lock_guard<std::mutex> lock(g_mutex);
	g_queue.insert(g_queue.end(), interleaved, interleaved + frames * g_channels);
}

void SetPaused(bool paused)
{
	if (!g_open) return;
	if (paused) PauseAudioStream(g_stream); else ResumeAudioStream(g_stream);
}

std::size_t QueuedFrames()
{
	std::lock_guard<std::mutex> lock(g_mutex);
	return g_channels ? g_queue.size() / g_channels : 0;
}

double PlayedSeconds() { return g_open && g_rate ? (double)g_consumed.load() / g_rate : 0.0; }

void SetVolume(float volume) { if (g_open) SetAudioStreamVolume(g_stream, volume); }

namespace
{
AudioStream g_gen{};
bool g_genOpen = false;
Generator g_generate = nullptr;
void GenCallback(void* buffer, unsigned frames)
{
	if (g_generate) g_generate(static_cast<std::int16_t*>(buffer), frames);
}
}

bool OpenGenerated(int sampleRate, int channels, Generator generate)
{
	if (g_genOpen) CloseGenerated();
	if (!IsAudioDeviceReady()) return false;
	SetAudioStreamBufferSizeDefault(1024);
	g_gen = LoadAudioStream((unsigned)sampleRate, 16, (unsigned)channels);
	if (!IsAudioStreamValid(g_gen)) return false;
	g_generate = generate;
	SetAudioStreamCallback(g_gen, GenCallback);
	PlayAudioStream(g_gen);
	g_genOpen = true;
	return true;
}

void CloseGenerated()
{
	if (!g_genOpen) return;
	StopAudioStream(g_gen);
	UnloadAudioStream(g_gen);
	g_genOpen = false;
}
}
