#include "SoundManager.hpp"

#include <cstdio>
#include <cstdlib>

#include "snd/Mixer.hpp"

// DOL: cues are strings handed to FUN_80025b6c (see WiiSnd.hpp). Names from the Wii_Wander.brsar label table.
namespace
{
const char* SeCue(ESEKind kind)
{
	switch (kind)
	{
	case ESEKind::RESIDENT_SND_SE01_SELECT: return "m00se_01idou";
	case ESEKind::RESIDENT_SND_SE02_DECIDE: return "m00se_02Kettei";
	case ESEKind::RESIDENT_SND_SE03_CANCEL: return "m00se_03Cancel";
	case ESEKind::RESIDENT_SND_SE04_GAMEOVER: return "m00se_04Gameover";
	case ESEKind::RESIDENT_SND_SE05_COUNT: return "m00se_05Count";
	case ESEKind::RESIDENT_SND_SE06_HISCORE: return "m00se_06Hiscore";
	case ESEKind::RESIDENT_SND_SE07_WORLDSELECT: return "m00se_07select";
	case ESEKind::RESIDENT_SND_SE08_DECIDE_NG: return "m00se_08NG";
	case ESEKind::RESIDENT_SND_SE09_NEWSTAGE: return "m00se_09Newstage";
	case ESEKind::RESIDENT_SND_SE10_PAUSE: return "m00se_10Pause";
	default: return nullptr;
	}
}

const char* BgmCue(EBGMKind kind)
{
	switch (kind)
	{
	case EBGMKind::PACN_ROLL_SND_BGM01_SELECT: return "m03bgm_Select";
	case EBGMKind::PACN_ROLL_SND_BGM02_WORLD1: return "m03bgm_World1";
	case EBGMKind::PACN_ROLL_SND_BGM03_WORLD2: return "m03bgm_World2";
	case EBGMKind::PACN_ROLL_SND_BGM04_WORLD3: return "m03bgm_World3";
	case EBGMKind::PACN_ROLL_SND_BGM05_WORLD4: return "m03bgm_World4";
	case EBGMKind::PACN_ROLL_SND_BGM06_WORLD5: return "m03bgm_World5";
	case EBGMKind::PACN_ROLL_SND_BGM07_BOSS1: return "m03bgm_Boss1";
	case EBGMKind::PACN_ROLL_SND_BGM08_BOSS2: return "m03bgm_Boss2";
	case EBGMKind::PACN_ROLL_SND_BGM09_ENDING: return "m03bgm_Ending";
	case EBGMKind::PACN_ROLL_SND_BGM10_WROLD3B: return "m03bgm_World3_2";
	case EBGMKind::PACN_ROLL_SND_BGM11_GOAL: return "m03bgm_Goal";
	// Pac-Motos cues (m07bgm_* in main.dol; the C# names: WORLD1_3 = World1, WROLD2_4 = World2, WROLD5 = World5)
	case EBGMKind::MOTOS_SND_BGM01_SELECT: return "m07bgm_Select";
	case EBGMKind::MOTOS_SND_BGM02_WORLD1_3: return "m07bgm_World1";
	case EBGMKind::MOTOS_SND_BGM03_WROLD2_4: return "m07bgm_World2";
	case EBGMKind::MOTOS_SND_BGM04_WROLD5: return "m07bgm_World5";
	case EBGMKind::MOTOS_SND_BGM05_BOSS1: return "m07bgm_Boss1";
	case EBGMKind::MOTOS_SND_BGM06_BOSS2: return "m07bgm_Boss2";
	case EBGMKind::MOTOS_SND_BGM07_ENDING: return "m07bgm_Ending";
	case EBGMKind::GALAGA_SND_BGM01_SELECT: return "m01bgm_Select";
	case EBGMKind::GALAGA_SND_BGM02_WORLD1: return "m01bgm_World1";
	case EBGMKind::GALAGA_SND_BGM03_WORLD2: return "m01bgm_World2";
	case EBGMKind::GALAGA_SND_BGM04_WORLD3: return "m01bgm_World3";
	case EBGMKind::GALAGA_SND_BGM05_BOSS1: return "m01bgm_Boss1";
	case EBGMKind::GALAGA_SND_BGM06_BOSS2: return "m01bgm_Boss2";
	case EBGMKind::GALAGA_SND_BGM07_ENDING: return "m01bgm_Ending";
	default: return nullptr;
	}
}
}

void SoundBgm::Play(EBGMKind kind, float, bool, bool, bool checkSameAudioPlaying, float)
{
	if (checkSameAudioPlaying && m_voice >= 0 && m_kind == kind && snd::IsPlaying(m_voice)) return;
	Stop();
	const char* cue = BgmCue(kind);
	if (cue == nullptr) return;
	m_voice = snd::Play(cue);
	m_kind = kind;
}

void SoundBgm::Stop(float fadeSeconds)
{
	if (m_voice >= 0)
	{
		if (fadeSeconds > 0.0f) snd::FadeOut(m_voice, (int)(fadeSeconds * 1000.0f));
		else snd::Stop(m_voice);
	}
	m_voice = -1;
	m_kind = EBGMKind::BGM_STOP;
}

GameObject* SoundSeBank::PlaySE(ESEKind kind)
{
	if (const char* cue = SeCue(kind)) snd::Play(cue);
	return nullptr;
}

void SoundSeBank::Stop() {}

SoundBgm* SoundManager::get_BGM()
{
	static SoundBgm bgm;
	return &bgm;
}

SoundSeBank* SoundManager::get_SE()
{
	static SoundSeBank se;
	return &se;
}
