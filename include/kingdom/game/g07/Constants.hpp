#pragma once

#include "common.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"
#include "kingdom/game/g07/cAppG07MotosGameOver.hpp"
#include "kingdom/game/g07/cAppG07MotosMatching.hpp"
#include "kingdom/game/g07/cAppG07MotosMenu.hpp"
#include "kingdom/game/g07/cAppG07MotosMultiMenu.hpp"
#include "kingdom/game/g07/cAppG07MotosStaffRoll.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

namespace kingdom::game::g07
{
class cTaskPanel;
}

namespace kingdom::game::g07
{
class cTaskCharEnemy;
}

namespace kingdom::game::g07
{
class cTaskChar;
}

namespace kingdom::game::g07
{
class cMotosGameSys;
}

namespace kingdom::game::g07
{
class cAppG07MotosGame;
}

namespace kingdom::game::g07
{

class Constants
{
public:
	enum EMOSyncKind
	{
		MO_SYNCKIND_CLEAR = 0,
		MO_SYNCKIND_SETTING_END = 1,
		MO_SYNCKIND_READY = 2,
		MO_SYNCKIND_PAUSE_ON = 3,
		MO_SYNCKIND_PAUSE_OFF = 4,
		MO_SYNCKIND_ROUND_RESULT = 5,
		MO_SYNCKIND_TOTAL_RESULT = 6,
		MO_SYNCKIND_EXIT = 7,
		MO_SYNCKIND_QUIT = 8
	};

	enum EMOExitKind
	{
		MO_EXITKIND_CLEAR = 0,
		MO_EXITKIND_RETRY1 = 1,
		MO_EXITKIND_RETRY2 = 2,
		MO_EXITKIND_GAMEOVER = 3,
		MO_EXITKIND_VSROUNDEND = 4,
		MO_EXITKIND_VSEND = 5,
		MO_EXITKIND_QUIT = 6
	};

	struct sMOTOS_P_RECT
	{
	public:
		float x{};

		float y{};

		float w{};

		float h{};
	};

	struct sMOTOS_S32_RECT
	{
	public:
		int x{};

		int y{};

		int w{};

		int h{};
	};

	struct UNI_FL
	{
	public:
		// [FieldOffset(0)]
		uint u{};

		// [FieldOffset(0)]
		float f{};
	};

	enum EBGPtk
	{
		eBG_PTK_01 = 0,
		eBG_PTK_02 = 1,
		eBG_PTK_03 = 2,
		eBG_PTK_04 = 3,
		eBG_PTK_05 = 4,
		eBG_PTK_06 = 5,
		eBG_PTK_07 = 6,
		eBG_PTK_08 = 7,
		eBG_PTK_MAX = 8
	};

	enum EMOCharUGPanel
	{
		MO_CHAR_UGPANEL_CENTER = 0,
		MO_CHAR_UGPANEL_FRONT = 1,
		MO_CHAR_UGPANEL_BACK = 2,
		MO_CHAR_UGPANEL_RIGHT = 3,
		MO_CHAR_UGPANEL_LEFT = 4,
		MO_UGPANEL_NUM = 5
	};

	enum EExHitKind
	{
		EX_HIT_KIND_RECT = 0,
		EX_HIT_KIND_CIRCLE = 1,
		DUMMY = 2
	};

	class EX_HIT_PARAM
	{
	public:
		struct Square
		{
		public:
			Vector4 m_vPoint0{};

			Vector4 m_vPoint1{};
		};

		struct Circle
		{
		public:
			Vector4 m_vOrigin{};

			float m_fRange{};
		};

		ushort m_nFlag{};

		ushort m_nKind{};

		Constants::EX_HIT_PARAM::Square m_square{};

		Constants::EX_HIT_PARAM::Circle m_circle{};
	};

	class CHAR_GIVE_POINTS
	{
	public:
		short m_nTargetID{};

		ushort m_nAddPoints{};

		ushort m_nSubPoints{};

		uint m_nGameCounter{};
	};

	class MO_CHAR_REFLECT
	{
	public:
		ushort m_nIndex{};

		ushort m_nRefID{};

		uint m_nOtherID{};

		uint m_nCharFlag{};

		uint m_nCount{};

		float m_fPower{};

		float m_fCharge{};
	};

	class MO_BOSS_MODE_INFO
	{
	public:
		int m_nMode{};

		int m_nMove{};

		int m_nWait{};

		int m_nWork[4]{};
	};

	class MO_RING_INFO
	{
	public:
		Vector4 m_vRingPos{};

		float m_fGoPosY{};

		float m_fUPSp{};

		float m_fUpPow{};

		int m_nHitCnt{};
	};

	enum EFormInfo
	{
		FORM_INFO_KEEP = 0,
		FORM_INFO_NO_CHILDREN = 1,
		FORM_INFO_MEET_FINISH = 2,
		FORM_INFO_TIMEUP = 3
	};

	class FORMATION_INFO
	{
	public:
		cTaskCharEnemy* m_pParent{};

		cTaskCharEnemy* m_pChild{};

		int m_ID{};

		int m_nChildNum{};

		int m_nDeg{};

		int m_nLine{};

		int m_nLineID{};
	};

	class FORM_GROUP_INFO
	{
	public:
		Constants::FORM_GROUP_INFO* m_pNext{};

		int m_nType{};

		int m_nNo{};

		int m_nMeetTime{};

		int m_nMoveTime{};
	};

	enum EMOModeForm
	{
		MO_MODE_FORM_NORMAL = 1,
		MO_MODE_FORM_WAIT = 2,
		MO_MODE_FORM_MEET = 3,
		MO_MODE_FORM_MOVE = 4
	};

	class MO_PLAYER_PARAMS
	{
	public:
		MO_CHAR_PARAMS* m_UnionParam{};

