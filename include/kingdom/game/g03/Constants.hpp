#pragma once

#include "common.hpp"
#include "AABB.hpp"
#include "kingdom/game/g03/AREA_POINT.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGameOver.hpp"
#include "kingdom/game/g03/cAppG03PacnRollMenu.hpp"
#include "kingdom/game/g03/cAppG03PacnRollStaffRoll.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"

namespace kingdom::game::g03
{
class cTaskCharPacman;
}

namespace kingdom::game::g03
{
class cTaskBgHit;
}

namespace kingdom::game::g03
{
class cAppG03PacnRollGame;
}

namespace kingdom::game::g03
{

class Constants
{
public:
	enum EPri : uint
	{
		ePRI_BG = 65536u,
		ePRI_OBJ = 65537u,
		ePRI_GROUND = 65538u,
		ePRI_CHAR = 65539u,
		ePRI_CAMERA = 65540u,
		ePRI_CONSOLE = 2147483648u,
		ePRI_RETRY = 2147483649u
	};

	enum EEff
	{
		eEFF_DASH = 0,
		eEFF_STAR = 1,
		eEFF_DUST = 2,
		eEFF_CHANGE = 3,
		eEFF_HIT_S = 4,
		eEFF_HIT_L = 5,
		eEFF_SHOCK = 6,
		eEFF_WING = 7,
		eEFF_RIPPLE = 8,
		eEFF_SPLASH = 9,
		eEFF_SPLASH_POISON = 10,
		eEFF_BUBBLE = 11,
		eEFF_POW_COOKIE = 12,
		eEFF_SWEAT = 13,
		eEFF_FLAME = 14,
		eEFF_WIND = 15,
		eEFF_STORM = 16,
		eEFF_LEAF = 17,
		eEFF_CHECKPOINT = 18,
		eEFF_GOAL = 19,
		eEFF_EXPLOSION = 20,
		eEFF_BOSS_STORM = 21,
		eEFF_PIYO = 22,
		eEFF_SCREEN_SMOKE = 23,
		eEFF_CAR_SMOKE = 24,
		eEFF_BOSS_UFO = 25,
		eEFF_HOVER = 26,
		eEFF_BOAT_SPLASH = 27,
		eEFF_PLAIN_SMOKE = 28,
		eEFF_LOG_SMOKE = 29,
		eEFF_SWITCH = 30,
		eEFF_GATE_BREAK = 31,
		eEFF_GATE_S_BREAK = 32,
		eEFF_JET = 33,
		eEFF_WALL = 34,
		eEFF_FLOW = 35,
		eEFF_WORLD5WIND = 36,
		eEFF_BOSS_DEMO_00 = 37,
		eEFF_MAX = 38
	};

	enum EHit
	{
		eHIT_WALL = 0,
		eHIT_FLOOR = 1,
		eHIT_DASH = 2,
		eHIT_CAMERA = 3,
		eHIT_MAX = 4
	};

	enum eFACE_KIND
	{
		eFACE_KIND_WALL = 0,
		eFACE_KIND_WAXX = 1,
		eFACE_KIND_BOUN = 2,
		eFACE_KIND_NORM = 3,
		eFACE_KIND_WATE = 4,
		eFACE_KIND_SWIM = 5,
		eFACE_KIND_GRAS = 6,
		eFACE_KIND_DASH = 7,
		eFACE_KIND_DALL = 8,
		eFACE_KIND_JUMP = 9,
		eFACE_KIND_MAGM = 10,
		eFACE_KIND_POIS = 11,
		eFACE_KIND_CAME = 12,
		eFACE_KIND_MENU_G00 = 13,
		eFACE_KIND_MENU_G01 = 14,
		eFACE_KIND_MENU_G02 = 15,
		eFACE_KIND_MENU_G03 = 16,
		eFACE_KIND_MENU_G04 = 17,
		eFACE_KIND_MENU_G05 = 18,
		eFACE_KIND_MENU_G06 = 19,
		eFACE_KIND_MENU_G07 = 20,   // "mgro" (DOL 0x800b9e1c has 8 hub kinds: mgal mral mpac mwan mmot mold mbac mgro = 13..20; the C# lacked the last one)
		eFACE_KIND_UNKNOWN = 21,
		eFACE_KIND_MAX = 22
	};

