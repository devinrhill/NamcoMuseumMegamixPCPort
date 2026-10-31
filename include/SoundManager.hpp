#pragma once

#include <vector>

#include "common.hpp"
#include "ESEKind.hpp"
#include "EBGMKind.hpp"
#include "SoundSe.hpp"

#include <type_traits>

// Stand-in for the Unity SoundManager of the C# port (scrapped: AudioMixer/AudioSource). The game plays cues by NAME through
// WiiSnd/snd::Mixer, so SE and BGM kinds are mapped to the cue names of Wii_Wander.brsar. A kind without an entry plays nothing.
// The original Unity version is kept in the project history only (it never compiled).
class SoundBgm
{
public:
	// play: the cue replaces the current music (the game stops its own music first)
	void Play(EBGMKind kind, float volume = 1.0f, bool bLoop = false, bool requestStopActive = true, bool checkSameAudioPlaying = false, float fade = 0.0f);
	void Stop(float fadeSeconds = 0.0f);
	// kingdom FUN_80026394 on the music handle: the volume moves to `volume` (the music keeps playing, also at 0)
	void SetVolume(float volume, float fadeSeconds = 0.0f);
	bool IsPlaying() const { return m_voice >= 0; }
private:
	int m_voice{-1};
	EBGMKind m_kind{EBGMKind::BGM_STOP};
};

class SoundSeBank
{
public:
	// returns a handle for Stop (GameObject* in the C#; natively the mixer voice id + 1 cast to a pointer, never dereferenced); null when nothing plays
	// the C#'s same-kind check (SoundSe.PlaySE): eEARLY = do not start while the kind plays, eLATE = fade the playing one out (0.1 s), eLATE_IMMEDIATE = stop it.
	// PlaySE(kind, [bLoop, fade, pitch, ...]) defaults to eLATE_IMMEDIATE, PlaySE(kind, EMixerType, ...) to eNONE (C# overload defaults)
	GameObject* PlaySE(ESEKind kind) { return Play(kind, SoundSe::eLATE_IMMEDIATE); }
	// the C#'s 3D target (stalkTarget, a GameObject) makes it a 3D sound placed at that object (natively: the model it stands for); the Unity
	// distances that follow it are not used (the listener's nw4r parameters are, see SetListener). The other C# arguments (volume, mixer,
	// SE type, ...) are not used natively.
	template<class A, class... R> GameObject* PlaySE(ESEKind kind, A&&, R&&... rest)
	{
		SoundSe::ESameKindCheck check = std::is_same_v<std::decay_t<A>, SoundSe::EMixerType> ? SoundSe::eNONE : SoundSe::eLATE_IMMEDIATE;
		(TakeCheck(check, rest), ...);
		GameObject* target = nullptr;
		(TakeTarget(target, rest), ...);
		Voice3D pos;
		pos.target = target;
		return Play(kind, check, target != nullptr ? &pos : nullptr);
	}
	// a 3D sound at a fixed position (game / Wii space): the DOL's kingdom 3D actor SetPosition (0x80026620) + play (0x80025988)
	GameObject* PlaySE3D(ESEKind kind, const Vector4& pos, SoundSe::ESameKindCheck check = SoundSe::eNONE)
	{
		Voice3D p;
		p.pos = Vector3(pos.x, pos.y, pos.z);
		return Play(kind, check, &p);
	}
	// the nw4r Sound3DListener of the running game (one listener): the camera's look-at matrix (eye, target, up: the DOL's 0x80137824)
	// and its interior size / max volume distance / unit distance. While no listener is set 3D sounds play unattenuated.
	void SetListenerParam(float interiorSize, float maxVolumeDistance, float unitDistance);
	void SetListener(const Vector4& eye, const Vector4& target, const Vector4& up);
	void ClearListener();
	// the kingdom 3D sound actors (cKingdomSndActor, nw4r Sound3DActor): a cue played by NAME on an actor (DOL 0x80025988 with the actor) is
	// placed at the actor's position and follows it (SetActorPosition, DOL 0x80026620); a forgotten actor's sounds stay where it was last.
	// Returns the mixer voice (-1 = nothing plays).
	int PlayActor(const char* cue, const void* actor, const Vector3& pos);
	// the same by SE kind; returns a handle for IsPlaying / Stop (voice id + 1, null when nothing plays)
	GameObject* PlayActor(ESEKind kind, const void* actor, const Vector3& pos);
	void SetActorPosition(const void* actor, const Vector3& pos);
	void ForgetActor(const void* actor);
	// the object is going away (cKingdomModel::Term): its 3D sounds stay where it was last
	void ForgetTarget(const GameObject* target);
	// the C# Stop overloads; fades are in seconds (the C# writes frames / 60)
	void Stop(float fadeSeconds = 0.0f);          // every SE this bank started
	void Stop(ESEKind kind, float fadeSeconds = 0.0f);
	void Stop(GameObject* handle, float fadeSeconds = 0.0f);
	bool IsPlayinSEKind(ESEKind kind);
	// the handle still has a sound (kingdom sound handle test, DOL 0x80026424); a handle whose sound ended stays non-null
	bool IsPlaying(GameObject* handle);
	void Pause() {}
	void Resume() {}
	void Suspend() {}
private:
	struct Voice3D
	{
		GameObject* target{};     // follows this object (its model's world position) when set
		Vector3 pos{};            // else this fixed position
		unsigned flags{};
		int decayCurve{1};
		float decayRatio{0.5f};
	};
	struct Voice { ESEKind kind; int id; bool is3d{false}; Voice3D p3d{}; const void* actor{}; };
	GameObject* Play(ESEKind kind, SoundSe::ESameKindCheck check, const Voice3D* p3d = nullptr);
	void Calc3D(Voice& v, float& volume, float& pan);
	void Update3D(Voice& v);
	template<class T> static void TakeTarget(GameObject*& target, const T& arg)
	{
		if constexpr (std::is_convertible_v<T, GameObject*> && !std::is_same_v<std::decay_t<T>, std::nullptr_t>) target = arg;
	}
	template<class T> static void TakeCheck(SoundSe::ESameKindCheck& check, const T& arg)
	{
		if constexpr (std::is_same_v<std::decay_t<T>, SoundSe::ESameKindCheck>) check = arg;
	}
	std::vector<Voice> m_voices;
	std::vector<Voice> m_actorVoices;   // PlayActor's (kept apart from the C# kinds: Stop(kind) / Stop() never touch them)
	void Prune();
};

class SoundManager
{
public:
	static SoundBgm* get_BGM();
	static SoundSeBank* get_SE();
};
