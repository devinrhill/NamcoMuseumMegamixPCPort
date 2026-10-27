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
// the same, starting with a 3D volume / pan already applied (SetSpatial)
int Play(const std::string& cue, float volume3d, float pan3d);
void Stop(int voice);
void FadeOut(int voice, int milliseconds);
bool IsPlaying(int voice);
// nw4r BasicSound::SetVolume(volume, frames): the voice's external volume (1 = as authored) moves to `volume` in `milliseconds` (0 = at once)
void SetVolume(int voice, float volume, int milliseconds);
// 3D sound (nw4r Sound3DEngine's ambient volume / pan): volume 0..1, pan -1 (left) .. 1 (right); kept until changed
void SetSpatial(int voice, float volume, float pan);
// the cue's Sound3DParam from the archive; false when the cue is unknown (no archive)
bool Sound3DParamOf(const std::string& cue, unsigned& flags, int& decayCurve, float& decayRatio);
void StopAll();
// every playing voice fades out over `ms` (kingdom FUN_80025560(KingSys +0xa54, ms): SoundPlayer::StopAllSound(frames) on each player)
void FadeOutAll(int milliseconds);
}