	enum eFACE_KIND_EX
	{
		eFACE_KIND_EX_NONE = 0,
		eFACE_KIND_EX_IRON_BALL_01 = 1,
		eFACE_KIND_EX_IRON_BALL_02 = 2,
		eFACE_KIND_EX_BOUND_GIMMICK_01 = 3,
		eFACE_KIND_EX_BOUND_GIMMICK_02 = 4
	};

	class sFace
	{
	public:
		Vector4 vVrt[3]{};

		Vector4 vNorm = Vector4::zero;

		Constants::eFACE_KIND nKind{};

		uint nHangFlag{};

		float fWork{};

		uint nKindEx{};

		AABB aabb{};

		AABB calcedAABB{};

		sFace();
	};

	enum ECamera
	{
		eCAMERA_TYPE = 0,
		eCAMERA_HEIGHT = 1,
		eCAMERA_DISTANCE = 2,
		eCAMERA_RECOANGLE = 3,
		eCAMERA_INITANGLE = 4,
		eCAMERA_PHIANGLE = 5,
		eCAMERA_CAMDISTANCE = 6,
		eCAMERA_TARGET_DELAY = 7,
		eCAMERA_CAMERA_DELAY = 8,
		eCAMERA_PHI_ACCEL = 9,
		eCAMERA_DIST_ACCEL = 10,
		eCAMERA_RECO_POWER = 11,
		eCAMERA_ANG_LOCK = 12,
		eCAMERA_EXRANGE00 = 13,
		eCAMERA_EXRANGE01 = 14,
		eCAMERA_GOAL_PHI = 15,
		eCAMERA_GOAL_DIST = 16,
		eCAMERA_GOAL_SPEED = 17,
		eCAMERA_START_SPEED = 18,
		eCAMERA_START_ANGLE = 19,
		eCAMERA_CLIP_FAR = 20,
		eCAMERA_CLIP_ANGLE = 21,
		eCSV_MAX = 22
	};

	class CAM_PARAM
	{
	public:
		uint m_nFlag{};

		int m_nType{};

		float m_fParamDistanceXZ{};

		float m_fParamHeight{};

		float m_fInitAngle{};

		float m_fPhiAngle{};

		float m_fCamDistance{};

		float m_fTargetDelay{};

		float m_fCameraDelay{};

		float m_fRecoAngle{};

		float m_fRecoPower{};

		float m_fExRange[2][2]{};

		float m_fPhiAccel{};

		float m_fDistAccel{};

		float m_fGoalPhi{};

		float m_fGoalDist{};

		float m_fGoalSpeed{};

		float m_fStartSpeed{};

		float m_fStartAngle{};

		float m_fClipFar{};

		float m_fClipAngle{};

		Constants::CAM_PARAM* Clone();

		CAM_PARAM();
	};

	enum EHitFlag
	{
		HIT_FLAG_NONE = 0,
		HIT_FLAG_NORMAL = 1,
		HIT_FLAG_EDGE = 2,
		HIT_FLAG_MOVE = 4,
		HIT_FLAG_CAVE = 8
	};

	class sHitParam
	{
	public:
		Vector4 vPos{};

		Vector4 vNorm{};

		Vector4 vRay{};

		Vector4 vVrt[3]{};

		Vector4 vObjMove{};

		Vector4 vEdgePos{};

		Vector4 vEdgeVec{};

		uint nFlag{};

		float fDist{};

		int nEdgeVrtIdx{};

		cTaskBgHit* pObj{};

		Constants::sFace* pFace{};
	};