		float m_fMaxSpeedHover{};

		float m_fAccelSpeedHover{};

		float m_fDecelSpeedHover{};
	};

	enum EMOModePl
	{
		MO_MODE_PL_NORMAL = 1,
		MO_MODE_PL_JUMP = 2,
		MO_MODE_PL_HOVER = 3,
		MO_MODE_PL_CHARGE = 4,
		MO_MODE_PL_FUNBARI = 5,
		MO_MODE_PL_FALL = 6,
		MO_MODE_PL_REBIRTH = 7,
		MO_MODE_PL_SP_CHARGE = 8,
		MO_MODE_PL_JUMP_UP = 9,
		MO_MODE_PL_JUMP_DOWN = 10,
		MO_MODE_PL_MAX = 11
	};

	enum EConsState
	{
		eCONS_STATE_NORMAL = 1,
		eCONS_STATE_CHARGE = 2,
		eCONS_STATE_JUMP = 3,
		eCONS_STATE_TOUCHDOWN = 4,
		eCONS_STATE_SHOCK = 5,
		eCONS_STATE_ATTACK = 6,
		eCONS_STATE_FALL = 7,
		eCONS_STATE_DEAD = 8,
		eCONS_STATE_REBIRTH = 9
	};

	enum EMotosPl
	{
		MOTOS_PL_MODEL = 0,
		MOTOS_PL_MODELW = 1,
		MOTOS_PL_MODELE = 2,
		MOTOS_PL_MODELWE = 3,
		MOTOS_PL_MDA_01 = 4,
		MOTOS_PL_MDA_02 = 5,
		MOTOS_PL_MODEL_MAX = 6
	};

	class sPLAYER_CURSOR_PTK
	{
	public:
		cKingdomPtkHnd* phPlayerCursor{};

		cKingdomPtkHnd* phPlayerArrow{};
	};

	enum EMOEffKind
	{
		MO_EFF_KIND_TEST = 0,
		MO_EFF_KIND_COLI01 = 1,
		MO_EFF_KIND_COLI02 = 2,
		MO_EFF_KIND_COLI03 = 3,
		MO_EFF_KIND_COLI_SCHARGE = 4,
		MO_EFF_KIND_CHIP_BLU = 5,
		MO_EFF_KIND_CHIP_GRN = 6,
		MO_EFF_KIND_CHIP_RED = 7,
		MO_EFF_KIND_CHIP_GLY = 8,
		MO_EFF_KIND_CHIP_STA = 9,
		MO_EFF_KIND_VS_CHIP_BLU = 10,
		MO_EFF_KIND_VS_CHIP_YEL = 11,
		MO_EFF_KIND_VS_CHIP_RED = 12,
		MO_EFF_KIND_VS_CHIP_GRN = 13,
		MO_EFF_KIND_CHARGE = 14,
		MO_EFF_KIND_DASH_ST = 15,
		MO_EFF_KIND_JUMP_ST = 16,
		MO_EFF_KIND_JUMP_ED = 17,
		MO_EFF_KIND_PANEL_BREAK = 18,
		MO_EFF_KIND_EXPL01 = 19,
		MO_EFF_KIND_EXPL_PANEL = 20,
		MO_EFF_KIND_EXPL03 = 21,
		MO_EFF_KIND_EXPL05 = 22,
		MO_EFF_KIND_PLAYER_FLASH = 23,
		MO_EFF_KIND_PANEL_FLASH = 24,
		MO_EFF_KIND_SMOKE = 25,
		MO_EFF_KIND_LAMP = 26,
		MO_EFF_KIND_B2HOVER = 27,
		MO_EFF_KIND_B4HOVER = 28,
		MO_EFF_KIND_ME_BREAK = 29,
		MO_EFF_KIND_ME_NOBREAK = 30,
		MO_EFF_KIND_HOVER = 31,
		MO_EFF_KIND_RINGHIT = 32,
		MO_EFF_KIND_PIYORI = 33,
		MO_EFF_KIND_MYARROW = 34,
		MO_EFF_KIND_GIMMICK = 35,
		MO_EFF_KIND_ENESPAWN = 36,
		MO_EFF_KIND_METEO = 37
	};

	enum EPtk
	{
		ePTK_00_null = 0,
		ePTK_01_HIT = 1,
		ePTK_02_HIT_CHARGE = 2,
		ePTK_03_HIT_CHARGE_MAX = 3,
		ePTK_05_HIT_CHARGE_SP = 4,
		ePTK_06_CTHIP_BLUE = 5,
		ePTK_07_CTHIP_GREEN = 6,
		ePTK_08_CTHIP_ORANGE = 7,
		ePTK_09_CHIP_GRAY = 8,
		ePTK_10_CHIP_STAR = 9,
		ePTK_12_CHIP_VS_BLUE = 10,
		ePTK_13_CHIP_VS_GREEN = 11,
		ePTK_14_CHIP_VS_ORANGE = 12,
		ePTK_15_CHIP_VS_GRAY = 13,
		ePTK_18_CHARGE = 14,
		ePTK_19_CHARGE_DASH = 15,
		ePTK_20_JUMP_START = 16,
		ePTK_21_JUMP_END = 17,
		ePTK_22_CHIP_PANEL = 18,
		ePTK_23_EXPL = 19,
		ePTK_24_EXPL_GIMMICK = 20,
		ePTK_26_EXPL_BOSS_BODY = 21,
		ePTK_28_EXPL_BOSS_OPTION = 22,
		ePTK_29_FLASH_PLAYER = 23,
		ePTK_30_FLASH_PANEL = 24,
		ePTK_31_SMOKE = 25,
		ePTK_32_BOSS_LAMP = 26,
		ePTK_33_BOSS_HOVER = 27,
		ePTK_34_BOSS_HOVER = 28,
		ePTK_36_METEOR_STRIKE = 29,
		ePTK_37_METEOR_BREAK = 30,
		ePTK_38_HOVER = 31,
		ePTK_39_HIT_RING = 32,
		ePTK_40_HIT_RING_CRITICAL = 33,
		ePTK_41_TARGET = 34,
		ePTK_42_PANEL_GIMMICK = 35,
		ePTK_44_SPAWN = 36,
		ePTK_45_METEOR_TAIL = 37,
		ePTK_46_HORN_SPARK1 = 38,
		ePTK_47_HORN_SPARK2 = 39,
		ePTK_48_POINTS3000 = 40,
		ePTK_49_POINTS1500 = 41,
		ePTK_50_PLAYER1 = 42,
		ePTK_51_PLAYER2 = 43,
		ePTK_52_PLAYER3 = 44,
		ePTK_53_PLAYER4 = 45,
		ePTK_MAX = 46
	};

