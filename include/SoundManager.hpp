#pragma once

#include "common.hpp"
#include "ESEKind.hpp"
#include "EBGMKind.hpp"

// Stand-in for the Unity SoundManager of the C# port (scrapped: AudioMixer/AudioSource). The game plays cues by NAME through
// WiiSnd/snd::Mixer, so SE and BGM kinds are mapped to the cue names of Wii_Wander.brsar. A kind without an entry plays nothing.
// The original Unity version is kept in the project history only (it never compiled).
class SoundBgm
{
public:
	// play: the cue replaces the current music (the game stops its own music first)
	void Play(EBGMKind kind, float volume = 1.0f, bool bLoop = false, bool requestStopActive = true, bool checkSameAudioPlaying = false, float fade = 0.0f);
	void Stop(float fadeSeconds = 0.0f);
	bool IsPlaying() const { return m_voice >= 0; }
private:
	int m_voice{-1};
	EBGMKind m_kind{EBGMKind::BGM_STOP};
};

class SoundSeBank
{
public:
	// returns the C# SE object handle (GameObject*): always null here (the game only keeps / stops it)
	GameObject* PlaySE(ESEKind kind);
	// the C# overloads (volume, mixer, SE type, 3D target, same-kind check, ...): only the cue is used here
	template<class A, class... R> GameObject* PlaySE(ESEKind kind, A&&, R&&...) { return PlaySE(kind); }
	void Stop();
	bool IsPlayinSEKind(ESEKind) { return false; }
	void Pause() {}
	void Resume() {}
	void Suspend() {}
	// C# overloads (kind / 3D target): stopping single SEs is not supported by the mixer wrapper yet
	template<class... A> void Stop(A&&...) {}
};

class SoundManager
{
public:
	static SoundBgm* get_BGM();
	static SoundSeBank* get_SE();
};
