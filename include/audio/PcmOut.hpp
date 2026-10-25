#pragma once

#include <cstddef>
#include <cstdint>

// Backend independent PCM output for streamed audio (the THP movie sound). Game code includes this header; platform/audio_raylib.cpp
// implements it. Everything is a no-op returning false when no audio device exists (headless runs, --no-audio).
namespace audio
{
bool Open(int sampleRate, int channels);                      // 16-bit signed, interleaved
void Close();
bool IsOpen();
void Push(const std::int16_t* interleaved, std::size_t frames);
void SetPaused(bool paused);
std::size_t QueuedFrames();                                   // pushed but not yet played
double PlayedSeconds();                                       // audio clock: frames the device has consumed / sample rate
void SetVolume(float volume);                                 // 0..1

// A second output whose samples are produced on demand by `generate` (called on the audio thread with the number of frames wanted;
// it must fill `out` with interleaved 16-bit samples). Used by the sound mixer; independent of Open/Push (the THP movie sound).
using Generator = void (*)(std::int16_t* out, unsigned frames);
bool OpenGenerated(int sampleRate, int channels, Generator generate);
void CloseGenerated();
}
