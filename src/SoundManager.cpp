#include "SoundManager.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "snd/Mixer.hpp"
#include "GamePad.hpp"
#include "WiiPad.hpp"
#include "WiiSnd.hpp"
#include "WiiSystem.hpp"
#include "kingdom/cKingdomModel.hpp"

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
	// Pac-N-Roll (m03se_*) and Pac-Motos (m07se_*): matched to the cue strings of main.dol by the SE number of the C# names; the four
	// MOTOS SE057_* kinds are 57..60 by name. No cue in the DOL: PACN_ROLL SE060_MATAL, SE083_DAMAGE2 (silent).
	case ESEKind::MOTOS_SND_SE001_HIT_ZAKO01: return "m07se_01_zako01";
	case ESEKind::MOTOS_SND_SE002_HIT_ZAKO02: return "m07se_02_zako02";
	case ESEKind::MOTOS_SND_SE003_HIT_ZAKO03: return "m07se_03_zako03";
	case ESEKind::MOTOS_SND_SE004_HIT_BOSS01: return "m07se_04_boss01";
	case ESEKind::MOTOS_SND_SE005_HIT_WALL: return "m07se_05_syoutotu01";
	case ESEKind::MOTOS_SND_SE006_SMOLE_BOM: return "m07se_06_bakuha_syou";
	case ESEKind::MOTOS_SND_SE007_BOSS_DEATH: return "m07se_07_boss_bakuha";
	case ESEKind::MOTOS_SND_SE008_DASH: return "m07se_08_dush";
	case ESEKind::MOTOS_SND_SE009_CHARGE: return "m07se_09_charge01";
	case ESEKind::MOTOS_SND_SE010_CHARGE_MAX: return "m07se_10_charge02";
	case ESEKind::MOTOS_SND_SE011_GIMMICK_SW: return "m07se_11_switch";
	case ESEKind::MOTOS_SND_SE012_ITEM_GET: return "m07se_12_item_get";
	case ESEKind::MOTOS_SND_SE015_JINGLE1: return "m07se_15_stage_start";
	case ESEKind::MOTOS_SND_SE016_GAMECLEAR: return "m07se_16_stage_clear";
	case ESEKind::MOTOS_SND_SE017_JUMP: return "m07se_17_jump";
	case ESEKind::MOTOS_SND_SE019_TOUCHDOWN: return "m07se_19_tyakuti";
	case ESEKind::MOTOS_SND_SE020_BREAK: return "m07se_20_kuzure";
	case ESEKind::MOTOS_SND_SE021_EXE_BOM: return "m07se_21_bakuha_yuka";
	case ESEKind::MOTOS_SND_SE022_EXE_REACION: return "m07se_22_handou_yuka";
	case ESEKind::MOTOS_SND_SE023_BOSS_MOVE1: return "m07se_23_boss_move01";
	case ESEKind::MOTOS_SND_SE024_BOSS_ROT1: return "m07se_24_boss_turn";
	case ESEKind::MOTOS_SND_SE025_BOSS_MOVE1: return "m07se_25_boss_move02";
	case ESEKind::MOTOS_SND_SE026_BOSS_FALL1: return "m07se_26_boss_kouka";
	case ESEKind::MOTOS_SND_SE027_HATCH_SE: return "m07se_27_hatch";
	case ESEKind::MOTOS_SND_SE028_METEO: return "m07se_28_meteor";
	case ESEKind::MOTOS_SND_SE029_SPECIAL: return "m07se_29_attack";
	case ESEKind::MOTOS_SND_SE030_DEAD: return "m07se_30_yarare";
	case ESEKind::MOTOS_SND_SE031_BOSS_APPEAR1: return "m07se_31_boss_appear01";
	case ESEKind::MOTOS_SND_SE032_ITEM_SELECT: return "m07se_32_item_sentaku";
	case ESEKind::MOTOS_SND_SE033_ITEM_: return "m07se_33_item_kettei";
	case ESEKind::MOTOS_SND_SE034_GIGA_PETA: return "m07se_34_giga_sekkinn";
	case ESEKind::MOTOS_SND_SE037_SLOPE_SE: return "m07se_37_panel_incline";
	case ESEKind::MOTOS_SND_SE038_BOSS_APPEAR2: return "m07se_38_boss_appear02";
	case ESEKind::MOTOS_SND_SE039_BOSS_APPEAR3: return "m07se_39_boss_appear03";
	case ESEKind::MOTOS_SND_SE046_BOSS_HIT: return "m07se_46_boss_hit";
	case ESEKind::MOTOS_SND_SE047_BOSS_FALL: return "m07se_47_boss_down";
	case ESEKind::MOTOS_SND_SE048_JINGLE2: return "m07se_48_result";
	case ESEKind::MOTOS_SND_SE049_JINGLE3: return "m07se_49_champion";
	case ESEKind::MOTOS_SND_SE050_COUNTDOWN: return "m07se_50_count_down";
	case ESEKind::MOTOS_SND_SE051_BLAST_BOM: return "m07se_51_yuka_bakuha";
	case ESEKind::MOTOS_SND_SE052_BOSS_CORE: return "m07se_52_boss_core";
	case ESEKind::MOTOS_SND_SE053_FAINT: return "m07se_53_piyori";
	case ESEKind::MOTOS_SND_SE054_HIT_ZAKO04: return "m07se_54_zako04";
	case ESEKind::MOTOS_SND_SE056_HIT_BEACON: return "m07se_56_beacon";
	case ESEKind::MOTOS_SND_SE057_CHARGE03: return "m07se_58_charge03";
	case ESEKind::MOTOS_SND_SE057_ENERGY: return "m07se_59_energy";
	case ESEKind::MOTOS_SND_SE057_HIT_BALL: return "m07se_57_ball";
	case ESEKind::MOTOS_SND_SE057_SPARK: return "m07se_60_spark";
	case ESEKind::PACN_ROLL_SND_SE002_FLAG: return "m03se_002_1UP";
	case ESEKind::PACN_ROLL_SND_SE015_COOKIE: return "m03se_015_Cookie";
	case ESEKind::PACN_ROLL_SND_SE016_FLUIT: return "m03se_016_Fluit";
	case ESEKind::PACN_ROLL_SND_SE017_GHOST: return "m03se_017_Ghost";
	case ESEKind::PACN_ROLL_SND_SE018_KOROGARI1: return "m03se_018_korogari1";
	case ESEKind::PACN_ROLL_SND_SE019_KOROGARI2: return "m03se_019_Korogari2";
	case ESEKind::PACN_ROLL_SND_SE020_KOROGARI3: return "m03se_020_Korogari3";
	case ESEKind::PACN_ROLL_SND_SE021_DAMAGE: return "m03se_021_Damage";
	case ESEKind::PACN_ROLL_SND_SE022_DASH: return "m03se_022_Dash";
	case ESEKind::PACN_ROLL_SND_SE023_RAKKA: return "m03se_023_rakka";
	case ESEKind::PACN_ROLL_SND_SE024_BRAKE: return "m03se_024_Brake";
	case ESEKind::PACN_ROLL_SND_SE025_HENNSHINN: return "m03se_025_Hennsinn";
	case ESEKind::PACN_ROLL_SND_SE026_KAZE: return "m03se_026_Kaze";
	case ESEKind::PACN_ROLL_SND_SE027_COUNT: return "m03se_027_Count";
	case ESEKind::PACN_ROLL_SND_SE028_POINT_TUUKA: return "m03se_028_Point_Tuuka";
	case ESEKind::PACN_ROLL_SND_SE029_ESA_TARINAI: return "m03se_029_Esa_Tarinai";
	case ESEKind::PACN_ROLL_SND_SE030_ESA_TRITA: return "m03se_030_Esa_Trita";
	case ESEKind::PACN_ROLL_SND_SE031_JUMP: return "m03se_031_Jump";
	case ESEKind::PACN_ROLL_SND_SE032_IJIKE: return "m03se_032_Ijike";
	case ESEKind::PACN_ROLL_SND_SE033_IJIKE_OWARI: return "m03se_033_Ijike_Owari";
	case ESEKind::PACN_ROLL_SND_SE035_METAL_BOX: return "m03se_035_Metal_Box";
	case ESEKind::PACN_ROLL_SND_SE037_BOX_HAKAI: return "m03se_037_Box_Hakai";
	case ESEKind::PACN_ROLL_SND_SE038_SUITYUU: return "m03se_038_Suityuu";
	case ESEKind::PACN_ROLL_SND_SE039_WATER: return "m03se_039_Water";
	case ESEKind::PACN_ROLL_SND_SE040_TRAIN: return "m03se_040_Train";
	case ESEKind::PACN_ROLL_SND_SE041_KUUTYUU: return "m03se_041_Kuutyuu";
	case ESEKind::PACN_ROLL_SND_SE043_RIVER: return "m03se_043_River";
	case ESEKind::PACN_ROLL_SND_SE044_ENGINE1: return "m03se_044_Engine1";
	case ESEKind::PACN_ROLL_SND_SE045_ENGINE2: return "m03se_045_Engine2";
	case ESEKind::PACN_ROLL_SND_SE046_WALL: return "m03se_046_Wall";
	case ESEKind::PACN_ROLL_SND_SE048_GRASS: return "m03se_048_Kusa";
	case ESEKind::PACN_ROLL_SND_SE049_YOUGAN: return "m03se_049_Yougan";
	case ESEKind::PACN_ROLL_SND_SE050_PIZA: return "m03se_050_Piza";
	case ESEKind::PACN_ROLL_SND_SE051_PIYORI: return "m03se_051_Piyori";
	case ESEKind::PACN_ROLL_SND_SE052_KAKYUU: return "m03se_052_Kakkuu";
	case ESEKind::PACN_ROLL_SND_SE056_YUKA_YURE: return "m03se_056_Yuka_Yure";
	case ESEKind::PACN_ROLL_SND_SE058_SHOT: return "m03se_058_Shot";
	case ESEKind::PACN_ROLL_SND_SE059_YA: return "m03se_059_Ya";
	case ESEKind::PACN_ROLL_SND_SE061_TYAKUTI1: return "m03se_061_Tyakuti1";
	case ESEKind::PACN_ROLL_SND_SE062_TYAKUTI2: return "m03se_062_Tyakuti2";
	case ESEKind::PACN_ROLL_SND_SE063_TYAKUTI3: return "m03se_063_Tyakuti3";
	case ESEKind::PACN_ROLL_SND_SE064_AIM_HYOUJI: return "m03se_064_Aim_Hyouji";
	case ESEKind::PACN_ROLL_SND_SE065_AIM_KOTEI: return "m03se_065_Aim_Kotei";
	case ESEKind::PACN_ROLL_SND_SE072_BOSS_BOM: return "m03se_072_Boss_Bom";
	case ESEKind::PACN_ROLL_SND_SE073_KAITEN: return "m03se_073_Guitar_Kaiten";
	case ESEKind::PACN_ROLL_SND_SE074_TATAKI: return "m03se_074_Guitar_Tataki";
	case ESEKind::PACN_ROLL_SND_SE075_ENSOU: return "m03se_075_Guitar_Ensou";
	case ESEKind::PACN_ROLL_SND_SE076_BOSS_JUMP: return "m03se_076_Boss_Jump";
	case ESEKind::PACN_ROLL_SND_SE077_BOSS_RAKKA_1: return "m03se_077_Rakka_1";
	case ESEKind::PACN_ROLL_SND_SE078_BOSS_RAKKA_2: return "m03se_078_Rakka_2";
	case ESEKind::PACN_ROLL_SND_SE079_BOSS_RAKKA_3: return "m03se_079_Rakka_3";
	case ESEKind::PACN_ROLL_SND_SE080_BOSS_TATAKI: return "m03se_080_Boss_Tataki";
	case ESEKind::PACN_ROLL_SND_SE084_BOSS_IJIKE_1: return "m03se_084_Boss_Ijike_1";
	case ESEKind::PACN_ROLL_SND_SE085_BOSS_IJIKE_2: return "m03se_085_Boss_Ijike_2";
	case ESEKind::PACN_ROLL_SND_SE086_BOSS_IJIKE_3: return "m03se_086_Boss_Ijike_3";
	case ESEKind::PACN_ROLL_SND_SE087_BOSS_IJIKE_4: return "m03se_087_Boss_Ijike_4";
	case ESEKind::PACN_ROLL_SND_SE088_AKUBI: return "m03se_088_Pac_Akubi";
	case ESEKind::PACN_ROLL_SND_SE089_PAC_DASH: return "m03se_089_Pac_Dash";
	case ESEKind::PACN_ROLL_SND_SE090_PAC_DASH_DANSA: return "m03se_090_Pac_Dansa";
	case ESEKind::PACN_ROLL_SND_SE091_PAC_WATER_DASH: return "m03se_091_Pac";
	case ESEKind::PACN_ROLL_SND_SE092_PAC_SIGAMI: return "m03se_092_Pac_sigami";
	case ESEKind::PACN_ROLL_SND_SE093_PAC_DASH2: return "m03se_093_Pac_Dash_2";
	case ESEKind::PACN_ROLL_SND_SE094_GHOST: return "m03se_094_Pac_Ghost";
	case ESEKind::PACN_ROLL_SND_SE095_RAKKA: return "m03se_095_Pac_Rakka";
	case ESEKind::PACN_ROLL_SND_SE096_DAMAGE: return "m03se_096_Pac_Damage";
	case ESEKind::PACN_ROLL_SND_SE097_DAMAGE2: return "m03se_097_Pac_Damage_2";
	case ESEKind::PACN_ROLL_SND_SE098_TATAKI: return "m03se_098_Gol_Tataki";
	case ESEKind::PACN_ROLL_SND_SE099_GOL_TOUJYOU: return "m03se_099_Gol_Toujyou";
	case ESEKind::PACN_ROLL_SND_SE100_GOL_HANYOU: return "m03se_100_Gol_Bonnyou";
	case ESEKind::PACN_ROLL_SND_SE101_GOL_IJIKE: return "m03se_101_Gol_Ijike";
	case ESEKind::PACN_ROLL_SND_SE102_GOL: return "m03se_102_Gol";
	case ESEKind::PACN_ROLL_SND_SE103_GOL_2: return "m03se_103_Gol_2";
	case ESEKind::PACN_ROLL_SND_SE104_GH_TORITUKI: return "m03se_104_Gh_Torituki";
	case ESEKind::PACN_ROLL_SND_SE105_GH_TORITUKI2: return "m03se_105_Gh_Torituki_2";
	case ESEKind::PACN_ROLL_SND_SE107_GOAL_1: return "m03se_107_Goal_1";
	case ESEKind::PACN_ROLL_SND_SE108_GOAL_2: return "m03se_108_Goal_2";
	case ESEKind::PACN_ROLL_SND_SE109_GOAL_3: return "m03se_109_Goal_3";
	case ESEKind::PACN_ROLL_SND_SE110_GEAR_01: return "m03se_110_Gear_01";
	case ESEKind::PACN_ROLL_SND_SE111_GEAR_02: return "m03se_111_Gear_02";
	case ESEKind::PACN_ROLL_SND_SE112_GEAR_03: return "m03se_112_Gear_03";
	case ESEKind::PACN_ROLL_SND_SE113_SPRING: return "m03se_113_Spring";
	case ESEKind::PACN_ROLL_SND_SE114_PENDULUM: return "m03se_114_Pendulum";
	case ESEKind::PACN_ROLL_SND_SE115_PISTON: return "m03se_115_Piston";
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
	case EBGMKind::RALLYX_SND_BGM_ENDING: return "m02bgm_Ending";
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

