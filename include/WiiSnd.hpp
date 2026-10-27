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

// kingdom FUN_80026394(volume, handle, ms): BasicSound::SetVolume on the handle's sound (it keeps playing, also at 0)
inline void SetVolume(Handle& h, float volume, int ms)
{
	if (h.voice >= 0) snd::SetVolume(h.voice, volume, ms);
}

// SoundHandle::DetachSound: the handle lets go, the sound keeps playing
inline void Detach(Handle& h)
{
	h.cue = nullptr;
	h.voice = -1;
}

// kingdom FUN_80025b6c(handles, pad, cue, fadeMs): the cue on the TV (handles[0]) and a copy on Wiimote `pad`'s speaker (handles[1]; pad < 0 =
// each of the four remotes), output line REMOTE_n (table 0x803691e8 = 2, 4, 8, 0x10) at the remote out volume FLOAT_80478f58 (0x80025218:
// WPADGetSpeakerVolume / 127 * 0.5). Nothing goes to the remotes while HOME is held (get_pad1(0)->hold & 0x400). The fade-in (fadeMs) is
// not ported (every port call site passes 0). Native: a PC has no Wiimote speaker, so the copy is mixed into the main output (as Dolphin
// does) for connected remotes only, at the `remote_speaker` volume of megamix.cfg (0 = off, the default). The remote copy keeps no handle.
// UNVERIFIED: with pad < 0 the DOL starts the TV copy into handles[0] again before each remote copy (five TV starts); the port plays it once.
void PlayRemote(Handle& h, int pad, const char* cue);

inline void Stop(Handle& h)
{
	if (h.voice >= 0) snd::Stop(h.voice);
	Detach(h);
}
}