	enum EEnemyKindTbl
	{
		eENEMY_KIND_TBL_null = 0,
		eENEMY_KIND_TBL_PUPA_RED = 1,
		eENEMY_KIND_TBL_PUPA_BLUE = 2,
		eENEMY_KIND_TBL_PUPA_BLACK = 3,
		eENEMY_KIND_TBL_POLAR = 4,
		eENEMY_KIND_TBL_SPRIRUS = 5,
		eENEMY_KIND_TBL_FIREBUG = 6,
		eENEMY_KIND_TBL_FIREBEE = 7,
		eENEMY_KIND_TBL_TAITORIAN = 8,
		eENEMY_KIND_TBL_LADYBUG = 9,
		eENEMY_KIND_TBL_BEETLE = 10,
		eENEMY_KIND_TBL_MEGA = 11,
		eENEMY_KIND_TBL_GIGA = 12,
		eENEMY_KIND_TBL_null13 = 13,
		eENEMY_KIND_TBL_BEECON = 14,
		eENEMY_KIND_TBL_NABICON = 15,
		eENEMY_KIND_TBL_BOSS01 = 16,
		eENEMY_KIND_TBL_BOSS02 = 17,
		eENEMY_KIND_TBL_BOSS03 = 18,
		eENEMY_KIND_TBL_BOSS04 = 19,
		eENEMY_KIND_TBL_BOSS05 = 20,
		eENEMY_KIND_TBL_null21 = 21,
		eENEMY_KIND_TBL_BALL_A = 22,
		eENEMY_KIND_TBL_BALL_B = 23,
		eENEMY_KIND_TBL_PETA = 24,
		eENEMY_KIND_TBL_null25 = 25,
		eENEMY_KIND_TBL_BOSS05L = 26,
		eENEMY_KIND_TBL_END = 27
	};

	enum EItemKindTbl
	{
		eITEM_KIND_TBL_null = 0,
		eITEM_KIND_TBL_POWER = 1,
		eITEM_KIND_TBL_JUMP = 2,
		eITEM_KIND_TBL_CHARGE = 3,
		eITEM_KIND_TBL_END = 4
	};

	enum EGimmickKindTbl
	{
		eGIMMICK_KIND_TBL_null = 0,
		eGIMMICK_KIND_TBL_REACTION = 1,
		eGIMMICK_KIND_TBL_HATCH = 2,
		eGIMMICK_KIND_TBL_SWITCH = 3,
		eGIMMICK_KIND_TBL_END = 4
	};

	enum ePANEL : uint
	{
		ePANEL_INVALID = 0u,
		ePANEL_TYPE_N = 4096u,
		ePANEL_N_A = ePANEL_TYPE_N,
		ePANEL_N_B = 4097u,
		ePANEL_N_C = 4098u,
		ePANEL_TYPE_X = 8192u,
		ePANEL_X = ePANEL_TYPE_X,
		ePANEL_TYPE_G = 16384u,
		ePANEL_G_EXPLODED = ePANEL_TYPE_G,
		ePANEL_TYPE_W = 32768u,
		ePANEL_W_A = ePANEL_TYPE_W,
		ePANEL_W_B = 32769u,
		ePANEL_W_C = 32770u,
		ePANEL_W_D = 32771u,
		ePANEL_W_E = 32772u,
		ePANEL_W_F = 32773u,
		ePANEL_TYPE_MASK = 268431360u
	};

	enum ePANEL_STEP
	{
		ePANEL_STEP_NORMAL = 0,
		ePANEL_STEP_CRACK = 1,
		ePANEL_STEP_FALLING = 2,
		ePANEL_STEP_EXPLODE = 3,
		ePANEL_STEP_HIDE = 4,
		ePANEL_STEP_REBIRTH = 5
	};

	class PANEL_INFO
	{
	public:
		uint m_nFlag{};

		ushort m_nID{};

		ushort m_nKind{};

		int m_nTimer{};

		short m_nStep{};

		short m_nComa{};

		short m_nModel{};

		short m_nNComa{};

		short m_nNModel{};

		ushort m_nExplodeCount{};

		uint m_nMapPos[2]{};

		Vector4 m_vWTrans{};

		Vector4 m_vRot{};

		uint m_nStencil{};

		cTaskPanel* m_pTaskPanel{};
	};

	class PANEL_KIND_INFO
	{
	public:
		uint m_nFlag{};

		uint m_nStep{};

		ushort m_nModel[2]{};

		short m_nComa[2]{};
	};

	class PANEL_ATTACK_INFO
	{
	public:
		uint m_nSyncCounter{};

		byte m_nID{};

		byte m_nLevelAndID{};
	};