	enum EEffData
	{
		eEFF_DATA_GATE = 0,
		eEFF_DATA_DASH = 1,
		eEFF_DATA_STAR = 2,
		eEFF_DATA_DUST = 3,
		eEFF_DATA_CHANGE = 4,
		eEFF_DATA_HIT_S = 5,
		eEFF_DATA_HIT_L = 6,
		eEFF_DATA_SHOCK = 7,
		eEFF_DATA_WING = 8,
		eEFF_DATA_RIPPLE = 9,
		eEFF_DATA_SPLASH = 10,
		eEFF_DATA_SPLASH_POISON = 11,
		eEFF_DATA_BUBBLE = 12,
		eEFF_DATA_POW_COOKIE = 13,
		eEFF_DATA_SWEAT = 14,
		eEFF_DATA_FLAME = 15,
		eEFF_DATA_WIND = 16,
		eEFF_DATA_STORM = 17,
		eEFF_DATA_LEAF = 18,
		eEFF_DATA_CHECKPOINT = 19,
		eEFF_DATA_GOAL = 20,
		eEFF_DATA_EXPLOSION = 21,
		eEFF_DATA_BOSS_STORM = 22,
		eEFF_DATA_PIYO = 23,
		eEFF_DATA_SCREEN_SMOKE = 24,
		eEFF_DATA_CAR_SMOKE = 25,
		eEFF_DATA_BOSS_UFO = 26,
		eEFF_DATA_HOVER = 27,
		eEFF_DATA_BOAT_SPLASH = 28,
		eEFF_DATA_PLAIN_SMOKE = 29,
		eEFF_DATA_LOG_SMOKE = 30,
		eEFF_DATA_SWITCH = 31,
		eEFF_DATA_JET = 32,
		eEFF_DATA_WALL = 33,
		eEFF_DATA_FLOW = 34,
		eEFF_DATA_WORLD5WIND = 35,
		eEFF_DATA_BOSS_DEMO_00 = 36,
		eEFF_DATA_MAX = 37
	};

	enum EAiMode
	{
		AIMODE_GOLVIS_WAIT = 0,
		AIMODE_GOLVIS_APPEAR_RAIL = 1,
		AIMODE_GOLVIS_FREE = 2,
		AIMODE_GOLVIS_ESCAPE = 3,
		AIMODE_GOLVIS_DAMAGE = 4,
		AIMODE_GOLVIS_POSSESS = 5,
		AIMODE_GOLVIS_KNOCK = 6,
		AIMODE_GOLVIS_ATTACK = 7,
		AIMODE_GOLVIS_MOTION = 8,
		AIMODE_GOLVIS_EATEN = 9,
		AIMODE_GOLVIS_MEDAMA = 10,
		AIMODE_GOLVIS_APPEAR = 11,
		AIMODE_GOLVIS_REVIVE = 12,
		AIMODE_GOLVIS_DEATH = 13,
		AIMODE_GOLVIS_NEXT_STAGE = 14,
		AIMODE_GOLVIS_BUG = 15,
		AIMODE_GOLVIS_TEST = 16,
		AIMODE_GOLVIS_MAX = 17
	};

	enum EGolvisAtk
	{
		GOLVIS_ATK_NONE = 0,
		GOLVIS_ATK_GUITAR_V_S = 1,
		GOLVIS_ATK_GUITAR_V_M = 2,
		GOLVIS_ATK_GUITAR_V_F = 3,
		GOLVIS_ATK_GUITAR_H_S = 4,
		GOLVIS_ATK_GUITAR_H_M = 5,
		GOLVIS_ATK_GUITAR_H_F = 6,
		GOLVIS_ATK_BOMB = 7,
		GOLVIS_ATK_CYCLONE_INHALE = 8,
		GOLVIS_ATK_CYCLONE_DISGORGE = 9,
		GOLVIS_ATK_HEADING_S = 10,
		GOLVIS_ATK_HEADING_M = 11,
		GOLVIS_ATK_HEADING_F = 12,
		GOLVIS_ATK_ELEC = 13,
		GOLVIS_ATK_FAINT = 14,
		GOLVIS_ATK_BOOT_0 = 15,
		GOLVIS_ATK_BOOT_1 = 16,
		GOLVIS_ATK_BOOT_2 = 17,
		GOLVIS_ATK_BOOT_3 = 18,
		GOLVIS_ATK_BOOT_4 = 19,
		GOLVIS_ATK_BOOT_5 = 20,
		GOLVIS_ATK_BOOT_6 = 21,
		GOLVIS_ATK_BOOT_7 = 22,
		GOLVIS_ATK_BOOT_8 = 23,
		GOLVIS_ATK_BOOT_9 = 24,
		GOLVIS_ATK_BOOT_0_1 = 25,
		GOLVIS_ATK_BOOT_2_3 = 26,
		GOLVIS_ATK_BOOT_4_5 = 27,
		GOLVIS_ATK_BOOT_6_7 = 28,
		GOLVIS_ATK_BOOT_8_9 = 29,
		GOLVIS_ATK_BOOT_0_1_2 = 30,
		GOLVIS_ATK_BOOT_3_4_5 = 31,
		GOLVIS_ATK_BOOT_6_7_8 = 32,
		GOLVIS_ATK_LAND_00 = 33,
		GOLVIS_ATK_LAND_01 = 34,
		GOLVIS_ATK_LAND_02 = 35,
		GOLVIS_ATK_LAND_03 = 36,
		GOLVIS_ATK_LAND_04 = 37,
		GOLVIS_ATK_LAND_05 = 38,
		GOLVIS_ATK_LAND_06 = 39,
		GOLVIS_ATK_LAND_07 = 40,
		GOLVIS_ATK_LAND_08 = 41,
		GOLVIS_ATK_LAND_09 = 42,
		GOLVIS_ATK_LAND_10 = 43,
		GOLVIS_ATK_LAND_11 = 44,
		GOLVIS_ATK_LAND_12 = 45,
		GOLVIS_ATK_LAND_13 = 46,
		GOLVIS_ATK_LAND_14 = 47,
		GOLVIS_ATK_LAND_15 = 48,
		GOLVIS_ATK_LAND_16 = 49,
		GOLVIS_ATK_LAND_17 = 50,
		GOLVIS_ATK_LAND_18 = 51,
		GOLVIS_ATK_LAND_19 = 52,
		GOLVIS_ATK_LAND_20 = 53,
		GOLVIS_ATK_LAND_21 = 54,
		GOLVIS_ATK_LAND_22 = 55,
		GOLVIS_ATK_LAND_23 = 56,
		GOLVIS_ATK_LAND_24 = 57,
		GOLVIS_ATK_LAND_25 = 58,
		GOLVIS_ATK_MAX = 59
	};