// DOL 0x80026394 [audited: handle pair = native id; fade-time argument ignored where unsupported] (BasicSound::SetVolume on the handle's sound)
void SoundBgm::SetVolume(float volume, float fadeSeconds)
{
	if (m_voice >= 0) snd::SetVolume(m_voice, volume, (int)(fadeSeconds * 1000.0f));
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

namespace
{
// nw4r::snd::Sound3DListener (one listener) + Sound3DManager defaults (pan range 0.9)
struct Listener
{
	bool active{false};
	float interiorSize{1.0f}, maxVolumeDistance{1.0f}, unitDistance{1.0f};
	Vector3 eye{}, right{1, 0, 0}, up{0, 1, 0}, look{0, 0, 1};   // C_MTXLookAt rows (look = eye - target)
} g_listener;

Vector3 Sub(const Vector3& a, const Vector3& b) { return Vector3(a.x - b.x, a.y - b.y, a.z - b.z); }
float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vector3 Cross(const Vector3& a, const Vector3& b) { return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }
Vector3 Norm(const Vector3& a)
{
	const float l = std::sqrt(Dot(a, a));
	return l > 0.0f ? Vector3(a.x / l, a.y / l, a.z / l) : a;
}

// nw4r Sound3DCalculator::CalcVolumeAndPriorityImpl (volume part)
float CalcVolume(float dist, int curve, float ratio)
{
	const Listener& L = g_listener;
	if (dist <= L.maxVolumeDistance) return 1.0f;
	if (curve == 2) return std::max(0.0f, 1.0f - (dist - L.maxVolumeDistance) / L.unitDistance * (1.0f - ratio));
	return std::pow(ratio, (dist - L.maxVolumeDistance) / L.unitDistance);
}

// nw4r Sound3DCalculator::CalcPanStereo + CalcAngleAndDistance (speaker angle 45 degrees, pan range 0.9); pos in listener space
float CalcPan(const Vector3& pos)
{
	const float kPi = 3.14159265f, kSpeaker = kPi / 4.0f, kPanRange = 0.9f;
	const float interior = g_listener.interiorSize;
	const float actorDistance = std::sqrt(Dot(pos, pos));
	float ix = 0.0f, iz = 0.0f;
	if (actorDistance != 0.0f)
	{
		float px = pos.x, pz = pos.z;
		const float d = std::sqrt(px * px + pz * pz);
		if (d > interior) { px *= interior / d; pz *= interior / d; }
		const float yPlane = std::sqrt(px * px + pz * pz);
		ix = pos.x * yPlane / actorDistance;
		iz = pos.z * yPlane / actorDistance;
	}
	const float angle = std::atan2(ix, -iz);
	const float distance = std::sqrt(ix * ix + iz * iz) / interior;
	auto lin = [](float x, float x0, float x1, float y0, float y1) { return x0 == x1 ? y0 : y0 + (x - x0) * (y1 - y0) / (x1 - x0); };
	float x;
	if (angle < -kPi + kSpeaker) x = lin(angle, -kPi, -kPi + kSpeaker, 0.0f, -1.0f);
	else if (angle < -kSpeaker) x = -1.0f;
	else if (angle < kSpeaker) x = lin(angle, -kSpeaker, kSpeaker, -1.0f, 1.0f);
	else if (angle < kPi - kSpeaker) x = 1.0f;
	else x = lin(angle, kPi - kSpeaker, kPi, 1.0f, 0.0f);
	return x * kPanRange * distance;
}
}

void SoundSeBank::SetListenerParam(float interiorSize, float maxVolumeDistance, float unitDistance)
{
	g_listener.interiorSize = interiorSize;
	g_listener.maxVolumeDistance = maxVolumeDistance;
	g_listener.unitDistance = unitDistance;
}

void SoundSeBank::SetListener(const Vector4& eye, const Vector4& target, const Vector4& up)
{
	Listener& L = g_listener;
	L.eye = Vector3(eye.x, eye.y, eye.z);
	L.look = Norm(Sub(L.eye, Vector3(target.x, target.y, target.z)));
	L.right = Norm(Cross(Vector3(up.x, up.y, up.z), L.look));
	L.up = Cross(L.look, L.right);
	L.active = true;
	Prune();
	for (Voice& v : m_voices) if (v.is3d) Update3D(v);
	for (Voice& v : m_actorVoices) Update3D(v);
}

void SoundSeBank::ClearListener()
{
	g_listener = Listener{};
	for (Voice& v : m_voices) if (v.is3d) snd::SetSpatial(v.id, 1.0f, 0.0f);
	for (Voice& v : m_actorVoices) snd::SetSpatial(v.id, 1.0f, 0.0f);
}

// nw4r Sound3DEngine::UpdateAmbientParam for one listener: volume (unless FLAG_NOT_CTRL_VOLUME) and pan (unless FLAG_NOT_CTRL_PAN)
void SoundSeBank::Calc3D(Voice& v, float& volume, float& pan)
{
	const Listener& L = g_listener;
	volume = 1.0f;
	pan = 0.0f;
	if (!L.active) return;
	Vector3 p = v.p3d.pos;
	if (v.p3d.target != nullptr && v.p3d.target->native != nullptr)
	{
		float m[16];
		static_cast<kingdom::cKingdomModel*>(v.p3d.target->native)->CalcWorld(m);
		p = Vector3(m[12], m[13], m[14]);
		v.p3d.pos = p;
	}
	else if (v.p3d.target != nullptr && v.p3d.target->transform != nullptr) p = v.p3d.target->transform->position;
	const Vector3 d = Sub(p, L.eye);
	if (!(v.p3d.flags & 1u)) volume = CalcVolume(std::sqrt(Dot(d, d)), v.p3d.decayCurve, v.p3d.decayRatio);
	if (!(v.p3d.flags & 2u)) pan = CalcPan(Vector3(Dot(L.right, d), Dot(L.up, d), Dot(L.look, d)));
}

void SoundSeBank::Update3D(Voice& v)
{
	float volume, pan;
	Calc3D(v, volume, pan);
	snd::SetSpatial(v.id, volume, pan);
}

void SoundSeBank::ForgetTarget(const GameObject* target)
{
	for (Voice& v : m_voices)
		if (v.is3d && v.p3d.target == target)
		{
			Update3D(v);            // last position
			v.p3d.target = nullptr;
		}
}

GameObject* SoundSeBank::Play(ESEKind kind, SoundSe::ESameKindCheck check, const Voice3D* p3d)
{
	const char* cue = SeCue(kind);
	if (cue == nullptr) return nullptr;
	switch (check)
	{
	case SoundSe::eEARLY: if (IsPlayinSEKind(kind)) return nullptr; break;
	case SoundSe::eLATE: Stop(kind, 0.1f); break;
	case SoundSe::eLATE_IMMEDIATE: Stop(kind); break;
	default: break;
	}
	Voice v{kind, -1};
	float volume = 1.0f, pan = 0.0f;
	if (p3d != nullptr)
	{
		v.is3d = true;
		v.p3d = *p3d;
		snd::Sound3DParamOf(cue, v.p3d.flags, v.p3d.decayCurve, v.p3d.decayRatio);
		Calc3D(v, volume, pan);
	}
	v.id = snd::Play(cue, volume, pan);
	if (v.id < 0) return nullptr;
	const int id = v.id;
	Prune();
	m_voices.push_back(v);
	return reinterpret_cast<GameObject*>(static_cast<std::uintptr_t>(id) + 1u);
}

void SoundSeBank::Prune()
{
	auto ended = [](const Voice& v) { return !snd::IsPlaying(v.id); };
	m_voices.erase(std::remove_if(m_voices.begin(), m_voices.end(), ended), m_voices.end());
	m_actorVoices.erase(std::remove_if(m_actorVoices.begin(), m_actorVoices.end(), ended), m_actorVoices.end());
}

// DOL 0x80025988 with an actor (nw4r SoundStartable on the Sound3DActor): the cue's 3D parameters, placed at the actor
int SoundSeBank::PlayActor(const char* cue, const void* actor, const Vector3& pos)
{
	Voice v{ESEKind{}, -1};
	v.is3d = true;
	v.actor = actor;
	v.p3d.pos = pos;
	snd::Sound3DParamOf(cue, v.p3d.flags, v.p3d.decayCurve, v.p3d.decayRatio);
	float volume = 1.0f, pan = 0.0f;
	Calc3D(v, volume, pan);
	v.id = snd::Play(cue, volume, pan);
	if (v.id < 0) return -1;
	Prune();
	m_actorVoices.push_back(v);
	return v.id;
}

GameObject* SoundSeBank::PlayActor(ESEKind kind, const void* actor, const Vector3& pos)
{
	const char* cue = SeCue(kind);
	if (cue == nullptr) return nullptr;
	const int id = PlayActor(cue, actor, pos);
	return id < 0 ? nullptr : reinterpret_cast<GameObject*>(static_cast<std::uintptr_t>(id) + 1u);
}

// DOL 0x80026620 (Sound3DActor::SetPosition): the actor's playing sounds follow
void SoundSeBank::SetActorPosition(const void* actor, const Vector3& pos)
{
	for (Voice& v : m_actorVoices)
		if (v.actor == actor)
		{
			v.p3d.pos = pos;
			Update3D(v);
		}
}

// DOL 0x800265c8 (~Sound3DActor): the sounds go on where the actor was last
void SoundSeBank::ForgetActor(const void* actor)
{
	for (Voice& v : m_actorVoices)
		if (v.actor == actor) v.actor = nullptr;
}

namespace
{
void StopVoice(int id, float fadeSeconds)
{
	if (fadeSeconds > 0.0f) snd::FadeOut(id, (int)(fadeSeconds * 1000.0f));
	else snd::Stop(id);
}
}

void SoundSeBank::Stop(float fadeSeconds)
{
	for (const Voice& v : m_voices) StopVoice(v.id, fadeSeconds);
	m_voices.clear();
}

void SoundSeBank::Stop(ESEKind kind, float fadeSeconds)
{
	for (const Voice& v : m_voices) if (v.kind == kind) StopVoice(v.id, fadeSeconds);
	m_voices.erase(std::remove_if(m_voices.begin(), m_voices.end(), [kind](const Voice& v) { return v.kind == kind; }), m_voices.end());
}

void SoundSeBank::Stop(GameObject* handle, float fadeSeconds)
{
	if (handle == nullptr) return;
	const int id = (int)(reinterpret_cast<std::uintptr_t>(handle) - 1u);
	StopVoice(id, fadeSeconds);
	m_voices.erase(std::remove_if(m_voices.begin(), m_voices.end(), [id](const Voice& v) { return v.id == id; }), m_voices.end());
}

// DOL 0x80026424 [audited: handle validity/fade test is the native IsPlaying]
bool SoundSeBank::IsPlaying(GameObject* handle)
{
	if (handle == nullptr) return false;
	return snd::IsPlaying((int)(reinterpret_cast<std::uintptr_t>(handle) - 1u));
}

bool SoundSeBank::IsPlayinSEKind(ESEKind kind)
{
	Prune();
	for (const Voice& v : m_voices) if (v.kind == kind) return true;
	return false;
}

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

// DOL 0x80025b6c (see WiiSnd.hpp)
void WiiSnd::PlayRemote(Handle& h, int pad, const char* cue)
{
	Play(h, cue);
	const int setting = WiiSystem::RemoteSpeakerVolume();
	if (setting <= 0 || (GamePad::Get(0).hold & GamePad::PAD_HOME) != 0) return;
	const float volume = (float)setting / 127.0f * 0.5f;   // 0x80025218: FLOAT_80479ed0 = 127, FLOAT_80479ed4 = 0.5
	for (int n = pad < 0 ? 0 : pad; n < (pad < 0 ? 4 : pad + 1); n++)
		if (WiiPad::IsConnected(n)) snd::Play(cue, volume, 0.0f);
}