	enum EPanelKind
	{
		PANEL_KIND_A = 0,
		PANEL_KIND_B = 1,
		PANEL_KIND_C = 2,
		PANEL_KIND_CAN_BREAK = 3,
		PANEL_KIND_IRON = PANEL_KIND_CAN_BREAK,
		PANEL_KIND_EXPLODED = 4,
		PANEL_KIND_CAN_MOVE = 5,
		PANEL_KIND_WALL_A = PANEL_KIND_CAN_MOVE,
		PANEL_KIND_WALL_B = 6,
		PANEL_KIND_WALL_C = 7,
		PANEL_KIND_WALL_D = 8,
		PANEL_KIND_WALL_E = 9,
		PANEL_KIND_WALL_F = 10,
		PANEL_KIND_MAX = 11
	};

	enum EPanelModel
	{
		PANEL_MODEL_A_NORMAL = 0,
		PANEL_MODEL_A_CRACK1 = 1,
		PANEL_MODEL_A_CRACK2 = 2,
		PANEL_MODEL_A_CRACK3 = 3,
		PANEL_MODEL_A_BREAK = 4,
		PANEL_MODEL_B_NORMAL = 5,
		PANEL_MODEL_B_CRACK1 = 6,
		PANEL_MODEL_B_CRACK2 = 7,
		PANEL_MODEL_B_CRACK3 = 8,
		PANEL_MODEL_B_BREAK = 9,
		PANEL_MODEL_C_NORMAL = 10,
		PANEL_MODEL_C_CRACK1 = 11,
		PANEL_MODEL_C_CRACK2 = 12,
		PANEL_MODEL_C_CRACK3 = 13,
		PANEL_MODEL_C_BREAK = 14,
		PANEL_MODEL_IRON = 15,
		PANEL_MODEL_EXPLODED = 16,
		PANEL_MODEL_EXPLODED_BREAK = 17,
		PANEL_MODEL_WALL_A = 18,
		PANEL_MODEL_WALL_B = 19,
		PANEL_MODEL_WALL_C = 20,
		PANEL_MODEL_WALL_D = 21,
		PANEL_MODEL_WALL_E = 22,
		PANEL_MODEL_WALL_F = 23,
		PANEL_MODEL_MAX = 24
	};

	enum EMOModeGKHatch
	{
		eMO_MODE_GK_HATCH_OPEN = 1,
		eMO_MODE_GK_HATCH_CLOSE = 2,
		eMO_MODE_GK_HATCH_TO_OPEN = 3,
		eMO_MODE_GK_HATCH_TO_CLOSE = 4
	};

	enum EMOModeGKSwitch
	{
		eMO_MODE_GK_SWITCH_NORMAL = 0,
		eMO_MODE_GK_SWITCH_BOOT = 1,
		eMO_MODE_GK_SWITCH_PUSH = 2,
		eMO_MODE_GK_SWITCH_END = 3
	};

	static cAppG07MotosMenu* m_motosMenu;

	static cAppG07MotosMultiMenu* m_motosMultiMenu;

	static cAppG07MotosGame* m_motosGame;

	static cAppG07MotosGameOver* m_motosGameOver;

	static cAppG07MotosStaffRoll* m_motosStaffRoll;

	static cAppG07MotosMatching* m_motosMatching;

	static constexpr float COMING_SE_LENGTH = 5.0f;

	static constexpr float COMING_SE_VOLMAX_LANGE = 2.0f;

	static constexpr float WII_MOTOS_POSITION_RIVISE = 16.0f;

	static constexpr uint MO_MGFLAG_CLEAR = 1u;

	static constexpr uint MO_MGFLAG_ROUND_END = 2u;

	static constexpr uint MO_MGFLAG_ROUND_RESULT = 4u;

	static constexpr uint MO_MGFLAG_TOTAL_RESULT = 8u;

	static constexpr uint MO_MGFLAG_EXIT = 16u;

	static constexpr uint MO_MGFLAG_PAUSE = 32u;

	static constexpr uint MO_MGFLAG_EMPTY00 = 64u;

	static constexpr uint MO_MGFLAG_VS_WINNER = 4026531840u;

	static constexpr uint MO_MGFLAG_VS_CHAMPION = 251658240u;

	static constexpr uint MO_SGFLAG_CHARSYNCSTART = 1u;

	static constexpr uint MO_SGFLAG_SETTING_END = 2u;

	static constexpr uint MO_SGFLAG_READY = 4u;

	static constexpr uint MO_SGFLAG_CLEAR = 8u;

	static constexpr uint MO_SGFLAG_ROUND_RESULT = 32u;

	static constexpr uint MO_SGFLAG_TOTAL_RESULT = 64u;

	static constexpr uint MO_SGFLAG_EXIT = 128u;

	static constexpr uint MO_SGFLAG_PAUSE = 256u;

	static constexpr uint MO_SGFLAG_EMPTY00 = 512u;

	static constexpr uint MO_SGFLAG_SPCOUNT_MASK = 3221225472u;

	static constexpr uint MO_SGFLAG_SPCOUNT_SHIFT = 30u;

	static constexpr int MO_MAX_PLAYERS = 4;

	static constexpr int MO_MAX_ENEMYS = 32;

	static constexpr int MO_MAX_CHAR = 36;

	static constexpr int MO_MAX_PLCOLOR = 6;

	static constexpr int MO_MAX_WORLD = 5;

	static constexpr int MO_MAX_STAGE = 7;

	static constexpr int MO_MAX_VSMAP = 10;

	static constexpr int MO_MAX_SVSMAP = 3;

	static constexpr int MO_MAX_SCORE_TOKO = 999999;

	static constexpr int MO_MAX_SCORE_LAN = 999;

	static constexpr float TSP_PI = (float)std::numbers::pi;

	static constexpr float TSP_PI2 = (float)std::numbers::pi * 2.0f;

	static constexpr float TSP_2PI = (float)std::numbers::pi * 2.0f;

	static constexpr int DIR_NONE = 0;

	static constexpr int DIR_UP = 1;