	class gmSGolvisAttack
	{
	public:
		int uiKind{};

		float fRangeMin{};

		float fRangeMax{};

		short iTimeMin{};

		short iTimeMax{};
	};

	enum gmEMonsterStateGolvisLand
	{
		STATE_GOLVISLAND_PLAY = 0,
		STATE_GOLVISLAND_MOVE = 1,
		STATE_GOLVISLAND_ESCAPE = 2,
		STATE_GOLVISLAND_MAX = 3
	};

	enum gmEMonsterPlayMotionGolvisLand
	{
		PLAYMOTION_GOLVISLAND_NONE = 0,
		PLAYMOTION_GOLVISLAND_NOKEZORI = 1,
		PLAYMOTION_GOLVISLAND_MAEKAGAMI = 2,
		PLAYMOTION_GOLVISLAND_SAYUU = 3,
		PLAYMOTION_GOLVISLAND_FAINT = 4,
		PLAYMOTION_GOLVISLAND_MAX = 5
	};

	class LAND_ATTACK_INFO
	{
	public:
		uint m_nFrame{};

		uint m_nKind{};

		string m_pStrParam{};

		uint m_pUintParam{};
	};

private:
	static cAppG03PacnRollMenu* m_pacnRollMenu;

	static cAppG03PacnRollGame* m_pacnRollGame;

	static cAppG03PacnRollGameOver* m_pacnRollGameOver;

	static cAppG03PacnRollStaffRoll* m_pacnRollStaffRoll;

public:
	static constexpr float VEC_LEN_NORMALIZE_MIN_SQ = (-std::numeric_limits<float>::infinity());

	static constexpr uint WORLD_MAX = 5u;

	static constexpr uint STAGE_MAX = 6u;

	static constexpr uint COOKIE_MAX = 999u;

	static constexpr uint GOST_MAX = 99u;

	static constexpr uint TIME_MAX = 599999u;

	static inline const string DISPOSE_NODE_START = "start";

	static inline const string DISPOSE_NODE_BG = "bg";

	static inline const string DISPOSE_NODE_OBJ = "obj";

	static inline const string DISPOSE_NODE_HIT = "hit";

	static inline const string DISPOSE_NODE_EFFECT = "effect";

	static const float scAudioMinDistance;

	static const float scAudioMaxDistance;

private:
	static AREA_POINT* StageArea3_5_0;

	static AREA_POINT* StageArea3_5_1;

	static AREA_POINT* StageArea3_5_2;

	static AREA_POINT* StageArea3_5_3;

	static AREA_POINT* StageArea3_5_4;

	static AREA_POINT* StageArea3_5_5;

	static AREA_POINT* StageArea3_5_6;

