#pragma once

#include "common.hpp"
#include "EScene.hpp"

class GameDefine
{
public:
	enum ELanguage
	{
		eENGLISH = 0,
		eJAPANESE = 1,
		eFRENCH = 2,
		eITALIAN = 3,
		eGERMAN = 4,
		eSPANISH = 5,
		eTRA_CHINESE = 6,
		eSIM_CHINESE = 7,
		eRUSSIAN = 8,
		eKOREAN = 9
	};

	enum class ENpc
	{
		eBLINKY = 0,
		eCLIDE = 1,
		ePINKY = 2,
		eINKY = 3,
		ePROFESSOR = 4,
		eLADY = 5,
		eBUDY = 6,
		eNUM = 7
	};

	enum class ETextureQuality
	{
		eHIGH = 0,
		eMID = 1,
		eLOW = 2
	};

	enum class EAntialiasQuality
	{
		eHIGH = 0,
		eMID = 1,
		eLOW = 2,
		eOFF = 3
	};

	enum EWindowMode
	{
		eFULL = 0,
		eWINDOW = 1,
		eNO_BORDER = 2
	};

	enum class EShadowQuality
	{
		eHIGH = 0,
		eMID = 1,
		eLOW = 2
	};

	enum EScreenSize
	{
		eSCREEN_SIZE_3 = 0,
		eSCREEN_SIZE_4 = 1,
		eSCREEN_SIZE_5 = 2,
		eSCREEN_SIZE_6 = 3,
		eSCREEN_SIZE_7 = 4,
		eSCREEN_SIZE_8 = 5,
		eSCREEN_SIZE_9 = 6,
		eSCREEN_SIZE_10 = 7,
		eSCREEN_SIZE_11 = 8,
		eSCREEN_SIZE_NUM = 9
	};

	enum class ETrophy
	{
		BEPACTIVE = 0,
		FIRST_GAME = 1,
		BEGINNER = 2,
		CORE_GAMER = 3,
		BGM_LOVER = 4,
		BGM_MANIA = 5,
		BGM_COLLECTOR = 6,
		INTERIOR_LOVER = 7,
		INTERIOR_MANIA = 8,
		INTERIOR_COLLECTOR = 9,
		BALLOON_ARTIST = 10,
		WALL_ARTIST = 11,
		MONEY_CHANGER = 12,
		JUKE_BOX_MANIA = 13,
		FIRST_VISITOR = 14,
		MORE_VISITOR = 15,
		MASS_VISITOR = 16,
		COIN_1 = 17,
		COIN_2 = 18,
		COIN_3 = 19,
		CREDIT_1 = 20,
		CREDIT_2 = 21,
		PACMAN_MASTER = 22,
		PACLAND_MASTER = 23,
		PACMANIA_MASTER = 24,
		PACANDPAL_MASTER = 25,
		ARRANGEMENT_CS_MASTER = 26,
		ARRANGEMENT_AC_MASTER = 27,
		PACMAN256_CS_MASTER = 28,
		PACATTACK_MASTER = 29,
		PACMOTOS_MASTER = 30,
		PACINTIME_MASTER = 31,
		PACNROLL_MASTER = 32,
		SUPERPACMAN_MASTER = 33,
		BATTLEROYALE_MASTER = 34,
		CE_MASTER = 35,
		PACMASTER = 36,
		COODINATER = 37,
		MY_GAMECENTER = 38,
		SMALLEST = 39,
		FORTYTH = 40,
		TOTALSCORE = 41,
		NUM = 42
	};

	enum EIngameSize
	{
		SMALL = 0,
		NORMAL = 1,
		LARGE = 2
	};

	enum class ETutorialType
	{
		eFIRST = 0,
		eCUSTOMIZE = 1,
		eVENDING = 2,
		eGASHAPON = 3,
		eJUKEBOX = 4,
		eMISSION = 5,
		eRANKING = 6,
		eSTART_GAME = 7,
		eMENU_GAMECENTER = 8,
		eMENU_GAMEPLAY = 9,
		eRESERVE_4 = 10,
		NUM = 11
	};

	enum EJoyCon
	{
		eLEFT = 0,
		eRIGHT = 1
	};

	enum class EGameTitle
	{
		ePACMAN = 0,
		eSUPERPACMAN = 1,
		ePACANDPAL = 2,
		ePACLAND = 3,
		ePACMANIA = 4,
		ePACATTACK = 5,
		ePACINTIME = 6,
		eARRANGEMENT_AC = 7,
		eARRANGEMENT_PSP = 8,
		ePACMAN_CE = 9,
		ePACMOTOS = 10,
		ePACNROLL = 11,
		eBATTLEROYEL = 12,
		ePACMAN_256 = 13,
		eNUM = 14,
		eNONE = -1
	};

	static std::vector<Vector2Int*> scScreenSize;

	static std::vector<string> scScreenSizeText;

	static std::vector<float> scInGameSizeData;

	static std::vector<float> scInGameSizeDataSFC;

	static std::vector<int> scDLCObjectId;

	static std::vector<int> scEventWallId;

	static int scSwitchHandheldResolutionWidth;

	static int scSwitchHandheldResolutionHeight;

	static int scSwitchTVResolutionWidth;

	static int scSwitchTVResolutionHeight;

	static int scMissionNum;

	static int scCustomizeSetNumMax;

	static int scCoinNumMax;

	static int scTotalCoinNumMax;

	static int scScoreMax;

	static int scDefaultCoinNum;

	static int scActiveMissionNum;

	static float scJukeBoxOpenConditionTime;

	static int scPlayTitleCount;

	static int scPlayTitleCountFull;

	static int scTotalGetCoinNum_1;

	static int scTotalGetCoinNum_2;

	static int scTotalGetCoinNum_3;

	static int scFirstVisitorNum;

	static int scMoreVisitor;

	static int scMassVisitor;

	static int scBGMManiaNum;

	static int scBGMLoverNum;

	static int scSetBalloonNum;

	static int scSetScreenNum;

	static int scSetChangeMachineNum;

	static int scSetConsoleNum;

	static int scSetObjectTypeNum;

	static int scSetObjectMinNum;

	static int scInputCreditNum_1;

	static int scInputCreditNum_2;

	static int scJukeBoxChangeBGMNum;

	static int scPacmanHiScore;

	static int scTotalPlayCount;

	static int scPurchasedObjectNum_1;

	static int scPurchasedObjectNum_2;

	static int scPurchasedObjectNum_3;

	static int scTotalGashaDraw;

	static int scGAMEOVER_TIME;

	static std::vector<Color> scJoyConDefaultColor;

	static std::vector<string> scSceneName;

	static EScene GetSceneEnum(EGameTitle title);
};