	static constexpr int DIR_DW = 2;

	static constexpr int DIR_LT = 4;

	static constexpr int DIR_RT = 8;

	static constexpr float MO_METEO_SIZE = 0.1f;

	static constexpr int HIT_RESULT_REFX = 1;

	static constexpr int HIT_RESULT_REFZ = 2;

	static constexpr int HIT_RESULT_COURNER = 4;

	static constexpr int ARRAY_AXIS_ATTACK_NUM = 6;

	static constexpr uint MOTOS_KIND_TOKOTON_PLAY = 1u;

	static constexpr uint MOTOS_KIND_FREE_PLAY = 2u;

	static constexpr uint MOTOS_KIND_LAN_PLAY = 4u;

	static constexpr uint MOTOS_KIND_LAN_SHARE_PLAY = 8u;

	static constexpr uint MOTOS_KIND_LAN_ALL = 12u;

	static const uint MULTI_ITEM_LIST[6];

	static const uint MULTI_TIME_LIST[5];

	static const uint MULTI_ROUND_LIST[4];

	static constexpr uint PLAYER_MAX = 4u;

	static constexpr int WORLD_MAX = 5;

	static constexpr int VS_WORLD_MAX = 10;

	static constexpr int STAGE_MAX = 7;

	static constexpr uint TIME_MAX = 3599999u;

	static constexpr int NET_PACKET_INITIALIZE = 1;

	static constexpr int NOT_PACKET_REF_NUM = 12;

	static constexpr int MAX_STAGE_ENEMYS = 16;

	static constexpr int MO_BOSS_HIT_FLASH = 15;

	static constexpr int PANEL_MAX_ATREGIST = 16;

	static constexpr int PANEL_MAX_ATPACKET = 8;

	static constexpr int RF_PACKET_AXIS_MASK = 15;

	static constexpr int MO_MAX_VS_ROUND = 3;

	static constexpr int MO_MAX_VS_ROUNDEX = 10;

	static constexpr int MO_POWUP_MAX = 5;

	static constexpr int HIT_NONE = 1;

	static constexpr int HIT_MOVE = 2;

	static constexpr int HIT_CANBREAK = 4;

	static constexpr int HIT_BROKEN = 8;

	static constexpr int HIT_FALL = 16;

	static constexpr int MAX_HIT_RESULT = 4;

	static constexpr int SLOPE_PAT_MAX = 5;

	static inline const string MOTOS_BIN_null = "<nullpo>";

	static constexpr int COMMET_L1_TIME = 600;

	static constexpr int COMMET_L2_TIME = 900;

	static constexpr int COMMET_L3_TIME = 1080;

	static constexpr int COMMET_L3OVER_CYCLE = 180;

	static constexpr int RESULT_WAIT = 300;

	static const uint g_aMotosStage2RebirthTime[13][2];

	static constexpr float FRAME_RATE = 60.0f;

	static constexpr float PRECISION = 0.0001f;

	static const Vector4 s_Vec[9];

	static constexpr uint MO_CHAR_FLAG_HIDE = 1u;

	static constexpr uint MO_CHAR_FLAG_PLAYER = 2u;

	static constexpr uint MO_CHAR_FLAG_ENEMY = 4u;

	static constexpr uint MO_CHAR_FLAG_BOSS = 8u;

	static constexpr uint MO_CHAR_FLAG_ITEM = 16u;

	static constexpr uint MO_CHAR_FLAG_NOHIT = 32u;

	static constexpr uint MO_CHAR_FLAG_FALL = 64u;

	static constexpr uint MO_CHAR_FLAG_JUMP = 128u;

	static constexpr uint MO_CHAR_FLAG_HOVER = 256u;

	static constexpr uint MO_CHAR_FLAG_NOINPUT = 512u;

	static constexpr uint MO_CHAR_FLAG_HERETIC = 1024u;

	static constexpr uint MO_CHAR_FLAG_LARGE = 2048u;

	static constexpr uint MO_CHAR_FLAG_CHARGE = 4096u;

	static constexpr uint MO_CHAR_FLAG_TAME = 8192u;

	static constexpr uint MO_CHAR_FLAG_TAIKI = 16384u;

	static constexpr uint MO_CHAR_FLAG_FORM = 32768u;

	static constexpr uint MO_CHAR_FLAG_MEET = 65536u;

	static constexpr uint MO_CHAR_FLAG_CANFALL = 131072u;

	static constexpr uint MO_CHAR_FLAG_MUTEKI = 262144u;

	static constexpr uint MO_CHAR_FLAG_REBIRTH = 524288u;

	static constexpr uint MO_CHAR_FLAG_SPCRASH = 1048576u;

	static constexpr uint MO_CHAR_FLAG_PIYO = 2097152u;

	static constexpr uint MO_CHAR_FLAG_DIRECTION = 4194304u;

	static constexpr uint MO_CHAR_FLAG_DELETEREQ = 8388608u;

	static constexpr uint MO_CHAR_FLAG_WALLREF = 16777216u;

	static constexpr uint MO_CHAR_FLAGS_AIR = 384u;

	static constexpr uint MO_CHAR_HIT_THROUGH = 278624u;

	static constexpr uint MO_CHAR_SHADOW_OFF = 16449u;

	static constexpr float MO_CHAR_TAIKI_LINE = 50.0f;

	static constexpr int MO_CHAR_STEP_MAX = 4;

	static constexpr int MO_CHAR_HIT_FLAG_LOW = 1;

	static constexpr int MO_CHAR_HIT_FLAG_MID = 2;

	static constexpr int MO_CHAR_HIT_FLAG_HIGH = 4;

	static constexpr int MO_CHAR_HIT_FLAG_BOTH = 7;

	static constexpr int MO_CHAR_HIT_ROTCLOCK = 8;

