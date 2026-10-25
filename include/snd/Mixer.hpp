#pragma once

#include <string>

// The game's sound system on the PC: plays cues by name from the sound archive (Wii_Wander.brsar: waves = sound effects, streams = music
// from sound/stream/*.brstm) through a software mixer. Stand-in for nw4r::snd::SoundArchivePlayer / SoundHandle.
namespace snd
{
// Loads the sound archive and opens the audio device (once, lazily on the first Play). False when audio is disabled or unavailable.
bool Init();
void Shutdown();

// Starts the sound with this label; returns a voice id (>= 0) or -1 (unknown cue, no audio). Streams replace nothing: the game stops
// its own music (FadeOut / Stop).
int Play(const std::string& cue);
void Stop(int voice);
void FadeOut(int voice, int milliseconds);
bool IsPlaying(int voice);
void StopAll();
}
