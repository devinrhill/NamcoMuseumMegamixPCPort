#pragma once

#include <cstdio>
#include <string>

#include "snd/Mixer.hpp"

// The game's sound calls (nw4r SoundHandle / PlaySound / fade) on top of the PC mixer (snd/Mixer: Wii_Wander.brsar waves + .brstm streams).
// The game plays sounds by cue name ("m00bgm_Title", "m00se_02Kettei", ...). Set MEGAMIX_SND_LOG=1 to print every cue.
namespace WiiSnd
{
struct Handle
{
	const char* cue{nullptr};
	int voice{-1};
};

inline void Play(Handle& h, const char* cue)
{
	h.cue = cue;
	h.voice = snd::Play(cue);
}

inline void FadeOut(Handle& h, int ms)
{
	if (h.voice >= 0) snd::FadeOut(h.voice, ms);
}

// SoundHandle::DetachSound: the handle lets go, the sound keeps playing
inline void Detach(Handle& h)
{
	h.cue = nullptr;
	h.voice = -1;
}

inline void Stop(Handle& h)
{
	if (h.voice >= 0) snd::Stop(h.voice);
	Detach(h);
}
}