	static constexpr int MO_CHAR_HIT_ROTUNCLOCK = 16;

	static constexpr int MAX_REF_HISTORY = 60;

	static constexpr int MAX_REF_HIST_FRAME = 15;

	static constexpr int MAX_SPWAN_BOMB = 32;

	static constexpr int BOMB_EXPLODE_TIME = 30;

	static constexpr float BOMB_EXPLODE_SIZE = 2.5f;

	static constexpr float FORMATION_CHILD_MEET_MAX_SPEED = 0.12f;

	static constexpr float FORMATION_CHILD_MOVE_MAX_SPEED = 0.23f;

	static constexpr float FORMATION_PARENT_MAX_SPEED_RATE = 0.7f;

	static constexpr int FORMATION_HERETIC_TIME = 4;

	static constexpr float FORM_CIRCLE_LENGTH = 1.3f;

	static const uint s_TransFlagTbl[6];

	static constexpr int MO_EFF_FLAG_USE = 1;

	static constexpr int MO_EFF_FLAG_INVISIBLE = 2;

	static constexpr int MO_EFF_FLAG_TIMEEND = 4;

	static constexpr int MO_EFF_FLAG_ANIMEEND = 8;

	static constexpr int MO_EFF_FLAG_CUSTOMFUNC = 16;

	static constexpr int MO_EFF_FLAG_BBOARD = 32;

	static constexpr int MO_EFF_FLAG_ENPAUSE = 1024;

	static constexpr int CHARGE_MINFRAME = 28;

	static constexpr float CHARGE_RATE = 0.2f;

	static constexpr uint MAX_MOTOS_EFFECT = 128u;

	static constexpr int MAX_REF_INFO = 32;

	static constexpr int HIT_DMYID_NOCHAR = 128;

	static constexpr int HIT_DMYID_REACTION = 129;

	static constexpr int HIT_CHFLAG_CHARGE = 1;

	static constexpr int HIT_CHFLAG_CRASH = 2;

	static constexpr int MO_HIT_REF_COUNTER = 5;

	static constexpr int GIMMICK_MAX_NUM = 256;

	static constexpr int MAX_GIMMICK = 32;

	static constexpr int MAX_ENEMY_STACK = 16;

	static constexpr int PANEL_STRIKE_NONE = 1;

	static constexpr int PANEL_STRIKE_NOBREAK = 2;

	static constexpr int PANEL_STRIKE_BREAK = 4;

	static constexpr int PANEL_MAX_NUM = 256;

	static constexpr int PANEL_FALL_WAIT = 15;

	static constexpr int PANEL_EXP_FALLWAIT = 30;

	static constexpr int PANEL_EXP_RAG = 50;

	static constexpr uint PANEL_FLAG_EXIST = 1u;

	static constexpr uint PANEL_FLAG_DRAW = 2u;

	static constexpr uint PANEL_FLAG_CRACK = 4u;

	static constexpr uint PANEL_FLAG_FALLING = 8u;

	static constexpr uint PANEL_FLAG_REBIRTH = 16u;

	static constexpr uint PANEL_FLAG_IRON = 16u;

	static constexpr uint PANEL_FLAG_DIAMOND = 16u;

	static constexpr uint PANEL_FLAG_STOP = 32u;

	static constexpr uint PANEL_FLAG_ANIMATE = 64u;

	static constexpr uint PANEL_FLAG_CRACKFALL = 12u;

	static constexpr uint FLAG_ITEM_NUMBER_LEFT_POS = 1u;

	static constexpr uint FLAG_ITEM_NUMBER_RIGHT_POS = 2u;

	static constexpr uint FLAG_ITEM_NUMBER_X_INVISIBLE = 4u;

	static constexpr uint FLAG_ITEM_NUMBER_ZERO_INVISIBLE = 8u;

	static constexpr uint FLAG_ITEM_NUMBER_MINUS_DRAW = 16u;

	static constexpr uint FLAG_ITEM_NUMBER_EXP_INVISIBLE = 32u;

	static constexpr uint FLAG_ITEM_NUMBER_ANIME_OFF = 64u;

	static constexpr uint FLAG_ITEM_NUMBER_DEFAULT = 1u;

	static constexpr uint GIMMICK_FLAG_EXIST = 1u;

	static constexpr uint GIMMICK_FLAG_DRAW = 2u;

	static constexpr uint GIMMICK_FLAG_STOP = 4u;

	static constexpr uint GIMMICK_FLAG_ANIMATE = 8u;

	static uint get_MO_SELECT_DEFAULT_COLOR_1P();

	static uint get_MO_SELECT_DEFAULT_COLOR_2P();

	static uint get_MO_SELECT_DEFAULT_COLOR_3P();

	static uint get_MO_SELECT_DEFAULT_COLOR_4P();

	static MOTOS_VS_PARAM* get_g_MoVSParam();

	static float* get_g_AxisAttack();
	static void set_g_AxisAttack(float* value);

	static float* get_g_PowerLevel2Power();

	static short* get_g_HoverLevel2Time();

	static short* get_g_ChargeLevel2Time();

	static float* get_g_ChargeLevel2Power();

	static float* get_g_ChargeLevel2MyRush();

	static float* get_g_ChargeLevel2Heretic();

	static float* get_g_ChargeLevel2Speed();

	static short* get_g_ChargeLevel2Weight();

	static short* get_g_ChargeLevel2PiyoTime();

	static MO_CHAR_PARAMS* get_g_MoPlayerParam();
	static void set_g_MoPlayerParam(MO_CHAR_PARAMS* value);

	static MO_CHAR_PARAMS** get_g_MoEnemyParam();

	static uint get_AXIS_ATTACK_SIZE();

	static int get_MO_VS_ENEMY_TOUCH();

	static int get_MO_VS_MOTOS_TOUCH();