	static AREA_POINT* StageArea3_5_7;

	static AREA_POINT* StageArea3_5_8;

	static AREA_POINT* StageArea3_5_9;

	static AREA_POINT* StageArea3_5_10;

	static AREA_POINT* StageArea3_5_11;

	static AREA_POINT* StageArea3_5_12;

	static AREA_POINT* StageArea3_5_13;

	static AREA_POINT* StageArea3_5_14;

	static AREA_POINT* StageArea3_5_15;

	static AREA_POINT* StageArea3_5_16;

	static AREA_POINT* StageArea3_5_17;

	static AREA_POINT* StageArea3_5_18;

	static AREA_POINT* StageArea3_5_19;

	static AREA_POINT* StageArea3_5[21];

	static std::vector<std::vector<AREA_POINT*>> AllStageArea;

public:
	static constexpr float GOLVIS_FALL_HEIGHT = -30.0f;

	static string s_aModelName[5];

	static string s_aAnimeName[34];

	static float iAttackPermitSubY[15];

	static sbyte iCarryGhostSet[18][3];

	static constexpr int GOLVIS_BIJIKE_WARNING_COUNT = 180;

	static constexpr int GOLVIS_ATTACK_NUM = 8;

	static constexpr int GOLVIS_CARRY_GHOST_NUM = 10;

	static constexpr int EVENT_IZIKE_ADDCOUNT = 600;

	static constexpr int EVENT_MONSTER_EAT_MAXCOUNT = 25;

	static constexpr float ROTATE_ACCEL = 5E-05f;

	static constexpr float ROTATE_STOP_ACCEL = 0.0002f;

	static constexpr float GEARS_RADIUS_L = 10.0f;

	static constexpr float GEARS_RADIUS_S = 5.0f;

	static constexpr float GEARS_RADIUS_C = 4.0f;

	static const Vector4 CENTER_GEAR_POS;

	static cG03PacnRollSys* PacnRoll_System();

	static void SetPacnRoll_System(cG03PacnRollSys* system);

	static void Kingdom_G03_Init_ChangeInit();

	static void Kingdom_G03_Term_ChangeInit();

	static void Kingdom_G03_Menu_ChangeInit();

	static void Kingdom_G03_Menu_ChangeTerm();

	static void Kingdom_G03_Game_ChangeInit();

	static void Kingdom_G03_Game_ChangeTerm();

	static void Kingdom_G03_GameOver_ChangeInit();

	static void Kingdom_G03_GameOver_ChangeTerm();

	static void Kingdom_G03_StaffRoll_ChangeInit();

	static void Kingdom_G03_StaffRoll_ChangeTerm();

	static void Kingdom_G03_Nunchuk_ChangeInit();

	static void Kingdom_G03_Nunchuk_ChangeTerm();

	static void ReleasePacnRoll();

	static bool IS_PAC_PAUSE();

	static void ON_PAUSE();

	static void OFF_PAUSE();

	static bool IS_NAKAJIMA_PAUSE();

	static bool IsVecNormalize(const Vector4& v);

	static cAppG03PacnRollGame* PACN_APP();

	static cTaskCharPacman* PACMAN_OBJ();

	static bool CAM_PARAM_CHECK(Constants::CAM_PARAM* ptr, int flg);

	static AREA_POINT* areaGetInfo(const uint& nStage, const int& index);

	static int areaGetNo(const uint& nStage, const Vector4& vPos);

	static int areaGetFarNo(const uint& nStage, const Vector4& vPos);

	static int areaGetRevNo(const uint& nStage, const Vector4& vSrc, const Vector4& vPos);

	static int s_areaSearch(uint nStage, AREA_POINT** pArray, uint nMax, int level, ulong flag, int nSrc, int nDst);

	static int areaSearch(const uint& nStage, AREA_POINT** pArray, const uint& nMax, const int& nSrc, const int& nDst, const ulong& exflag = 0uL);

	static bool areaCheckArrive(const uint& nStage, const int& nSrc, const Vector4& vPos);

private:
	static void _StaticCtor();
	// run once from Constants.cpp after its own definitions (an inline static here ran BEFORE them: AllStageArea's ctor then emptied the areas and
	// CENTER_GEAR_POS{} zeroed the gear centre, so the area-AI Golvis indexed area -1 and crashed)
	static const bool s_staticCtorDone;
};

}