	static int get_MO_VS_LAND_POINT_FRAME();

	static int get_MO_VS_LAND_POINT_POINT();

	static float get_MO_VS_ROB_POINT_RATE();

	static float get_MO_VS_FALL_POINT_RATE();

	static int get_MO_VS_POINT_DECEL_POINT();

	static float get_MO_VS_POINT_DECEL_DECEL();

	static int get_MO_VS_PANEL_REBIRTH();

	static int get_MO_VS_PLAYER_MUTEKI();

	static int get_MO_VS_SP_CHARGE_POINT();

	static int get_MO_VS_SP_PIYO_ENEMY();

	static int get_MO_VS_SP_PIYO_SELF();

	static uint get_SYNC_COUNTER();

	static uint get_GAME_COUNTER();

	static uint get_SYNC_COUNTER_LOCAL();

	static uint get_GAME_COUNTER_LOCAL();

	static void set_SYNC_COUNTER(uint v);

	static void set_GAME_COUNTER(uint v);

	static void set_SYNC_COUNTER_LOCAL(uint v);

	static void set_GAME_COUNTER_LOCAL(uint v);

	static int get_STAGE_SELECT_NO();

	static uint get_MO_HITFLAG_REFLECT();

	static uint get_MO_HITFLAG_REPEL();

	static void Kingdom_G07_Init_ChangeInit();

	static void Kingdom_G07_Term_ChangeInit();

	static void Kingdom_G07_Menu_ChangeInit();

	static void Kingdom_G07_Menu_ChangeTerm();

	static void Kingdom_G07_MultiMenu_ChangeInit();

	static void Kingdom_G07_MultiMenu_ChangeTerm();

	static void Kingdom_G07_Game_ChangeInit();

	static void Kingdom_G07_Game_ChangeTerm();

	static void Kingdom_G07_GameOver_ChangeInit();

	static void Kingdom_G07_GameOver_ChangeTerm();

	static void Kingdom_G07_StaffRoll_ChangeInit();

	static void Kingdom_G07_StaffRoll_ChangeTerm();

	static void Kingdom_G07_Matching_ChangeInit();

	static void Kingdom_G07_Matching_ChangeTerm();

	static void Kingdom_G07_Nunchuk_ChangeInit();

	static void Kingdom_G07_Nunchuk_ChangeTerm();

	static void ReleaseMotos();

private:
	static bool IsExecGCCollect();

public:
	static void MOTOS_CAUTION(string str);

	static void MOTOS_NO_SOURCE(string str);

	static int MO_VS_REBIRTH_LIMIT();

	static void Motos_System_Init();

	static void Motos_System_Term();

	static cG07MotosSys* Motos_System();

	static uint MOTOS_RANDU();

	static float MOTOS_RANDF();

	static uint MOTOS_RANDRANGEU(uint min, uint max);

	static float MOTOS_RANDRANGEF(float min, float max);

	static uint MOTOS_IRANDU(uint index);

	static float MOTOS_IRANDF(uint index);

	static uint MOTOS_IRANDRANGEU(uint min, uint max, uint index);

	static float MOTOS_IRANDRANGEF(float min, float max, uint index);

	static bool IS_MYCHAR(cTaskChar* p);

	static bool IS_LANPLAY();

	static bool IS_MASTER();

	static bool IS_JOIN(int n);

	static uint MY_ID();

	static bool IS_BOSS_STAGE();

	static long SYNC_COUNTER_FROMBYTE(int n);

	static long GAME_COUNTER_FROMBYTE(int n);

	static long COUNTER_FROMBYTE(int c, int n);

	static void Motos_Game_System_Init();

	static void Motos_Game_System_Term();

	static cMotosGameSys* Motos_Game_System();

	static int MO_GAME_LEVEL_C2M(int n);

	static cKingdomPtkHnd* SpawnEffect(int nKind, Vector4 pMPos, cJetTask* pParam = nullptr, int nDelay = 0, bool bDead = true, float loadFactor = -1.0f);

	static void SpawnColEffect(cTaskChar* pChar0, cTaskChar* pChar1);

	static int motosParamRebirthTime();

	static bool motosIsRebirth();

	static bool IS_CLEARPAUSE();

	static bool MOTOS_IS_PAUSE();

	static bool SYSTEM_IS_PAUSE();

	static float FPS_LEN(float f);

	static float FPS_SPEED(float f);

	static int FPS_TIME(float t);

	static float MOTOS_P_ABS(float x);

	static float MOTOS_P_MIN(float a, float b);

	static float MOTOS_P_MAX(float a, float b);

	static void VEC4_SET(Vector4& _out, float x1, float y1, float z1);

	static void VEC4_ADD(Vector4& _out, Vector4 v1, Vector4 v2);

	static void VEC4_SUB(Vector4& _out, Vector4 v1, Vector4 v2);

	static void VEC4_MUL(Vector4& _out, Vector4 v1, float n);

	static void VEC4_DIV(Vector4& _out, Vector4 v1, float n);

	static float VEC4_LENGTH_SQUARE(float x, float y, float z);

	static float VEC4_DIST_SQUARE(Vector4 v0, Vector4 v1);

	static void Motos_P_vecCross(Vector4& _out, const Vector4& vl, const Vector4& vr);

	static float Motos_P_vecDot(const Vector4& vl, const Vector4& vr);

	static float Motos_P_sqrtf_c(float a);

	static float Motos_P_vecLength_c(Vector4 v);

	static float Motos_P_vecLength2(const Vector4& v);

	static float Motos_P_vecDistance(const Vector4& v1, const Vector4& v2);

	static void Motos_P_vecNormalize(Vector4& _out, const Vector4& v);

	static void Motos_P_vecFVec2SVec(ushort* pOut, const Vector4& pVec);

	static void Motos_P_vecSVec2FVec(Vector4& pOut, const ushort* pVec);

	static float Motos_P_vecGetRad(const Vector4& v0_ptr, const Vector4& v1_ptr);

	static float Motos_P_vecGetRad2(const Vector4& v0_ptr, const Vector4& v1_ptr, int axis);

	static void Motos_P_vecReflect(Vector4& pOut, const Vector4& pNormal, const Vector4& pVec);

	static bool Motos_P_vecPointCheck(const Vector4& pPos, const Vector4& pV1, const Vector4& pV2);

	static bool Motos_P_vecEquation(float& pA, float& pB, float& pC, const Vector4& pVec1, const Vector4& pVec2);

	static int Motos_P_vecCrossCircleEx(Vector4& pOut0, Vector4& pOut1, const Vector4& pVec1, const Vector4& pVec2, const Vector4& pOrg, const float& fRange, bool flag);

	static int Motos_P_vecCrossCircle(Vector4& pOut0, Vector4& pOut1, const Vector4& pVec1, const Vector4& pVec2, const Vector4& pOrg, const float& fRange);

	static void Motos_P_vecLookRotate(Vector4& pOut, const Vector4& pP1, const Vector4& pP2);

	static void Motos_P_vecCalcBBMatrix(Matrix4x4 pOut, Vector4 pEye, Vector4 pLookAt, Vector4 pUp);

	static bool Motos_P_rectHitPoint(const Constants::sMOTOS_P_RECT& rect, const float& x, float y);

	static bool Motos_P_rectHitRect(const Constants::sMOTOS_P_RECT& rect1, const Constants::sMOTOS_P_RECT& rect2);

	static int Motos_P_bitGet(const byte* pData, int bitnum, int num);

	static void Motos_P_bitSet(byte* pData, int bitnum, int num, int wdata);

	static float fabs(float f);

	static float fw2fl(ushort s);

	static ushort fl2fw(float f);

	static float Motos_P_radGap(float src, float dist);

	static float ABS(float x);

	static int ABS(int x);

	static float MIN(float a, float b);

	static int MIN(int a, int b);

	static float MAX(float a, float b);

	static int MAX(int a, int b);

	static void VEC3_SET(Vector4& _out, float x1, float y1, float z1);

	static Vector4 VEC3_GET(float x1, float y1, float z1);

	static void VEC3_ADD(Vector4& _out, Vector4 v1, Vector4 v2);

	static void VEC3_SUB(Vector4& _out, Vector4 v1, Vector4 v2);

	static void VEC3_MUL(Vector4& _out, Vector4 v1, float n);

	static void VEC3_DIV(Vector4& _out, Vector4 v1, float n);

	static float VEC3_LENGTH_SQUARE(float x, float y, float z);

	static Vector4 MOTOS_VEC4_SET(Vector4& v, float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f);

	static float VEC3_DIST_SQUARE(Vector4 v0, Vector4 v1);

	static float TSP_DEG2RAD(float deg);

	static float TSP_RAD2DEG(float rad);

	static int motosAxisReverse(int axis);

	static bool MO_DEBUG_GPI(int no);

	static bool motosNetJoin(int id);

	static bool motosNetRoundStart(int id);

	static int motosNetJoinNum();

	static void motosDefaultEnv3D();

	static void motosDefaultEnv2D();

	static void motosDrawGrid(float step, int num);

	static int motosVector2Dir(const Vector4& v);

	static Vector4 motosDir2Vector(int dir);

	static Vector4 motosAxis2Vector(int axis);

	static int motosRad2Axis(float rad);

	static float motosAxis2Rad(int axis);

	static int motosAxisRotate(int axis, float rad);

	static Vector4 Motos_P_vecRotateVector(Vector4& pOut, Vector4 pVec, float rad);

	static bool Motos_P_vecCheckRect(Vector4& pOutCross, const Constants::sMOTOS_P_RECT& pRect, const Vector4& pVec1, const Vector4& pVec2);

	static bool motosCharMove(cTaskChar* pChar, Vector4 pGoal, float fRange = 0.05f, float fAccel = -1.0f);

	static int motosReverseDir(int dir);

	static void tspVuMtxCopy(Matrix4x4& m0, Matrix4x4 m1);

	static void tspVector3Scale(Vector4& v0, const Vector4& v1, float t);

	static void tspVuMtxMultTrans(Matrix4x4& m0, const Matrix4x4& m1, const Matrix4x4& m2);

	static void tspVuMtxMultRot(Matrix4x4& m0, const Matrix4x4& m1, const Matrix4x4& m2);

	static void tspVuMtxUnit(Matrix4x4& m0);

	static float tspVector3Length(const Vector4& v0);

	static float tspVector2Length(const Vector2& v0);

	static float tspVector3Distance(Vector4 v0, Vector4 v1);

	static void tspVector3Normalize(Vector4& v0, const Vector4& v1);

	static void tspVuMtxApply(Vector4& v0, const Matrix4x4& m0, const Vector4& v1);

	static void tspVector3RotX(Vector4& v0, const Vector4& v1, float rx);

	static void tspVector3RotY(Vector4& v0, const Vector4& v1, float ry);

	static void tspVector3RotZ(Vector4& v0, const Vector4& v1, float rz);

	static void tspVuMtxRotZ(Matrix4x4& m0, const Matrix4x4& m1, float rz);

	static void tspVector3RotXYZ(Vector4& v0, const Vector4& v1, const Vector4& rot);

	static void tspVector3RotZYX(Vector4& v0, const Vector4& v1, const Vector4& rot);

	static void tspVuMtxTrans(Matrix4x4& m0, const Matrix4x4& m1, const Vector4& tv);

	static bool SEARCH_GIMMICK(int n, float x, float z);

	static void DefaultPlayerParam();
};

}
